#pragma once

#include "strategybase.h"
#include <deque>
#include <vector>

class MovingAverageStrategy : public StrategyBase
{
    Q_OBJECT

public:
    explicit MovingAverageStrategy(QObject *parent = nullptr);
    ~MovingAverageStrategy() override = default;

    void processMarketData(const MarketData& data) override;
    void reset() override;

private:
    void checkTradingSignal(double price);
    void updateStopLossTakeProfit(double price);
    void calculateIndicators();

private:
    std::deque<MarketData> m_priceHistory;
    std::vector<double> m_closePrices;
    std::vector<double> m_highPrices;
    std::vector<double> m_lowPrices;
    std::vector<double> m_volumes;

    double m_fastMA;
    double m_slowMA;
    double m_rsi;
    double m_macd;
    double m_bollingerUpper;
    double m_bollingerLower;
};
