#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "indicator/indicators.h"
#include "strategy/movingaveragestrategy.h"
#include "strategy/prosperitygrowthstrategy.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include <vector>

namespace {
constexpr int MaxStoredTradeRecords = 50;

void showThemedWarning(QWidget* parent, const QString& title, const QString& message)
{
    QDialog dialog(parent);
    dialog.setModal(true);
    dialog.setWindowTitle(title);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setObjectName(QStringLiteral("themedWarningDialog"));
    dialog.setMinimumWidth(560);
    dialog.setStyleSheet(QStringLiteral(R"(
        QDialog#themedWarningDialog {
            background-color: #1a1a2e;
            border: 1px solid #4a5568;
            border-radius: 8px;
        }
        QLabel#dialogTitle {
            color: #f7fafc;
            font-size: 15px;
            font-weight: bold;
        }
        QLabel#dialogIcon {
            background-color: #f6ad55;
            color: #1a1a2e;
            border-radius: 14px;
            min-width: 28px;
            max-width: 28px;
            min-height: 28px;
            max-height: 28px;
            font-size: 18px;
            font-weight: bold;
        }
        QLabel#dialogMessage {
            color: #e2e8f0;
            font-size: 13px;
        }
        QPushButton#dialogCloseButton {
            background-color: transparent;
            color: #a0aec0;
            border: none;
            border-radius: 4px;
            min-width: 28px;
            max-width: 28px;
            min-height: 28px;
            max-height: 28px;
            padding: 0;
            font-size: 16px;
        }
        QPushButton#dialogCloseButton:hover {
            background-color: #2d3748;
            color: white;
        }
        QPushButton#dialogOkButton {
            background-color: #2b6cb0;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 24px;
            min-width: 72px;
            min-height: 32px;
            font-size: 13px;
        }
        QPushButton#dialogOkButton:hover {
            background-color: #3182ce;
        }
        QPushButton#dialogOkButton:pressed {
            background-color: #2c5282;
        }
    )"));

    auto* rootLayout = new QVBoxLayout(&dialog);
    rootLayout->setContentsMargins(18, 16, 18, 16);
    rootLayout->setSpacing(14);

    auto* titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(10);

    auto* titleLabel = new QLabel(title, &dialog);
    titleLabel->setObjectName(QStringLiteral("dialogTitle"));

    auto* closeButton = new QPushButton(QStringLiteral("×"), &dialog);
    closeButton->setObjectName(QStringLiteral("dialogCloseButton"));
    closeButton->setCursor(Qt::PointingHandCursor);

    titleLayout->addWidget(titleLabel, 1);
    titleLayout->addWidget(closeButton, 0, Qt::AlignTop);
    rootLayout->addLayout(titleLayout);

    auto* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 2, 0, 2);
    contentLayout->setSpacing(12);

    auto* iconLabel = new QLabel(QStringLiteral("!"), &dialog);
    iconLabel->setObjectName(QStringLiteral("dialogIcon"));
    iconLabel->setAlignment(Qt::AlignCenter);

    auto* messageLabel = new QLabel(message, &dialog);
    messageLabel->setObjectName(QStringLiteral("dialogMessage"));
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageLabel->setMinimumWidth(430);

    contentLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    contentLayout->addWidget(messageLabel, 1);
    rootLayout->addLayout(contentLayout);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 4, 0, 0);
    buttonLayout->addStretch();

    auto* okButton = new QPushButton(QStringLiteral("确定"), &dialog);
    okButton->setObjectName(QStringLiteral("dialogOkButton"));
    okButton->setCursor(Qt::PointingHandCursor);
    okButton->setDefault(true);
    buttonLayout->addWidget(okButton);
    rootLayout->addLayout(buttonLayout);

    QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}
}

MainWindow::MainWindow(bool guestMode, const QString& accountName, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_marketData(nullptr)
    , m_strategyMarketData(nullptr)
    , m_manualTradeMarketData(nullptr)
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
    , m_pendingManualTradeType(SignalType::NONE)
    , m_pendingManualTradeVolume(0.0)
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
        "QComboBox { background-color: #4a5568; color: white; border: 1px solid #718096; border-radius: 6px; padding: 6px 10px; font-size: 13px; min-height: 30px; }"
        "QComboBox:disabled { background-color: #3b4556; color: #8ea0b8; }"
        "QComboBox QAbstractItemView { background-color: #1a1a2e; color: #e0e0e0; border: 1px solid #4a5568; selection-background-color: #2b6cb0; selection-color: white; }"
        "QCheckBox { color: #e0e0e0; spacing: 8px; font-size: 13px; }"
        "QCheckBox:disabled { color: #8ea0b8; }"
        "QCheckBox::indicator { width: 14px; height: 14px; }"
        "QCheckBox::indicator:unchecked { background-color: #253b6e; border: 1px solid #718096; border-radius: 3px; }"
        "QCheckBox::indicator:checked { background-color: #63b3ed; border: 1px solid #90cdf4; border-radius: 3px; image: url(:/icons/checkbox_check.png); }"
        "QCheckBox::indicator:unchecked:disabled { background-color: #2d3748; border-color: #4a5568; }"
        "QCheckBox::indicator:checked:disabled { background-color: #4a6b8f; border-color: #5f7894; image: url(:/icons/checkbox_check.png); }"
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
        "QPushButton#favoriteBuyBtn { background-color: #38a169; }"
        "QPushButton#favoriteBuyBtn:hover { background-color: #2f855a; }"
        "QPushButton#favoriteSellBtn { background-color: #e53e3e; }"
        "QPushButton#favoriteSellBtn:hover { background-color: #c53030; }"
        "QTableWidget { background-color: #1a1a2e; border: 1px solid #4a5568; border-radius: 6px; gridline-color: #4a5568; }"
        "QTableWidget::item { color: #e0e0e0; padding: 4px 8px; }"
        "QTableWidget::item:selected { background-color: #4299e1; }"
        "QTableWidget::horizontalHeader { background-color: #2d3748; }"
        "QTableWidget::horizontalHeader::section { color: #63b3ed; background-color: #2d3748; padding: 6px 8px; border: 1px solid #4a5568; }"
        "QTextEdit { background-color: #1a1a2e; color: #e0e0e0; border: 1px solid #4a5568; border-radius: 6px; font-family: Consolas,Monaco,monospace; font-size: 12px; }"
        "QTextBrowser { background-color: #1a1a2e; color: #e0e0e0; border: 1px solid #4a5568; border-radius: 6px; font-size: 13px; }"
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
    m_manualTradeMarketData = new MarketDataSimulator(this);
    m_marketData->setFeedMode(MarketDataFeedMode::QuoteWhenOpenTrendWhenClosed);
    m_strategyMarketData->setFeedMode(MarketDataFeedMode::RealtimeQuoteOnly);
    m_manualTradeMarketData->setFeedMode(MarketDataFeedMode::QuoteOnly);
    m_strategy = nullptr;

    connect(m_marketData, &MarketDataSimulator::dataUpdated, this, &MainWindow::onMarketDataUpdated);
    connect(m_marketData, &MarketDataSimulator::intradayDataUpdated, this, &MainWindow::onIntradayDataUpdated);
    connect(m_marketData, &MarketDataSimulator::fallbackIntradayDataUsed, this, &MainWindow::onFallbackIntradayDataUsed);
    connect(m_marketData, &MarketDataSimulator::errorOccurred, this, &MainWindow::onMarketDataError);
    connect(m_strategyMarketData, &MarketDataSimulator::dataUpdated, this, &MainWindow::onStrategyMarketDataUpdated);
    connect(m_strategyMarketData, &MarketDataSimulator::intradayDataUpdated, this, &MainWindow::onStrategyIntradayDataUpdated);
    connect(m_strategyMarketData, &MarketDataSimulator::errorOccurred, this, &MainWindow::onStrategyMarketDataError);
    connect(ui->strategyPanel, &StrategyPanel::startClicked, this, &MainWindow::onStartStrategy);
    connect(ui->strategyPanel, &StrategyPanel::stopClicked, this, &MainWindow::onStopStrategy);
    connect(ui->strategyPanel, &StrategyPanel::parametersChanged, this, &MainWindow::onUpdateParameters);
    connect(ui->strategyPanel, &StrategyPanel::viewSymbolChanged, this, &MainWindow::onViewSymbolChanged);
    connect(ui->strategyPanel, &StrategyPanel::favoriteSelected, this, &MainWindow::onFavoriteSelected);
    connect(ui->strategyPanel, &StrategyPanel::favoriteBuyRequested, this, &MainWindow::onFavoriteBuyRequested);
    connect(ui->strategyPanel, &StrategyPanel::favoriteSellRequested, this, &MainWindow::onFavoriteSellRequested);
    connect(m_manualTradeMarketData, &MarketDataSimulator::dataUpdated, this, &MainWindow::onManualTradeQuoteUpdated);
    connect(m_manualTradeMarketData, &MarketDataSimulator::errorOccurred, this, &MainWindow::onManualTradeQuoteError);

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
    loadAccountState();

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
    if (m_manualTradeMarketData) {
        m_manualTradeMarketData->stopSimulation();
    }
    saveAccountState();
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
        watchlist->setMaximumHeight(QWIDGETSIZE_MAX);
        watchlist->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
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

void MainWindow::onFallbackIntradayDataUsed(const QString& symbol, const QString& reason)
{
    const QString detail = reason.isEmpty() ? QStringLiteral("分时接口未返回可用数据") : reason;
    ui->strategyPanel->addSystemLog(
        QStringLiteral("离线分时行情获取失败，已使用 %1 的开盘/最高/最低/收盘 4 点摘要兜底绘图；这不是完整分时行情。原因：%2")
            .arg(symbol, detail));
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
        showThemedWarning(this,
                          QStringLiteral("策略已停止"),
                          QStringLiteral("未获取到可用于策略执行的实时行情。\n\n%1").arg(message));
        onStopStrategy();
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

void MainWindow::loadAccountState()
{
    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    m_initialCapital = settings.value(QStringLiteral("account/initialCapital"), 100000.0).toDouble();
    if (m_initialCapital <= 0.0) {
        m_initialCapital = 100000.0;
    }
    m_currentCash = settings.value(QStringLiteral("account/currentCash"), m_initialCapital).toDouble();

    m_positions.clear();
    const QByteArray positionsPayload = settings.value(QStringLiteral("account/positions")).toString().toUtf8();
    if (!positionsPayload.isEmpty()) {
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(positionsPayload, &error);
        if (error.error == QJsonParseError::NoError && document.isArray()) {
            const QJsonArray positions = document.array();
            for (const QJsonValue& value : positions) {
                if (!value.isObject()) {
                    continue;
                }

                const QJsonObject object = value.toObject();
                const QString symbol = MarketDataSimulator::normalizeSymbol(object.value(QStringLiteral("symbol")).toString());
                const double quantity = object.value(QStringLiteral("quantity")).toDouble();
                if (symbol.isEmpty() || quantity <= 0.0) {
                    continue;
                }

                PositionInfo position;
                position.symbol = symbol;
                position.quantity = quantity;
                position.avgCost = object.value(QStringLiteral("avgCost")).toDouble();
                position.currentPrice = object.value(QStringLiteral("currentPrice")).toDouble(position.avgCost);
                position.marketValue = position.quantity * position.currentPrice;
                position.profit = (position.currentPrice - position.avgCost) * position.quantity;
                position.profitPercent = position.avgCost > 0.0 ? (position.currentPrice / position.avgCost - 1.0) * 100.0 : 0.0;
                m_positions[symbol] = position;
            }
        }
    }

    m_tradeRecords.clear();
    const QByteArray tradesPayload = settings.value(QStringLiteral("account/trades")).toString().toUtf8();
    if (!tradesPayload.isEmpty()) {
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(tradesPayload, &error);
        if (error.error == QJsonParseError::NoError && document.isArray()) {
            const QJsonArray trades = document.array();
            for (const QJsonValue& value : trades) {
                if (!value.isObject()) {
                    continue;
                }

                const QJsonObject object = value.toObject();
                TradeRecordInfo record;
                record.time = object.value(QStringLiteral("time")).toString();
                record.symbol = MarketDataSimulator::normalizeSymbol(object.value(QStringLiteral("symbol")).toString());
                record.type = object.value(QStringLiteral("type")).toString();
                record.price = object.value(QStringLiteral("price")).toDouble();
                record.volume = object.value(QStringLiteral("volume")).toDouble();
                record.amount = object.value(QStringLiteral("amount")).toDouble();
                if (record.symbol.isEmpty() || record.type.isEmpty()) {
                    continue;
                }

                m_tradeRecords.append(record);
                while (m_tradeRecords.size() > MaxStoredTradeRecords) {
                    m_tradeRecords.removeFirst();
                }
            }
        }
    }

    if (m_accountPanel) {
        for (const TradeRecordInfo& record : m_tradeRecords) {
            m_accountPanel->addTradeRecord(record.symbol, record.type, record.price, record.volume, record.amount, record.time);
        }
    }
}

void MainWindow::saveAccountState() const
{
    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    settings.setValue(QStringLiteral("account/initialCapital"), m_initialCapital);
    settings.setValue(QStringLiteral("account/currentCash"), m_currentCash);
    settings.setValue(QStringLiteral("account/guestMode"), m_guestMode);
    settings.setValue(QStringLiteral("account/accountName"), m_accountName);

    QJsonArray positions;
    for (auto it = m_positions.cbegin(); it != m_positions.cend(); ++it) {
        const PositionInfo& position = it.value();
        if (position.quantity <= 0.0) {
            continue;
        }

        QJsonObject object;
        object.insert(QStringLiteral("symbol"), position.symbol.isEmpty() ? it.key() : position.symbol);
        object.insert(QStringLiteral("quantity"), position.quantity);
        object.insert(QStringLiteral("avgCost"), position.avgCost);
        object.insert(QStringLiteral("currentPrice"), position.currentPrice);
        positions.append(object);
    }
    settings.setValue(QStringLiteral("account/positions"), QString::fromUtf8(QJsonDocument(positions).toJson(QJsonDocument::Compact)));

    QJsonArray trades;
    for (const TradeRecordInfo& record : m_tradeRecords) {
        QJsonObject object;
        object.insert(QStringLiteral("time"), record.time);
        object.insert(QStringLiteral("symbol"), record.symbol);
        object.insert(QStringLiteral("type"), record.type);
        object.insert(QStringLiteral("price"), record.price);
        object.insert(QStringLiteral("volume"), record.volume);
        object.insert(QStringLiteral("amount"), record.amount);
        trades.append(object);
    }
    settings.setValue(QStringLiteral("account/trades"), QString::fromUtf8(QJsonDocument(trades).toJson(QJsonDocument::Compact)));
    settings.sync();
}

void MainWindow::appendTradeRecord(const QString& symbol, const QString& type, double price, double volume, double amount, const QString& time)
{
    TradeRecordInfo record;
    record.time = time;
    record.symbol = symbol;
    record.type = type;
    record.price = price;
    record.volume = volume;
    record.amount = amount;

    m_tradeRecords.append(record);
    while (m_tradeRecords.size() > MaxStoredTradeRecords) {
        m_tradeRecords.removeFirst();
    }

    if (m_accountPanel) {
        m_accountPanel->addTradeRecord(symbol, type, price, volume, amount, time);
    }
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

            appendTradeRecord(symbol, typeStr, price, lotSize, totalCost,
                              QDateTime::currentDateTime().toString("HH:mm:ss"));
            updateAccountInfo();
            saveAccountState();
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

    appendTradeRecord(symbol, typeStr, price, lotSize, totalRevenue,
                      QDateTime::currentDateTime().toString("HH:mm:ss"));
    updateAccountInfo();
    saveAccountState();
}

double MainWindow::latestPriceForSymbol(const QString& symbol) const
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    if (m_hasLastMarketData && MarketDataSimulator::normalizeSymbol(m_lastMarketData.symbol) == normalized) {
        if (m_lastMarketData.close > 0.0) {
            return m_lastMarketData.close;
        }
        if (m_lastMarketData.previousClose > 0.0) {
            return m_lastMarketData.previousClose;
        }
    }
    if (m_hasLastStrategyMarketData && MarketDataSimulator::normalizeSymbol(m_lastStrategyMarketData.symbol) == normalized) {
        if (m_lastStrategyMarketData.close > 0.0) {
            return m_lastStrategyMarketData.close;
        }
        if (m_lastStrategyMarketData.previousClose > 0.0) {
            return m_lastStrategyMarketData.previousClose;
        }
    }
    if (m_positions.contains(normalized) && m_positions.value(normalized).currentPrice > 0.0) {
        return m_positions.value(normalized).currentPrice;
    }
    return 0.0;
}

void MainWindow::onFavoriteSelected(const QString& symbol)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    const double price = latestPriceForSymbol(normalized);
    if (price > 0.0) {
        ui->strategyPanel->setManualTradePrice(normalized, price);
        return;
    }

    m_manualPriceQuoteSymbol = normalized;
    m_manualTradeMarketData->setSymbol(normalized);
    m_manualTradeMarketData->startSimulation();
}

void MainWindow::onFavoriteBuyRequested(const QString& symbol, double price, double volume)
{
    requestManualTrade(symbol, SignalType::BUY, price, volume);
}

void MainWindow::onFavoriteSellRequested(const QString& symbol, double price, double volume)
{
    requestManualTrade(symbol, SignalType::SELL, price, volume);
}

void MainWindow::requestManualTrade(const QString& symbol, SignalType type, double price, double volume)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    const double tradePrice = price > 0.0 ? price : latestPriceForSymbol(normalized);
    if (tradePrice > 0.0) {
        executeManualTrade(normalized, type, tradePrice, volume);
        return;
    }

    m_pendingManualTradeSymbol = normalized;
    m_pendingManualTradeType = type;
    m_pendingManualTradeVolume = volume;
    m_manualTradeMarketData->setSymbol(normalized);
    m_manualTradeMarketData->startSimulation();
    ui->statusbar->showMessage(QStringLiteral("正在获取 %1 最新行情，获取后执行模拟%2...")
        .arg(normalized, type == SignalType::BUY ? QStringLiteral("买入") : QStringLiteral("卖出")), 3000);
}

void MainWindow::onManualTradeQuoteUpdated(const MarketData& data)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(data.symbol);
    const double quotePrice = data.close > 0.0 ? data.close : data.previousClose;
    ui->strategyPanel->rememberStockName(normalized, data.name);

    if (quotePrice > 0.0 && normalized == m_manualPriceQuoteSymbol) {
        ui->strategyPanel->setManualTradePrice(normalized, quotePrice);
        m_manualPriceQuoteSymbol.clear();
    }

    if (m_pendingManualTradeType == SignalType::NONE || normalized != m_pendingManualTradeSymbol || quotePrice <= 0.0) {
        if (m_pendingManualTradeType == SignalType::NONE && m_manualPriceQuoteSymbol.isEmpty()) {
            m_manualTradeMarketData->stopSimulation();
        }
        return;
    }

    const SignalType type = m_pendingManualTradeType;
    const double volume = m_pendingManualTradeVolume;
    m_pendingManualTradeSymbol.clear();
    m_pendingManualTradeType = SignalType::NONE;
    m_pendingManualTradeVolume = 0.0;
    m_manualTradeMarketData->stopSimulation();
    executeManualTrade(normalized, type, quotePrice, volume);
}

void MainWindow::onManualTradeQuoteError(const QString& message)
{
    if (m_pendingManualTradeType == SignalType::NONE && m_manualPriceQuoteSymbol.isEmpty()) {
        return;
    }

    m_pendingManualTradeSymbol.clear();
    m_manualPriceQuoteSymbol.clear();
    m_pendingManualTradeType = SignalType::NONE;
    m_pendingManualTradeVolume = 0.0;
    m_manualTradeMarketData->stopSimulation();
    ui->statusbar->showMessage(QStringLiteral("手动模拟交易行情获取失败：%1").arg(message), 5000);
}

void MainWindow::executeManualTrade(const QString& symbol, SignalType type, double price, double volume)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    if (volume <= 0.0 || price <= 0.0) {
        return;
    }

    if (type == SignalType::BUY) {
        const double amount = price * volume;
        if (amount > m_currentCash) {
            ui->statusbar->showMessage(QStringLiteral("模拟买入失败：可用资金不足"), 4000);
            return;
        }
    } else if (type == SignalType::SELL) {
        if (!m_positions.contains(normalized) || m_positions.value(normalized).quantity <= 0.0) {
            ui->statusbar->showMessage(QStringLiteral("模拟卖出失败：当前没有 %1 持仓").arg(normalized), 4000);
            return;
        }
        volume = qMin(volume, m_positions.value(normalized).quantity);
        m_positions[normalized].currentPrice = price;
    } else {
        return;
    }

    StrategySignal signal;
    signal.type = type;
    signal.symbol = normalized;
    signal.price = price;
    signal.volume = volume;
    signal.timestamp = QDateTime::currentDateTime();
    signal.comment = type == SignalType::BUY ? QStringLiteral("手动模拟买入") : QStringLiteral("手动模拟卖出");
    executeOrder(signal);

    ui->statusbar->showMessage(QStringLiteral("已%1 %2 %3 股 @ %4")
        .arg(type == SignalType::BUY ? QStringLiteral("买入") : QStringLiteral("卖出"), normalized)
        .arg(volume, 0, 'f', 0)
        .arg(price, 0, 'f', 2), 4000);
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

    if (!MarketDataSimulator::isAShareContinuousTradingTime()) {
        const QString message = QStringLiteral("当前未处于 A 股连续竞价时段。\n\n策略只能在交易日 09:30-11:30、13:00-15:00 使用实时行情运行；收盘后主图可以查看最近可用交易日行情，但不能启动策略。");
        showThemedWarning(this, QStringLiteral("不能启动策略"), message);
        ui->strategyPanel->addSystemLog(message);
        ui->statusbar->showMessage(QStringLiteral("未开盘，不能执行策略"), 4000);
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