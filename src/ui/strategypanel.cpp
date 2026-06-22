#include "strategypanel.h"
#include "ui_strategypanel.h"
#include "../market/marketdata.h"

#include <QCompleter>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
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
}

StrategyPanel::StrategyPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StrategyPanel),
    m_symbolCompleter(nullptr),
    m_searchModel(nullptr),
    m_searchTimer(nullptr),
    m_searchNetwork(nullptr),
    m_searchReply(nullptr),
    m_watchlistWidget(nullptr),
    m_addFavoriteBtn(nullptr),
    m_removeFavoriteBtn(nullptr),
    m_updatingSymbol(false)
{
    ui->setupUi(this);
    ui->logTextEdit->setReadOnly(true);

    ui->label->setBuddy(nullptr);
    ui->label_2->setBuddy(nullptr);
    ui->label_3->setBuddy(nullptr);
    ui->label_4->setBuddy(nullptr);
    ui->label_5->setBuddy(nullptr);
    ui->label_6->setBuddy(nullptr);

    setupStockSearchUi();
    loadWatchlist();
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
    const QString resolved = resolveSymbolText(ui->symbolEdit->text());
    return resolved.isEmpty() ? ui->symbolEdit->text().trimmed() : resolved;
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

void StrategyPanel::setRunningState(bool running)
{
    ui->startBtn->setEnabled(!running);
    ui->stopBtn->setEnabled(running);
    ui->symbolEdit->setEnabled(true);
    ui->fastMASpin->setEnabled(!running);
    ui->slowMASpin->setEnabled(!running);
    ui->stopLossSpin->setEnabled(!running);
    ui->takeProfitSpin->setEnabled(!running);
    ui->lotSizeSpin->setEnabled(!running);

    if (m_addFavoriteBtn) {
        m_addFavoriteBtn->setEnabled(true);
    }
    onFavoriteSelectionChanged();
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
    if (m_favorites.contains(normalized)) {
        refreshWatchlist();
        saveWatchlist();
    }
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

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(SearchDelayMs);
    connect(m_searchTimer, &QTimer::timeout, this, &StrategyPanel::onSearchTimerTimeout);

    m_searchNetwork = new QNetworkAccessManager(this);

    auto favoriteGroup = new QGroupBox(QStringLiteral("自选股"), this);
    auto favoriteLayout = new QVBoxLayout(favoriteGroup);
    favoriteLayout->setContentsMargins(10, 12, 10, 10);
    favoriteLayout->setSpacing(8);

    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_addFavoriteBtn = new QPushButton(QStringLiteral("加入自选"), favoriteGroup);
    m_removeFavoriteBtn = new QPushButton(QStringLiteral("删除"), favoriteGroup);
    buttonLayout->addWidget(m_addFavoriteBtn);
    buttonLayout->addWidget(m_removeFavoriteBtn);
    favoriteLayout->addLayout(buttonLayout);

    m_watchlistWidget = new QListWidget(favoriteGroup);
    m_watchlistWidget->setMinimumHeight(86);
    m_watchlistWidget->setAlternatingRowColors(false);
    favoriteLayout->addWidget(m_watchlistWidget);

    ui->verticalLayout_4->insertWidget(1, favoriteGroup);

    connect(m_addFavoriteBtn, &QPushButton::clicked, this, &StrategyPanel::onAddFavoriteClicked);
    connect(m_removeFavoriteBtn, &QPushButton::clicked, this, &StrategyPanel::onRemoveFavoriteClicked);
    connect(m_watchlistWidget, &QListWidget::itemClicked, this, &StrategyPanel::onFavoriteActivated);
    connect(m_watchlistWidget, &QListWidget::itemDoubleClicked, this, &StrategyPanel::onFavoriteActivated);
    connect(m_watchlistWidget, &QListWidget::itemSelectionChanged, this, &StrategyPanel::onFavoriteSelectionChanged);
}

void StrategyPanel::loadWatchlist()
{
    QSettings settings(QStringLiteral("QTQuant"), QStringLiteral("QTQuant"));
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

void StrategyPanel::saveWatchlist() const
{
    QStringList entries;
    for (const QString& symbol : m_favorites) {
        entries.append(symbol + QLatin1Char('|') + m_stockNames.value(symbol));
    }

    QSettings settings(QStringLiteral("QTQuant"), QStringLiteral("QTQuant"));
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
    if (!m_updatingSymbol && ui->symbolEdit->hasFocus() && !displays.isEmpty()) {
        m_symbolCompleter->complete();
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

void StrategyPanel::onSearchTimerTimeout()
{
    const QString keyword = ui->symbolEdit->text().trimmed();
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
                      QStringLiteral("Mozilla/5.0 QTQuant/0.1 EastMoneySearch"));
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

    updateSearchSuggestions(ui->symbolEdit->text(), parseSearchResponse(payload));
}

void StrategyPanel::onCompleterActivated(const QString& text)
{
    const QString symbol = m_completionSymbols.value(text, resolveSymbolText(text));
    if (!isValidMarketSymbol(symbol)) {
        return;
    }

    selectSymbol(symbol, stockNameForSymbol(symbol), true);
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
