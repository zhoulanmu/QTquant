#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include "../market/marketdata.h"

enum class SignalType {
    NONE = 0,
    BUY = 1,
    SELL = 2,
    STOP_LOSS = 3,
    TAKE_PROFIT = 4
};

struct StrategySignal {
    SignalType type;
    QString symbol;
    double price;
    double volume;
    QDateTime timestamp;
    QString comment;
};

struct StrategyParameters {
    QString symbol;
    int fastMA;
    int slowMA;
    double stopLossPercent;
    double takeProfitPercent;
    double lotSize;
};

class StrategyBase : public QObject
{
    Q_OBJECT

signals:
    void signalGenerated(const StrategySignal& signal);

public:
    explicit StrategyBase(QObject *parent = nullptr);
    virtual ~StrategyBase() = default;

    virtual void setParameters(const StrategyParameters& params);
    virtual void processMarketData(const MarketData& data) = 0;
    virtual void reset();

protected:
    void emitSignal(SignalType type, double price, double volume, const QString& comment = "");

protected:
    StrategyParameters m_params;
    QString m_symbol;
    bool m_hasPosition;
    double m_entryPrice;
    double m_stopLossPrice;
    double m_takeProfitPrice;
    QDateTime m_entryTime;
};
