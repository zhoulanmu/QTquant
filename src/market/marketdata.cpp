#include "marketdata.h"
#include <QDateTime>
#include <random>

MarketDataSimulator::MarketDataSimulator(QObject *parent)
    : QObject(parent)
    , m_symbol("000001.SH")
    , m_basePrice(10.0)
    , m_lastPrice(10.0)
    , m_isRunning(false)
    , m_dayCount(1)
    , m_minuteCount(1)
    , m_timer(new QTimer(this))
    , m_randomGenerator(std::random_device{}())
    , m_priceDistribution(-0.02, 0.02)
    , m_volumeDistribution(1000, 5000)
{
    connect(m_timer, &QTimer::timeout, this, &MarketDataSimulator::generateNewData);
    m_timer->setInterval(1000);
}

MarketDataSimulator::~MarketDataSimulator()
{
    stopSimulation();
}

void MarketDataSimulator::startSimulation()
{
    if (m_isRunning) return;
    m_isRunning = true;
    m_timer->start();
}

void MarketDataSimulator::stopSimulation()
{
    m_isRunning = false;
    m_timer->stop();
}

void MarketDataSimulator::setSymbol(const QString &symbol)
{
    m_symbol = symbol;
    m_basePrice = 10.0 + (std::uniform_real_distribution<double>(0, 20)(m_randomGenerator));
    m_lastPrice = m_basePrice;
}

void MarketDataSimulator::generateNewData()
{
    double change = m_priceDistribution(m_randomGenerator);
    double newPrice = m_lastPrice * (1 + change);
    newPrice = qMax(0.01, newPrice);

    double open = m_lastPrice;
    double close = newPrice;
    double high = qMax(open, close) * (1 + std::uniform_real_distribution<double>(0, 0.01)(m_randomGenerator));
    double low = qMin(open, close) * (1 - std::uniform_real_distribution<double>(0, 0.01)(m_randomGenerator));
    int volume = m_volumeDistribution(m_randomGenerator);
    double turnover = close * volume * 100;

    QDateTime timestamp = QDateTime::currentDateTime();

    m_minuteCount++;
    if (m_minuteCount > 240) {
        m_minuteCount = 1;
        m_dayCount++;
    }

    MarketData data;
    data.symbol = m_symbol;
    data.timestamp = timestamp;
    data.open = open;
    data.high = high;
    data.low = low;
    data.close = close;
    data.volume = volume;
    data.turnover = turnover;
    data.tradeCount = volume / 10;

    m_lastPrice = close;

    emit dataUpdated(data);
}
