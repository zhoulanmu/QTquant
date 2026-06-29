#ifndef STOCKSCREENER_H
#define STOCKSCREENER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QDateTime>

enum class ScreenerFilterType {
    PriceRange,
    MarketCap,
    Volume,
    TurnoverRate,
    PE,
    PB,
    ROE,
    RevenueGrowth,
    ProfitGrowth,
    MAUp,
    MACD,
    RSI,
    Breakthrough
};

struct ScreenerFilter {
    ScreenerFilterType type;
    double minValue;
    double maxValue;
    bool enabled;
    QString name;
};

struct StockInfo {
    Q_GADGET
    Q_PROPERTY(QString symbol MEMBER symbol)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(double price MEMBER price)
    Q_PROPERTY(double changePercent MEMBER changePercent)
    Q_PROPERTY(double changeAmount MEMBER changeAmount)
    Q_PROPERTY(double open MEMBER open)
    Q_PROPERTY(double high MEMBER high)
    Q_PROPERTY(double low MEMBER low)
    Q_PROPERTY(double close MEMBER close)
    Q_PROPERTY(double volume MEMBER volume)
    Q_PROPERTY(double turnover MEMBER turnover)
    Q_PROPERTY(double turnoverRate MEMBER turnoverRate)
    Q_PROPERTY(double marketCap MEMBER marketCap)
    Q_PROPERTY(double pe MEMBER pe)
    Q_PROPERTY(double pb MEMBER pb)
    Q_PROPERTY(double roe MEMBER roe)
    Q_PROPERTY(double revenueGrowth MEMBER revenueGrowth)
    Q_PROPERTY(double profitGrowth MEMBER profitGrowth)

public:
    QString symbol;
    QString name;
    double price = 0;
    double changePercent = 0;
    double changeAmount = 0;
    double open = 0;
    double high = 0;
    double low = 0;
    double close = 0;
    double volume = 0;
    double turnover = 0;
    double turnoverRate = 0;
    double marketCap = 0;
    double pe = 0;
    double pb = 0;
    double roe = 0;
    double revenueGrowth = 0;
    double profitGrowth = 0;
    QDateTime updateTime;
};
Q_DECLARE_METATYPE(StockInfo)

class StockScreener : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVector<StockInfo> results READ results NOTIFY resultsChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(int resultCount READ resultCount NOTIFY resultsChanged)
    Q_PROPERTY(QStringList presetStrategies READ presetStrategies CONSTANT)

signals:
    void resultsChanged();
    void isRunningChanged();
    void filterChanged();

public:
    explicit StockScreener(QObject *parent = nullptr);
    ~StockScreener() override;

    Q_INVOKABLE void startScreener();
    Q_INVOKABLE void stopScreener();
    Q_INVOKABLE void resetFilters();
    QVector<StockInfo> results() const { return m_results; }
    bool isRunning() const { return m_isRunning; }
    int resultCount() const { return m_results.size(); }

    Q_INVOKABLE void setPriceRange(double minPrice, double maxPrice);
    Q_INVOKABLE void setMarketCapRange(double min, double max);
    Q_INVOKABLE void setVolumeRange(double min, double max);
    Q_INVOKABLE void setTurnoverRateRange(double min, double max);
    Q_INVOKABLE void setPERange(double min, double max);
    Q_INVOKABLE void setPBRange(double min, double max);
    Q_INVOKABLE void setROERange(double min, double max);
    Q_INVOKABLE void setRevenueGrowthRange(double min, double max);
    Q_INVOKABLE void setProfitGrowthRange(double min, double max);
    Q_INVOKABLE void setMA60UpFilter(bool enabled);
    Q_INVOKABLE void setMACDGoldCrossFilter(bool enabled);
    Q_INVOKABLE void setRSIRange(double min, double max);
    Q_INVOKABLE void setBreakthroughMA(bool enabled);

    QStringList presetStrategies() const;
    Q_INVOKABLE void applyPreset(const QString& name);

private:
    bool passesAllFilters(const StockInfo& stock) const;
    void updateResults();
    void generateMockStocks();
    ScreenerFilter* findFilter(ScreenerFilterType type);

    QList<ScreenerFilter> m_filters;
    QVector<StockInfo> m_results;
    QVector<StockInfo> m_allStocks;
    bool m_isRunning;
};

#endif
