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
    int closedBarCount() const;
    double fastMA() const;
    double slowMA() const;

private:
    int barPeriodMinutes() const;
    QDateTime barStartFor(const QDateTime& timestamp) const;
    void startBar(const MarketData& data, const QDateTime& barStart);
    void updateCurrentBar(const MarketData& data);
    void processClosedBar(const MarketData& bar);
    void checkTradingSignal(const MarketData& bar);
    void updateStopLossTakeProfit(double price);
    void calculateIndicators();
    int confirmationBars() const;
    int cooldownBars() const;
    double minSpreadPercent() const;
    double minRewardCostMultiple() const;
    bool isLateBuyBlocked(const QDateTime& barTime) const;
    double estimatedRoundTripCostPercent(double price) const;
    void enterCooldown();

private:
    bool m_hasCurrentBar;
    QDateTime m_currentBarStart;
    MarketData m_currentBar;
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
    int m_buyConfirmBars;
    int m_sellConfirmBars;
    int m_cooldownBarsRemaining;
};
