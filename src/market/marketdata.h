#pragma once

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <deque>
#include <random>

struct MarketData {
    MarketData()
        : symbol("")
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

public:
    explicit MarketDataSimulator(QObject *parent = nullptr);
    ~MarketDataSimulator();

    void startSimulation();
    void stopSimulation();
    void setSymbol(const QString& symbol);
    const QString& getSymbol() const { return m_symbol; }

private slots:
    void generateNewData();

private:
    QTimer* m_timer;
    QString m_symbol;
    double m_basePrice;
    double m_lastPrice;
    std::mt19937 m_randomGenerator;
    std::uniform_real_distribution<double> m_priceDistribution;
    std::uniform_int_distribution<int> m_volumeDistribution;
    bool m_isRunning;
    int m_dayCount;
    int m_minuteCount;
};
