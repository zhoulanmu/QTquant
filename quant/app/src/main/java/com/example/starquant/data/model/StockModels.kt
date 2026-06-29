package com.example.starquant.data.model

import java.util.Date

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
}

data class ScreenerFilter(
    var type: ScreenerFilterType,
    var minValue: Double = 0.0,
    var maxValue: Double = 999999.0,
    var enabled: Boolean = false,
    var name: String = ""
)

data class StockInfo(
    var symbol: String = "",
    var name: String = "",
    var price: Double = 0.0,
    var changePercent: Double = 0.0,
    var changeAmount: Double = 0.0,
    var open: Double = 0.0,
    var high: Double = 0.0,
    var low: Double = 0.0,
    var close: Double = 0.0,
    var volume: Double = 0.0,
    var turnover: Double = 0.0,
    var turnoverRate: Double = 0.0,
    var marketCap: Double = 0.0,
    var pe: Double = 0.0,
    var pb: Double = 0.0,
    var roe: Double = 0.0,
    var revenueGrowth: Double = 0.0,
    var profitGrowth: Double = 0.0,
    var updateTime: Date = Date()
)

data class StockFeatureTag(
    val label: String,
    val detail: String
)

data class SimilarStockAnalysis(
    val seed: StockInfo,
    val features: List<StockFeatureTag>,
    val results: List<StockInfo>
)

data class PositionInfo(
    var symbol: String = "",
    var name: String = "",
    var quantity: Double = 0.0,
    var avgCost: Double = 0.0,
    var todayOpenPrice: Double = 0.0,
    var currentPrice: Double = 0.0,
    var marketValue: Double = 0.0,
    var profit: Double = 0.0,
    var profitPercent: Double = 0.0
)

data class TradeRecord(
    var time: String = "",
    var symbol: String = "",
    var type: String = "",
    var price: Double = 0.0,
    var volume: Double = 0.0,
    var amount: Double = 0.0,
    var fee: Double = 0.0
)

data class AccountState(
    var name: String = "默认账户",
    var initialCapital: Double = 100000.0,
    var currentCash: Double = 100000.0,
    var positions: MutableMap<String, PositionInfo> = mutableMapOf(),
    var tradeRecords: MutableList<TradeRecord> = mutableListOf()
) {
    val totalAssets: Double
        get() = currentCash + positions.values.sumOf { it.marketValue }

    val totalProfit: Double
        get() = totalAssets - initialCapital

    val totalProfitPercent: Double
        get() = if (initialCapital > 0) totalProfit / initialCapital * 100.0 else 0.0
}
