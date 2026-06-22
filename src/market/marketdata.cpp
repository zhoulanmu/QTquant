#include "marketdata.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>

namespace {
constexpr int QuoteIntervalMs = 3000;
constexpr int QuoteTimeoutMs = 2500;
constexpr auto QuoteEndpoint = "https://push2.eastmoney.com/api/qt/stock/get";
constexpr auto EastMoneyUt = "fa5fd1943c7b386f172d6893dbfba10b";
constexpr auto EastMoneyFields = "f43,f44,f45,f46,f47,f48,f57,f58,f60,f169,f170";
}

MarketDataSimulator::MarketDataSimulator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_symbol(normalizeSymbol("000001.SH"))
    , m_secid(secIdForSymbol(m_symbol))
    , m_lastPrice(0.0)
    , m_network(new QNetworkAccessManager(this))
    , m_activeReply(nullptr)
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
        fetchLatestQuote();
        if (!m_timer->isActive()) {
            m_timer->start();
        }
        return;
    }

    m_isRunning = true;
    fetchLatestQuote();
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

    if (m_activeReply) {
        disconnect(m_activeReply, nullptr, this, nullptr);
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }

    if (m_isRunning) {
        fetchLatestQuote();
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

void MarketDataSimulator::generateNewData()
{
    fetchLatestQuote();
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

    QNetworkRequest request(buildQuoteUrl());
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 StarQuant/0.1 MarketQuote"));
    request.setRawHeader("Referer", "https://quote.eastmoney.com/");
    request.setTransferTimeout(QuoteTimeoutMs);

    m_activeReply = m_network->get(request);
    connect(m_activeReply, &QNetworkReply::finished, this, &MarketDataSimulator::onQuoteReplyFinished);
}

void MarketDataSimulator::onQuoteReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply == m_activeReply) {
        m_activeReply = nullptr;
    }

    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorText = reply->errorString();
    const QByteArray payload = reply->readAll();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError) {
        emit errorOccurred(QStringLiteral("Market quote request failed: %1").arg(networkErrorText));
        return;
    }

    MarketData data;
    QString parseError;
    if (!parseQuoteResponse(payload, &data, &parseError)) {
        emit errorOccurred(parseError);
        return;
    }

    m_lastPrice = data.close;
    emit dataUpdated(data);
}

QUrl MarketDataSimulator::buildQuoteUrl() const
{
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

bool MarketDataSimulator::parseQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const
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
    data->tradeCount = data->volume > 0.0 ? static_cast<int>(data->volume) : 0;
    return true;
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
