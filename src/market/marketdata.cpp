#include "marketdata.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTime>
#include <QUrlQuery>
#include <algorithm>

namespace {
constexpr int QuoteIntervalMs = 3000;
constexpr int QuoteTimeoutMs = 2500;
constexpr int TrendTimeoutMs = 6000;
constexpr int TrendRefreshIntervalMs = 30000;
constexpr int HistoricalTrendRefreshIntervalMs = 5 * 60 * 1000;
constexpr int TrendHistoryPointCount = 1200;
constexpr auto QuoteEndpoint = "https://push2.eastmoney.com/api/qt/stock/get";
constexpr auto SinaQuoteEndpoint = "https://hq.sinajs.cn/list=%1";
constexpr auto TencentQuoteEndpoint = "https://qt.gtimg.cn/q=%1";
constexpr auto TrendEndpoint = "https://push2his.eastmoney.com/api/qt/stock/trends2/get";
constexpr auto SinaTrendEndpoint = "https://money.finance.sina.com.cn/quotes_service/api/json_v2.php/CN_MarketData.getKLineData";
constexpr auto TencentTrendEndpoint = "https://web.ifzq.gtimg.cn/appstock/app/kline/mkline";
constexpr auto EastMoneyUt = "fa5fd1943c7b386f172d6893dbfba10b";
constexpr auto EastMoneyFields = "f43,f44,f45,f46,f47,f48,f57,f58,f60,f169,f170";
constexpr auto TrendFields1 = "f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13";
constexpr auto TrendFields2 = "f51,f52,f53,f54,f55,f56,f57,f58";

bool isAShareWeekday(const QDate& date)
{
    const int day = date.dayOfWeek();
    return day >= 1 && day <= 5;
}

bool isBeforeAShareOpen(const QDateTime& now = QDateTime())
{
    const QDateTime current = now.isValid() ? now : QDateTime::currentDateTime();
    return isAShareWeekday(current.date()) && current.time() < QTime(9, 30);
}

bool canUseQuoteSummaryTrendFallback(const QDateTime& now = QDateTime())
{
    const QDateTime current = now.isValid() ? now : QDateTime::currentDateTime();
    if (!isAShareWeekday(current.date())) {
        return true;
    }
    return current.time() >= QTime(15, 0);
}

QDate previousAShareWeekday(QDate date)
{
    QDate value = date.addDays(-1);
    while (value.isValid() && !isAShareWeekday(value)) {
        value = value.addDays(-1);
    }
    return value;
}

QDate trendTargetDateFor(const QDateTime& now)
{
    const QDateTime current = now.isValid() ? now : QDateTime::currentDateTime();
    if (!isAShareWeekday(current.date()) || current.time() < QTime(9, 30)) {
        return previousAShareWeekday(current.date());
    }
    return current.date();
}

QString extractJsonText(const QByteArray& payload)
{
    const QString text = QString::fromUtf8(payload).trimmed();
    const int objectStart = text.indexOf(QLatin1Char('{'));
    const int arrayStart = text.indexOf(QLatin1Char('['));
    int start = -1;
    if (objectStart >= 0 && arrayStart >= 0) {
        start = qMin(objectStart, arrayStart);
    } else {
        start = qMax(objectStart, arrayStart);
    }
    if (start < 0) {
        return text;
    }

    const QChar opener = text.at(start);
    const QChar closer = opener == QLatin1Char('{') ? QLatin1Char('}') : QLatin1Char(']');
    const int end = text.lastIndexOf(closer);
    if (end < start) {
        return text.mid(start);
    }
    return text.mid(start, end - start + 1);
}

double jsonNumber(const QJsonValue& value)
{
    if (value.isDouble()) {
        return value.toDouble();
    }
    if (value.isString()) {
        QString text = value.toString().trimmed();
        if (text.isEmpty() || text == QStringLiteral("-") || text == QStringLiteral("--")) {
            return 0.0;
        }
        text.remove(QLatin1Char(','));
        bool ok = false;
        const double number = text.toDouble(&ok);
        return ok ? number : 0.0;
    }
    return 0.0;
}

QDateTime parseTrendTimestamp(QString text, const QDate& defaultDate)
{
    text = text.trimmed();
    if (text.isEmpty()) {
        return QDateTime();
    }

    const QStringList formats = {
        QStringLiteral("yyyy-MM-dd HH:mm:ss"),
        QStringLiteral("yyyy-MM-dd HH:mm"),
        QStringLiteral("yyyyMMddHHmmss"),
        QStringLiteral("yyyyMMddHHmm"),
        QStringLiteral("yyyyMMdd HH:mm:ss"),
        QStringLiteral("yyyyMMdd HH:mm")
    };
    for (const QString& format : formats) {
        const QDateTime timestamp = QDateTime::fromString(text, format);
        if (timestamp.isValid()) {
            return timestamp;
        }
    }

    if (defaultDate.isValid()) {
        const QTime time = QTime::fromString(text, QStringLiteral("HH:mm:ss"));
        if (time.isValid()) {
            return QDateTime(defaultDate, time);
        }
        if (text.size() == 4) {
            bool ok = false;
            const int value = text.toInt(&ok);
            if (ok) {
                const QTime compactTime(value / 100, value % 100);
                if (compactTime.isValid()) {
                    return QDateTime(defaultDate, compactTime);
                }
            }
        }
    }

    return QDateTime();
}

bool appendDelimitedTrendPoint(const QString& row,
                               const QDate& targetDate,
                               const QString& symbol,
                               const QString& name,
                               QVector<MarketData>* points)
{
    const QStringList rawParts = row.split(QRegularExpression(QStringLiteral("[,\\s]+")), Qt::SkipEmptyParts);
    if (rawParts.size() < 2) {
        return false;
    }

    QString timestampText = rawParts.value(0);
    int numberStart = 1;
    if (rawParts.size() > 2 && rawParts.value(0).contains(QLatin1Char('-')) && rawParts.value(1).contains(QLatin1Char(':'))) {
        timestampText += QLatin1Char(' ') + rawParts.value(1);
        numberStart = 2;
    }

    const QDateTime timestamp = parseTrendTimestamp(timestampText, targetDate);
    if (!timestamp.isValid()) {
        return false;
    }

    QVector<double> numbers;
    for (int i = numberStart; i < rawParts.size(); ++i) {
        bool ok = false;
        const double number = rawParts.at(i).toDouble(&ok);
        if (ok) {
            numbers.append(number);
        }
    }
    if (numbers.isEmpty()) {
        return false;
    }

    double open = numbers.value(0);
    double close = numbers.size() >= 4 ? numbers.value(1) : numbers.value(0);
    double high = numbers.size() >= 4 ? numbers.value(2) : close;
    double low = numbers.size() >= 4 ? numbers.value(3) : close;
    const double volume = numbers.size() >= 5 ? numbers.value(4) : 0.0;
    const double turnover = numbers.size() >= 6 ? numbers.value(5) : 0.0;
    const double averagePrice = numbers.size() >= 2 && numbers.size() < 4 ? numbers.value(1) : 0.0;

    if (close <= 0.0) {
        return false;
    }
    if (open <= 0.0) {
        open = close;
    }
    if (high <= 0.0) {
        high = qMax(open, close);
    }
    if (low <= 0.0) {
        low = qMin(open, close);
    }

    MarketData point;
    point.symbol = symbol;
    point.name = name;
    point.timestamp = timestamp;
    point.open = open;
    point.high = high;
    point.low = low;
    point.close = close;
    point.volume = volume;
    point.turnover = turnover;
    point.averagePrice = averagePrice > 0.0 ? averagePrice : close;
    point.tradeCount = volume > 0.0 ? static_cast<int>(volume) : 0;
    points->append(point);
    return true;
}

bool appendObjectTrendPoint(const QJsonObject& object,
                            const QDate& targetDate,
                            const QString& symbol,
                            const QString& name,
                            QVector<MarketData>* points)
{
    QString timestampText = object.value(QStringLiteral("day")).toString().trimmed();
    if (timestampText.isEmpty()) {
        timestampText = object.value(QStringLiteral("datetime")).toString().trimmed();
    }
    if (timestampText.isEmpty()) {
        const QString date = object.value(QStringLiteral("date")).toString().trimmed();
        const QString time = object.value(QStringLiteral("time")).toString().trimmed();
        timestampText = time.isEmpty() ? date : date + QLatin1Char(' ') + time;
    }
    if (timestampText.isEmpty()) {
        timestampText = object.value(QStringLiteral("t")).toString().trimmed();
    }

    const QDateTime timestamp = parseTrendTimestamp(timestampText, targetDate);
    if (!timestamp.isValid()) {
        return false;
    }

    double close = jsonNumber(object.value(QStringLiteral("close")));
    if (close <= 0.0) {
        close = jsonNumber(object.value(QStringLiteral("price")));
    }
    if (close <= 0.0) {
        close = jsonNumber(object.value(QStringLiteral("c")));
    }
    if (close <= 0.0) {
        return false;
    }

    double open = jsonNumber(object.value(QStringLiteral("open")));
    double high = jsonNumber(object.value(QStringLiteral("high")));
    double low = jsonNumber(object.value(QStringLiteral("low")));
    const double volume = jsonNumber(object.value(QStringLiteral("volume")));
    double turnover = jsonNumber(object.value(QStringLiteral("amount")));
    if (turnover <= 0.0) {
        turnover = jsonNumber(object.value(QStringLiteral("turnover")));
    }

    if (open <= 0.0) {
        open = close;
    }
    if (high <= 0.0) {
        high = qMax(open, close);
    }
    if (low <= 0.0) {
        low = qMin(open, close);
    }

    MarketData point;
    point.symbol = symbol;
    point.name = name;
    point.timestamp = timestamp;
    point.open = open;
    point.high = high;
    point.low = low;
    point.close = close;
    point.volume = volume;
    point.turnover = turnover;
    point.averagePrice = close;
    point.tradeCount = volume > 0.0 ? static_cast<int>(volume) : 0;
    points->append(point);
    return true;
}

bool appendArrayTrendPoint(const QJsonArray& array,
                           const QDate& targetDate,
                           const QString& symbol,
                           const QString& name,
                           QVector<MarketData>* points)
{
    if (array.size() < 2) {
        return false;
    }

    const QString timestampText = array.first().isString()
        ? array.first().toString()
        : QString::number(static_cast<qlonglong>(array.first().toDouble()));
    const QDateTime timestamp = parseTrendTimestamp(timestampText, targetDate);
    if (!timestamp.isValid()) {
        return false;
    }

    QVector<double> numbers;
    for (int i = 1; i < array.size(); ++i) {
        const double number = jsonNumber(array.at(i));
        if (number > 0.0 || array.at(i).isDouble() || array.at(i).isString()) {
            numbers.append(number);
        }
    }
    if (numbers.isEmpty()) {
        return false;
    }

    double open = numbers.value(0);
    double close = numbers.size() >= 4 ? numbers.value(1) : numbers.value(0);
    double high = numbers.size() >= 4 ? numbers.value(2) : close;
    double low = numbers.size() >= 4 ? numbers.value(3) : close;
    const double volume = numbers.size() >= 5 ? numbers.value(4) : 0.0;
    const double turnover = numbers.size() >= 6 ? numbers.value(5) : 0.0;
    const double averagePrice = numbers.size() >= 2 && numbers.size() < 4 ? numbers.value(1) : 0.0;

    if (close <= 0.0) {
        return false;
    }
    if (open <= 0.0) {
        open = close;
    }
    if (high <= 0.0) {
        high = qMax(open, close);
    }
    if (low <= 0.0) {
        low = qMin(open, close);
    }

    MarketData point;
    point.symbol = symbol;
    point.name = name;
    point.timestamp = timestamp;
    point.open = open;
    point.high = high;
    point.low = low;
    point.close = close;
    point.volume = volume;
    point.turnover = turnover;
    point.averagePrice = averagePrice > 0.0 ? averagePrice : close;
    point.tradeCount = volume > 0.0 ? static_cast<int>(volume) : 0;
    points->append(point);
    return true;
}

void collectKlinePoints(const QJsonValue& value,
                        const QDate& targetDate,
                        const QString& symbol,
                        const QString& name,
                        QVector<MarketData>* points)
{
    if (value.isObject()) {
        const QJsonObject object = value.toObject();
        if (appendObjectTrendPoint(object, targetDate, symbol, name, points)) {
            return;
        }
        for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
            collectKlinePoints(it.value(), targetDate, symbol, name, points);
        }
        return;
    }

    if (value.isArray()) {
        const QJsonArray array = value.toArray();
        if (appendArrayTrendPoint(array, targetDate, symbol, name, points)) {
            return;
        }
        for (const QJsonValue& item : array) {
            collectKlinePoints(item, targetDate, symbol, name, points);
        }
        return;
    }

    if (value.isString()) {
        appendDelimitedTrendPoint(value.toString(), targetDate, symbol, name, points);
    }
}

QVector<MarketData> quoteSummaryTrend(const MarketData& quote, const QDate& targetDate = QDate())
{
    QVector<MarketData> points;
    if (quote.close <= 0.0 || !quote.timestamp.isValid()) {
        return points;
    }

    const QDate date = targetDate.isValid() ? targetDate : quote.timestamp.date();
    if (date != quote.timestamp.date() && quote.previousClose > 0.0) {
        MarketData point = quote;
        point.timestamp = QDateTime(date, QTime(15, 0));
        point.open = quote.previousClose;
        point.high = quote.previousClose;
        point.low = quote.previousClose;
        point.close = quote.previousClose;
        point.averagePrice = quote.previousClose;
        point.previousClose = quote.previousClose;
        points.append(point);
        return points;
    }

    const double open = quote.open > 0.0 ? quote.open : quote.close;
    const double close = quote.close;
    const double high = quote.high > 0.0 ? qMax(quote.high, qMax(open, close)) : qMax(open, close);
    const double low = quote.low > 0.0 ? qMin(quote.low, qMin(open, close)) : qMin(open, close);
    const double averagePrice = quote.averagePrice > 0.0 ? quote.averagePrice : close;

    const bool closeAboveOpen = close >= open;
    const QList<QPair<QTime, double>> summary = {
        {QTime(9, 30), open},
        {QTime(10, 30), closeAboveOpen ? low : high},
        {QTime(14, 0), closeAboveOpen ? high : low},
        {QTime(15, 0), close}
    };

    for (const auto& item : summary) {
        MarketData point = quote;
        point.timestamp = QDateTime(date, item.first);
        point.open = item.second;
        point.high = item.second;
        point.low = item.second;
        point.close = item.second;
        point.averagePrice = averagePrice;
        points.append(point);
    }

    return points;
}
}
MarketDataSimulator::MarketDataSimulator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_symbol(normalizeSymbol("000001.SH"))
    , m_secid(secIdForSymbol(m_symbol))
    , m_lastPrice(0.0)
    , m_lastSuccessfulDataAt()
    , m_network(new QNetworkAccessManager(this))
    , m_activeReply(nullptr)
    , m_trendReply(nullptr)
    , m_activeQuoteSource(QuoteSource::EastMoney)
    , m_activeTrendSource(TrendSource::EastMoney)
    , m_quoteFailureReasons()
    , m_trendFailureReasons()
    , m_lastQuoteData()
    , m_hasLastQuoteData(false)
    , m_lastTrendFetchAt()
    , m_requestedTrendDate()
    , m_cachedTrendRequestDate()
    , m_cachedTrendDate()
    , m_cachedTrendSymbol()
    , m_cachedTrendPoints()
    , m_pendingTrendFallbackReason()
    , m_isUsingQuoteFallbackTrend(false)
    , m_feedMode(MarketDataFeedMode::QuoteAndTrend)
    , m_isRunning(false)
{
    connect(m_timer, &QTimer::timeout, this, &MarketDataSimulator::generateNewData);
    m_timer->setTimerType(Qt::PreciseTimer);
    m_timer->setInterval(QuoteIntervalMs);
}

MarketDataSimulator::~MarketDataSimulator()
{
    stopSimulation();
}

void MarketDataSimulator::startSimulation()
{
    if (m_isRunning) {
        generateNewData();
        if (!m_timer->isActive()) {
            m_timer->start();
        }
        return;
    }

    m_isRunning = true;
    generateNewData();
    m_timer->start();
}

void MarketDataSimulator::stopSimulation()
{
    m_isRunning = false;
    m_timer->stop();

    if (m_activeReply) {
        disconnect(m_activeReply, nullptr, this, nullptr);
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }

    if (m_trendReply) {
        disconnect(m_trendReply, nullptr, this, nullptr);
        m_trendReply->abort();
        m_trendReply->deleteLater();
        m_trendReply = nullptr;
    }

    m_activeQuoteSource = QuoteSource::EastMoney;
    m_activeTrendSource = TrendSource::EastMoney;
    m_quoteFailureReasons.clear();
    m_trendFailureReasons.clear();
}

void MarketDataSimulator::setSymbol(const QString &symbol)
{
    const QString normalized = normalizeSymbol(symbol);
    const QString secid = secIdForSymbol(normalized);
    if (secid.isEmpty()) {
        emit errorOccurred(QStringLiteral("Invalid symbol: %1").arg(symbol));
        return;
    }

    if (m_symbol == normalized && m_secid == secid) {
        return;
    }

    m_symbol = normalized;
    m_secid = secid;
    m_lastPrice = 0.0;
    m_hasLastQuoteData = false;
    m_pendingTrendFallbackReason.clear();
    m_isUsingQuoteFallbackTrend = false;
    m_lastTrendFetchAt = QDateTime();
    m_requestedTrendDate = QDate();
    m_cachedTrendRequestDate = QDate();
    m_cachedTrendDate = QDate();
    m_cachedTrendSymbol.clear();
    m_cachedTrendPoints.clear();
    m_activeQuoteSource = QuoteSource::EastMoney;
    m_activeTrendSource = TrendSource::EastMoney;
    m_quoteFailureReasons.clear();
    m_trendFailureReasons.clear();

    if (m_activeReply) {
        disconnect(m_activeReply, nullptr, this, nullptr);
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }

    if (m_trendReply) {
        disconnect(m_trendReply, nullptr, this, nullptr);
        m_trendReply->abort();
        m_trendReply->deleteLater();
        m_trendReply = nullptr;
    }

    if (m_isRunning) {
        generateNewData();
    }
}

void MarketDataSimulator::setFeedMode(MarketDataFeedMode mode)
{
    if (m_feedMode == mode) {
        return;
    }

    m_feedMode = mode;
    m_pendingTrendFallbackReason.clear();
    m_isUsingQuoteFallbackTrend = false;
    m_lastTrendFetchAt = QDateTime();
    m_requestedTrendDate = QDate();
    m_quoteFailureReasons.clear();
    m_trendFailureReasons.clear();
    if (m_isRunning) {
        generateNewData();
    }
}

QString MarketDataSimulator::normalizeSymbol(const QString &symbol)
{
    QString value = symbol.trimmed().toUpper();
    value.replace('_', '.');
    value.remove(' ');

    if (value.isEmpty()) {
        return QStringLiteral("000001.SH");
    }

    const QRegularExpression secIdPattern(QStringLiteral(R"(^([01])\.(\d{6})$)"));
    const QRegularExpressionMatch secIdMatch = secIdPattern.match(value);
    if (secIdMatch.hasMatch()) {
        return secIdMatch.captured(2) + (secIdMatch.captured(1) == QStringLiteral("1")
            ? QStringLiteral(".SH")
            : QStringLiteral(".SZ"));
    }

    QString code = value;
    QString exchange;

    if (value.startsWith(QStringLiteral("SH")) && value.size() >= 8) {
        code = value.mid(2, 6);
        exchange = QStringLiteral("SH");
    } else if (value.startsWith(QStringLiteral("SZ")) && value.size() >= 8) {
        code = value.mid(2, 6);
        exchange = QStringLiteral("SZ");
    } else if (value.contains('.')) {
        const QStringList parts = value.split('.', Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            code = parts.first();
        }
        if (parts.size() > 1) {
            const QString suffix = parts.at(1);
            if (suffix == QStringLiteral("SH") || suffix == QStringLiteral("SS") || suffix == QStringLiteral("SSE") || suffix == QStringLiteral("XSHG")) {
                exchange = QStringLiteral("SH");
            } else if (suffix == QStringLiteral("SZ") || suffix == QStringLiteral("SZSE") || suffix == QStringLiteral("XSHE")) {
                exchange = QStringLiteral("SZ");
            }
        }
    }

    const QRegularExpression codePattern(QStringLiteral(R"((\d{6}))"));
    const QRegularExpressionMatch codeMatch = codePattern.match(code);
    if (codeMatch.hasMatch()) {
        code = codeMatch.captured(1);
    }

    if (code.size() != 6) {
        return value;
    }

    if (exchange.isEmpty()) {
        const QChar first = code.at(0);
        exchange = (first == QLatin1Char('5') || first == QLatin1Char('6') || first == QLatin1Char('9'))
            ? QStringLiteral("SH")
            : QStringLiteral("SZ");
    }

    return code + QLatin1Char('.') + exchange;
}

bool MarketDataSimulator::isAShareContinuousTradingTime(const QDateTime& now)
{
    const QDateTime current = now.isValid() ? now : QDateTime::currentDateTime();
    const int day = current.date().dayOfWeek();
    if (day < 1 || day > 5) {
        return false;
    }

    const QTime time = current.time();
    const bool morning = time >= QTime(9, 30) && time <= QTime(11, 30);
    const bool afternoon = time >= QTime(13, 0) && time <= QTime(15, 0);
    return morning || afternoon;
}
void MarketDataSimulator::generateNewData()
{
    const QDateTime now = QDateTime::currentDateTime();
    const bool marketOpen = isAShareContinuousTradingTime(now);
    switch (m_feedMode) {
    case MarketDataFeedMode::QuoteOnly:
        fetchLatestQuote();
        break;
    case MarketDataFeedMode::QuoteAndTrend:
        fetchLatestQuote();
        fetchIntradayTrend();
        break;
    case MarketDataFeedMode::QuoteWhenOpenTrendWhenClosed:
        fetchLatestQuote();
        if (!hasReusableTrendCache(trendTargetDateFor(now), now)
            && (!m_lastTrendFetchAt.isValid()
                || m_lastTrendFetchAt.msecsTo(now) >= (marketOpen ? TrendRefreshIntervalMs : HistoricalTrendRefreshIntervalMs))) {
            fetchIntradayTrend();
        }
        break;
    case MarketDataFeedMode::RealtimeQuoteOnly:
        if (marketOpen) {
            fetchLatestQuote();
        } else {
            emit errorOccurred(QStringLiteral("A 股未处于连续竞价时段，策略只能在交易日 09:30-11:30、13:00-15:00 使用实时行情运行。"));
        }
        break;
    }
}

void MarketDataSimulator::fetchLatestQuote()
{
    if (!m_isRunning || m_activeReply) {
        return;
    }

    if (m_secid.isEmpty()) {
        emit errorOccurred(QStringLiteral("Invalid symbol: %1").arg(m_symbol));
        return;
    }

    m_quoteFailureReasons.clear();
    fetchQuote(QuoteSource::EastMoney);
}

void MarketDataSimulator::fetchQuote(QuoteSource source)
{
    if (!m_isRunning || m_activeReply) {
        return;
    }

    const QUrl url = buildQuoteUrl(source);
    if (!url.isValid()) {
        emit errorOccurred(QStringLiteral("Invalid quote URL for %1").arg(m_symbol));
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 StarQuant/0.1 MarketQuote"));
    switch (source) {
    case QuoteSource::EastMoney:
        request.setRawHeader("Referer", "https://quote.eastmoney.com/");
        break;
    case QuoteSource::Sina:
        request.setRawHeader("Referer", "https://finance.sina.com.cn/");
        break;
    case QuoteSource::Tencent:
        request.setRawHeader("Referer", "https://gu.qq.com/");
        break;
    }
    request.setTransferTimeout(QuoteTimeoutMs);

    m_activeQuoteSource = source;
    m_activeReply = m_network->get(request);
    connect(m_activeReply, &QNetworkReply::finished, this, &MarketDataSimulator::onQuoteReplyFinished);
}

bool MarketDataSimulator::fetchNextQuoteSource(QuoteSource source, const QString& failureReason)
{
    const QString reason = failureReason.isEmpty()
        ? QStringLiteral("returned no usable quote")
        : failureReason;
    m_quoteFailureReasons.append(QStringLiteral("%1: %2").arg(sourceName(source), reason));

    QuoteSource nextSource = QuoteSource::EastMoney;
    switch (source) {
    case QuoteSource::EastMoney:
        nextSource = QuoteSource::Sina;
        break;
    case QuoteSource::Sina:
        nextSource = QuoteSource::Tencent;
        break;
    case QuoteSource::Tencent:
        return false;
    }

    fetchQuote(nextSource);
    return m_activeReply != nullptr;
}

bool MarketDataSimulator::hasReusableTrendCache(const QDate& targetDate, const QDateTime& now) const
{
    if (isAShareContinuousTradingTime(now)) {
        return false;
    }
    if (m_cachedTrendPoints.isEmpty()
        || m_cachedTrendSymbol != m_symbol
        || !m_cachedTrendRequestDate.isValid()
        || m_cachedTrendRequestDate != targetDate) {
        return false;
    }
    if (!m_lastTrendFetchAt.isValid()) {
        return true;
    }
    return m_lastTrendFetchAt.msecsTo(now) < HistoricalTrendRefreshIntervalMs;
}

bool MarketDataSimulator::emitCachedTrend(const QDate& targetDate)
{
    if (m_cachedTrendPoints.isEmpty()
        || m_cachedTrendSymbol != m_symbol
        || !m_cachedTrendDate.isValid()
        || !targetDate.isValid()
        || m_cachedTrendDate > targetDate) {
        return false;
    }

    emit intradayDataUpdated(m_cachedTrendPoints);
    return true;
}

void MarketDataSimulator::fetchIntradayTrend()
{
    if (!m_isRunning || m_trendReply) {
        return;
    }

    if (m_secid.isEmpty()) {
        return;
    }

    fetchTrend(TrendSource::EastMoney, trendTargetDateFor(QDateTime::currentDateTime()));
}

void MarketDataSimulator::fetchTrend(TrendSource source, const QDate& targetDate)
{
    if (!m_isRunning || m_trendReply) {
        return;
    }

    m_requestedTrendDate = targetDate;
    m_activeTrendSource = source;

    const QUrl url = buildTrendUrl(source, targetDate);
    if (!url.isValid()) {
        fetchNextTrendSource(source, QStringLiteral("Invalid trend URL for %1").arg(m_symbol));
        return;
    }

    m_lastTrendFetchAt = QDateTime::currentDateTime();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 StarQuant/0.1 MarketTrend"));
    switch (source) {
    case TrendSource::EastMoney:
        request.setRawHeader("Referer", "https://quote.eastmoney.com/");
        break;
    case TrendSource::Sina:
        request.setRawHeader("Referer", "https://finance.sina.com.cn/");
        break;
    case TrendSource::Tencent:
        request.setRawHeader("Referer", "https://gu.qq.com/");
        break;
    }
    request.setTransferTimeout(TrendTimeoutMs);

    m_trendReply = m_network->get(request);
    connect(m_trendReply, &QNetworkReply::finished, this, &MarketDataSimulator::onTrendReplyFinished);
}

bool MarketDataSimulator::fetchNextTrendSource(TrendSource source, const QString& failureReason)
{
    const QString reason = failureReason.isEmpty()
        ? QStringLiteral("returned no usable trend")
        : failureReason;
    m_trendFailureReasons.append(QStringLiteral("%1: %2").arg(trendSourceName(source), reason));

    TrendSource nextSource = TrendSource::EastMoney;
    switch (source) {
    case TrendSource::EastMoney:
        nextSource = TrendSource::Sina;
        break;
    case TrendSource::Sina:
        nextSource = TrendSource::Tencent;
        break;
    case TrendSource::Tencent:
        return false;
    }

    fetchTrend(nextSource, m_requestedTrendDate);
    return m_trendReply != nullptr;
}

void MarketDataSimulator::onQuoteReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    const QuoteSource source = m_activeQuoteSource;
    if (reply == m_activeReply) {
        m_activeReply = nullptr;
    }

    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorText = reply->errorString();
    const QByteArray payload = reply->readAll();
    reply->deleteLater();

    MarketData data;
    QString parseError;
    if (parseQuoteResponse(source, payload, &data, &parseError)) {
        m_quoteFailureReasons.clear();
        m_lastPrice = data.close;
        m_lastQuoteData = data;
        m_hasLastQuoteData = true;
        m_lastSuccessfulDataAt = QDateTime::currentDateTime();
        emit dataUpdated(data);
        if (m_feedMode == MarketDataFeedMode::QuoteWhenOpenTrendWhenClosed
            && isAShareContinuousTradingTime()) {
            m_pendingTrendFallbackReason.clear();
            m_isUsingQuoteFallbackTrend = false;
        }
        if (!m_pendingTrendFallbackReason.isEmpty()) {
            const QString pendingReason = m_pendingTrendFallbackReason;
            emitQuoteFallbackTrend(pendingReason);
        }
        return;
    }

    const bool connectionClosed = networkError == QNetworkReply::OperationCanceledError
        || networkError == QNetworkReply::RemoteHostClosedError
        || networkErrorText.contains(QStringLiteral("Operation canceled"), Qt::CaseInsensitive)
        || networkErrorText.contains(QStringLiteral("Connection closed"), Qt::CaseInsensitive);

    QString failureReason;
    if (networkError != QNetworkReply::NoError || connectionClosed) {
        failureReason = networkErrorText.isEmpty() ? parseError : networkErrorText;
    } else {
        failureReason = parseError;
    }
    if (failureReason.isEmpty()) {
        failureReason = QStringLiteral("Market quote returned no usable data for %1").arg(m_symbol);
    }

    if (fetchNextQuoteSource(source, failureReason)) {
        return;
    }

    if (m_lastSuccessfulDataAt.isValid()
        && m_lastSuccessfulDataAt.secsTo(QDateTime::currentDateTime()) <= 15) {
        m_quoteFailureReasons.clear();
        return;
    }

    const QString combinedReason = m_quoteFailureReasons.isEmpty()
        ? failureReason
        : m_quoteFailureReasons.join(QStringLiteral("; "));
    m_quoteFailureReasons.clear();
    emit errorOccurred(QStringLiteral("Market quote request failed after EastMoney, Sina and Tencent fallback: %1")
                           .arg(combinedReason));
}

void MarketDataSimulator::onTrendReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply == m_trendReply) {
        m_trendReply = nullptr;
    }

    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorText = reply->errorString();
    const QByteArray payload = reply->readAll();
    reply->deleteLater();
    const QDateTime now = QDateTime::currentDateTime();

    const TrendSource source = m_activeTrendSource;
    const QDate requestedDate = m_requestedTrendDate;
    QVector<MarketData> points;
    QDate actualDate;
    QString parseError;
    if (parseTrendResponse(source, payload, requestedDate, &points, &actualDate, &parseError)) {
        if (!points.isEmpty()) {
            m_lastSuccessfulDataAt = QDateTime::currentDateTime();
            m_trendFailureReasons.clear();
            m_cachedTrendRequestDate = requestedDate;
            m_cachedTrendDate = actualDate.isValid() ? actualDate : points.constLast().timestamp.date();
            m_cachedTrendSymbol = m_symbol;
            m_cachedTrendPoints = points;
            m_pendingTrendFallbackReason.clear();
            m_isUsingQuoteFallbackTrend = false;
            emit intradayDataUpdated(points);
        }
        return;
    }

    const bool connectionClosed = networkError == QNetworkReply::OperationCanceledError
        || networkError == QNetworkReply::RemoteHostClosedError
        || networkErrorText.contains(QStringLiteral("Operation canceled"), Qt::CaseInsensitive)
        || networkErrorText.contains(QStringLiteral("Connection closed"), Qt::CaseInsensitive);

    QString failureReason;
    if (networkError != QNetworkReply::NoError) {
        failureReason = QStringLiteral("Market trend request failed: %1").arg(networkErrorText);
    } else {
        failureReason = parseError;
    }
    if (failureReason.isEmpty()) {
        failureReason = QStringLiteral("Market trend returned no usable data for %1").arg(m_symbol);
    }

    if (fetchNextTrendSource(source, failureReason)) {
        return;
    }

    const QString combinedReason = m_trendFailureReasons.isEmpty()
        ? failureReason
        : m_trendFailureReasons.join(QStringLiteral("; "));
    m_trendFailureReasons.clear();

    if (emitCachedTrend(requestedDate)) {
        return;
    }

    if (emitQuoteFallbackTrend(combinedReason)) {
        return;
    }
    if (m_feedMode == MarketDataFeedMode::QuoteWhenOpenTrendWhenClosed
        && !isAShareContinuousTradingTime(now)
        && m_activeReply) {
        m_pendingTrendFallbackReason = combinedReason;
        return;
    }

    if (m_lastSuccessfulDataAt.isValid()
        && m_lastSuccessfulDataAt.secsTo(QDateTime::currentDateTime()) <= 15) {
        return;
    }

    if (networkError != QNetworkReply::NoError) {
        emit errorOccurred(combinedReason);
        return;
    }

    emit errorOccurred(combinedReason);
}

QUrl MarketDataSimulator::buildQuoteUrl(QuoteSource source) const
{
    switch (source) {
    case QuoteSource::EastMoney: {
        QUrl url(QString::fromLatin1(QuoteEndpoint));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("ut"), QString::fromLatin1(EastMoneyUt));
        query.addQueryItem(QStringLiteral("fltt"), QStringLiteral("2"));
        query.addQueryItem(QStringLiteral("invt"), QStringLiteral("2"));
        query.addQueryItem(QStringLiteral("secid"), m_secid);
        query.addQueryItem(QStringLiteral("fields"), QString::fromLatin1(EastMoneyFields));
        url.setQuery(query);
        return url;
    }
    case QuoteSource::Sina:
        return QUrl(QString::fromLatin1(SinaQuoteEndpoint).arg(prefixedMarketSymbol(m_symbol)));
    case QuoteSource::Tencent:
        return QUrl(QString::fromLatin1(TencentQuoteEndpoint).arg(prefixedMarketSymbol(m_symbol)));
    }

    return QUrl();
}

QUrl MarketDataSimulator::buildTrendUrl(TrendSource source, const QDate& targetDate) const
{
    switch (source) {
    case TrendSource::EastMoney: {
        QUrl url(QString::fromLatin1(TrendEndpoint));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("ut"), QString::fromLatin1(EastMoneyUt));
        query.addQueryItem(QStringLiteral("fields1"), QString::fromLatin1(TrendFields1));
        query.addQueryItem(QStringLiteral("fields2"), QString::fromLatin1(TrendFields2));
        query.addQueryItem(QStringLiteral("secid"), m_secid);
        query.addQueryItem(QStringLiteral("ndays"), targetDate == QDate::currentDate() ? QStringLiteral("1") : QStringLiteral("5"));
        query.addQueryItem(QStringLiteral("iscr"), QStringLiteral("0"));
        query.addQueryItem(QStringLiteral("iscca"), QStringLiteral("0"));
        url.setQuery(query);
        return url;
    }
    case TrendSource::Sina: {
        const QString code = prefixedMarketSymbol(m_symbol);
        if (code.isEmpty()) {
            return QUrl();
        }
        QUrl url(QString::fromLatin1(SinaTrendEndpoint));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("symbol"), code);
        query.addQueryItem(QStringLiteral("scale"), QStringLiteral("1"));
        query.addQueryItem(QStringLiteral("ma"), QStringLiteral("no"));
        query.addQueryItem(QStringLiteral("datalen"), QString::number(TrendHistoryPointCount));
        url.setQuery(query);
        return url;
    }
    case TrendSource::Tencent: {
        const QString code = prefixedMarketSymbol(m_symbol);
        if (code.isEmpty()) {
            return QUrl();
        }
        QUrl url(QString::fromLatin1(TencentTrendEndpoint));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("param"),
                           QStringLiteral("%1,m1,,%2").arg(code).arg(TrendHistoryPointCount));
        url.setQuery(query);
        return url;
    }
    }

    return QUrl();
}

bool MarketDataSimulator::emitQuoteFallbackTrend(const QString& reason)
{
    const QDateTime now = QDateTime::currentDateTime();
    if (m_feedMode != MarketDataFeedMode::QuoteWhenOpenTrendWhenClosed
        || isAShareContinuousTradingTime(now)) {
        m_pendingTrendFallbackReason.clear();
        m_isUsingQuoteFallbackTrend = false;
        return false;
    }
    const QString fallbackReason = reason.isEmpty()
        ? QStringLiteral("分时接口未返回可用数据")
        : reason;
    if (!m_hasLastQuoteData) {
        m_pendingTrendFallbackReason = fallbackReason;
        return false;
    }

    const QVector<MarketData> fallbackPoints = quoteSummaryTrend(m_lastQuoteData, m_requestedTrendDate);
    if (fallbackPoints.isEmpty()) {
        m_pendingTrendFallbackReason = fallbackReason;
        return false;
    }

    m_pendingTrendFallbackReason.clear();
    emit intradayDataUpdated(fallbackPoints);
    if (!m_isUsingQuoteFallbackTrend) {
        m_isUsingQuoteFallbackTrend = true;
        emit fallbackIntradayDataUsed(m_symbol, fallbackReason);
    }
    return true;
}

bool MarketDataSimulator::parseQuoteResponse(QuoteSource source, const QByteArray& payload, MarketData* data, QString* errorMessage) const
{
    switch (source) {
    case QuoteSource::EastMoney:
        return parseEastMoneyQuoteResponse(payload, data, errorMessage);
    case QuoteSource::Sina:
        return parseSinaQuoteResponse(payload, data, errorMessage);
    case QuoteSource::Tencent:
        return parseTencentQuoteResponse(payload, data, errorMessage);
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Unknown quote source for %1").arg(m_symbol);
    }
    return false;
}

bool MarketDataSimulator::parseEastMoneyQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const
{
    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Market quote returned invalid JSON: %1").arg(jsonError.errorString());
        }
        return false;
    }

    const QJsonObject root = document.object();
    const QJsonValue quoteValue = root.value(QStringLiteral("data"));
    if (!quoteValue.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Market quote returned no data for %1").arg(m_symbol);
        }
        return false;
    }

    const QJsonObject quote = quoteValue.toObject();
    const double previousClose = valueToDouble(quote.value(QStringLiteral("f60")));
    double close = valueToDouble(quote.value(QStringLiteral("f43")));
    double open = valueToDouble(quote.value(QStringLiteral("f46")));
    double high = valueToDouble(quote.value(QStringLiteral("f44")));
    double low = valueToDouble(quote.value(QStringLiteral("f45")));

    if (close <= 0.0 && m_feedMode == MarketDataFeedMode::RealtimeQuoteOnly) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Realtime EastMoney quote is unavailable for %1").arg(m_symbol);
        }
        return false;
    }
    if (close <= 0.0) {
        close = previousClose > 0.0 ? previousClose : m_lastPrice;
    }
    if (close <= 0.0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Market quote has no usable price for %1").arg(m_symbol);
        }
        return false;
    }

    if (open <= 0.0) {
        open = close;
    }
    if (high <= 0.0) {
        high = qMax(open, close);
    }
    if (low <= 0.0) {
        low = qMin(open, close);
    }

    data->symbol = m_symbol;
    data->name = quote.value(QStringLiteral("f58")).toString();
    data->timestamp = QDateTime::currentDateTime();
    data->open = open;
    data->high = high;
    data->low = low;
    data->close = close;
    data->volume = valueToDouble(quote.value(QStringLiteral("f47")));
    data->turnover = valueToDouble(quote.value(QStringLiteral("f48")));
    data->previousClose = previousClose;
    data->averagePrice = data->volume > 0.0 ? data->turnover / (data->volume * 100.0) : close;
    data->tradeCount = data->volume > 0.0 ? static_cast<int>(data->volume) : 0;
    return true;
}

bool MarketDataSimulator::parseSinaQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const
{
    const QString text = QString::fromLocal8Bit(payload).trimmed();
    const int valueStart = text.indexOf(QLatin1Char('"'));
    const int valueEnd = text.lastIndexOf(QLatin1Char('"'));
    if (valueStart < 0 || valueEnd <= valueStart) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Sina quote returned invalid text for %1").arg(m_symbol);
        }
        return false;
    }

    const QStringList parts = text.mid(valueStart + 1, valueEnd - valueStart - 1).split(QLatin1Char(','));
    if (parts.size() < 32 || parts.value(0).trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Sina quote returned no usable data for %1").arg(m_symbol);
        }
        return false;
    }

    const double previousClose = textToDouble(parts.value(2));
    double close = textToDouble(parts.value(3));
    double open = textToDouble(parts.value(1));
    double high = textToDouble(parts.value(4));
    double low = textToDouble(parts.value(5));

    if (close <= 0.0 && m_feedMode == MarketDataFeedMode::RealtimeQuoteOnly) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Realtime Sina quote is unavailable for %1").arg(m_symbol);
        }
        return false;
    }
    if (close <= 0.0) {
        close = previousClose > 0.0 ? previousClose : m_lastPrice;
    }
    if (close <= 0.0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Sina quote has no usable price for %1").arg(m_symbol);
        }
        return false;
    }

    if (open <= 0.0) {
        open = close;
    }
    if (high <= 0.0) {
        high = qMax(open, close);
    }
    if (low <= 0.0) {
        low = qMin(open, close);
    }

    const double volumeShares = textToDouble(parts.value(8));
    const double volumeLots = volumeShares > 0.0 ? volumeShares / 100.0 : 0.0;
    const double turnover = textToDouble(parts.value(9));
    QDateTime timestamp = QDateTime::fromString(parts.value(30).trimmed() + QLatin1Char(' ') + parts.value(31).trimmed(),
                                                QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (!timestamp.isValid()) {
        timestamp = QDateTime::currentDateTime();
    }

    data->symbol = m_symbol;
    data->name = parts.value(0).trimmed();
    data->timestamp = timestamp;
    data->open = open;
    data->high = high;
    data->low = low;
    data->close = close;
    data->volume = volumeLots;
    data->turnover = turnover;
    data->previousClose = previousClose;
    data->averagePrice = volumeLots > 0.0 && turnover > 0.0 ? turnover / (volumeLots * 100.0) : close;
    data->tradeCount = volumeLots > 0.0 ? static_cast<int>(volumeLots) : 0;
    return true;
}

bool MarketDataSimulator::parseTencentQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const
{
    const QString text = QString::fromLocal8Bit(payload).trimmed();
    const int valueStart = text.indexOf(QLatin1Char('"'));
    const int valueEnd = text.lastIndexOf(QLatin1Char('"'));
    if (valueStart < 0 || valueEnd <= valueStart) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tencent quote returned invalid text for %1").arg(m_symbol);
        }
        return false;
    }

    const QStringList parts = text.mid(valueStart + 1, valueEnd - valueStart - 1).split(QLatin1Char('~'));
    if (parts.size() < 35 || parts.value(3).trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tencent quote returned no usable data for %1").arg(m_symbol);
        }
        return false;
    }

    const double previousClose = textToDouble(parts.value(4));
    double close = textToDouble(parts.value(3));
    double open = textToDouble(parts.value(5));
    double high = textToDouble(parts.value(33));
    double low = textToDouble(parts.value(34));

    if (close <= 0.0 && m_feedMode == MarketDataFeedMode::RealtimeQuoteOnly) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Realtime Tencent quote is unavailable for %1").arg(m_symbol);
        }
        return false;
    }
    if (close <= 0.0) {
        close = previousClose > 0.0 ? previousClose : m_lastPrice;
    }
    if (close <= 0.0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tencent quote has no usable price for %1").arg(m_symbol);
        }
        return false;
    }

    if (open <= 0.0) {
        open = close;
    }
    if (high <= 0.0) {
        high = qMax(open, close);
    }
    if (low <= 0.0) {
        low = qMin(open, close);
    }

    const double volumeLots = textToDouble(parts.value(6));
    double turnover = 0.0;
    const QStringList summary = parts.value(35).split(QLatin1Char('/'));
    if (summary.size() >= 3) {
        turnover = textToDouble(summary.value(2));
    }
    if (turnover <= 0.0) {
        turnover = textToDouble(parts.value(37)) * 10000.0;
    }

    QDateTime timestamp = QDateTime::fromString(parts.value(30).trimmed(), QStringLiteral("yyyyMMddHHmmss"));
    if (!timestamp.isValid()) {
        timestamp = QDateTime::currentDateTime();
    }

    data->symbol = m_symbol;
    data->name = parts.value(1).trimmed();
    data->timestamp = timestamp;
    data->open = open;
    data->high = high;
    data->low = low;
    data->close = close;
    data->volume = volumeLots;
    data->turnover = turnover;
    data->previousClose = previousClose;
    data->averagePrice = volumeLots > 0.0 && turnover > 0.0 ? turnover / (volumeLots * 100.0) : close;
    data->tradeCount = volumeLots > 0.0 ? static_cast<int>(volumeLots) : 0;
    return true;
}

bool MarketDataSimulator::parseTrendResponse(TrendSource source,
                                             const QByteArray& payload,
                                             const QDate& targetDate,
                                             QVector<MarketData>* points,
                                             QDate* actualDate,
                                             QString* errorMessage) const
{
    switch (source) {
    case TrendSource::EastMoney:
        return parseEastMoneyTrendResponse(payload, targetDate, points, actualDate, errorMessage);
    case TrendSource::Sina:
        return parseSinaTrendResponse(payload, targetDate, points, actualDate, errorMessage);
    case TrendSource::Tencent:
        return parseTencentTrendResponse(payload, targetDate, points, actualDate, errorMessage);
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Unknown trend source for %1").arg(m_symbol);
    }
    return false;
}

bool MarketDataSimulator::parseEastMoneyTrendResponse(const QByteArray& payload,
                                                      const QDate& targetDate,
                                                      QVector<MarketData>* points,
                                                      QDate* actualDate,
                                                      QString* errorMessage) const
{
    if (!points) {
        return false;
    }

    points->clear();
    if (actualDate) {
        *actualDate = QDate();
    }

    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(extractJsonText(payload).toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("EastMoney trend returned invalid JSON: %1").arg(jsonError.errorString());
        }
        return false;
    }

    const QJsonObject root = document.object();
    const QJsonValue trendValue = root.value(QStringLiteral("data"));
    if (!trendValue.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("EastMoney trend returned no data for %1").arg(m_symbol);
        }
        return false;
    }

    const QJsonObject trendData = trendValue.toObject();
    const QJsonArray trends = trendData.value(QStringLiteral("trends")).toArray();
    if (trends.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("EastMoney trend returned empty data for %1").arg(m_symbol);
        }
        return false;
    }

    double previousClose = valueToDouble(trendData.value(QStringLiteral("preClose")));
    if (previousClose <= 0.0) {
        previousClose = valueToDouble(trendData.value(QStringLiteral("prePrice")));
    }

    QVector<MarketData> allPoints;
    for (const QJsonValue& value : trends) {
        const QString row = value.toString().trimmed();
        if (row.isEmpty()) {
            continue;
        }

        const QStringList parts = row.split(QLatin1Char(','));
        if (parts.size() < 3) {
            continue;
        }

        QDateTime timestamp = QDateTime::fromString(parts.value(0), QStringLiteral("yyyy-MM-dd HH:mm"));
        if (!timestamp.isValid()) {
            timestamp = QDateTime::fromString(parts.value(0), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        }
        if (!timestamp.isValid()) {
            continue;
        }

        const double first = parts.value(1).toDouble();
        const double second = parts.value(2).toDouble();
        const double third = parts.size() > 3 ? parts.value(3).toDouble() : 0.0;
        const double fourth = parts.size() > 4 ? parts.value(4).toDouble() : 0.0;
        const double fifth = parts.size() > 5 ? parts.value(5).toDouble() : 0.0;
        const double sixth = parts.size() > 6 ? parts.value(6).toDouble() : 0.0;
        const double seventh = parts.size() > 7 ? parts.value(7).toDouble() : 0.0;
        const double priceBase = qMax(qAbs(first), qAbs(second));
        const bool looksLikeOhlc = parts.size() >= 7
            && priceBase > 0.0
            && third > 0.0
            && fourth > 0.0
            && third <= priceBase * 3.0
            && fourth <= priceBase * 3.0;

        double open = first;
        double close = second;
        double high = third;
        double low = fourth;
        double volume = fifth;
        double turnover = sixth;
        double averagePrice = seventh;

        if (!looksLikeOhlc) {
            close = first;
            open = close;
            high = close;
            low = close;
            averagePrice = second;
            volume = third;
            turnover = fourth;
        }

        if (close <= 0.0) {
            continue;
        }

        MarketData point;
        point.symbol = m_symbol;
        point.name = trendData.value(QStringLiteral("name")).toString();
        point.timestamp = timestamp;
        point.open = open > 0.0 ? open : close;
        point.high = high > 0.0 ? high : close;
        point.low = low > 0.0 ? low : close;
        point.close = close;
        point.volume = volume;
        point.turnover = turnover;
        point.previousClose = previousClose;
        point.averagePrice = averagePrice > 0.0
            ? averagePrice
            : (volume > 0.0 ? turnover / (volume * 100.0) : close);
        point.tradeCount = volume > 0.0 ? static_cast<int>(volume) : 0;
        allPoints.append(point);
    }

    return selectTrendSession(allPoints, targetDate, points, actualDate, errorMessage);
}

bool MarketDataSimulator::parseSinaTrendResponse(const QByteArray& payload,
                                                 const QDate& targetDate,
                                                 QVector<MarketData>* points,
                                                 QDate* actualDate,
                                                 QString* errorMessage) const
{
    if (!points) {
        return false;
    }

    points->clear();
    if (actualDate) {
        *actualDate = QDate();
    }

    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(extractJsonText(payload).toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError || (!document.isArray() && !document.isObject())) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Sina trend returned invalid JSON: %1").arg(jsonError.errorString());
        }
        return false;
    }

    QVector<MarketData> allPoints;
    const QJsonValue root = document.isArray()
        ? QJsonValue(document.array())
        : QJsonValue(document.object());
    collectKlinePoints(root, targetDate, m_symbol, QString(), &allPoints);
    return selectTrendSession(allPoints, targetDate, points, actualDate, errorMessage);
}

bool MarketDataSimulator::parseTencentTrendResponse(const QByteArray& payload,
                                                    const QDate& targetDate,
                                                    QVector<MarketData>* points,
                                                    QDate* actualDate,
                                                    QString* errorMessage) const
{
    if (!points) {
        return false;
    }

    points->clear();
    if (actualDate) {
        *actualDate = QDate();
    }

    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(extractJsonText(payload).toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError || (!document.isArray() && !document.isObject())) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tencent trend returned invalid JSON: %1").arg(jsonError.errorString());
        }
        return false;
    }

    QVector<MarketData> allPoints;
    const QJsonValue root = document.isArray()
        ? QJsonValue(document.array())
        : QJsonValue(document.object());
    collectKlinePoints(root, targetDate, m_symbol, QString(), &allPoints);
    return selectTrendSession(allPoints, targetDate, points, actualDate, errorMessage);
}

bool MarketDataSimulator::selectTrendSession(const QVector<MarketData>& allPoints,
                                             const QDate& targetDate,
                                             QVector<MarketData>* points,
                                             QDate* actualDate,
                                             QString* errorMessage) const
{
    if (!points) {
        return false;
    }

    points->clear();
    if (actualDate) {
        *actualDate = QDate();
    }

    if (allPoints.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Market trend had no usable points for %1").arg(m_symbol);
        }
        return false;
    }

    QDate selectedDate;
    for (const MarketData& point : allPoints) {
        if (!point.timestamp.isValid() || point.close <= 0.0) {
            continue;
        }

        const QDate pointDate = point.timestamp.date();
        if (targetDate.isValid() && pointDate > targetDate) {
            continue;
        }
        if (!selectedDate.isValid() || pointDate > selectedDate) {
            selectedDate = pointDate;
        }
    }

    if (!selectedDate.isValid()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Market trend has no session before %1 for %2")
                .arg(targetDate.toString(QStringLiteral("yyyy-MM-dd")), m_symbol);
        }
        return false;
    }

    double previousClose = 0.0;
    QVector<MarketData> selectedPoints;
    for (const MarketData& point : allPoints) {
        if (!point.timestamp.isValid() || point.close <= 0.0) {
            continue;
        }
        const QDate pointDate = point.timestamp.date();
        if (pointDate < selectedDate) {
            previousClose = point.close;
            continue;
        }
        if (pointDate == selectedDate) {
            selectedPoints.append(point);
        }
    }

    if (selectedPoints.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Market trend selected empty session for %1").arg(m_symbol);
        }
        return false;
    }

    std::sort(selectedPoints.begin(), selectedPoints.end(), [](const MarketData& left, const MarketData& right) {
        return left.timestamp < right.timestamp;
    });

    if (previousClose <= 0.0 && m_hasLastQuoteData && m_lastQuoteData.previousClose > 0.0
        && selectedDate == m_lastQuoteData.timestamp.date()) {
        previousClose = m_lastQuoteData.previousClose;
    }
    if (previousClose <= 0.0) {
        previousClose = selectedPoints.constFirst().open > 0.0
            ? selectedPoints.constFirst().open
            : selectedPoints.constFirst().close;
    }

    double volumeSum = 0.0;
    double weightedPriceSum = 0.0;
    for (MarketData& point : selectedPoints) {
        point.symbol = m_symbol;
        point.previousClose = previousClose;
        if (point.volume > 0.0) {
            volumeSum += point.volume;
            weightedPriceSum += point.close * point.volume;
        }
        if (point.averagePrice <= 0.0) {
            point.averagePrice = volumeSum > 0.0 ? weightedPriceSum / volumeSum : point.close;
        }
    }

    *points = selectedPoints;
    if (actualDate) {
        *actualDate = selectedDate;
    }
    return true;
}

QString MarketDataSimulator::sourceName(QuoteSource source)
{
    switch (source) {
    case QuoteSource::EastMoney:
        return QStringLiteral("EastMoney");
    case QuoteSource::Sina:
        return QStringLiteral("Sina");
    case QuoteSource::Tencent:
        return QStringLiteral("Tencent");
    }

    return QStringLiteral("Unknown");
}

QString MarketDataSimulator::trendSourceName(TrendSource source)
{
    switch (source) {
    case TrendSource::EastMoney:
        return QStringLiteral("EastMoney");
    case TrendSource::Sina:
        return QStringLiteral("Sina");
    case TrendSource::Tencent:
        return QStringLiteral("Tencent");
    }

    return QStringLiteral("Unknown");
}

QString MarketDataSimulator::prefixedMarketSymbol(const QString& symbol)
{
    const QString normalized = normalizeSymbol(symbol);
    const int dotIndex = normalized.indexOf(QLatin1Char('.'));
    if (dotIndex <= 0) {
        return QString();
    }

    const QString code = normalized.left(dotIndex);
    const QString exchange = normalized.mid(dotIndex + 1);
    if (code.size() != 6) {
        return QString();
    }

    if (exchange == QStringLiteral("SH")) {
        return QStringLiteral("sh") + code;
    }
    if (exchange == QStringLiteral("SZ")) {
        return QStringLiteral("sz") + code;
    }

    return QString();
}

QString MarketDataSimulator::secIdForSymbol(const QString &symbol)
{
    const QString normalized = normalizeSymbol(symbol);
    const int dotIndex = normalized.indexOf(QLatin1Char('.'));
    if (dotIndex <= 0) {
        return QString();
    }

    const QString code = normalized.left(dotIndex);
    const QString exchange = normalized.mid(dotIndex + 1);
    if (code.size() != 6) {
        return QString();
    }

    if (exchange == QStringLiteral("SH")) {
        return QStringLiteral("1.") + code;
    }
    if (exchange == QStringLiteral("SZ")) {
        return QStringLiteral("0.") + code;
    }

    return QString();
}

double MarketDataSimulator::valueToDouble(const QJsonValue& value)
{
    if (value.isDouble()) {
        return value.toDouble();
    }

    if (value.isString()) {
        QString text = value.toString().trimmed();
        if (text.isEmpty() || text == QStringLiteral("-") || text == QStringLiteral("--")) {
            return 0.0;
        }
        text.remove(QLatin1Char(','));
        bool ok = false;
        const double number = text.toDouble(&ok);
        return ok ? number : 0.0;
    }

    return 0.0;
}

double MarketDataSimulator::textToDouble(QString text)
{
    text = text.trimmed();
    if (text.isEmpty() || text == QStringLiteral("-") || text == QStringLiteral("--")) {
        return 0.0;
    }

    text.remove(QLatin1Char(','));
    bool ok = false;
    const double number = text.toDouble(&ok);
    return ok ? number : 0.0;
}
