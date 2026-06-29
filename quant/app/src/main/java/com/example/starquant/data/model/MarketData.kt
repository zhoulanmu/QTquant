package com.example.starquant.data.model

import java.util.Date

data class MarketData(
    var symbol: String = "",
    var name: String = "",
    var timestamp: Date = Date(),
    var open: Double = 0.0,
    var high: Double = 0.0,
    var low: Double = 0.0,
    var close: Double = 0.0,
    var volume: Double = 0.0,
    var turnover: Double = 0.0,
    var previousClose: Double = 0.0,
    var averagePrice: Double = 0.0,
    var tradeCount: Int = 0
) {
    val changePercent: Double
        get() = if (previousClose > 0) (close - previousClose) / previousClose * 100.0 else 0.0

    val changeAmount: Double
        get() = close - previousClose
}
