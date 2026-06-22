#include "prosperitygrowthstrategy.h"

#include <algorithm>
#include <numeric>

namespace {
constexpr int MaxHistory = 180;
constexpr double PullbackTolerance = 0.015;
constexpr double ShrinkVolumeRatio = 0.90;
constexpr double BreakVolumeRatio = 1.15;
}

ProsperityGrowthStrategy::ProsperityGrowthStrategy(QObject *parent)
    : StrategyBase(parent)
    , m_partialTaken(false)
{
}

void ProsperityGrowthStrategy::reset()
{
    StrategyBase::reset();
    m_priceHistory.clear();
    m_closePrices.clear();
    m_volumes.clear();
    m_partialTaken = false;
}

void ProsperityGrowthStrategy::processMarketData(const MarketData& data)
{
    if (data.symbol != m_symbol || data.close <= 0.0) {
        return;
    }

    appendData(data);
    if (m_closePrices.size() < 60) {
        return;
    }

    const double ma20 = simpleMA(20);
    const double ma60 = simpleMA(60);
    const double prevMA60 = previousSimpleMA(60);
    const double avgVolume20 = averageVolume(20);

    if (m_hasPosition) {
        checkExitRules(data, ma60, avgVolume20);
        return;
    }

    checkEntryRules(data, ma20, ma60, prevMA60, avgVolume20);
}

void ProsperityGrowthStrategy::appendData(const MarketData& data)
{
    m_priceHistory.push_back(data);
    m_closePrices.push_back(data.close);
    m_volumes.push_back(data.volume);

    while (m_priceHistory.size() > MaxHistory) {
        m_priceHistory.pop_front();
        m_closePrices.erase(m_closePrices.begin());
        m_volumes.erase(m_volumes.begin());
    }
}

double ProsperityGrowthStrategy::simpleMA(int period) const
{
    if (period <= 0 || m_closePrices.size() < static_cast<size_t>(period)) {
        return 0.0;
    }

    return std::accumulate(m_closePrices.end() - period, m_closePrices.end(), 0.0) / period;
}

double ProsperityGrowthStrategy::previousSimpleMA(int period) const
{
    if (period <= 0 || m_closePrices.size() <= static_cast<size_t>(period)) {
        return 0.0;
    }

    return std::accumulate(m_closePrices.end() - period - 1, m_closePrices.end() - 1, 0.0) / period;
}

double ProsperityGrowthStrategy::averageVolume(int period) const
{
    if (period <= 0 || m_volumes.size() < static_cast<size_t>(period)) {
        return 0.0;
    }

    return std::accumulate(m_volumes.end() - period, m_volumes.end(), 0.0) / period;
}

int ProsperityGrowthStrategy::recentPullbackCount() const
{
    int count = 0;
    for (int i = static_cast<int>(m_closePrices.size()) - 1; i > 0; --i) {
        if (m_closePrices[static_cast<size_t>(i)] < m_closePrices[static_cast<size_t>(i - 1)]) {
            ++count;
            continue;
        }
        break;
    }
    return count;
}

bool ProsperityGrowthStrategy::externalConditionsConfirmed() const
{
    const GrowthBuyConfig& growth = m_config.growthBuyConfig;
    return growth.profitGrowthConfirmed
        && growth.orderLandingConfirmed
        && growth.noPureConceptConfirmed;
}

bool ProsperityGrowthStrategy::hasEnabledTrack() const
{
    return !m_config.growthBuyConfig.enabledTracks.isEmpty();
}

void ProsperityGrowthStrategy::checkExitRules(const MarketData& data, double ma60, double avgVolume20)
{
    const RiskConfig& risk = m_config.riskConfig;
    const double lotSize = m_config.positionConfig.lotSize;
    const double profitPercent = m_entryPrice > 0.0 ? (data.close / m_entryPrice - 1.0) * 100.0 : 0.0;
    const bool volumeBreak = avgVolume20 <= 0.0 || data.volume >= avgVolume20 * BreakVolumeRatio;

    if (risk.breakMA60VolumeStopEnabled && ma60 > 0.0 && data.close < ma60 && volumeBreak) {
        m_hasPosition = false;
        m_partialTaken = false;
        emitSignal(SignalType::STOP_LOSS, data.close, lotSize,
                   QStringLiteral("景气成长：放量跌破 60 均线，模拟离场"));
        return;
    }

    if (profitPercent <= -risk.stopLossPercent) {
        m_hasPosition = false;
        m_partialTaken = false;
        emitSignal(SignalType::STOP_LOSS, data.close, lotSize,
                   QStringLiteral("景气成长：达到通用止损线，模拟离场"));
        return;
    }

    if (profitPercent >= risk.surgeTakeProfitPercent && !m_partialTaken) {
        m_partialTaken = risk.partialTakeProfitEnabled;
        const double sellVolume = risk.partialTakeProfitEnabled ? lotSize / 2.0 : lotSize;
        if (!risk.partialTakeProfitEnabled) {
            m_hasPosition = false;
        }
        emitSignal(SignalType::TAKE_PROFIT, data.close, sellVolume,
                   QStringLiteral("景气成长：短期涨幅触发分批止盈"));
        return;
    }

    if (profitPercent >= risk.takeProfitPercent && !risk.partialTakeProfitEnabled) {
        m_hasPosition = false;
        emitSignal(SignalType::TAKE_PROFIT, data.close, lotSize,
                   QStringLiteral("景气成长：达到通用止盈线，模拟离场"));
    }
}

void ProsperityGrowthStrategy::checkEntryRules(const MarketData& data, double ma20, double ma60, double prevMA60, double avgVolume20)
{
    const GrowthBuyConfig& growth = m_config.growthBuyConfig;
    const bool ma60Up = !growth.requireMA60Up || (ma60 > 0.0 && prevMA60 > 0.0 && ma60 >= prevMA60);
    const bool nearMA = !growth.requirePullbackToMA20OrMA60
        || (ma20 > 0.0 && data.close <= ma20 * (1.0 + PullbackTolerance))
        || (ma60 > 0.0 && data.close <= ma60 * (1.0 + PullbackTolerance));
    const bool shrinkVolume = !growth.requireVolumePullback
        || avgVolume20 <= 0.0
        || data.volume <= avgVolume20 * ShrinkVolumeRatio;
    const int pullbackCount = recentPullbackCount();
    const bool pullbackWindow = pullbackCount >= growth.pullbackMinDays
        && pullbackCount <= growth.pullbackMaxDays;

    if (!hasEnabledTrack() || !externalConditionsConfirmed() || !ma60Up || !nearMA || !shrinkVolume || !pullbackWindow) {
        return;
    }

    m_hasPosition = true;
    m_partialTaken = false;
    m_entryPrice = data.close;
    m_entryTime = data.timestamp;
    m_stopLossPrice = ma60 > 0.0 ? ma60 : data.close * (1.0 - m_config.riskConfig.stopLossPercent / 100.0);
    m_takeProfitPrice = data.close * (1.0 + m_config.riskConfig.surgeTakeProfitPercent / 100.0);

    emitSignal(SignalType::BUY, data.close, m_config.positionConfig.lotSize,
               QStringLiteral("景气成长：人工确认项通过，技术回踩条件触发模拟分批低吸"));
}