#pragma once

#include <QObject>
#include <QDateTime>
#include <QJsonValue>
#include <QString>
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
    void fetchLatestQuote();
    void fetchIntradayTrend();
    QUrl buildQuoteUrl() const;
    QUrl buildTrendUrl() const;
    bool parseQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const;
    bool parseTrendResponse(const QByteArray& payload, QVector<MarketData>* points, QString* errorMessage) const;
    static QString secIdForSymbol(const QString& symbol);
    static double valueToDouble(const QJsonValue& value);

private:
    QTimer* m_timer;
    QString m_symbol;
    QString m_secid;
    double m_lastPrice;
    QDateTime m_lastSuccessfulDataAt;
    QNetworkAccessManager* m_network;
    QNetworkReply* m_activeReply;
    QNetworkReply* m_trendReply;
    MarketDataFeedMode m_feedMode;
    bool m_isRunning;
};
