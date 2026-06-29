package com.example.starquant.domain.screener

import com.example.starquant.data.model.ScreenerFilter
import com.example.starquant.data.model.ScreenerFilterType
import com.example.starquant.data.model.StockInfo
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlin.random.Random

class StockScreener {
    private val _results = MutableStateFlow<List<StockInfo>>(emptyList())
    val results: StateFlow<List<StockInfo>> = _results.asStateFlow()

    private val _isRunning = MutableStateFlow(false)
    val isRunning: StateFlow<Boolean> = _isRunning.asStateFlow()

    private val _resultCount = MutableStateFlow(0)
    val resultCount: StateFlow<Int> = _resultCount.asStateFlow()

    private val _filters = MutableStateFlow<List<ScreenerFilter>>(emptyList())
    val filters: StateFlow<List<ScreenerFilter>> = _filters.asStateFlow()

    private val activeFilters = mutableListOf<ScreenerFilter>()
    private val allStocks = mutableListOf<StockInfo>()

    private val presetStrategiesList = listOf(
        "强势股选股",
        "低估值蓝筹",
        "成长股选股",
        "超跌反弹",
        "均线多头排列",
        "MACD金叉",
        "突破平台",
        "小市值高成长"
    )

    init {
        val filterDefs = listOf(
            ScreenerFilterType.PriceRange to "价格区间",
            ScreenerFilterType.MarketCap to "市值区间",
            ScreenerFilterType.Volume to "成交量",
            ScreenerFilterType.TurnoverRate to "换手率",
            ScreenerFilterType.PE to "市盈率",
            ScreenerFilterType.PB to "市净率",
            ScreenerFilterType.ROE to "净资产收益率",
            ScreenerFilterType.RevenueGrowth to "营收增长",
            ScreenerFilterType.ProfitGrowth to "利润增长",
            ScreenerFilterType.MAUp to "均线多头",
            ScreenerFilterType.MACD to "MACD金叉",
            ScreenerFilterType.RSI to "RSI区间",
            ScreenerFilterType.Breakthrough to "突破形态"
        )

        for ((type, name) in filterDefs) {
            activeFilters.add(
                ScreenerFilter(
                    type = type,
                    minValue = 0.0,
                    maxValue = 999999.0,
                    enabled = false,
                    name = name
                )
            )
        }

        generateMockStocks()
    }

    fun getFilters(): List<ScreenerFilter> = activeFilters.map { it.copy() }

    fun getPresetStrategies(): List<String> = presetStrategiesList

    suspend fun startScreener() {
        if (_isRunning.value) return
        _isRunning.value = true
        delay(500)
        updateResults()
        _isRunning.value = false
    }

    fun stopScreener() {
        _isRunning.value = false
    }

    fun resetFilters() {
        for (f in activeFilters) {
            f.enabled = false
            f.minValue = 0.0
            f.maxValue = 999999.0
        }
        updateResults()
    }

    private fun findFilter(type: ScreenerFilterType): ScreenerFilter? {
        return activeFilters.find { it.type == type }
    }

    fun setPriceRange(minPrice: Double, maxPrice: Double) {
        findFilter(ScreenerFilterType.PriceRange)?.let {
            it.minValue = minPrice
            it.maxValue = maxPrice
            it.enabled = true
        }
        updateResults()
    }

    fun setMarketCapRange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.MarketCap)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setVolumeRange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.Volume)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setTurnoverRateRange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.TurnoverRate)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setPERange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.PE)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setPBRange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.PB)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setROERange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.ROE)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setRevenueGrowthRange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.RevenueGrowth)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setProfitGrowthRange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.ProfitGrowth)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setMA60UpFilter(enabled: Boolean) {
        findFilter(ScreenerFilterType.MAUp)?.let {
            it.enabled = enabled
        }
        updateResults()
    }

    fun setMACDGoldCrossFilter(enabled: Boolean) {
        findFilter(ScreenerFilterType.MACD)?.let {
            it.enabled = enabled
        }
        updateResults()
    }

    fun setRSIRange(min: Double, max: Double) {
        findFilter(ScreenerFilterType.RSI)?.let {
            it.minValue = min
            it.maxValue = max
            it.enabled = true
        }
        updateResults()
    }

    fun setBreakthroughMA(enabled: Boolean) {
        findFilter(ScreenerFilterType.Breakthrough)?.let {
            it.enabled = enabled
        }
        updateResults()
    }

    fun applyPreset(name: String) {
        resetFilters()

        when (name) {
            "强势股选股" -> {
                setPriceRange(5.0, 200.0)
                setTurnoverRateRange(3.0, 30.0)
                setProfitGrowthRange(20.0, 500.0)
                setMA60UpFilter(true)
                setMACDGoldCrossFilter(true)
            }
            "低估值蓝筹" -> {
                setMarketCapRange(50000000000.0, 999999999999.0)
                setPERange(0.0, 15.0)
                setPBRange(0.0, 2.0)
                setROERange(10.0, 50.0)
            }
            "成长股选股" -> {
                setRevenueGrowthRange(20.0, 500.0)
                setProfitGrowthRange(30.0, 500.0)
                setROERange(15.0, 50.0)
                setMA60UpFilter(true)
            }
            "超跌反弹" -> {
                setPriceRange(2.0, 50.0)
                setRSIRange(0.0, 30.0)
            }
            "均线多头排列" -> {
                setMA60UpFilter(true)
                setPriceRange(5.0, 100.0)
            }
            "MACD金叉" -> {
                setMACDGoldCrossFilter(true)
                setPriceRange(5.0, 100.0)
            }
            "突破平台" -> {
                setBreakthroughMA(true)
                setVolumeRange(50000000.0, 999999999999.0)
                setTurnoverRateRange(3.0, 30.0)
            }
            "小市值高成长" -> {
                setMarketCapRange(1000000000.0, 50000000000.0)
                setProfitGrowthRange(30.0, 500.0)
                setRevenueGrowthRange(20.0, 500.0)
            }
        }
    }

    private fun passesAllFilters(stock: StockInfo): Boolean {
        for (f in activeFilters) {
            if (!f.enabled) continue

            when (f.type) {
                ScreenerFilterType.PriceRange ->
                    if (stock.price < f.minValue || stock.price > f.maxValue) return false
                ScreenerFilterType.MarketCap ->
                    if (stock.marketCap < f.minValue || stock.marketCap > f.maxValue) return false
                ScreenerFilterType.Volume ->
                    if (stock.volume < f.minValue || stock.volume > f.maxValue) return false
                ScreenerFilterType.TurnoverRate ->
                    if (stock.turnoverRate < f.minValue || stock.turnoverRate > f.maxValue) return false
                ScreenerFilterType.PE ->
                    if (stock.pe <= 0 || stock.pe < f.minValue || stock.pe > f.maxValue) return false
                ScreenerFilterType.PB ->
                    if (stock.pb <= 0 || stock.pb < f.minValue || stock.pb > f.maxValue) return false
                ScreenerFilterType.ROE ->
                    if (stock.roe < f.minValue || stock.roe > f.maxValue) return false
                ScreenerFilterType.RevenueGrowth ->
                    if (stock.revenueGrowth < f.minValue || stock.revenueGrowth > f.maxValue) return false
                ScreenerFilterType.ProfitGrowth ->
                    if (stock.profitGrowth < f.minValue || stock.profitGrowth > f.maxValue) return false
                ScreenerFilterType.MAUp ->
                    if (stock.changePercent < 0) return false
                ScreenerFilterType.MACD ->
                    if (stock.changePercent <= 0.5) return false
                ScreenerFilterType.RSI -> {
                    val rsi = 30 + stock.changePercent * 3
                    if (rsi < f.minValue || rsi > f.maxValue) return false
                }
                ScreenerFilterType.Breakthrough ->
                    if (stock.changePercent < 2) return false
            }
        }
        return true
    }

    private fun updateResults() {
        _filters.value = activeFilters.map { it.copy() }
        _results.value = allStocks.filter { passesAllFilters(it) }
        _resultCount.value = _results.value.size
    }

    private fun generateMockStocks() {
        val sampleStocks = listOf(
            "000001.SZ" to "平安银行",
            "000002.SZ" to "万科A",
            "600519.SH" to "贵州茅台",
            "600036.SH" to "招商银行",
            "601318.SH" to "中国平安",
            "000858.SZ" to "五粮液",
            "600276.SH" to "恒瑞医药",
            "000333.SZ" to "美的集团",
            "601166.SH" to "兴业银行",
            "002415.SZ" to "海康威视",
            "002594.SZ" to "比亚迪",
            "300750.SZ" to "宁德时代",
            "600030.SH" to "中信证券",
            "600900.SH" to "长江电力",
            "000568.SZ" to "泸州老窖",
            "600887.SH" to "伊利股份",
            "601899.SH" to "紫金矿业",
            "000651.SZ" to "格力电器",
            "300059.SZ" to "东方财富",
            "002304.SZ" to "洋河股份",
            "603288.SH" to "海天味业",
            "601012.SH" to "隆基绿能",
            "300015.SZ" to "爱尔眼科",
            "002714.SZ" to "牧原股份",
            "600585.SH" to "海螺水泥",
            "000063.SZ" to "中兴通讯",
            "600031.SH" to "三一重工",
            "600009.SH" to "上海机场",
            "002475.SZ" to "立讯精密",
            "300142.SZ" to "沃森生物"
        )

        val random = Random(System.currentTimeMillis())
        for ((symbol, name) in sampleStocks) {
            val stock = StockInfo(
                symbol = symbol,
                name = name,
                price = 10 + random.nextDouble(200.0),
                changePercent = -10.0 + random.nextDouble(20.0),
                open = 0.0,
                high = 0.0,
                low = 0.0,
                close = 0.0,
                volume = 1000000 + random.nextDouble(100000000.0),
                turnover = 0.0,
                turnoverRate = 0.5 + random.nextDouble(15.0),
                marketCap = 10000000000.0 + random.nextDouble(500000000000.0),
                pe = 5 + random.nextDouble(80.0),
                pb = 0.5 + random.nextDouble(15.0),
                roe = -5 + random.nextDouble(40.0),
                revenueGrowth = -20 + random.nextDouble(100.0),
                profitGrowth = -30 + random.nextDouble(150.0)
            )
            stock.changeAmount = stock.price * stock.changePercent / 100.0
            stock.open = stock.price * (0.95 + random.nextDouble(0.1))
            stock.high = stock.price * (1.0 + random.nextDouble(0.05))
            stock.low = stock.price * (0.95 + random.nextDouble(0.05))
            stock.close = stock.price
            stock.turnover = stock.volume * stock.price

            allStocks.add(stock)
        }

        updateResults()
    }
}
