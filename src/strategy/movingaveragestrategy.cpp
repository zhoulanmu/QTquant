#include "movingaveragestrategy.h"
#include "../indicator/indicators.h"
#include <QDateTime>
#include <algorithm>

MovingAverageStrategy::MovingAverageStrategy(QObject *parent)
    : StrategyBase(parent)
    , m_fastMA(0.0)
    , m_slowMA(0.0)
    , m_rsi(50.0)
    , m_macd(0.0)
    , m_bollingerUpper(0.0)
    , m_bollingerLower(0.0)
{
}

void MovingAverageStrategy::reset()
{
    StrategyBase::reset();
    m_priceHistory.clear();
    m_closePrices.clear();
    m_highPrices.clear();
    m_lowPrices.clear();
    m_volumes.clear();
    m_fastMA = 0.0;
    m_slowMA = 0.0;
}

void MovingAverageStrategy::processMarketData(const MarketData &data)
{
    if (data.symbol != m_symbol) return;

    m_priceHistory.push_back(data);
    m_closePrices.push_back(data.close);
    m_highPrices.push_back(data.high);
    m_lowPrices.push_back(data.low);
    m_volumes.push_back(data.volume);

    const int maxHistory = 100;
    if (m_priceHistory.size() > maxHistory) {
        m_priceHistory.pop_front();
        m_closePrices.erase(m_closePrices.begin());
        m_highPrices.erase(m_highPrices.begin());
        m_lowPrices.erase(m_lowPrices.begin());
        m_volumes.erase(m_volumes.begin());
    }

    calculateIndicators();

    if (m_hasPosition) {
        updateStopLossTakeProfit(data.close);
    }

    checkTradingSignal(data.close);
}

StrategyRuntimeSnapshot MovingAverageStrategy::runtimeSnapshot() const
{
    StrategyRuntimeSnapshot snapshot;
    snapshot.sampleCount = static_cast<int>(m_closePrices.size());
    snapshot.requiredSamples = std::max(m_params.fastMA, m_params.slowMA);
    snapshot.readyForSignal = snapshot.requiredSamples > 0 && snapshot.sampleCount >= snapshot.requiredSamples;
    snapshot.hasFastSlowMA = snapshot.readyForSignal && m_fastMA > 0.0 && m_slowMA > 0.0;
    snapshot.fastMA = m_fastMA;
    snapshot.slowMA = m_slowMA;
    return snapshot;
}

void MovingAverageStrategy::calculateIndicators()
{
    if (m_closePrices.size() < m_params.slowMA) {
        m_fastMA = 0.0;
        m_slowMA = 0.0;
        return;
    }

    m_fastMA = TechnicalIndicators::calculateSMA(m_closePrices, m_params.fastMA);
    m_slowMA = TechnicalIndicators::calculateSMA(m_closePrices, m_params.slowMA);
    m_rsi = TechnicalIndicators::calculateRSI(m_closePrices, 14);
    auto [macd, signal] = TechnicalIndicators::calculateMACD(m_closePrices, 12, 26, 9);
    m_macd = macd;
    auto [lower, upper] = TechnicalIndicators::calculateBollingerBands(m_closePrices, 20, 2.0);
    m_bollingerLower = lower;
    m_bollingerUpper = upper;
}

void MovingAverageStrategy::checkTradingSignal(double price)
{
    if (m_closePrices.size() < m_params.slowMA) return;

    double prevFastMA = TechnicalIndicators::calculateSMA(
        std::vector<double>(m_closePrices.begin(), m_closePrices.end() - 1), m_params.fastMA);
    double prevSlowMA = TechnicalIndicators::calculateSMA(
        std::vector<double>(m_closePrices.begin(), m_closePrices.end() - 1), m_params.slowMA);

    bool goldenCross = (prevFastMA <= prevSlowMA) && (m_fastMA > m_slowMA);
    bool deathCross = (prevFastMA >= prevSlowMA) && (m_fastMA < m_slowMA);

    if (!m_hasPosition) {
        if (goldenCross && m_rsi < 70) {
            m_hasPosition = true;
            m_entryPrice = price;
            m_entryTime = QDateTime::currentDateTime();

            m_stopLossPrice = m_entryPrice * (1 - m_params.stopLossPercent / 100.0);
            m_takeProfitPrice = m_entryPrice * (1 + m_params.takeProfitPercent / 100.0);

            emitSignal(SignalType::BUY, price, m_params.lotSize,
                       QString("金叉信号: 快速MA(%1)上穿慢速MA(%2)").arg(m_params.fastMA).arg(m_params.slowMA));
        }
    } else {
        if (deathCross) {
            m_hasPosition = false;
            emitSignal(SignalType::SELL, price, m_params.lotSize,
                       QString("死叉信号: 快速MA(%1)下穿慢速MA(%2)").arg(m_params.fastMA).arg(m_params.slowMA));
        } else if (price <= m_stopLossPrice) {
            m_hasPosition = false;
            emitSignal(SignalType::STOP_LOSS, price, m_params.lotSize,
                       QString("止损触发: 亏损%1%").arg(m_params.stopLossPercent));
        } else if (price >= m_takeProfitPrice) {
            m_hasPosition = false;
            emitSignal(SignalType::TAKE_PROFIT, price, m_params.lotSize,
                       QString("止盈触发: 盈利%1%").arg(m_params.takeProfitPercent));
        }
    }
}

void MovingAverageStrategy::updateStopLossTakeProfit(double price)
{
    double currentProfit = (price - m_entryPrice) / m_entryPrice * 100.0;

    if (currentProfit >= 3.0) {
        double newSL = m_entryPrice * (1 + 0.01);
        if (newSL > m_stopLossPrice) {
            m_stopLossPrice = newSL;
        }
    }

    if (currentProfit >= 5.0) {
        double newTP = m_entryPrice * (1 + 0.10);
        if (newTP > m_takeProfitPrice) {
            m_takeProfitPrice = newTP;
        }
    }
}
