package com.example.starquant.data.source

import com.example.starquant.data.model.MarketData
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.withContext
import okhttp3.OkHttpClient
import okhttp3.Request
import java.util.Date
import java.util.concurrent.TimeUnit

class MarketDataRepository {
    private val client = OkHttpClient.Builder()
        .connectTimeout(10, TimeUnit.SECONDS)
        .readTimeout(10, TimeUnit.SECONDS)
        .build()

    private val quoteParser = QuoteParser()
    private val trendParser = TrendParser()

    private val _quoteData = MutableStateFlow<MarketData?>(null)
    val quoteData: StateFlow<MarketData?> = _quoteData.asStateFlow()

    private val _intradayData = MutableStateFlow<List<MarketData>>(emptyList())
    val intradayData: StateFlow<List<MarketData>> = _intradayData.asStateFlow()

    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _errorMessage = MutableStateFlow<String?>(null)
    val errorMessage: StateFlow<String?> = _errorMessage.asStateFlow()

    private var currentSymbol: String = ""
    private var lastPrice: Double = 0.0
    private var lastQuoteData: MarketData? = null

    suspend fun fetchQuote(symbol: String) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        if (normalized.isBlank()) return
        currentSymbol = normalized
        _isLoading.value = true
        _errorMessage.value = null
        val result = fetchQuoteForSymbol(normalized)
        if (result != null) {
            lastPrice = result.close
            lastQuoteData = result
            _quoteData.value = result
        } else {
            _errorMessage.value = "行情获取失败：$normalized"
        }
        _isLoading.value = false
    }

    suspend fun fetchIntradayTrend(symbol: String) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        if (normalized.isBlank()) return
        currentSymbol = normalized
        _isLoading.value = true
        _errorMessage.value = null
        val result = fetchIntradayForSymbol(normalized, lastQuoteData)
        if (result.isNotEmpty()) {
            _intradayData.value = result
            val last = result.last()
            lastPrice = last.close
            if (_quoteData.value == null) _quoteData.value = last
        } else {
            _errorMessage.value = "分时走势获取失败：$normalized"
        }
        _isLoading.value = false
    }

    suspend fun fetchQuoteForSymbol(symbol: String): MarketData? = withContext(Dispatchers.IO) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        val sources = listOf<() -> MarketData?>(
            { fetchEastMoneyQuote(normalized) },
            { fetchSinaQuote(normalized) },
            { fetchTencentQuote(normalized) }
        )
        for (source in sources) {
            runCatching { source() }.getOrNull()?.takeIf { it.close > 0.0 }?.let { return@withContext it }
        }
        null
    }

    suspend fun fetchIntradayForSymbol(
        symbol: String,
        quoteData: MarketData? = null,
        targetDate: Date = SymbolHelper.trendTargetDate()
    ): List<MarketData> = withContext(Dispatchers.IO) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        val quote = quoteData ?: lastQuoteData?.takeIf { it.symbol == normalized }
        val sources = listOf<() -> List<MarketData>>(
            { fetchEastMoneyTrend(normalized, targetDate, quote) },
            { fetchSinaTrend(normalized, targetDate, quote) },
            { fetchTencentTrend(normalized, targetDate, quote) }
        )
        for (source in sources) {
            val data = runCatching { source() }.getOrDefault(emptyList())
            if (data.isNotEmpty()) return@withContext data
        }
        emptyList()
    }

    private fun fetchEastMoneyQuote(symbol: String): MarketData? {
        val secId = SymbolHelper.secIdForSymbol(symbol)
        if (secId.isEmpty()) return null
        val url = "https://push2.eastmoney.com/api/qt/stock/get" +
            "?secid=$secId&ut=fa5fd1943c7b386f172d6893dbfba10b" +
            "&fields=f43,f44,f45,f46,f47,f48,f57,f58,f60,f169,f170,f59"
        val body = get(url) ?: return null
        return quoteParser.parseEastMoneyQuote(body, symbol, lastPrice)
    }

    private fun fetchSinaQuote(symbol: String): MarketData? {
        val prefixed = SymbolHelper.prefixedMarketSymbol(symbol)
        if (prefixed.isEmpty()) return null
        val body = get(
            "https://hq.sinajs.cn/list=$prefixed",
            mapOf("Referer" to "https://finance.sina.com.cn")
        ) ?: return null
        return quoteParser.parseSinaQuote(body, symbol, lastPrice)
    }

    private fun fetchTencentQuote(symbol: String): MarketData? {
        val prefixed = SymbolHelper.prefixedMarketSymbol(symbol)
        if (prefixed.isEmpty()) return null
        val body = get("https://qt.gtimg.cn/q=$prefixed") ?: return null
        return quoteParser.parseTencentQuote(body, symbol, lastPrice)
    }

    private fun fetchEastMoneyTrend(symbol: String, targetDate: Date, quoteData: MarketData?): List<MarketData> {
        val secId = SymbolHelper.secIdForSymbol(symbol)
        if (secId.isEmpty()) return emptyList()
        val url = "https://push2his.eastmoney.com/api/qt/stock/trends2/get" +
            "?secid=$secId&fields1=f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13" +
            "&fields2=f51,f52,f53,f54,f55,f56,f57,f58&iscr=0"
        val body = get(url) ?: return emptyList()
        return trendParser.parseEastMoneyTrend(body, symbol, targetDate, quoteData)
    }

    private fun fetchSinaTrend(symbol: String, targetDate: Date, quoteData: MarketData?): List<MarketData> {
        val prefixed = SymbolHelper.prefixedMarketSymbol(symbol)
        if (prefixed.isEmpty()) return emptyList()
        val url = "https://money.finance.sina.com.cn/quotes_service/api/json_v2.php/CN_MarketData.getKLineData" +
            "?symbol=$prefixed&scale=5&ma=no&datalen=1200"
        val body = get(url, mapOf("Referer" to "https://finance.sina.com.cn")) ?: return emptyList()
        return trendParser.parseSinaTrend(body, symbol, targetDate, quoteData)
    }

    private fun fetchTencentTrend(symbol: String, targetDate: Date, quoteData: MarketData?): List<MarketData> {
        val prefixed = SymbolHelper.prefixedMarketSymbol(symbol)
        if (prefixed.isEmpty()) return emptyList()
        val url = "https://web.ifzq.gtimg.cn/appstock/app/kline/mkline?param=$prefixed,m5,,1200"
        val body = get(url, mapOf("Referer" to "https://stockapp.finance.qq.com")) ?: return emptyList()
        return trendParser.parseTencentTrend(body, symbol, targetDate, quoteData)
    }

    private fun get(url: String, headers: Map<String, String> = emptyMap()): String? {
        val builder = Request.Builder()
            .url(url)
            .header("User-Agent", "Mozilla/5.0")
        headers.forEach { (name, value) -> builder.header(name, value) }
        client.newCall(builder.build()).execute().use { response ->
            if (!response.isSuccessful) return null
            return response.body?.string()
        }
    }

    fun getCurrentSymbol(): String = currentSymbol

    fun getLastPrice(): Double = lastPrice
}
