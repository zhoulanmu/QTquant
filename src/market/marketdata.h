#pragma once

#include <QObject>
#include <QDateTime>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVector>

class QNetworkAccessManager;
class QNetworkReply;

enum class MarketDataFeedMode {
    QuoteOnly,
    QuoteAndTrend,
    QuoteWhenOpenTrendWhenClosed,
    RealtimeQuoteOnly
};

struct MarketData {
    MarketData()
        : symbol("")
        , name("")
        , timestamp(QDateTime())
        , open(0.0)
        , high(0.0)
        , low(0.0)
        , close(0.0)
        , volume(0.0)
        , turnover(0.0)
        , previousClose(0.0)
        , averagePrice(0.0)
        , tradeCount(0)
    {}

    QString symbol;
    QString name;
    QDateTime timestamp;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double turnover;
    double previousClose;
    double averagePrice;
    int tradeCount;
};

class MarketDataSimulator : public QObject
{
    Q_OBJECT

signals:
    void dataUpdated(const MarketData& data);
    void intradayDataUpdated(const QVector<MarketData>& data);
    void fallbackIntradayDataUsed(const QString& symbol, const QString& reason);
    void errorOccurred(const QString& message);

public:
    explicit MarketDataSimulator(QObject *parent = nullptr);
    ~MarketDataSimulator();

    void startSimulation();
    void stopSimulation();
    void setSymbol(const QString& symbol);
    void setFeedMode(MarketDataFeedMode mode);
    const QString& getSymbol() const { return m_symbol; }
    static QString normalizeSymbol(const QString& symbol);
    static bool isAShareContinuousTradingTime(const QDateTime& now = QDateTime());

private slots:
    void generateNewData();
    void onQuoteReplyFinished();
    void onTrendReplyFinished();

private:
    enum class QuoteSource {
        EastMoney,
        Sina,
        Tencent
    };

    void fetchLatestQuote();
    void fetchQuote(QuoteSource source);
    bool fetchNextQuoteSource(QuoteSource source, const QString& failureReason);
    void fetchIntradayTrend();
    QUrl buildQuoteUrl(QuoteSource source) const;
    QUrl buildTrendUrl() const;
    bool emitQuoteFallbackTrend(const QString& reason);
    bool parseQuoteResponse(QuoteSource source, const QByteArray& payload, MarketData* data, QString* errorMessage) const;
    bool parseEastMoneyQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const;
    bool parseSinaQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const;
    bool parseTencentQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const;
    bool parseTrendResponse(const QByteArray& payload, QVector<MarketData>* points, QString* errorMessage) const;
    static QString sourceName(QuoteSource source);
    static QString prefixedMarketSymbol(const QString& symbol);
    static QString secIdForSymbol(const QString& symbol);
    static double valueToDouble(const QJsonValue& value);
    static double textToDouble(QString text);

private:
    QTimer* m_timer;
    QString m_symbol;
    QString m_secid;
    double m_lastPrice;
    QDateTime m_lastSuccessfulDataAt;
    QNetworkAccessManager* m_network;
    QNetworkReply* m_activeReply;
    QNetworkReply* m_trendReply;
    QuoteSource m_activeQuoteSource;
    QStringList m_quoteFailureReasons;
    MarketData m_lastQuoteData;
    bool m_hasLastQuoteData;
    QDateTime m_lastTrendFetchAt;
    QString m_pendingTrendFallbackReason;
    bool m_isUsingQuoteFallbackTrend;
    MarketDataFeedMode m_feedMode;
    bool m_isRunning;
};
