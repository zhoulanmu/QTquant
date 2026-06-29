package com.example.starquant.data.source

enum class MarketDataFeedMode {
    QuoteOnly,
    QuoteAndTrend,
    QuoteWhenOpenTrendWhenClosed,
    RealtimeQuoteOnly
}

enum class QuoteSource {
    EastMoney, Sina, Tencent
}

enum class TrendSource {
    EastMoney, Sina, Tencent
}
