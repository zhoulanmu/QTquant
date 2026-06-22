#include "strategypanel.h"
#include "ui_strategypanel.h"
#include "../market/marketdata.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QStringListModel>
#include <QTime>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace {
constexpr int SearchDelayMs = 280;
constexpr int SearchTimeoutMs = 2500;
constexpr auto SearchEndpoint = "https://searchapi.eastmoney.com/api/suggest/get";
constexpr auto SearchToken = "D43BF722C8E33EC61";

enum class StrategyKind
{
    DoubleMA,
    ProsperityGrowth
};

struct StrategyPreset
{
    StrategyKind kind;
    QString name;
    QString description;
    int fastMA;
    int slowMA;
    double stopLoss;
    double takeProfit;
    double lotSize;
};

QVector<StrategyPreset> commonStrategyPresets()
{
    return QVector<StrategyPreset>{
        {StrategyKind::DoubleMA,
         QStringLiteral("双均线策略"),
         QStringLiteral("快慢均线金叉买入，死叉或风控卖出，是当前自动交易执行的基础策略。"),
         5, 20, 2.0, 5.0, 1000.0},
        {StrategyKind::ProsperityGrowth,
         QStringLiteral("景气成长分批低吸策略"),
         QStringLiteral("适配 AI 算力、半导体设备、机器人零部件、储能；强调基本面确认、板块回调和分批低吸。"),
         20, 60, 8.0, 25.0, 1000.0}
    };
}

StrategyKind strategyKindAt(int index)
{
    const QVector<StrategyPreset> presets = commonStrategyPresets();
    if (index < 0 || index >= presets.size()) {
        return StrategyKind::DoubleMA;
    }
    return presets[index].kind;
}
void appendUniqueCandidate(QVector<StockCandidate>* candidates, const StockCandidate& candidate)
{
    if (!candidates || candidate.symbol.isEmpty()) {
        return;
    }

    for (const auto& existing : *candidates) {
        if (existing.symbol == candidate.symbol) {
            return;
        }
    }

    candidates->append(candidate);
}

void collectSearchObjects(const QJsonValue& value, QVector<QJsonObject>* objects)
{
    if (!objects) {
        return;
    }

    if (value.isArray()) {
        const QJsonArray array = value.toArray();
        for (const auto& item : array) {
            collectSearchObjects(item, objects);
        }
        return;
    }

    if (!value.isObject()) {
        return;
    }

    const QJsonObject object = value.toObject();
    const bool hasStockFields = (object.contains(QStringLiteral("Code")) || object.contains(QStringLiteral("code")))
        && (object.contains(QStringLiteral("Name")) || object.contains(QStringLiteral("name")));
    if (hasStockFields) {
        objects->append(object);
    }

    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        collectSearchObjects(it.value(), objects);
    }
}

QString strategyDetailTooltip(StrategyKind kind)
{
    if (kind == StrategyKind::ProsperityGrowth) {
        return QStringLiteral("<qt><b>景气成长分批低吸策略说明</b><br>"
            "<br><b>实际买入条件</b><br>"
            "1. 至少勾选一个适配主线；全不勾选时不会开仓。<br>"
            "2. 三个人工确认项都必须勾选；任意一个不勾选，策略视为条件未通过，不会开仓。<br>"
            "3. 连续回调天数必须落在配置窗口内，例如 3 至 5 天。当前用策略标的连续下跌近似判断。<br>"
            "4. 勾选 60 日均线持续向上时，买入必须满足 60 日均线不低于上一周期；不勾选则放宽这条过滤。<br>"
            "5. 勾选缩量回踩 20/60 日线时，买入必须同时接近 20 或 60 日线并缩量；不勾选则放宽这条过滤。<br>"
            "<br><b>实际卖出条件</b><br>"
            "1. 达到短期暴涨止盈线会触发止盈；勾选分批减仓时卖出一半，不勾选时一次性清仓。<br>"
            "2. 勾选放量跌破 60 日线时，跌破 60 日线且放量会离场；不勾选则不执行这条破位止损。<br>"
            "3. 基本交易参数里的通用止损仍会生效。<br>"
            "<br><b>当前尚未自动执行</b><br>"
            "单赛道上限、分散主线数量目前进入配置模型，但还没有真正限制模拟账户仓位；所属赛道、财报、公告也暂时由人工确认代替。</qt>");
    }

    return QStringLiteral("<qt><b>双均线策略说明</b><br>"
        "<br><b>实际买入条件</b><br>"
        "快速 MA 从下方向上穿过慢速 MA，并且 RSI 小于 70 时，触发模拟买入。<br>"
        "<br><b>实际卖出条件</b><br>"
        "快速 MA 下穿慢速 MA 时卖出；价格达到止损或止盈线时，也会触发模拟离场。<br>"
        "<br><b>参数影响</b><br>"
        "快速 MA、慢速 MA、止损、止盈、持仓数量都会进入双均线策略。策略按当前策略标的的实时行情逐笔处理，主页股票只用于看盘。</qt>");
}
}

StrategyPanel::StrategyPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StrategyPanel),
    m_symbolCompleter(nullptr),
    m_strategySymbolCompleter(nullptr),
    m_searchModel(nullptr),
    m_strategySearchModel(nullptr),
    m_searchTimer(nullptr),
    m_searchNetwork(nullptr),
    m_searchReply(nullptr),
    m_strategySymbolEdit(nullptr),
    m_activeSearchEdit(nullptr),
    m_strategyPresetCombo(nullptr),
    m_strategyPresetDescLabel(nullptr),
    m_strategyDetailButton(nullptr),
    m_applyStrategyPresetBtn(nullptr),
    m_strategyConfigGroup(nullptr),
    m_strategyConfigHintLabel(nullptr),
    m_ma60UpCheck(nullptr),
    m_profitGrowthCheck(nullptr),
    m_orderLandingCheck(nullptr),
    m_noPureConceptCheck(nullptr),
    m_volumePullbackCheck(nullptr),
    m_pullbackMinSpin(nullptr),
    m_pullbackMaxSpin(nullptr),
    m_sectorCapSpin(nullptr),
    m_diversifyMainlineSpin(nullptr),
    m_surgeTakeProfitSpin(nullptr),
    m_partialTakeProfitCheck(nullptr),
    m_breakMA60VolumeStopCheck(nullptr),
    m_symbolSelectorWidget(nullptr),
    m_symbolAddFavoriteBtn(nullptr),
    m_strategyControlGroup(nullptr),
    m_watchlistGroup(nullptr),
    m_watchlistWidget(nullptr),
    m_addFavoriteBtn(nullptr),
    m_removeFavoriteBtn(nullptr),
    m_updatingSymbol(false)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(QStringLiteral("基本交易参数"));
    ui->groupBox->setMinimumHeight(170);
    ui->groupBox->setMaximumHeight(230);
    ui->logTextEdit->setReadOnly(true);

    ui->label->setBuddy(nullptr);
    ui->label_2->setBuddy(nullptr);
    ui->label_3->setBuddy(nullptr);
    ui->label_4->setBuddy(nullptr);
    ui->label_5->setBuddy(nullptr);
    ui->label_6->setBuddy(nullptr);

    setupCommonStrategyUi();
    setupStockSearchUi();
    loadWatchlist();
    loadViewSymbol();
    loadStrategySymbol();
    updateSearchSuggestions(ui->symbolEdit->text());
    addSystemLog(QStringLiteral("行情连接中，等待策略启动。"));

    connect(ui->startBtn, &QPushButton::clicked, this, &StrategyPanel::startClicked);
    connect(ui->stopBtn, &QPushButton::clicked, this, &StrategyPanel::stopClicked);
    connect(ui->symbolEdit, &QLineEdit::textChanged, this, &StrategyPanel::onSymbolTextChanged);
    connect(ui->symbolEdit, &QLineEdit::editingFinished, this, &StrategyPanel::onSymbolEditingFinished);
    connect(ui->fastMASpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->slowMASpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->stopLossSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->takeProfitSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->lotSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
}

StrategyPanel::~StrategyPanel()
{
    abortSearchReply();
    delete ui;
}

QString StrategyPanel::getSymbol() const
{
    return getStrategySymbol();
}

QString StrategyPanel::getViewSymbol() const
{
    const QString resolved = resolveSymbolText(ui->symbolEdit->text());
    return resolved.isEmpty() ? ui->symbolEdit->text().trimmed() : resolved;
}

QString StrategyPanel::getStrategySymbol() const
{
    if (!m_strategySymbolEdit) {
        return getViewSymbol();
    }

    const QString resolved = resolveSymbolText(m_strategySymbolEdit->text());
    return resolved.isEmpty() ? m_strategySymbolEdit->text().trimmed() : resolved;
}

StrategyConfig StrategyPanel::strategyConfig() const
{
    StrategyConfig config;
    const int index = m_strategyPresetCombo ? m_strategyPresetCombo->currentIndex() : 0;
    config.strategyType = strategyKindAt(index) == StrategyKind::ProsperityGrowth
        ? StrategyType::ProsperityGrowth
        : StrategyType::DoubleMA;
    config.symbol = MarketDataSimulator::normalizeSymbol(getStrategySymbol());

    config.doubleMAConfig.fastMA = getFastMA();
    config.doubleMAConfig.slowMA = getSlowMA();

    config.riskConfig.stopLossPercent = getStopLossPercent();
    config.riskConfig.takeProfitPercent = getTakeProfitPercent();
    config.riskConfig.surgeTakeProfitPercent = m_surgeTakeProfitSpin ? m_surgeTakeProfitSpin->value() : 25.0;
    config.riskConfig.partialTakeProfitEnabled = !m_partialTakeProfitCheck || m_partialTakeProfitCheck->isChecked();
    config.riskConfig.breakMA60VolumeStopEnabled = !m_breakMA60VolumeStopCheck || m_breakMA60VolumeStopCheck->isChecked();

    config.positionConfig.lotSize = getLotSize();
    config.positionConfig.maxSingleTrackPercent = m_sectorCapSpin ? m_sectorCapSpin->value() : 20.0;
    config.positionConfig.diversifyTrackCount = m_diversifyMainlineSpin ? m_diversifyMainlineSpin->value() : 3;

    for (QCheckBox* check : m_growthTrackChecks) {
        if (check && check->isChecked()) {
            config.growthBuyConfig.enabledTracks.append(check->text());
        }
    }
    config.growthBuyConfig.requireMA60Up = !m_ma60UpCheck || m_ma60UpCheck->isChecked();
    config.growthBuyConfig.requireVolumePullback = !m_volumePullbackCheck || m_volumePullbackCheck->isChecked();
    config.growthBuyConfig.requirePullbackToMA20OrMA60 = !m_volumePullbackCheck || m_volumePullbackCheck->isChecked();
    config.growthBuyConfig.profitGrowthConfirmed = m_profitGrowthCheck && m_profitGrowthCheck->isChecked();
    config.growthBuyConfig.orderLandingConfirmed = m_orderLandingCheck && m_orderLandingCheck->isChecked();
    config.growthBuyConfig.noPureConceptConfirmed = m_noPureConceptCheck && m_noPureConceptCheck->isChecked();
    config.growthBuyConfig.pullbackMinDays = m_pullbackMinSpin ? m_pullbackMinSpin->value() : 3;
    config.growthBuyConfig.pullbackMaxDays = m_pullbackMaxSpin ? m_pullbackMaxSpin->value() : 5;

    return config;
}

int StrategyPanel::getFastMA() const
{
    return ui->fastMASpin->value();
}

int StrategyPanel::getSlowMA() const
{
    return ui->slowMASpin->value();
}

double StrategyPanel::getStopLossPercent() const
{
    return ui->stopLossSpin->value();
}

double StrategyPanel::getTakeProfitPercent() const
{
    return ui->takeProfitSpin->value();
}

double StrategyPanel::getLotSize() const
{
    return ui->lotSizeSpin->value();
}

QString StrategyPanel::currentStrategyName() const
{
    const QVector<StrategyPreset> presets = commonStrategyPresets();
    const int index = m_strategyPresetCombo ? m_strategyPresetCombo->currentIndex() : 0;
    if (index < 0 || index >= presets.size()) {
        return presets.first().name;
    }
    return presets[index].name;
}

QString StrategyPanel::currentStrategyConfigurationSummary() const
{
    const StrategyConfig config = strategyConfig();
    if (config.strategyType == StrategyType::ProsperityGrowth) {
        return QStringLiteral("%1；策略标的:%2；主线:%3；自动项:60日线%4、缩量回踩%5、回调窗口%6-%7；人工确认:利润%8、订单%9、剔除纯概念%10；单赛道≤%11%；分散%12条主线；短期涨幅≥%13%分批止盈。")
            .arg(currentStrategyName(),
                 config.symbol,
                 selectedGrowthTracks(),
                 config.growthBuyConfig.requireMA60Up ? QStringLiteral("启用") : QStringLiteral("关闭"),
                 config.growthBuyConfig.requireVolumePullback ? QStringLiteral("启用") : QStringLiteral("关闭"))
            .arg(config.growthBuyConfig.pullbackMinDays)
            .arg(config.growthBuyConfig.pullbackMaxDays)
            .arg(config.growthBuyConfig.profitGrowthConfirmed ? QStringLiteral("已确认") : QStringLiteral("未确认"),
                 config.growthBuyConfig.orderLandingConfirmed ? QStringLiteral("已确认") : QStringLiteral("未确认"),
                 config.growthBuyConfig.noPureConceptConfirmed ? QStringLiteral("已确认") : QStringLiteral("未确认"))
            .arg(config.positionConfig.maxSingleTrackPercent, 0, 'f', 1)
            .arg(config.positionConfig.diversifyTrackCount)
            .arg(config.riskConfig.surgeTakeProfitPercent, 0, 'f', 1);
    }

    return QStringLiteral("%1；策略标的:%2；快MA:%3 慢MA:%4 止损:%5% 止盈:%6% 单笔数量:%7。")
        .arg(currentStrategyName(), config.symbol)
        .arg(config.doubleMAConfig.fastMA)
        .arg(config.doubleMAConfig.slowMA)
        .arg(config.riskConfig.stopLossPercent, 0, 'f', 1)
        .arg(config.riskConfig.takeProfitPercent, 0, 'f', 1)
        .arg(config.positionConfig.lotSize, 0, 'f', 0);
}
void StrategyPanel::setRunningState(bool running)
{
    ui->startBtn->setEnabled(!running);
    ui->stopBtn->setEnabled(running);
    ui->symbolEdit->setEnabled(true);
    if (m_strategySymbolEdit) {
        m_strategySymbolEdit->setEnabled(!running);
    }
    ui->fastMASpin->setEnabled(!running);
    ui->slowMASpin->setEnabled(!running);
    ui->stopLossSpin->setEnabled(!running);
    ui->takeProfitSpin->setEnabled(!running);
    ui->lotSizeSpin->setEnabled(!running);
    if (m_strategyPresetCombo) {
        m_strategyPresetCombo->setEnabled(!running);
    }
    if (m_applyStrategyPresetBtn) {
        m_applyStrategyPresetBtn->setEnabled(!running);
    }
    setGrowthConfigEnabled(!running && strategyKindAt(m_strategyPresetCombo ? m_strategyPresetCombo->currentIndex() : 0) == StrategyKind::ProsperityGrowth);

    if (m_addFavoriteBtn) {
        m_addFavoriteBtn->setEnabled(true);
    }
    onFavoriteSelectionChanged();
}

QWidget* StrategyPanel::takeSymbolSelectorWidget(QWidget* parent)
{
    if (!m_symbolSelectorWidget) {
        m_symbolSelectorWidget = new QWidget(parent);
        m_symbolSelectorWidget->setObjectName(QStringLiteral("symbolSelectorWidget"));
        auto* selectorLayout = new QHBoxLayout(m_symbolSelectorWidget);
        selectorLayout->setContentsMargins(0, 0, 0, 0);
        selectorLayout->setSpacing(8);

        const QFormLayout::TakeRowResult symbolRow = ui->formLayout->takeRow(ui->symbolEdit);
        delete symbolRow.labelItem;
        delete symbolRow.fieldItem;

        ui->label->setParent(m_symbolSelectorWidget);
        ui->symbolEdit->setParent(m_symbolSelectorWidget);
        ui->label->setText(QStringLiteral("股票代码"));
        ui->symbolEdit->setMinimumWidth(170);

        m_symbolAddFavoriteBtn = new QPushButton(QStringLiteral("加入自选"), m_symbolSelectorWidget);
        m_symbolAddFavoriteBtn->setObjectName(QStringLiteral("favoriteAddBtn"));

        selectorLayout->addWidget(ui->label);
        selectorLayout->addWidget(ui->symbolEdit, 1);
        selectorLayout->addWidget(m_symbolAddFavoriteBtn);

        connect(m_symbolAddFavoriteBtn, &QPushButton::clicked, this, &StrategyPanel::onAddFavoriteClicked);
    }

    m_symbolSelectorWidget->setParent(parent);
    return m_symbolSelectorWidget;
}

QWidget* StrategyPanel::takeStrategyControlWidget(QWidget* parent)
{
    if (!m_strategyControlGroup) {
        m_strategyControlGroup = new QGroupBox(QStringLiteral("策略操作"), parent);
        auto* controlLayout = new QHBoxLayout(m_strategyControlGroup);
        controlLayout->setContentsMargins(10, 12, 10, 10);
        controlLayout->setSpacing(10);

        ui->horizontalLayout_2->removeWidget(ui->startBtn);
        ui->horizontalLayout_2->removeWidget(ui->stopBtn);
        ui->verticalLayout_4->removeItem(ui->horizontalLayout_2);

        ui->startBtn->setParent(m_strategyControlGroup);
        ui->stopBtn->setParent(m_strategyControlGroup);
        ui->startBtn->setMinimumWidth(120);
        ui->stopBtn->setMinimumWidth(120);
        controlLayout->addWidget(ui->startBtn);
        controlLayout->addWidget(ui->stopBtn);
    }

    m_strategyControlGroup->setParent(parent);
    return m_strategyControlGroup;
}

QWidget* StrategyPanel::takeTradeLogWidget(QWidget* parent)
{
    ui->verticalLayout_4->removeWidget(ui->groupBox_2);
    ui->groupBox_2->setParent(parent);
    ui->groupBox_2->setMinimumHeight(190);
    ui->logTextEdit->setMinimumHeight(160);
    return ui->groupBox_2;
}

QWidget* StrategyPanel::takeWatchlistWidget(QWidget* parent)
{
    if (!m_watchlistGroup) {
        return nullptr;
    }

    ui->verticalLayout_4->removeWidget(m_watchlistGroup);
    m_watchlistGroup->setParent(parent);
    m_watchlistGroup->setMinimumWidth(300);
    if (m_watchlistWidget) {
        m_watchlistWidget->setMinimumHeight(180);
    }
    return m_watchlistGroup;
}
void StrategyPanel::rememberStockName(const QString& symbol, const QString& name)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    if (!isValidMarketSymbol(normalized) || name.trimmed().isEmpty()) {
        return;
    }

    if (m_stockNames.value(normalized) == name.trimmed()) {
        return;
    }

    m_stockNames[normalized] = name.trimmed();
    if (MarketDataSimulator::normalizeSymbol(getViewSymbol()) == normalized) {
        saveViewSymbol();
    }
    if (MarketDataSimulator::normalizeSymbol(getStrategySymbol()) == normalized) {
        saveStrategySymbol();
    }

    if (m_favorites.contains(normalized)) {
        refreshWatchlist();
        saveWatchlist();
    }
}

void StrategyPanel::setupCommonStrategyUi()
{
    auto* presetGroup = new QGroupBox(QStringLiteral("当前策略"), this);
    auto* presetLayout = new QVBoxLayout(presetGroup);
    presetLayout->setContentsMargins(10, 12, 10, 10);
    presetLayout->setSpacing(8);

    auto* strategySymbolWidget = new QWidget(presetGroup);
    auto* strategySymbolLayout = new QHBoxLayout(strategySymbolWidget);
    strategySymbolLayout->setContentsMargins(0, 0, 0, 0);
    strategySymbolLayout->setSpacing(8);
    auto* strategySymbolLabel = new QLabel(QStringLiteral("策略标的"), strategySymbolWidget);
    m_strategySymbolEdit = new QLineEdit(strategySymbolWidget);
    m_strategySymbolEdit->setPlaceholderText(QStringLiteral("代码 / 名称 / 拼音"));
    m_strategySymbolEdit->setText(ui->symbolEdit->text());
    strategySymbolLayout->addWidget(strategySymbolLabel);
    strategySymbolLayout->addWidget(m_strategySymbolEdit, 1);
    presetLayout->addWidget(strategySymbolWidget);

    m_strategyPresetCombo = new QComboBox(presetGroup);
    const QVector<StrategyPreset> presets = commonStrategyPresets();
    for (const auto& preset : presets) {
        m_strategyPresetCombo->addItem(preset.name);
    }

    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    const int savedType = settings.value(QStringLiteral("strategy/currentType"), 0).toInt();
    if (savedType >= 0 && savedType < presets.size()) {
        m_strategyPresetCombo->setCurrentIndex(savedType);
    }

    m_strategyPresetDescLabel = new QLabel(presetGroup);
    m_strategyPresetDescLabel->setWordWrap(true);
    m_strategyPresetDescLabel->setMinimumHeight(42);

    m_strategyDetailButton = new QPushButton(QStringLiteral("策略详细说明"), presetGroup);
    m_strategyDetailButton->setMinimumWidth(140);
    m_strategyDetailButton->setToolTipDuration(60000);

    m_applyStrategyPresetBtn = new QPushButton(QStringLiteral("应用当前策略配置"), presetGroup);

    auto* descLayout = new QHBoxLayout();
    descLayout->setContentsMargins(0, 0, 0, 0);
    descLayout->setSpacing(8);
    descLayout->addWidget(m_strategyPresetDescLabel, 1);
    descLayout->addWidget(m_strategyDetailButton);

    presetLayout->addWidget(m_strategyPresetCombo);
    presetLayout->addLayout(descLayout);
    presetLayout->addWidget(m_applyStrategyPresetBtn);
    presetGroup->setMinimumHeight(190);
    presetGroup->setMaximumHeight(260);

    ui->verticalLayout_4->removeWidget(ui->groupBox);
    ui->verticalLayout_4->insertWidget(0, presetGroup);
    ui->verticalLayout_4->insertWidget(1, ui->groupBox);

    setupCurrentStrategyConfigUi();
    ui->verticalLayout_4->setStretch(0, 0);
    ui->verticalLayout_4->setStretch(1, 0);
    ui->verticalLayout_4->setStretch(2, 1);

    connect(m_strategyPresetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StrategyPanel::onStrategyPresetChanged);
    connect(m_applyStrategyPresetBtn, &QPushButton::clicked,
            this, &StrategyPanel::onApplyStrategyPresetClicked);

    onStrategyPresetChanged(m_strategyPresetCombo->currentIndex());
}
void StrategyPanel::setupCurrentStrategyConfigUi()
{
    m_strategyConfigGroup = new QGroupBox(QStringLiteral("当前策略配置"), this);
    auto* configLayout = new QVBoxLayout(m_strategyConfigGroup);
    configLayout->setContentsMargins(10, 12, 10, 10);
    configLayout->setSpacing(8);

    m_strategyConfigHintLabel = new QLabel(m_strategyConfigGroup);
    m_strategyConfigHintLabel->setWordWrap(true);
    configLayout->addWidget(m_strategyConfigHintLabel);

    auto* autoGroup = new QGroupBox(QStringLiteral("可自动判断"), m_strategyConfigGroup);
    auto* autoLayout = new QVBoxLayout(autoGroup);
    m_ma60UpCheck = new QCheckBox(QStringLiteral("启用：60 日均线持续向上"), autoGroup);
    m_volumePullbackCheck = new QCheckBox(QStringLiteral("启用：缩量回踩 20/60 日线"), autoGroup);
    m_ma60UpCheck->setChecked(true);
    m_volumePullbackCheck->setChecked(true);
    autoLayout->addWidget(m_ma60UpCheck);
    autoLayout->addWidget(m_volumePullbackCheck);
    auto* autoHint = new QLabel(QStringLiteral("自动诊断还包括：板块连续回调窗口、短期涨幅止盈线、放量跌破 60 日线。当前先使用实时行情近似，接入日线/板块数据后会升级为日线判断。"), autoGroup);
    autoHint->setWordWrap(true);
    autoLayout->addWidget(autoHint);
    configLayout->addWidget(autoGroup);
    m_growthConfigWidgets.append(autoGroup);

    auto* dataGroup = new QGroupBox(QStringLiteral("需要数据源 / 人工确认"), m_strategyConfigGroup);
    auto* dataLayout = new QVBoxLayout(dataGroup);
    m_profitGrowthCheck = new QCheckBox(QStringLiteral("人工确认：季度净利润增速为正"), dataGroup);
    m_orderLandingCheck = new QCheckBox(QStringLiteral("人工确认：有落地订单"), dataGroup);
    m_noPureConceptCheck = new QCheckBox(QStringLiteral("人工确认：剔除纯概念无业绩标的"), dataGroup);
    for (QCheckBox* check : {m_profitGrowthCheck, m_orderLandingCheck, m_noPureConceptCheck}) {
        check->setChecked(true);
        dataLayout->addWidget(check);
        connect(check, &QCheckBox::toggled, this, &StrategyPanel::onStrategyConfigChanged);
    }
    auto* dataHint = new QLabel(QStringLiteral("所属赛道识别、财报和公告语义当前尚未接入数据源；以上勾选代表用户已人工确认，后续可由财报/公告/行业分类自动校验。"), dataGroup);
    dataHint->setWordWrap(true);
    dataLayout->addWidget(dataHint);
    configLayout->addWidget(dataGroup);
    m_growthConfigWidgets.append(dataGroup);

    auto* userGroup = new QGroupBox(QStringLiteral("用户配置"), m_strategyConfigGroup);
    auto* userLayout = new QVBoxLayout(userGroup);

    auto* trackGroup = new QGroupBox(QStringLiteral("适配主线"), userGroup);
    auto* trackLayout = new QGridLayout(trackGroup);
    const QStringList tracks = {
        QStringLiteral("AI 算力"),
        QStringLiteral("半导体设备"),
        QStringLiteral("机器人零部件"),
        QStringLiteral("储能")
    };
    for (int i = 0; i < tracks.size(); ++i) {
        auto* check = new QCheckBox(tracks.at(i), trackGroup);
        check->setChecked(true);
        m_growthTrackChecks.append(check);
        trackLayout->addWidget(check, i / 2, i % 2);
        connect(check, &QCheckBox::toggled, this, &StrategyPanel::onStrategyConfigChanged);
    }
    userLayout->addWidget(trackGroup);

    auto* ruleGroup = new QGroupBox(QStringLiteral("买点 / 仓位 / 风控"), userGroup);
    auto* ruleLayout = new QFormLayout(ruleGroup);

    m_pullbackMinSpin = new QSpinBox(ruleGroup);
    m_pullbackMinSpin->setRange(1, 10);
    m_pullbackMinSpin->setValue(3);
    m_pullbackMaxSpin = new QSpinBox(ruleGroup);
    m_pullbackMaxSpin->setRange(1, 10);
    m_pullbackMaxSpin->setValue(5);
    auto* pullbackLayout = new QHBoxLayout();
    pullbackLayout->addWidget(m_pullbackMinSpin);
    pullbackLayout->addWidget(new QLabel(QStringLiteral("至"), ruleGroup));
    pullbackLayout->addWidget(m_pullbackMaxSpin);
    pullbackLayout->addWidget(new QLabel(QStringLiteral("天"), ruleGroup));
    ruleLayout->addRow(QStringLiteral("板块连续回调窗口"), pullbackLayout);

    m_sectorCapSpin = new QDoubleSpinBox(ruleGroup);
    m_sectorCapSpin->setRange(5.0, 50.0);
    m_sectorCapSpin->setDecimals(1);
    m_sectorCapSpin->setSingleStep(1.0);
    m_sectorCapSpin->setValue(20.0);
    m_sectorCapSpin->setSuffix(QStringLiteral("%"));
    ruleLayout->addRow(QStringLiteral("单赛道上限"), m_sectorCapSpin);

    m_diversifyMainlineSpin = new QSpinBox(ruleGroup);
    m_diversifyMainlineSpin->setRange(1, 6);
    m_diversifyMainlineSpin->setValue(3);
    ruleLayout->addRow(QStringLiteral("分散主线数量"), m_diversifyMainlineSpin);

    m_surgeTakeProfitSpin = new QDoubleSpinBox(ruleGroup);
    m_surgeTakeProfitSpin->setRange(5.0, 80.0);
    m_surgeTakeProfitSpin->setDecimals(1);
    m_surgeTakeProfitSpin->setSingleStep(1.0);
    m_surgeTakeProfitSpin->setValue(25.0);
    m_surgeTakeProfitSpin->setSuffix(QStringLiteral("%"));
    ruleLayout->addRow(QStringLiteral("短期暴涨止盈线"), m_surgeTakeProfitSpin);

    m_partialTakeProfitCheck = new QCheckBox(QStringLiteral("达到止盈线分批减仓，不一次性清仓"), ruleGroup);
    m_partialTakeProfitCheck->setChecked(true);
    ruleLayout->addRow(QStringLiteral("止盈方式"), m_partialTakeProfitCheck);

    m_breakMA60VolumeStopCheck = new QCheckBox(QStringLiteral("有效跌破 60 日线且放量破位离场"), ruleGroup);
    m_breakMA60VolumeStopCheck->setChecked(true);
    ruleLayout->addRow(QStringLiteral("止损"), m_breakMA60VolumeStopCheck);

    userLayout->addWidget(ruleGroup);
    configLayout->addWidget(userGroup);
    m_growthConfigWidgets.append(userGroup);
    ui->verticalLayout_4->insertWidget(2, m_strategyConfigGroup);
    ui->verticalLayout_4->setStretch(2, 1);

    connect(m_ma60UpCheck, &QCheckBox::toggled, this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_volumePullbackCheck, &QCheckBox::toggled, this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_pullbackMinSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_pullbackMaxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_sectorCapSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_diversifyMainlineSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_surgeTakeProfitSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_partialTakeProfitCheck, &QCheckBox::toggled, this, &StrategyPanel::onStrategyConfigChanged);
    connect(m_breakMA60VolumeStopCheck, &QCheckBox::toggled, this, &StrategyPanel::onStrategyConfigChanged);
}
void StrategyPanel::updateCurrentStrategyConfigUi()
{
    const StrategyKind kind = strategyKindAt(m_strategyPresetCombo ? m_strategyPresetCombo->currentIndex() : 0);
    const bool growth = kind == StrategyKind::ProsperityGrowth;

    if (m_strategyConfigHintLabel) {
        m_strategyConfigHintLabel->setText(growth
            ? QStringLiteral("景气成长策略分为三类：可自动判断、需要数据源/人工确认、用户配置。当前模拟交易使用真实行情，财报、公告和赛道识别先由人工确认。")
            : QStringLiteral("双均线策略使用统一策略配置中的快 MA、慢 MA、止损、止盈和单笔数量，可直接进行实时模拟交易。"));
    }

    if (m_strategyConfigGroup) {
        m_strategyConfigGroup->setTitle(growth
            ? QStringLiteral("当前策略配置：景气成长分批低吸")
            : QStringLiteral("当前策略配置：双均线"));
    }

    for (QWidget* widget : m_growthConfigWidgets) {
        if (widget) {
            widget->setVisible(growth);
        }
    }

    setGrowthConfigEnabled(growth && (!m_strategyPresetCombo || m_strategyPresetCombo->isEnabled()));
}

void StrategyPanel::setGrowthConfigEnabled(bool enabled)
{
    for (QCheckBox* check : m_growthTrackChecks) {
        check->setEnabled(enabled);
    }
    for (QCheckBox* check : {m_ma60UpCheck, m_profitGrowthCheck, m_orderLandingCheck, m_noPureConceptCheck,
                             m_volumePullbackCheck, m_partialTakeProfitCheck, m_breakMA60VolumeStopCheck}) {
        if (check) {
            check->setEnabled(enabled);
        }
    }
    for (QWidget* widget : {static_cast<QWidget*>(m_pullbackMinSpin), static_cast<QWidget*>(m_pullbackMaxSpin),
                            static_cast<QWidget*>(m_sectorCapSpin), static_cast<QWidget*>(m_diversifyMainlineSpin),
                            static_cast<QWidget*>(m_surgeTakeProfitSpin)}) {
        if (widget) {
            widget->setEnabled(enabled);
        }
    }
}

QString StrategyPanel::selectedGrowthTracks() const
{
    QStringList tracks;
    for (QCheckBox* check : m_growthTrackChecks) {
        if (check && check->isChecked()) {
            tracks.append(check->text());
        }
    }
    return tracks.isEmpty() ? QStringLiteral("未选择") : tracks.join(QStringLiteral("、"));
}
void StrategyPanel::setupStockSearchUi()
{
    ui->symbolEdit->setPlaceholderText(QStringLiteral("代码 / 名称 / 拼音"));

    m_searchModel = new QStringListModel(this);
    m_symbolCompleter = new QCompleter(m_searchModel, this);
    m_symbolCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_symbolCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_symbolCompleter->setFilterMode(Qt::MatchContains);
    ui->symbolEdit->setCompleter(m_symbolCompleter);
    connect(m_symbolCompleter, QOverload<const QString&>::of(&QCompleter::activated),
            this, &StrategyPanel::onCompleterActivated);

    m_strategySearchModel = new QStringListModel(this);
    m_strategySymbolCompleter = new QCompleter(m_strategySearchModel, this);
    m_strategySymbolCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_strategySymbolCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_strategySymbolCompleter->setFilterMode(Qt::MatchContains);
    if (m_strategySymbolEdit) {
        m_strategySymbolEdit->setCompleter(m_strategySymbolCompleter);
        connect(m_strategySymbolEdit, &QLineEdit::textChanged, this, &StrategyPanel::onStrategySymbolTextChanged);
        connect(m_strategySymbolEdit, &QLineEdit::editingFinished, this, &StrategyPanel::onStrategySymbolEditingFinished);
    }
    connect(m_strategySymbolCompleter, QOverload<const QString&>::of(&QCompleter::activated),
            this, &StrategyPanel::onStrategyCompleterActivated);

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(SearchDelayMs);
    connect(m_searchTimer, &QTimer::timeout, this, &StrategyPanel::onSearchTimerTimeout);

    m_searchNetwork = new QNetworkAccessManager(this);

    m_watchlistGroup = new QGroupBox(QStringLiteral("自选股"), this);
    auto favoriteLayout = new QVBoxLayout(m_watchlistGroup);
    favoriteLayout->setContentsMargins(10, 12, 10, 10);
    favoriteLayout->setSpacing(8);

    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_addFavoriteBtn = new QPushButton(QStringLiteral("加入自选"), m_watchlistGroup);
    m_addFavoriteBtn->setObjectName(QStringLiteral("favoriteAddBtn"));
    m_removeFavoriteBtn = new QPushButton(QStringLiteral("删除"), m_watchlistGroup);
    m_removeFavoriteBtn->setObjectName(QStringLiteral("favoriteRemoveBtn"));
    buttonLayout->addWidget(m_addFavoriteBtn);
    buttonLayout->addWidget(m_removeFavoriteBtn);
    favoriteLayout->addLayout(buttonLayout);

    m_watchlistWidget = new QListWidget(m_watchlistGroup);
    m_watchlistWidget->setMinimumHeight(86);
    m_watchlistWidget->setAlternatingRowColors(false);
    favoriteLayout->addWidget(m_watchlistWidget);

    ui->verticalLayout_4->insertWidget(3, m_watchlistGroup);

    connect(m_addFavoriteBtn, &QPushButton::clicked, this, &StrategyPanel::onAddFavoriteClicked);
    connect(m_removeFavoriteBtn, &QPushButton::clicked, this, &StrategyPanel::onRemoveFavoriteClicked);
    connect(m_watchlistWidget, &QListWidget::itemClicked, this, &StrategyPanel::onFavoriteActivated);
    connect(m_watchlistWidget, &QListWidget::itemDoubleClicked, this, &StrategyPanel::onFavoriteActivated);
    connect(m_watchlistWidget, &QListWidget::itemSelectionChanged, this, &StrategyPanel::onFavoriteSelectionChanged);
}
void StrategyPanel::loadWatchlist()
{
    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    const QStringList entries = settings.value(QStringLiteral("watchlist/items")).toStringList();

    m_favorites.clear();
    for (const QString& entry : entries) {
        const QStringList parts = entry.split(QLatin1Char('|'));
        const QString symbol = MarketDataSimulator::normalizeSymbol(parts.value(0));
        if (!isValidMarketSymbol(symbol) || m_favorites.contains(symbol)) {
            continue;
        }

        const QString name = parts.value(1).trimmed();
        if (!name.isEmpty()) {
            m_stockNames[symbol] = name;
        }
        m_favorites.append(symbol);
    }

    refreshWatchlist();
}

void StrategyPanel::loadViewSymbol()
{
    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    const QString symbol = MarketDataSimulator::normalizeSymbol(
        settings.value(QStringLiteral("market/viewSymbol"),
                       settings.value(QStringLiteral("market/currentSymbol"), ui->symbolEdit->text())).toString());
    const QString name = settings.value(QStringLiteral("market/viewName"),
                                        settings.value(QStringLiteral("market/currentName"))).toString().trimmed();

    if (!isValidMarketSymbol(symbol)) {
        return;
    }

    if (!name.isEmpty()) {
        m_stockNames[symbol] = name;
    }

    selectSymbol(symbol, name, false);
}

void StrategyPanel::saveViewSymbol() const
{
    const QString symbol = MarketDataSimulator::normalizeSymbol(ui->symbolEdit->text());
    if (!isValidMarketSymbol(symbol)) {
        return;
    }

    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    settings.setValue(QStringLiteral("market/viewSymbol"), symbol);
    settings.setValue(QStringLiteral("market/viewName"), stockNameForSymbol(symbol));
    settings.setValue(QStringLiteral("market/currentSymbol"), symbol);
    settings.setValue(QStringLiteral("market/currentName"), stockNameForSymbol(symbol));
    settings.sync();
}

void StrategyPanel::loadStrategySymbol()
{
    if (!m_strategySymbolEdit) {
        return;
    }

    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    const QString symbol = MarketDataSimulator::normalizeSymbol(
        settings.value(QStringLiteral("strategy/currentSymbol"), getViewSymbol()).toString());
    const QString name = settings.value(QStringLiteral("strategy/currentName")).toString().trimmed();

    if (!isValidMarketSymbol(symbol)) {
        return;
    }

    if (!name.isEmpty()) {
        m_stockNames[symbol] = name;
    }

    selectStrategySymbol(symbol, name, false);
}

void StrategyPanel::saveStrategySymbol() const
{
    if (!m_strategySymbolEdit) {
        return;
    }

    const QString symbol = MarketDataSimulator::normalizeSymbol(m_strategySymbolEdit->text());
    if (!isValidMarketSymbol(symbol)) {
        return;
    }

    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    settings.setValue(QStringLiteral("strategy/currentSymbol"), symbol);
    settings.setValue(QStringLiteral("strategy/currentName"), stockNameForSymbol(symbol));
    settings.sync();
}

void StrategyPanel::saveCurrentStrategyType() const
{
    if (!m_strategyPresetCombo) {
        return;
    }

    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    settings.setValue(QStringLiteral("strategy/currentType"), m_strategyPresetCombo->currentIndex());
    settings.sync();
}
void StrategyPanel::saveWatchlist() const
{
    QStringList entries;
    for (const QString& symbol : m_favorites) {
        entries.append(symbol + QLatin1Char('|') + m_stockNames.value(symbol));
    }

    QSettings settings(QStringLiteral("StarQuant"), QStringLiteral("StarQuant"));
    settings.setValue(QStringLiteral("watchlist/items"), entries);
}

void StrategyPanel::refreshWatchlist()
{
    if (!m_watchlistWidget) {
        return;
    }

    m_watchlistWidget->clear();
    for (const QString& symbol : m_favorites) {
        const QString name = stockNameForSymbol(symbol);
        auto* item = new QListWidgetItem(name.isEmpty()
            ? symbol
            : QStringLiteral("%1  %2").arg(name, symbol));
        item->setData(Qt::UserRole, symbol);
        m_watchlistWidget->addItem(item);
    }

    onFavoriteSelectionChanged();
}

void StrategyPanel::selectSymbol(const QString& symbol, const QString& name, bool emitChange)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    if (!isValidMarketSymbol(normalized)) {
        return;
    }

    if (!name.trimmed().isEmpty()) {
        m_stockNames[normalized] = name.trimmed();
    }

    m_updatingSymbol = true;
    ui->symbolEdit->setText(normalized);
    updateSearchSuggestions(normalized);
    m_updatingSymbol = false;
    saveViewSymbol();

    if (emitChange) {
        emit viewSymbolChanged(normalized);
    }
}

void StrategyPanel::selectStrategySymbol(const QString& symbol, const QString& name, bool emitChange)
{
    if (!m_strategySymbolEdit) {
        return;
    }

    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    if (!isValidMarketSymbol(normalized)) {
        return;
    }

    if (!name.trimmed().isEmpty()) {
        m_stockNames[normalized] = name.trimmed();
    }

    m_updatingSymbol = true;
    m_strategySymbolEdit->setText(normalized);
    updateSearchSuggestions(normalized);
    m_updatingSymbol = false;
    saveStrategySymbol();

    if (emitChange) {
        emit parametersChanged();
    }
}
void StrategyPanel::addFavoriteSymbol(const QString& symbol, const QString& name)
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    if (!isValidMarketSymbol(normalized)) {
        addSystemLog(QStringLiteral("请选择搜索结果或输入 6 位股票代码。"));
        return;
    }

    if (!name.trimmed().isEmpty()) {
        m_stockNames[normalized] = name.trimmed();
    }

    if (!m_favorites.contains(normalized)) {
        m_favorites.append(normalized);
        saveWatchlist();
        refreshWatchlist();
    }

    selectSymbol(normalized, stockNameForSymbol(normalized), true);
}

QString StrategyPanel::resolveSymbolText(const QString& text) const
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    if (m_completionSymbols.contains(trimmed)) {
        return m_completionSymbols.value(trimmed);
    }

    const QRegularExpression symbolPattern(QStringLiteral(R"((\d{6})(?:\.(SH|SZ))?)"),
                                           QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = symbolPattern.match(trimmed);
    if (match.hasMatch()) {
        const QString code = match.captured(1);
        const QString suffix = match.captured(2).toUpper();
        if (!suffix.isEmpty()) {
            return MarketDataSimulator::normalizeSymbol(code + QLatin1Char('.') + suffix);
        }
        return MarketDataSimulator::normalizeSymbol(code);
    }

    for (auto it = m_stockNames.constBegin(); it != m_stockNames.constEnd(); ++it) {
        if (it.value() == trimmed || it.value().contains(trimmed, Qt::CaseInsensitive)) {
            return it.key();
        }
    }

    const QVector<StockCandidate> localMatches = searchLocalStocks(trimmed);
    if (!localMatches.isEmpty()) {
        return localMatches.first().symbol;
    }

    const QString normalized = MarketDataSimulator::normalizeSymbol(trimmed);
    return isValidMarketSymbol(normalized) ? normalized : QString();
}

QString StrategyPanel::stockNameForSymbol(const QString& symbol) const
{
    const QString normalized = MarketDataSimulator::normalizeSymbol(symbol);
    if (m_stockNames.contains(normalized)) {
        return m_stockNames.value(normalized);
    }

    for (const auto& candidate : localStockCatalog()) {
        if (candidate.symbol == normalized) {
            return candidate.name;
        }
    }

    return QString();
}

QVector<StockCandidate> StrategyPanel::localStockCatalog() const
{
    return QVector<StockCandidate>{
        {QStringLiteral("000001.SH"), QStringLiteral("上证指数")},
        {QStringLiteral("399001.SZ"), QStringLiteral("深证成指")},
        {QStringLiteral("399006.SZ"), QStringLiteral("创业板指")},
        {QStringLiteral("000001.SZ"), QStringLiteral("平安银行")},
        {QStringLiteral("002156.SZ"), QStringLiteral("通富微电")},
        {QStringLiteral("600519.SH"), QStringLiteral("贵州茅台")},
        {QStringLiteral("300750.SZ"), QStringLiteral("宁德时代")},
        {QStringLiteral("000858.SZ"), QStringLiteral("五粮液")},
        {QStringLiteral("601398.SH"), QStringLiteral("工商银行")},
        {QStringLiteral("600036.SH"), QStringLiteral("招商银行")},
        {QStringLiteral("601318.SH"), QStringLiteral("中国平安")},
        {QStringLiteral("002594.SZ"), QStringLiteral("比亚迪")},
        {QStringLiteral("600900.SH"), QStringLiteral("长江电力")},
        {QStringLiteral("600276.SH"), QStringLiteral("恒瑞医药")},
        {QStringLiteral("000333.SZ"), QStringLiteral("美的集团")},
        {QStringLiteral("002415.SZ"), QStringLiteral("海康威视")},
        {QStringLiteral("601012.SH"), QStringLiteral("隆基绿能")},
        {QStringLiteral("600030.SH"), QStringLiteral("中信证券")},
        {QStringLiteral("300059.SZ"), QStringLiteral("东方财富")},
        {QStringLiteral("002475.SZ"), QStringLiteral("立讯精密")},
        {QStringLiteral("603259.SH"), QStringLiteral("药明康德")}
    };
}

QVector<StockCandidate> StrategyPanel::searchLocalStocks(const QString& keyword) const
{
    QVector<StockCandidate> results;
    const QString key = keyword.trimmed();
    if (key.isEmpty()) {
        return results;
    }

    const QString upperKey = key.toUpper();
    for (const auto& candidate : localStockCatalog()) {
        const QString compactSymbol = candidate.symbol.left(6);
        if (candidate.symbol.contains(upperKey, Qt::CaseInsensitive)
            || compactSymbol.contains(upperKey, Qt::CaseInsensitive)
            || candidate.name.contains(key, Qt::CaseInsensitive)) {
            appendUniqueCandidate(&results, candidate);
        }
    }

    for (auto it = m_stockNames.constBegin(); it != m_stockNames.constEnd(); ++it) {
        if (it.key().contains(upperKey, Qt::CaseInsensitive)
            || it.key().left(6).contains(upperKey, Qt::CaseInsensitive)
            || it.value().contains(key, Qt::CaseInsensitive)) {
            appendUniqueCandidate(&results, StockCandidate{it.key(), it.value()});
        }
    }

    return results;
}

QVector<StockCandidate> StrategyPanel::parseSearchResponse(const QByteArray& payload) const
{
    QVector<StockCandidate> results;
    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || document.isNull()) {
        return results;
    }

    QVector<QJsonObject> objects;
    collectSearchObjects(document.isArray() ? QJsonValue(document.array()) : QJsonValue(document.object()), &objects);

    for (const QJsonObject& object : objects) {
        const QString code = object.value(QStringLiteral("Code")).toString(object.value(QStringLiteral("code")).toString()).trimmed();
        const QString name = object.value(QStringLiteral("Name")).toString(object.value(QStringLiteral("name")).toString()).trimmed();
        const QString quoteId = object.value(QStringLiteral("QuoteID")).toString(object.value(QStringLiteral("quoteId")).toString()).trimmed();
        const QString symbol = symbolFromQuoteId(quoteId, code);
        if (!isValidMarketSymbol(symbol) || name.isEmpty()) {
            continue;
        }

        appendUniqueCandidate(&results, StockCandidate{symbol, name});
        if (results.size() >= 12) {
            break;
        }
    }

    return results;
}

void StrategyPanel::updateSearchSuggestions(const QString& keyword, const QVector<StockCandidate>& remoteCandidates)
{
    QVector<StockCandidate> candidates = searchLocalStocks(keyword);

    const QString resolved = resolveSymbolText(keyword);
    if (isValidMarketSymbol(resolved)) {
        appendUniqueCandidate(&candidates, StockCandidate{resolved, stockNameForSymbol(resolved)});
    }

    for (const auto& candidate : remoteCandidates) {
        appendUniqueCandidate(&candidates, candidate);
    }

    showSearchCandidates(candidates);
}

void StrategyPanel::showSearchCandidates(const QVector<StockCandidate>& candidates)
{
    QStringList displays;
    m_completionSymbols.clear();

    for (const auto& candidate : candidates) {
        if (!isValidMarketSymbol(candidate.symbol)) {
            continue;
        }

        const QString display = displayTextForCandidate(candidate);
        displays.append(display);
        m_completionSymbols[display] = candidate.symbol;
        m_completionSymbols[candidate.symbol] = candidate.symbol;
        if (!candidate.name.isEmpty()) {
            m_stockNames[candidate.symbol] = candidate.name;
        }
    }

    m_searchModel->setStringList(displays);
    if (m_strategySearchModel) {
        m_strategySearchModel->setStringList(displays);
    }

    if (!m_updatingSymbol && !displays.isEmpty()) {
        if (ui->symbolEdit->hasFocus()) {
            m_symbolCompleter->complete();
        } else if (m_strategySymbolEdit && m_strategySymbolEdit->hasFocus() && m_strategySymbolCompleter) {
            m_strategySymbolCompleter->complete();
        }
    }
}

void StrategyPanel::abortSearchReply()
{
    if (!m_searchReply) {
        return;
    }

    disconnect(m_searchReply, nullptr, this, nullptr);
    m_searchReply->abort();
    m_searchReply->deleteLater();
    m_searchReply = nullptr;
}

bool StrategyPanel::isValidMarketSymbol(const QString& symbol) const
{
    static const QRegularExpression pattern(QStringLiteral(R"(^\d{6}\.(SH|SZ)$)"));
    return pattern.match(symbol).hasMatch();
}

bool StrategyPanel::containsChinese(const QString& text) const
{
    for (const QChar ch : text) {
        const ushort unicode = ch.unicode();
        if (unicode >= 0x4E00 && unicode <= 0x9FFF) {
            return true;
        }
    }
    return false;
}

QString StrategyPanel::displayTextForCandidate(const StockCandidate& candidate)
{
    return candidate.name.isEmpty()
        ? candidate.symbol
        : QStringLiteral("%1  %2").arg(candidate.name, candidate.symbol);
}

QString StrategyPanel::symbolFromQuoteId(const QString& quoteId, const QString& code)
{
    const QString trimmedCode = code.trimmed();
    if (trimmedCode.size() != 6) {
        return QString();
    }

    const QString trimmedQuoteId = quoteId.trimmed();
    if (trimmedQuoteId.startsWith(QStringLiteral("1."))) {
        return trimmedCode + QStringLiteral(".SH");
    }
    if (trimmedQuoteId.startsWith(QStringLiteral("0."))) {
        return trimmedCode + QStringLiteral(".SZ");
    }

    return MarketDataSimulator::normalizeSymbol(trimmedCode);
}

void StrategyPanel::addSystemLog(const QString &message)
{
    const QString logLine = QStringLiteral("%1 [系统] %2")
            .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), message);

    ui->logTextEdit->append(logLine);

    QScrollBar *scrollBar = ui->logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void StrategyPanel::addSignalLog(const StrategySignal &signal)
{
    QString timeStr = signal.timestamp.toString("HH:mm:ss");
    QString typeStr;

    switch (signal.type) {
    case SignalType::BUY:
        typeStr = QStringLiteral("[买入]");
        break;
    case SignalType::SELL:
        typeStr = QStringLiteral("[卖出]");
        break;
    case SignalType::STOP_LOSS:
        typeStr = QStringLiteral("[止损]");
        break;
    case SignalType::TAKE_PROFIT:
        typeStr = QStringLiteral("[止盈]");
        break;
    default:
        typeStr = QStringLiteral("[未知]");
    }

    QString logLine = QString("%1 %2 价格: %3 数量: %4 %5\n")
            .arg(timeStr)
            .arg(typeStr)
            .arg(signal.price, 0, 'f', 2)
            .arg(signal.volume, 0, 'f', 0)
            .arg(signal.comment);

    ui->logTextEdit->append(logLine);

    QScrollBar *scrollBar = ui->logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void StrategyPanel::on_paramChanged()
{
    emit parametersChanged();
}

void StrategyPanel::onSymbolTextChanged(const QString& text)
{
    if (m_updatingSymbol) {
        return;
    }

    m_activeSearchEdit = ui->symbolEdit;
    updateSearchSuggestions(text);

    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    if (trimmed.size() >= 2 || containsChinese(trimmed)) {
        m_searchTimer->start();
    }
}

void StrategyPanel::onSymbolEditingFinished()
{
    const QString symbol = resolveSymbolText(ui->symbolEdit->text());
    if (isValidMarketSymbol(symbol)) {
        selectSymbol(symbol, stockNameForSymbol(symbol), true);
    }
}

void StrategyPanel::onStrategySymbolTextChanged(const QString& text)
{
    if (m_updatingSymbol) {
        return;
    }

    m_activeSearchEdit = m_strategySymbolEdit;
    updateSearchSuggestions(text);

    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    if (trimmed.size() >= 2 || containsChinese(trimmed)) {
        m_searchTimer->start();
    }
}

void StrategyPanel::onStrategySymbolEditingFinished()
{
    if (!m_strategySymbolEdit) {
        return;
    }

    const QString symbol = resolveSymbolText(m_strategySymbolEdit->text());
    if (isValidMarketSymbol(symbol)) {
        selectStrategySymbol(symbol, stockNameForSymbol(symbol), true);
    }
}
void StrategyPanel::onSearchTimerTimeout()
{
    QLineEdit* sourceEdit = m_activeSearchEdit ? m_activeSearchEdit : ui->symbolEdit;
    const QString keyword = sourceEdit->text().trimmed();
    if (keyword.size() < 2 && !containsChinese(keyword)) {
        return;
    }

    abortSearchReply();

    QUrl url(QString::fromLatin1(SearchEndpoint));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("input"), keyword);
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("14"));
    query.addQueryItem(QStringLiteral("token"), QString::fromLatin1(SearchToken));
    query.addQueryItem(QStringLiteral("count"), QStringLiteral("12"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 StarQuant/0.1 MarketSearch"));
    request.setRawHeader("Referer", "https://quote.eastmoney.com/");
    request.setTransferTimeout(SearchTimeoutMs);

    m_searchReply = m_searchNetwork->get(request);
    connect(m_searchReply, &QNetworkReply::finished, this, &StrategyPanel::onSearchReplyFinished);
}

void StrategyPanel::onSearchReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply == m_searchReply) {
        m_searchReply = nullptr;
    }

    const QByteArray payload = reply->readAll();
    const QNetworkReply::NetworkError error = reply->error();
    reply->deleteLater();

    if (error != QNetworkReply::NoError) {
        return;
    }

    QLineEdit* sourceEdit = m_activeSearchEdit ? m_activeSearchEdit : ui->symbolEdit;
    updateSearchSuggestions(sourceEdit->text(), parseSearchResponse(payload));
}

void StrategyPanel::onCompleterActivated(const QString& text)
{
    const QString symbol = m_completionSymbols.value(text, resolveSymbolText(text));
    if (!isValidMarketSymbol(symbol)) {
        return;
    }

    selectSymbol(symbol, stockNameForSymbol(symbol), true);
}

void StrategyPanel::onStrategyCompleterActivated(const QString& text)
{
    const QString symbol = m_completionSymbols.value(text, resolveSymbolText(text));
    if (!isValidMarketSymbol(symbol)) {
        return;
    }

    selectStrategySymbol(symbol, stockNameForSymbol(symbol), true);
}

void StrategyPanel::onStrategyPresetChanged(int index)
{
    const QVector<StrategyPreset> presets = commonStrategyPresets();
    if (!m_strategyPresetDescLabel || index < 0 || index >= presets.size()) {
        return;
    }

    const StrategyPreset& preset = presets[index];
    m_strategyPresetDescLabel->setText(
        QStringLiteral("%1  快MA:%2 慢MA:%3 止损:%4% 止盈:%5%")
            .arg(preset.description)
            .arg(preset.fastMA)
            .arg(preset.slowMA)
            .arg(preset.stopLoss, 0, 'f', 1)
            .arg(preset.takeProfit, 0, 'f', 1));
    if (m_strategyDetailButton) {
        m_strategyDetailButton->setToolTip(strategyDetailTooltip(preset.kind));
    }

    updateCurrentStrategyConfigUi();
    saveCurrentStrategyType();
    emit parametersChanged();
}

void StrategyPanel::onApplyStrategyPresetClicked()
{
    const QVector<StrategyPreset> presets = commonStrategyPresets();
    const int index = m_strategyPresetCombo ? m_strategyPresetCombo->currentIndex() : -1;
    if (index < 0 || index >= presets.size()) {
        return;
    }

    const StrategyPreset& preset = presets[index];
    ui->fastMASpin->setValue(preset.fastMA);
    ui->slowMASpin->setValue(preset.slowMA);
    ui->stopLossSpin->setValue(preset.stopLoss);
    ui->takeProfitSpin->setValue(preset.takeProfit);
    ui->lotSizeSpin->setValue(preset.lotSize);

    if (preset.kind == StrategyKind::ProsperityGrowth) {
        if (m_pullbackMinSpin) m_pullbackMinSpin->setValue(3);
        if (m_pullbackMaxSpin) m_pullbackMaxSpin->setValue(5);
        if (m_sectorCapSpin) m_sectorCapSpin->setValue(20.0);
        if (m_diversifyMainlineSpin) m_diversifyMainlineSpin->setValue(3);
        if (m_surgeTakeProfitSpin) m_surgeTakeProfitSpin->setValue(25.0);
    }

    updateCurrentStrategyConfigUi();
    addSystemLog(QStringLiteral("已应用策略：%1").arg(preset.name));
    addSystemLog(currentStrategyConfigurationSummary());
    emit parametersChanged();
}

void StrategyPanel::onStrategyConfigChanged()
{
    if (m_pullbackMinSpin && m_pullbackMaxSpin && m_pullbackMinSpin->value() > m_pullbackMaxSpin->value()) {
        m_pullbackMaxSpin->setValue(m_pullbackMinSpin->value());
    }
    emit parametersChanged();
}
void StrategyPanel::onAddFavoriteClicked()
{
    const QString symbol = resolveSymbolText(ui->symbolEdit->text());
    addFavoriteSymbol(symbol, stockNameForSymbol(symbol));
}

void StrategyPanel::onRemoveFavoriteClicked()
{
    if (!m_watchlistWidget || !m_watchlistWidget->currentItem()) {
        return;
    }

    const QString symbol = m_watchlistWidget->currentItem()->data(Qt::UserRole).toString();
    m_favorites.removeAll(symbol);
    saveWatchlist();
    refreshWatchlist();
}

void StrategyPanel::onFavoriteActivated(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    const QString symbol = item->data(Qt::UserRole).toString();
    selectSymbol(symbol, stockNameForSymbol(symbol), true);
}

void StrategyPanel::onFavoriteSelectionChanged()
{
    if (m_removeFavoriteBtn) {
        m_removeFavoriteBtn->setEnabled(m_watchlistWidget && m_watchlistWidget->currentItem());
    }
}
