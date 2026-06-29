#include "stockscreener.h"
#include <QTimer>
#include <QRandomGenerator>

StockScreener::StockScreener(QObject *parent)
    : QObject(parent)
    , m_isRunning(false)
{
    qRegisterMetaType<StockInfo>("StockInfo");

    QList<QPair<ScreenerFilterType, QString>> filterDefs = {
        {ScreenerFilterType::PriceRange, "价格区间"},
        {ScreenerFilterType::MarketCap, "市值区间"},
        {ScreenerFilterType::Volume, "成交量"},
        {ScreenerFilterType::TurnoverRate, "换手率"},
        {ScreenerFilterType::PE, "市盈率"},
        {ScreenerFilterType::PB, "市净率"},
        {ScreenerFilterType::ROE, "净资产收益率"},
        {ScreenerFilterType::RevenueGrowth, "营收增长"},
        {ScreenerFilterType::ProfitGrowth, "利润增长"},
        {ScreenerFilterType::MAUp, "均线多头"},
        {ScreenerFilterType::MACD, "MACD金叉"},
        {ScreenerFilterType::RSI, "RSI区间"},
        {ScreenerFilterType::Breakthrough, "突破形态"}
    };

    for (const auto& def : filterDefs) {
        ScreenerFilter f;
        f.type = def.first;
        f.minValue = 0;
        f.maxValue = 999999;
        f.enabled = false;
        f.name = def.second;
        m_filters.append(f);
    }

    generateMockStocks();
}

StockScreener::~StockScreener() = default;

ScreenerFilter* StockScreener::findFilter(ScreenerFilterType type)
{
    for (auto& f : m_filters) {
        if (f.type == type) return &f;
    }
    return nullptr;
}

void StockScreener::startScreener()
{
    if (m_isRunning) return;
    m_isRunning = true;
    emit isRunningChanged();

    QTimer::singleShot(500, this, [this]() {
        updateResults();
        m_isRunning = false;
        emit isRunningChanged();
    });
}

void StockScreener::stopScreener()
{
    m_isRunning = false;
    emit isRunningChanged();
}

void StockScreener::resetFilters()
{
    for (auto& f : m_filters) {
        f.enabled = false;
        f.minValue = 0;
        f.maxValue = 999999;
    }
    emit filterChanged();
    updateResults();
}

void StockScreener::setPriceRange(double minPrice, double maxPrice)
{
    if (auto* f = findFilter(ScreenerFilterType::PriceRange)) {
        f->minValue = minPrice;
        f->maxValue = maxPrice;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setMarketCapRange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::MarketCap)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setVolumeRange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::Volume)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setTurnoverRateRange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::TurnoverRate)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setPERange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::PE)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setPBRange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::PB)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setROERange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::ROE)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setRevenueGrowthRange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::RevenueGrowth)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setProfitGrowthRange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::ProfitGrowth)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setMA60UpFilter(bool enabled)
{
    if (auto* f = findFilter(ScreenerFilterType::MAUp)) {
        f->enabled = enabled;
    }
    emit filterChanged();
}

void StockScreener::setMACDGoldCrossFilter(bool enabled)
{
    if (auto* f = findFilter(ScreenerFilterType::MACD)) {
        f->enabled = enabled;
    }
    emit filterChanged();
}

void StockScreener::setRSIRange(double min, double max)
{
    if (auto* f = findFilter(ScreenerFilterType::RSI)) {
        f->minValue = min;
        f->maxValue = max;
        f->enabled = true;
    }
    emit filterChanged();
}

void StockScreener::setBreakthroughMA(bool enabled)
{
    if (auto* f = findFilter(ScreenerFilterType::Breakthrough)) {
        f->enabled = enabled;
    }
    emit filterChanged();
}

QStringList StockScreener::presetStrategies() const
{
    return {
        "强势股选股",
        "低估值蓝筹",
        "成长股选股",
        "超跌反弹",
        "均线多头排列",
        "MACD金叉",
        "突破平台",
        "小市值高成长"
    };
}

void StockScreener::applyPreset(const QString& name)
{
    resetFilters();

    if (name == "强势股选股") {
        setPriceRange(5, 200);
        setTurnoverRateRange(3, 30);
        setProfitGrowthRange(20, 500);
        setMA60UpFilter(true);
        setMACDGoldCrossFilter(true);
    } else if (name == "低估值蓝筹") {
        setMarketCapRange(50000000000, 999999999999);
        setPERange(0, 15);
        setPBRange(0, 2);
        setROERange(10, 50);
    } else if (name == "成长股选股") {
        setRevenueGrowthRange(20, 500);
        setProfitGrowthRange(30, 500);
        setROERange(15, 50);
        setMA60UpFilter(true);
    } else if (name == "超跌反弹") {
        setPriceRange(2, 50);
        setRSIRange(0, 30);
    } else if (name == "均线多头排列") {
        setMA60UpFilter(true);
        setPriceRange(5, 100);
    } else if (name == "MACD金叉") {
        setMACDGoldCrossFilter(true);
        setPriceRange(5, 100);
    } else if (name == "突破平台") {
        setBreakthroughMA(true);
        setVolumeRange(50000000, 999999999999);
        setTurnoverRateRange(3, 30);
    } else if (name == "小市值高成长") {
        setMarketCapRange(1000000000, 50000000000);
        setProfitGrowthRange(30, 500);
        setRevenueGrowthRange(20, 500);
    }

    emit filterChanged();
}

bool StockScreener::passesAllFilters(const StockInfo& stock) const
{
    for (const auto& f : m_filters) {
        if (!f.enabled) continue;

        switch (f.type) {
        case ScreenerFilterType::PriceRange:
            if (stock.price < f.minValue || stock.price > f.maxValue) return false;
            break;
        case ScreenerFilterType::MarketCap:
            if (stock.marketCap < f.minValue || stock.marketCap > f.maxValue) return false;
            break;
        case ScreenerFilterType::Volume:
            if (stock.volume < f.minValue || stock.volume > f.maxValue) return false;
            break;
        case ScreenerFilterType::TurnoverRate:
            if (stock.turnoverRate < f.minValue || stock.turnoverRate > f.maxValue) return false;
            break;
        case ScreenerFilterType::PE:
            if (stock.pe <= 0 || stock.pe < f.minValue || stock.pe > f.maxValue) return false;
            break;
        case ScreenerFilterType::PB:
            if (stock.pb <= 0 || stock.pb < f.minValue || stock.pb > f.maxValue) return false;
            break;
        case ScreenerFilterType::ROE:
            if (stock.roe < f.minValue || stock.roe > f.maxValue) return false;
            break;
        case ScreenerFilterType::RevenueGrowth:
            if (stock.revenueGrowth < f.minValue || stock.revenueGrowth > f.maxValue) return false;
            break;
        case ScreenerFilterType::ProfitGrowth:
            if (stock.profitGrowth < f.minValue || stock.profitGrowth > f.maxValue) return false;
            break;
        case ScreenerFilterType::MAUp:
            if (stock.changePercent < 0) return false;
            break;
        case ScreenerFilterType::MACD:
            if (stock.changePercent <= 0.5) return false;
            break;
        case ScreenerFilterType::RSI: {
            double rsi = 30 + stock.changePercent * 3;
            if (rsi < f.minValue || rsi > f.maxValue) return false;
            break;
        }
        case ScreenerFilterType::Breakthrough:
            if (stock.changePercent < 2) return false;
            break;
        }
    }
    return true;
}

void StockScreener::updateResults()
{
    m_results.clear();
    for (const auto& stock : m_allStocks) {
        if (passesAllFilters(stock)) {
            m_results.append(stock);
        }
    }
    emit resultsChanged();
}

void StockScreener::generateMockStocks()
{
    QVector<QPair<QString, QString>> sampleStocks = {
        {"000001.SZ", "平安银行"},
        {"000002.SZ", "万科A"},
        {"600519.SH", "贵州茅台"},
        {"600036.SH", "招商银行"},
        {"601318.SH", "中国平安"},
        {"000858.SZ", "五粮液"},
        {"600276.SH", "恒瑞医药"},
        {"000333.SZ", "美的集团"},
        {"601166.SH", "兴业银行"},
        {"002415.SZ", "海康威视"},
        {"002594.SZ", "比亚迪"},
        {"300750.SZ", "宁德时代"},
        {"600030.SH", "中信证券"},
        {"600900.SH", "长江电力"},
        {"000568.SZ", "泸州老窖"},
        {"600887.SH", "伊利股份"},
        {"601899.SH", "紫金矿业"},
        {"000651.SZ", "格力电器"},
        {"300059.SZ", "东方财富"},
        {"002304.SZ", "洋河股份"},
        {"603288.SH", "海天味业"},
        {"601012.SH", "隆基绿能"},
        {"300015.SZ", "爱尔眼科"},
        {"002714.SZ", "牧原股份"},
        {"600585.SH", "海螺水泥"},
        {"000063.SZ", "中兴通讯"},
        {"600031.SH", "三一重工"},
        {"600009.SH", "上海机场"},
        {"002475.SZ", "立讯精密"},
        {"300142.SZ", "沃森生物"}
    };

    for (const auto& pair : sampleStocks) {
        StockInfo stock;
        stock.symbol = pair.first;
        stock.name = pair.second;
        stock.price = 10 + QRandomGenerator::global()->bounded(200.0);
        stock.changePercent = -10.0 + QRandomGenerator::global()->bounded(20.0);
        stock.changeAmount = stock.price * stock.changePercent / 100.0;
        stock.open = stock.price * (0.95 + QRandomGenerator::global()->bounded(0.1));
        stock.high = stock.price * (1.0 + QRandomGenerator::global()->bounded(0.05));
        stock.low = stock.price * (0.95 + QRandomGenerator::global()->bounded(0.05));
        stock.close = stock.price;
        stock.volume = 1000000 + QRandomGenerator::global()->bounded(100000000.0);
        stock.turnover = stock.volume * stock.price;
        stock.turnoverRate = 0.5 + QRandomGenerator::global()->bounded(15.0);
        stock.marketCap = 10000000000.0 + QRandomGenerator::global()->bounded(500000000000.0);
        stock.pe = 5 + QRandomGenerator::global()->bounded(80.0);
        stock.pb = 0.5 + QRandomGenerator::global()->bounded(15.0);
        stock.roe = -5 + QRandomGenerator::global()->bounded(40.0);
        stock.revenueGrowth = -20 + QRandomGenerator::global()->bounded(100.0);
        stock.profitGrowth = -30 + QRandomGenerator::global()->bounded(150.0);
        stock.updateTime = QDateTime::currentDateTime();

        m_allStocks.append(stock);
    }

    updateResults();
}
