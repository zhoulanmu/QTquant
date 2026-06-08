#include "strategybase.h"

StrategyBase::StrategyBase(QObject *parent)
    : QObject(parent)
    , m_hasPosition(false)
    , m_entryPrice(0.0)
    , m_stopLossPrice(0.0)
    , m_takeProfitPrice(0.0)
{
    m_params.symbol = "000001.SH";
    m_params.fastMA = 5;
    m_params.slowMA = 20;
    m_params.stopLossPercent = 2.0;
    m_params.takeProfitPercent = 5.0;
    m_params.lotSize = 100.0;
}

void StrategyBase::setParameters(const StrategyParameters &params)
{
    m_params = params;
    m_symbol = params.symbol;
}

void StrategyBase::reset()
{
    m_hasPosition = false;
    m_entryPrice = 0.0;
    m_stopLossPrice = 0.0;
    m_takeProfitPrice = 0.0;
}

void StrategyBase::emitSignal(SignalType type, double price, double volume, const QString& comment)
{
    StrategySignal signal;
    signal.type = type;
    signal.symbol = m_symbol;
    signal.price = price;
    signal.volume = volume;
    signal.timestamp = QDateTime::currentDateTime();
    signal.comment = comment;

    emit signalGenerated(signal);
}
