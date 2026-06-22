#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "indicator/indicators.h"
#include "strategy/movingaveragestrategy.h"
#include "strategy/prosperitygrowthstrategy.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include <vector>

MainWindow::MainWindow(bool guestMode, const QString& accountName, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_marketData(nullptr)
    , m_strategyMarketData(nullptr)
    , m_strategy(nullptr)
    , m_activeStrategyType(StrategyType::DoubleMA)
    , m_accountPanel(nullptr)
    , m_statisticsPanel(nullptr)
    , m_signalPanel(nullptr)
    , m_newsPanel(nullptr)
    , m_mainTabs(nullptr)
    , m_isRunning(false)
    , m_initialCapital(100000.0)
    , m_currentCash(100000.0)
    , m_currentPrice(10.78)
    , m_hasLastMarketData(false)
    , m_hasLastStrategyMarketData(false)
    , m_guestMode(guestMode)
    , m_accountName(accountName)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("星策 StarQuant - 先以模拟谋方略，再持真仓逐行情"));

    const QString darkTheme =
        "QMainWindow { background-color: #253b6e; }"
        "QWidget#centralwidget { background-color: #253b6e; }"
        "QGroupBox { background-color: #1a1a2e; border: 1px solid #4a5568; border-radius: 8px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { color: #63b3ed; font-weight: bold; left: 10px; }"
        "QLabel { color: #e0e0e0; font-size: 13px; }"
        "QLineEdit { background-color: #4a5568; color: white; border: 1px solid #718096; border-radius: 6px; padding: 8px 10px; font-size: 13px; min-height: 32px; }"
        "QLineEdit:focus { border-color: #63b3ed; }"
        "QPushButton { background-color: #718096; color: white; border: none; border-radius: 6px; padding: 8px 20px; font-size: 13px; min-height: 32px; }"
        "QPushButton:hover { background-color: #5a6678; }"
        "QPushButton:disabled { background-color: #3b4556; color: #8ea0b8; }"
        "QPushButton#startBtn { background-color: #2196f3; }"
        "QPushButton#startBtn:hover { background-color: #1976d2; }"
        "QPushButton#stopBtn { background-color: #e53e3e; }"
        "QPushButton#stopBtn:hover { background-color: #c53030; }"
        "QPushButton#favoriteAddBtn { background-color: #38a169; }"
        "QPushButton#favoriteAddBtn:hover { background-color: #2f855a; }"
        "QPushButton#favoriteRemoveBtn { background-color: #718096; }"
        "QTableWidget { background-color: #1a1a2e; border: 1px solid #4a5568; border-radius: 6px; gridline-color: #4a5568; }"
        "QTableWidget::item { color: #e0e0e0; padding: 4px 8px; }"
        "QTableWidget::item:selected { background-color: #4299e1; }"
        "QTableWidget::horizontalHeader { background-color: #2d3748; }"
        "QTableWidget::horizontalHeader::section { color: #63b3ed; background-color: #2d3748; padding: 6px 8px; border: 1px solid #4a5568; }"
        "QTextEdit { background-color: #1a1a2e; color: #e0e0e0; border: 1px solid #4a5568; border-radius: 6px; font-family: Consolas,Monaco,monospace; font-size: 12px; }"
        "QListWidget { background-color: #1a1a2e; color: #e0e0e0; border: 1px solid #4a5568; border-radius: 6px; padding: 4px; }"
        "QListWidget::item { padding: 6px 8px; border-radius: 4px; }"
        "QListWidget::item:selected { background-color: #2b6cb0; color: white; }"
        "QSpinBox { background-color: #4a5568; color: white; border: 1px solid #718096; border-radius: 6px; padding: 4px 8px; font-size: 13px; }"
        "QDoubleSpinBox { background-color: #4a5568; color: white; border: 1px solid #718096; border-radius: 6px; padding: 4px 8px; font-size: 13px; }"
        "QTabWidget#mainTabs::pane { border: 1px solid #4a5568; border-radius: 8px; background-color: #253b6e; top: -1px; }"
        "QTabBar::tab { background-color: #2d3748; color: #cbd5e0; padding: 10px 24px; min-width: 72px; border: 1px solid #4a5568; border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; margin-right: 4px; }"
        "QTabBar::tab:selected { background-color: #1a1a2e; color: #63b3ed; font-weight: bold; }"
        "QTabBar::tab:!selected { margin-top: 4px; }"
        "QTabBar::tab:hover:!selected { background-color: #3b4a68; color: white; }"
        "QStatusBar { background-color: #2d3748; color: #e0e0e0; }";
    setStyleSheet(darkTheme);

    m_marketData = new MarketDataSimulator(this);
    m_strategyMarketData = new MarketDataSimulator(this);
    m_strategy = nullptr;

    connect(m_marketData, &MarketDataSimulator::dataUpdated, this, &MainWindow::onMarketDataUpdated);
    connect(m_marketData, &MarketDataSimulator::intradayDataUpdated, this, &MainWindow::onIntradayDataUpdated);
    connect(m_marketData, &MarketDataSimulator::errorOccurred, this, &MainWindow::onMarketDataError);
    connect(m_strategyMarketData, &MarketDataSimulator::dataUpdated, this, &MainWindow::onStrategyMarketDataUpdated);
    connect(m_strategyMarketData, &MarketDataSimulator::intradayDataUpdated, this, &MainWindow::onStrategyIntradayDataUpdated);
    connect(m_strategyMarketData, &MarketDataSimulator::errorOccurred, this, &MainWindow::onStrategyMarketDataError);
    connect(ui->strategyPanel, &StrategyPanel::startClicked, this, &MainWindow::onStartStrategy);
    connect(ui->strategyPanel, &StrategyPanel::stopClicked, this, &MainWindow::onStopStrategy);
    connect(ui->strategyPanel, &StrategyPanel::parametersChanged, this, &MainWindow::onUpdateParameters);
    connect(ui->strategyPanel, &StrategyPanel::viewSymbolChanged, this, &MainWindow::onViewSymbolChanged);

    m_accountPanel = new AccountPanel(this);
    ui->verticalLayout_3->replaceWidget(ui->accountPanel, m_accountPanel);
    delete ui->accountPanel;
    ui->accountPanel = m_accountPanel;

    m_statisticsPanel = new StatisticsPanel(this);
    ui->horizontalLayout_2->replaceWidget(ui->statisticsPanel, m_statisticsPanel);
    delete ui->statisticsPanel;
    ui->statisticsPanel = m_statisticsPanel;

    m_signalPanel = new SignalPanel(this);
    ui->horizontalLayout_2->replaceWidget(ui->signalPanel, m_signalPanel);
    delete ui->signalPanel;
    ui->signalPanel = m_signalPanel;

    m_newsPanel = new NewsPanel(this);

    buildTabbedLayout();

    ui->strategyPanel->setRunningState(false);
    configureStrategy(ui->strategyPanel->strategyConfig());
    m_marketData->setSymbol(ui->strategyPanel->getViewSymbol());
    updateAccountInfo();

    m_marketData->startSimulation();
    ui->statusbar->showMessage(m_guestMode
        ? QStringLiteral("游客看盘：实时行情已启动")
        : QStringLiteral("客户端已登录：%1，实时行情已启动").arg(m_accountName), 4000);
}

MainWindow::~MainWindow()
{
    m_marketData->stopSimulation();
    if (m_strategyMarketData) {
        m_strategyMarketData->stopSimulation();
    }
    delete ui;
}

void MainWindow::buildTabbedLayout()
{
    ui->verticalLayout_3->removeWidget(ui->strategyPanel);
    ui->verticalLayout_3->removeWidget(ui->marketPanel);
    ui->verticalLayout_3->removeWidget(m_accountPanel);
    ui->verticalLayout_4->removeWidget(ui->chartPanel);
    ui->horizontalLayout_2->removeWidget(m_statisticsPanel);
    ui->horizontalLayout_2->removeWidget(m_signalPanel);

    ui->strategyPanel->setParent(nullptr);
    ui->marketPanel->setParent(nullptr);
    m_accountPanel->setParent(nullptr);
    ui->chartPanel->setParent(nullptr);
    m_statisticsPanel->setParent(nullptr);
    m_signalPanel->setParent(nullptr);

    while (QLayoutItem* item = ui->horizontalLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->hide();
        }
        delete item;
    }

    m_mainTabs = new QTabWidget(ui->centralwidget);
    m_mainTabs->setObjectName(QStringLiteral("mainTabs"));
    m_mainTabs->setDocumentMode(false);
    m_mainTabs->setTabPosition(QTabWidget::North);
    m_mainTabs->setElideMode(Qt::ElideNone);
    m_mainTabs->setUsesScrollButtons(false);
    m_mainTabs->addTab(createMainTab(), QStringLiteral("主"));
    m_mainTabs->addTab(createStrategyTab(), QStringLiteral("策略"));
    m_mainTabs->addTab(createPersonalTab(), QStringLiteral("个人"));
    m_mainTabs->addTab(createNewsTab(), QStringLiteral("新闻"));

    ui->horizontalLayout->setContentsMargins(8, 8, 8, 8);
    ui->horizontalLayout->addWidget(m_mainTabs);
}

QWidget* MainWindow::createMainTab()
{
    auto* tab = new QWidget(m_mainTabs);
    auto* rootLayout = new QHBoxLayout(tab);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(12);

    auto* leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    QWidget* strategyControls = ui->strategyPanel->takeStrategyControlWidget(tab);
    QWidget* tradeLog = ui->strategyPanel->takeTradeLogWidget(tab);
    ui->marketPanel->setSymbolSelector(ui->strategyPanel->takeSymbolSelectorWidget(ui->marketPanel));

    ui->marketPanel->setMinimumHeight(200);
    m_statisticsPanel->setMinimumHeight(160);
    if (strategyControls) {
        strategyControls->setMinimumHeight(74);
    }
    if (tradeLog) {
        tradeLog->setMinimumHeight(190);
    }

    leftLayout->addWidget(ui->marketPanel);
    if (strategyControls) {
        leftLayout->addWidget(strategyControls);
    }
    leftLayout->addWidget(m_statisticsPanel);
    if (tradeLog) {
        leftLayout->addWidget(tradeLog, 1);
    }

    ui->chartPanel->setMinimumWidth(520);
    rootLayout->addLayout(leftLayout, 1);
    rootLayout->addWidget(ui->chartPanel, 3);
    return tab;
}

QWidget* MainWindow::createStrategyTab()
{
    auto* tab = new QWidget(m_mainTabs);
    auto* rootLayout = new QHBoxLayout(tab);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(12);

    ui->strategyPanel->setMinimumWidth(420);
    m_signalPanel->setMinimumWidth(360);
    rootLayout->addWidget(ui->strategyPanel, 2);
    rootLayout->addWidget(m_signalPanel, 1);
    return tab;
}

QWidget* MainWindow::createPersonalTab()
{
    auto* tab = new QWidget(m_mainTabs);
    auto* rootLayout = new QHBoxLayout(tab);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(12);

    QWidget* watchlist = ui->strategyPanel->takeWatchlistWidget(tab);
    if (watchlist) {
        watchlist->setMinimumWidth(300);
        watchlist->setMaximumWidth(420);
        rootLayout->addWidget(watchlist, 1);
    }

    m_accountPanel->setMinimumWidth(620);
    rootLayout->addWidget(m_accountPanel, 3);
    return tab;
}

QWidget* MainWindow::createNewsTab()
{
    auto* tab = new QWidget(m_mainTabs);
    auto* rootLayout = new QVBoxLayout(tab);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(12);

    m_newsPanel->setParent(tab);
    rootLayout->addWidget(m_newsPanel);
    return tab;
}void MainWindow::onMarketDataUpdated(const MarketData &data)
{
    m_currentPrice = data.close;
    m_lastMarketData = data;
    m_hasLastMarketData = true;

    ui->marketPanel->updateMarketData(data);
    ui->strategyPanel->rememberStockName(data.symbol, data.name);
    ui->chartPanel->updateChartData(data);
    m_statisticsPanel->setData(data.turnover, data.volume, data.volume, 0.0, 0.0, 0);

    if (m_positions.contains(data.symbol)) {
        m_positions[data.symbol].currentPrice = data.close;
    }

    updateAccountInfo();
}

void MainWindow::onIntradayDataUpdated(const QVector<MarketData>& data)
{
    ui->chartPanel->updateIntradayData(data);
}

void MainWindow::onStrategyMarketDataUpdated(const MarketData& data)
{
    m_lastStrategyMarketData = data;
    m_hasLastStrategyMarketData = true;

    ui->strategyPanel->rememberStockName(data.symbol, data.name);
    if (m_positions.contains(data.symbol)) {
        m_positions[data.symbol].currentPrice = data.close;
    }
    updateSignalIndicators();
    updateAccountInfo();

    if (m_isRunning && m_strategy) {
        m_strategy->processMarketData(data);
    }
}

void MainWindow::onStrategyIntradayDataUpdated(const QVector<MarketData>& data)
{
    m_indicatorHistory = data;
    updateSignalIndicators();
}

void MainWindow::updateSignalIndicators()
{
    if (!m_signalPanel) {
        return;
    }

    QVector<MarketData> points = m_indicatorHistory;
    if (m_hasLastStrategyMarketData && m_lastStrategyMarketData.close > 0.0) {
        if (!points.isEmpty() && points.last().symbol != m_lastStrategyMarketData.symbol) {
            points.clear();
        }

        if (points.isEmpty()) {
            points.append(m_lastStrategyMarketData);
        } else {
            const QDateTime lastTime = points.last().timestamp;
            const QDateTime quoteTime = m_lastStrategyMarketData.timestamp;
            const bool sameMinute = lastTime.isValid() && quoteTime.isValid()
                && lastTime.date() == quoteTime.date()
                && lastTime.time().hour() == quoteTime.time().hour()
                && lastTime.time().minute() == quoteTime.time().minute();

            if (sameMinute) {
                points.last() = m_lastStrategyMarketData;
            } else if (!quoteTime.isValid() || !lastTime.isValid() || quoteTime > lastTime) {
                points.append(m_lastStrategyMarketData);
            }
        }
    }

    std::vector<double> closes;
    closes.reserve(static_cast<size_t>(points.size()));
    for (const MarketData& point : points) {
        if (point.close > 0.0) {
            closes.push_back(point.close);
        }
    }

    constexpr size_t RequiredPeriods = 26;
    if (closes.size() < RequiredPeriods) {
        m_signalPanel->clearIndicators();
        return;
    }

    const double rsi = TechnicalIndicators::calculateRSI(closes, 14);
    const auto macdValues = TechnicalIndicators::calculateMACD(closes, 12, 26, 9);
    const auto bollValues = TechnicalIndicators::calculateBollingerBands(closes, 20, 2.0);
    const double currentPrice = m_hasLastStrategyMarketData && m_lastStrategyMarketData.close > 0.0
        ? m_lastStrategyMarketData.close
        : closes.back();

    m_signalPanel->updateIndicators(rsi, macdValues.first, bollValues.second, bollValues.first, currentPrice);
}
void MainWindow::onMarketDataError(const QString &message)
{
    ui->statusbar->showMessage(message, 5000);
}

void MainWindow::onStrategyMarketDataError(const QString& message)
{
    ui->statusbar->showMessage(message, 5000);
    if (m_isRunning) {
        ui->strategyPanel->addSystemLog(QStringLiteral("策略行情错误：%1").arg(message));
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
    default:
        signalText = QStringLiteral("[策略信号] %1 @ %2").arg(signal.symbol).arg(signal.price, 0, 'f', 2);
        break;
    }

    ui->statusbar->showMessage(signalText, 3000);
    executeOrder(signal);
}

void MainWindow::executeOrder(const StrategySignal& signal)
{
    const double price = signal.price;
    const double lotSize = signal.volume;
    const QString symbol = signal.symbol;
    const bool isBuy = signal.type == SignalType::BUY;
    const QString typeStr = isBuy ? QStringLiteral("买入") : QStringLiteral("卖出");

    if (isBuy) {
        const double totalCost = price * lotSize;
        if (totalCost <= m_currentCash) {
            m_currentCash -= totalCost;

            if (m_positions.contains(symbol)) {
                PositionInfo& pos = m_positions[symbol];
                const double totalQty = pos.quantity + lotSize;
                pos.avgCost = (pos.avgCost * pos.quantity + price * lotSize) / totalQty;
                pos.quantity = totalQty;
                pos.currentPrice = price;
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
        return;
    }

    if (!m_positions.contains(symbol)) {
        return;
    }

    PositionInfo& pos = m_positions[symbol];
    if (pos.quantity < lotSize) {
        return;
    }

    const double totalRevenue = price * lotSize;
    m_currentCash += totalRevenue;
    pos.quantity -= lotSize;

    if (pos.quantity <= 0) {
        m_positions.remove(symbol);
    }

    m_accountPanel->addTradeRecord(symbol, typeStr, price, lotSize, totalRevenue,
                                   QDateTime::currentDateTime().toString("HH:mm:ss"));
    updateAccountInfo();
}

void MainWindow::updateAccountInfo()
{
    double marketValue = 0.0;
    for (const auto& pos : m_positions) {
        marketValue += pos.quantity * pos.currentPrice;
    }

    const double totalAssets = m_currentCash + marketValue;
    const double totalProfit = totalAssets - m_initialCapital;
    const double profitPercent = (totalAssets / m_initialCapital - 1) * 100;

    m_accountPanel->updateAccount(totalAssets, m_currentCash, marketValue, totalProfit, profitPercent);

    QMap<QString, ::PositionInfo> accountPositions;
    for (auto it = m_positions.cbegin(); it != m_positions.cend(); ++it) {
        const PositionInfo& pos = it.value();
        ::PositionInfo info;
        info.symbol = pos.symbol;
        info.quantity = pos.quantity;
        info.avgCost = pos.avgCost;
        info.currentPrice = pos.currentPrice;
        info.marketValue = pos.quantity * pos.currentPrice;
        info.profit = (pos.currentPrice - pos.avgCost) * pos.quantity;
        info.profitPercent = pos.avgCost > 0.0 ? (pos.currentPrice / pos.avgCost - 1) * 100 : 0.0;
        accountPositions[it.key()] = info;
    }

    m_accountPanel->updatePositions(accountPositions);
}

void MainWindow::configureStrategy(const StrategyConfig& config)
{
    const bool needsNewStrategy = !m_strategy || m_activeStrategyType != config.strategyType;
    if (needsNewStrategy) {
        if (m_strategy) {
            disconnect(m_strategy, nullptr, this, nullptr);
            delete m_strategy;
            m_strategy = nullptr;
        }

        m_activeStrategyType = config.strategyType;
        if (config.strategyType == StrategyType::ProsperityGrowth) {
            m_strategy = new ProsperityGrowthStrategy(this);
        } else {
            m_strategy = new MovingAverageStrategy(this);
        }
        connect(m_strategy, &StrategyBase::signalGenerated, this, &MainWindow::onStrategySignal);
    }

    if (m_strategy) {
        m_strategy->setConfig(config);
    }

    if (!m_isRunning && m_strategyMarketData) {
        m_strategyMarketData->setSymbol(config.symbol);
    }
}

void MainWindow::onViewSymbolChanged(const QString& symbol)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    m_marketData->setSymbol(normalized);
    m_hasLastMarketData = false;
    ui->chartPanel->clearData();
}
void MainWindow::onStartStrategy()
{
    if (m_isRunning) {
        return;
    }

    const StrategyConfig config = ui->strategyPanel->strategyConfig();
    configureStrategy(config);
    m_strategyMarketData->setSymbol(config.symbol);

    m_isRunning = true;
    if (m_strategy) {
        m_strategy->reset();
        m_strategy->setConfig(config);
    }
    ui->strategyPanel->setRunningState(true);
    m_hasLastStrategyMarketData = false;
    m_indicatorHistory.clear();
    if (m_signalPanel) {
        m_signalPanel->clearIndicators();
    }
    m_strategyMarketData->startSimulation();

    const QString strategyMode = config.strategyType == StrategyType::ProsperityGrowth
        ? QStringLiteral("景气成长分批低吸")
        : QStringLiteral("双均线");
    ui->strategyPanel->addSystemLog(
        QStringLiteral("实时模拟策略已启动：%1，策略标的 %2。")
            .arg(strategyMode, config.symbol));
    ui->strategyPanel->addSystemLog(
        QStringLiteral("当前策略：%1").arg(ui->strategyPanel->currentStrategyName()));
    ui->strategyPanel->addSystemLog(ui->strategyPanel->currentStrategyConfigurationSummary());

    if (m_hasLastStrategyMarketData && m_lastStrategyMarketData.symbol == config.symbol && m_strategy) {
        m_strategy->processMarketData(m_lastStrategyMarketData);
        ui->strategyPanel->addSystemLog(
            QStringLiteral("已载入策略行情：%1 @ %2")
                .arg(m_lastStrategyMarketData.symbol)
                .arg(m_lastStrategyMarketData.close, 0, 'f', 2));
    } else {
        ui->strategyPanel->addSystemLog(QStringLiteral("正在等待策略标的第一笔实时行情..."));
    }

    ui->statusbar->showMessage(QStringLiteral("实时模拟策略已启动"), 2000);
}
void MainWindow::onStopStrategy()
{
    if (!m_isRunning) {
        return;
    }

    m_isRunning = false;
    if (m_strategyMarketData) {
        m_strategyMarketData->stopSimulation();
    }
    ui->strategyPanel->setRunningState(false);
    ui->strategyPanel->addSystemLog(QStringLiteral("实时模拟策略已停止，主页看盘行情继续刷新。"));

    ui->statusbar->showMessage(QStringLiteral("实时模拟策略已停止"), 2000);
}
void MainWindow::onUpdateParameters()
{
    const StrategyConfig config = ui->strategyPanel->strategyConfig();
    configureStrategy(config);
    if (!m_isRunning && m_strategyMarketData) {
        m_strategyMarketData->setSymbol(config.symbol);
        m_indicatorHistory.clear();
        m_hasLastStrategyMarketData = false;
        if (m_signalPanel) {
            m_signalPanel->clearIndicators();
        }
    }
}