package com.example.starquant.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.starquant.data.model.ScreenerFilter
import com.example.starquant.data.model.ScreenerFilterType
import com.example.starquant.data.model.SimilarStockAnalysis
import com.example.starquant.data.model.StockInfo
import com.example.starquant.domain.screener.StockScreener
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

class ScreenerViewModel : ViewModel() {
    private val screener = StockScreener()

    private val _results = MutableStateFlow<List<StockInfo>>(emptyList())
    val results: StateFlow<List<StockInfo>> = _results.asStateFlow()

    private val _isRunning = MutableStateFlow(false)
    val isRunning: StateFlow<Boolean> = _isRunning.asStateFlow()

    private val _resultCount = MutableStateFlow(0)
    val resultCount: StateFlow<Int> = _resultCount.asStateFlow()

    private val _filters = MutableStateFlow<List<ScreenerFilter>>(emptyList())
    val filters: StateFlow<List<ScreenerFilter>> = _filters.asStateFlow()

    private val _selectedPreset = MutableStateFlow<String?>(null)
    val selectedPreset: StateFlow<String?> = _selectedPreset.asStateFlow()

    private val _filterSummary = MutableStateFlow("默认（全部股票）")
    val filterSummary: StateFlow<String> = _filterSummary.asStateFlow()

    private val _similarAnalysis = MutableStateFlow<SimilarStockAnalysis?>(null)
    val similarAnalysis: StateFlow<SimilarStockAnalysis?> = _similarAnalysis.asStateFlow()

    private val _similarError = MutableStateFlow<String?>(null)
    val similarError: StateFlow<String?> = _similarError.asStateFlow()

    init {
        viewModelScope.launch {
            screener.results.collect { _results.value = it }
        }
        viewModelScope.launch {
            screener.isRunning.collect { _isRunning.value = it }
        }
        viewModelScope.launch {
            screener.resultCount.collect { _resultCount.value = it }
        }
        viewModelScope.launch {
            screener.filters.collect { currentFilters ->
                _filters.value = currentFilters
                _filterSummary.value = buildFilterSummary(currentFilters)
            }
        }
    }

    fun getFilters(): List<ScreenerFilter> = screener.getFilters()
    fun getPresetStrategies(): List<String> = screener.getPresetStrategies()

    fun startScreener() {
        clearSimilarAnalysis()
        viewModelScope.launch {
            screener.startScreener()
        }
    }

    fun resetFilters() {
        _selectedPreset.value = null
        clearSimilarAnalysis()
        screener.resetFilters()
    }

    fun applyPreset(name: String) {
        clearSimilarAnalysis()
        screener.applyPreset(name)
        _selectedPreset.value = name
    }

    fun analyzeSimilarStocks(query: String) {
        val trimmed = query.trim()
        _selectedPreset.value = null
        if (trimmed.isEmpty()) {
            _similarAnalysis.value = null
            _similarError.value = "请输入股票代码或名称"
            return
        }
        val analysis = screener.analyzeSimilarStocks(trimmed)
        _similarAnalysis.value = analysis
        _similarError.value = if (analysis == null) "未找到匹配股票" else null
    }

    fun setPriceRange(min: Double, max: Double) {
        clearPreset()
        screener.setPriceRange(min, max)
    }

    fun setMarketCapRange(min: Double, max: Double) {
        clearPreset()
        screener.setMarketCapRange(min, max)
    }

    fun setPERange(min: Double, max: Double) {
        clearPreset()
        screener.setPERange(min, max)
    }

    fun setPBRange(min: Double, max: Double) {
        clearPreset()
        screener.setPBRange(min, max)
    }

    fun setROERange(min: Double, max: Double) {
        clearPreset()
        screener.setROERange(min, max)
    }

    fun setTurnoverRateRange(min: Double, max: Double) {
        clearPreset()
        screener.setTurnoverRateRange(min, max)
    }

    fun setRevenueGrowthRange(min: Double, max: Double) {
        clearPreset()
        screener.setRevenueGrowthRange(min, max)
    }

    fun setProfitGrowthRange(min: Double, max: Double) {
        clearPreset()
        screener.setProfitGrowthRange(min, max)
    }

    fun setMA60UpFilter(enabled: Boolean) {
        clearPreset()
        screener.setMA60UpFilter(enabled)
    }

    fun setMACDGoldCrossFilter(enabled: Boolean) {
        clearPreset()
        screener.setMACDGoldCrossFilter(enabled)
    }

    fun setRSIRange(min: Double, max: Double) {
        clearPreset()
        screener.setRSIRange(min, max)
    }

    fun setBreakthroughMA(enabled: Boolean) {
        clearPreset()
        screener.setBreakthroughMA(enabled)
    }

    private fun clearPreset() {
        _selectedPreset.value = null
        clearSimilarAnalysis()
    }

    private fun clearSimilarAnalysis() {
        _similarAnalysis.value = null
        _similarError.value = null
    }

    private fun buildFilterSummary(filters: List<ScreenerFilter>): String {
        val enabled = filters.filter { it.enabled }
        if (enabled.isEmpty()) return "默认（全部股票）"
        return enabled.joinToString(" | ") { filter ->
            when (filter.type) {
                ScreenerFilterType.PriceRange -> "价格 ${format(filter.minValue)}-${format(filter.maxValue)}"
                ScreenerFilterType.MarketCap -> "市值 ${format(filter.minValue / 100000000)}-${format(filter.maxValue / 100000000)}亿"
                ScreenerFilterType.Volume -> "成交量 ${format(filter.minValue / 10000)}-${format(filter.maxValue / 10000)}万"
                ScreenerFilterType.TurnoverRate -> "换手 ${format(filter.minValue)}-${format(filter.maxValue)}%"
                ScreenerFilterType.PE -> "PE ${format(filter.minValue)}-${format(filter.maxValue)}"
                ScreenerFilterType.PB -> "PB ${format(filter.minValue)}-${format(filter.maxValue)}"
                ScreenerFilterType.ROE -> "ROE ${format(filter.minValue)}-${format(filter.maxValue)}%"
                ScreenerFilterType.RevenueGrowth -> "营收增长 ${format(filter.minValue)}-${format(filter.maxValue)}%"
                ScreenerFilterType.ProfitGrowth -> "利润增长 ${format(filter.minValue)}-${format(filter.maxValue)}%"
                ScreenerFilterType.MAUp -> "均线多头 ✓"
                ScreenerFilterType.MACD -> "MACD金叉 ✓"
                ScreenerFilterType.RSI -> "RSI ${format(filter.minValue)}-${format(filter.maxValue)}"
                ScreenerFilterType.Breakthrough -> "突破形态 ✓"
            }
        }
    }

    private fun format(value: Double): String {
        return if (value >= 999999.0) "不限" else String.format("%.0f", value)
    }
}
