#pragma once

#include <QObject>
#include <QDateTime>
#include <QJsonValue>
#include <QString>
#include <QTimer>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

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
    int tradeCount;
};

class MarketDataSimulator : public QObject
{
    Q_OBJECT

signals:
    void dataUpdated(const MarketData& data);
    void errorOccurred(const QString& message);

public:
    explicit MarketDataSimulator(QObject *parent = nullptr);
    ~MarketDataSimulator();

    void startSimulation();
    void stopSimulation();
    void setSymbol(const QString& symbol);
    const QString& getSymbol() const { return m_symbol; }
    static QString normalizeSymbol(const QString& symbol);

private slots:
    void generateNewData();
    void onQuoteReplyFinished();

private:
    void fetchLatestQuote();
    QUrl buildQuoteUrl() const;
    bool parseQuoteResponse(const QByteArray& payload, MarketData* data, QString* errorMessage) const;
    static QString secIdForSymbol(const QString& symbol);
    static double valueToDouble(const QJsonValue& value);

private:
    QTimer* m_timer;
    QString m_symbol;
    QString m_secid;
    double m_lastPrice;
    QNetworkAccessManager* m_network;
    QNetworkReply* m_activeReply;
    bool m_isRunning;
};
