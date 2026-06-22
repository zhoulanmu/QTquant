#include "mainwindow.h"
#include "ui_mainwindow.h"

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

MainWindow::MainWindow(bool guestMode, const QString& accountName, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_marketData(nullptr)
    , m_strategy(nullptr)
    , m_accountPanel(nullptr)
    , m_statisticsPanel(nullptr)
    , m_signalPanel(nullptr)
    , m_mainTabs(nullptr)
    , m_isRunning(false)
    , m_initialCapital(100000.0)
    , m_currentCash(100000.0)
    , m_currentPrice(10.78)
    , m_hasLastMarketData(false)
    , m_guestMode(guestMode)
    , m_accountName(accountName)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("星策 StarQuant - 真实行情看盘"));

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
    m_strategy = new MovingAverageStrategy(this);

    connect(m_marketData, &MarketDataSimulator::dataUpdated, this, &MainWindow::onMarketDataUpdated);
    connect(m_marketData, &MarketDataSimulator::errorOccurred, this, &MainWindow::onMarketDataError);
    connect(m_strategy, &StrategyBase::signalGenerated, this, &MainWindow::onStrategySignal);
    connect(ui->strategyPanel, &StrategyPanel::startClicked, this, &MainWindow::onStartStrategy);
    connect(ui->strategyPanel, &StrategyPanel::stopClicked, this, &MainWindow::onStopStrategy);
    connect(ui->strategyPanel, &StrategyPanel::parametersChanged, this, &MainWindow::onUpdateParameters);

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

    buildTabbedLayout();

    ui->strategyPanel->setRunningState(false);
    onUpdateParameters();
    updateAccountInfo();

    m_marketData->startSimulation();
    ui->statusbar->showMessage(m_guestMode
        ? QStringLiteral("游客看盘：实时行情已启动")
        : QStringLiteral("客户端已登录：%1，实时行情已启动").arg(m_accountName), 4000);
}

MainWindow::~MainWindow()
{
    m_marketData->stopSimulation();
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

    auto* newsGroup = new QGroupBox(QStringLiteral("市场快讯"), tab);
    auto* newsLayout = new QVBoxLayout(newsGroup);
    auto* newsTable = new QTableWidget(newsGroup);
    newsTable->setColumnCount(4);
    newsTable->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("来源"), QStringLiteral("标题"), QStringLiteral("相关")});
    newsTable->horizontalHeader()->setStretchLastSection(true);
    newsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    newsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    newsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    newsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    newsTable->setRowCount(3);

    const QList<QStringList> rows = {
        {QStringLiteral("--:--"), QStringLiteral("系统"), QStringLiteral("市场快讯源等待接入"), QStringLiteral("全市场")},
        {QStringLiteral("--:--"), QStringLiteral("自选"), QStringLiteral("自选股公告等待接入"), QStringLiteral("自选股")},
        {QStringLiteral("--:--"), QStringLiteral("日历"), QStringLiteral("宏观事件与财报日历等待接入"), QStringLiteral("日历")}
    };
    for (int row = 0; row < rows.size(); ++row) {
        for (int col = 0; col < rows[row].size(); ++col) {
            newsTable->setItem(row, col, new QTableWidgetItem(rows[row][col]));
        }
    }
    newsLayout->addWidget(newsTable);

    auto* detailGroup = new QGroupBox(QStringLiteral("新闻详情"), tab);
    auto* detailLayout = new QVBoxLayout(detailGroup);
    auto* detailText = new QTextEdit(detailGroup);
    detailText->setReadOnly(true);
    detailText->setText(QStringLiteral("新闻 tab 已预留市场快讯、自选股新闻、公告和宏观日历位置。"));
    detailLayout->addWidget(detailText);

    rootLayout->addWidget(newsGroup, 2);
    rootLayout->addWidget(detailGroup, 1);
    return tab;
}
void MainWindow::onMarketDataUpdated(const MarketData &data)
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

    if (m_isRunning) {
        m_strategy->processMarketData(data);
    }
}

void MainWindow::onMarketDataError(const QString &message)
{
    ui->statusbar->showMessage(message, 5000);
    if (m_isRunning) {
        ui->strategyPanel->addSystemLog(QStringLiteral("行情错误：%1").arg(message));
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
    const double lotSize = ui->strategyPanel->getLotSize();
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

void MainWindow::onStartStrategy()
{
    if (m_isRunning) {
        return;
    }

    onUpdateParameters();

    m_isRunning = true;
    m_strategy->reset();
    ui->strategyPanel->setRunningState(true);
    m_marketData->startSimulation();

    ui->strategyPanel->addSystemLog(
        QStringLiteral("策略已启动：%1，等待至少 %2 条行情计算慢速 MA。")
            .arg(m_marketData->getSymbol())
            .arg(ui->strategyPanel->getSlowMA()));
    ui->strategyPanel->addSystemLog(
        QStringLiteral("当前策略：%1").arg(ui->strategyPanel->currentStrategyName()));
    ui->strategyPanel->addSystemLog(ui->strategyPanel->currentStrategyConfigurationSummary());

    if (m_hasLastMarketData && m_lastMarketData.symbol == m_marketData->getSymbol()) {
        m_strategy->processMarketData(m_lastMarketData);
        ui->strategyPanel->addSystemLog(
            QStringLiteral("已载入当前行情：%1 @ %2")
                .arg(m_lastMarketData.symbol)
                .arg(m_lastMarketData.close, 0, 'f', 2));
    } else {
        ui->strategyPanel->addSystemLog(QStringLiteral("正在等待第一笔实时行情..."));
    }

    ui->statusbar->showMessage(QStringLiteral("策略已启动：使用真实行情"), 2000);
}

void MainWindow::onStopStrategy()
{
    if (!m_isRunning) {
        return;
    }

    m_isRunning = false;
    ui->strategyPanel->setRunningState(false);
    ui->strategyPanel->addSystemLog(QStringLiteral("策略已停止，行情继续刷新。"));

    ui->statusbar->showMessage(QStringLiteral("策略已停止，行情继续刷新"), 2000);
}

void MainWindow::onUpdateParameters()
{
    StrategyParameters params;
    params.symbol = MarketDataSimulator::normalizeSymbol(ui->strategyPanel->getSymbol());
    params.fastMA = ui->strategyPanel->getFastMA();
    params.slowMA = ui->strategyPanel->getSlowMA();
    params.stopLossPercent = ui->strategyPanel->getStopLossPercent();
    params.takeProfitPercent = ui->strategyPanel->getTakeProfitPercent();
    params.lotSize = ui->strategyPanel->getLotSize();

    m_strategy->setParameters(params);
    m_marketData->setSymbol(params.symbol);
}
