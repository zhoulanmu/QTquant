#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isRunning(false)
    , m_initialCapital(100000.0)
    , m_currentCash(100000.0)
    , m_currentPrice(0.0)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("QTQuant - 量化交易辅助程序"));

    m_marketData = new MarketDataSimulator(this);
    m_strategy = new MovingAverageStrategy(this);

    connect(m_marketData, &MarketDataSimulator::dataUpdated, this, &MainWindow::onMarketDataUpdated);
    connect(m_strategy, &StrategyBase::signalGenerated, this, &MainWindow::onStrategySignal);
    connect(ui->strategyPanel, &StrategyPanel::startClicked, this, &MainWindow::onStartStrategy);
    connect(ui->strategyPanel, &StrategyPanel::stopClicked, this, &MainWindow::onStopStrategy);
    connect(ui->strategyPanel, &StrategyPanel::parametersChanged, this, &MainWindow::onUpdateParameters);

    m_accountPanel = new AccountPanel(this);
    ui->verticalLayout_3->addWidget(m_accountPanel);

    onUpdateParameters();
    updateAccountInfo();
}

MainWindow::~MainWindow()
{
    onStopStrategy();
    delete ui;
}

void MainWindow::onMarketDataUpdated(const MarketData &data)
{
    m_currentPrice = data.close;

    ui->marketPanel->updateMarketData(data);
    ui->chartPanel->updateChartData(data);

    for (auto& pos : m_positions) {
        pos.currentPrice = data.close;
    }

    updateAccountInfo();

    if (m_isRunning) {
        m_strategy->processMarketData(data);
    }
}

void MainWindow::onStrategySignal(const StrategySignal &signal)
{
    ui->strategyPanel->addSignalLog(signal);

    QString signalText;
    switch (signal.type) {
    case SignalType::BUY:
        signalText = QStringLiteral("[买入信号] %1 @ %2").arg(signal.symbol).arg(signal.price, 0, 'f', 2);
        break;
    case SignalType::SELL:
        signalText = QStringLiteral("[卖出信号] %1 @ %2").arg(signal.symbol).arg(signal.price, 0, 'f', 2);
        break;
    case SignalType::STOP_LOSS:
        signalText = QStringLiteral("[止损信号] %1 @ %2").arg(signal.symbol).arg(signal.price, 0, 'f', 2);
        break;
    case SignalType::TAKE_PROFIT:
        signalText = QStringLiteral("[止盈信号] %1 @ %2").arg(signal.symbol).arg(signal.price, 0, 'f', 2);
        break;
    }

    ui->statusbar->showMessage(signalText, 3000);

    executeOrder(signal);
}

void MainWindow::executeOrder(const StrategySignal& signal)
{
    double price = signal.price;
    double lotSize = ui->strategyPanel->getLotSize();
    QString symbol = signal.symbol;

    QString typeStr;
    if (signal.type == SignalType::BUY || signal.type == SignalType::TAKE_PROFIT) {
        typeStr = QStringLiteral("买入");
    } else {
        typeStr = QStringLiteral("卖出");
    }

    if (signal.type == SignalType::BUY || signal.type == SignalType::TAKE_PROFIT) {
        double totalCost = price * lotSize;
        if (totalCost <= m_currentCash) {
            m_currentCash -= totalCost;

            if (m_positions.contains(symbol)) {
                PositionInfo& pos = m_positions[symbol];
                double totalQty = pos.quantity + lotSize;
                pos.avgCost = (pos.avgCost * pos.quantity + price * lotSize) / totalQty;
                pos.quantity = totalQty;
            } else {
                PositionInfo pos;
                pos.symbol = symbol;
                pos.quantity = lotSize;
                pos.avgCost = price;
                pos.currentPrice = price;
                m_positions[symbol] = pos;
            }

            m_accountPanel->addTradeRecord(symbol, typeStr, price, lotSize, totalCost,
                                           QDateTime::currentDateTime().toString("HH:mm:ss"));
            updateAccountInfo();
        }
    } else {
        if (m_positions.contains(symbol)) {
            PositionInfo& pos = m_positions[symbol];
            if (pos.quantity >= lotSize) {
                double totalRevenue = price * lotSize;
                m_currentCash += totalRevenue;
                pos.quantity -= lotSize;

                if (pos.quantity <= 0) {
                    m_positions.remove(symbol);
                }

                m_accountPanel->addTradeRecord(symbol, typeStr, price, lotSize, totalRevenue,
                                               QDateTime::currentDateTime().toString("HH:mm:ss"));
                updateAccountInfo();
            }
        }
    }
}

void MainWindow::updateAccountInfo()
{
    double marketValue = 0.0;
    for (const auto& pos : m_positions) {
        marketValue += pos.quantity * pos.currentPrice;
    }

    double totalAssets = m_currentCash + marketValue;
    double totalProfit = totalAssets - m_initialCapital;
    double profitPercent = (totalAssets / m_initialCapital - 1) * 100;

    m_accountPanel->updateAccount(totalAssets, m_currentCash, marketValue, totalProfit, profitPercent);

    QMap<QString, ::PositionInfo> accountPositions;
    for (const auto& [key, pos] : m_positions.asKeyValueRange()) {
        ::PositionInfo info;
        info.symbol = pos.symbol;
        info.quantity = pos.quantity;
        info.avgCost = pos.avgCost;
        info.currentPrice = pos.currentPrice;
        info.marketValue = pos.quantity * pos.currentPrice;
        info.profit = (pos.currentPrice - pos.avgCost) * pos.quantity;
        info.profitPercent = (pos.currentPrice / pos.avgCost - 1) * 100;
        accountPositions[key] = info;
    }

    m_accountPanel->updatePositions(accountPositions);
}

void MainWindow::onStartStrategy()
{
    if (m_isRunning) return;

    m_isRunning = true;
    ui->strategyPanel->setRunningState(true);
    m_marketData->startSimulation();

    ui->statusbar->showMessage(QStringLiteral("策略已启动"), 2000);
}

void MainWindow::onStopStrategy()
{
    if (!m_isRunning) return;

    m_marketData->stopSimulation();
    m_isRunning = false;
    ui->strategyPanel->setRunningState(false);

    ui->statusbar->showMessage(QStringLiteral("策略已停止"), 2000);
}

void MainWindow::onUpdateParameters()
{
    StrategyParameters params;
    params.symbol = ui->strategyPanel->getSymbol();
    params.fastMA = ui->strategyPanel->getFastMA();
    params.slowMA = ui->strategyPanel->getSlowMA();
    params.stopLossPercent = ui->strategyPanel->getStopLossPercent();
    params.takeProfitPercent = ui->strategyPanel->getTakeProfitPercent();
    params.lotSize = ui->strategyPanel->getLotSize();

    m_strategy->setParameters(params);
    m_marketData->setSymbol(params.symbol);
}
