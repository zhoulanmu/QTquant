#pragma once

#include "strategybase.h"
#include <deque>
#include <vector>

class ProsperityGrowthStrategy : public StrategyBase
{
    Q_OBJECT

public:
    explicit ProsperityGrowthStrategy(QObject *parent = nullptr);
    ~ProsperityGrowthStrategy() override = default;

    void processMarketData(const MarketData& data) override;
    StrategyRuntimeSnapshot runtimeSnapshot() const override;
    void reset() override;

private:
    double simpleMA(int period) const;
    double previousSimpleMA(int period) const;
    double averageVolume(int period) const;
    int recentPullbackCount() const;
    bool externalConditionsConfirmed() const;
    bool hasEnabledTrack() const;
    void appendData(const MarketData& data);
    void checkExitRules(const MarketData& data, double ma60, double avgVolume20);
    void checkEntryRules(const MarketData& data, double ma20, double ma60, double prevMA60, double avgVolume20);

private:
    std::deque<MarketData> m_priceHistory;
    std::vector<double> m_closePrices;
    std::vector<double> m_volumes;
    bool m_partialTaken;
};