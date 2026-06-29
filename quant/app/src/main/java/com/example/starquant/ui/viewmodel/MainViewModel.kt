package com.example.starquant.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.starquant.data.model.AccountState
import com.example.starquant.data.model.MarketData
import com.example.starquant.data.model.PositionInfo
import com.example.starquant.data.model.SignalType
import com.example.starquant.data.model.StrategyConfig
import com.example.starquant.data.model.StrategySignal
import com.example.starquant.data.model.StrategyType
import com.example.starquant.data.model.TradeRecord
import com.example.starquant.data.source.MarketDataRepository
import com.example.starquant.data.source.SymbolHelper
import com.example.starquant.domain.strategy.MovingAverageStrategy
import com.example.starquant.domain.strategy.ProsperityGrowthStrategy
import com.example.starquant.domain.strategy.StrategyBase
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.atomic.AtomicInteger
import kotlin.math.floor
import kotlin.math.max
import kotlin.math.min

data class LoginState(
    val isLoggedIn: Boolean = false,
    val isGuestMode: Boolean = true,
    val accountHint: String = "",
    val errorMessage: String? = null
)

data class MarketIndexDefinition(
    val symbol: String,
    val name: String
)

data class TradeNotice(
    val id: Int,
    val type: String,
    val symbol: String,
    val name: String,
    val price: Double,
    val volume: Double,
    val amount: Double,
    val fee: Double
)

data class StrategyRuntime(
    val id: Int,
    val config: StrategyConfig,
    val strategy: StrategyBase,
    var running: Boolean = false,
    var signals: MutableList<StrategySignal> = mutableListOf(),
    var job: Job? = null
)

private data class TradeFeeBreakdown(
    val commission: Double = 0.0,
    val stampDuty: Double = 0.0,
    val transferFee: Double = 0.0,
    val total: Double = 0.0
)

class MainViewModel : ViewModel() {
    private companion object {
        const val BoardLotSize = 100.0
        const val CommissionRate = 0.0002
        const val MinCommission = 5.0
        const val StampDutyRate = 0.001
        const val TransferFeeRate = 0.00001
    }

    private val repository = MarketDataRepository()
    private val screenerViewModel = ScreenerViewModel()
    private val runtimeIds = AtomicInteger(1)
    private val tradeNoticeIds = AtomicInteger(1)

    private var quoteJob: Job? = null

    private val _loginState = MutableStateFlow(LoginState())
    val loginState: StateFlow<LoginState> = _loginState.asStateFlow()

    private val _currentSymbol = MutableStateFlow("600519.SH")
    val currentSymbol: StateFlow<String> = _currentSymbol.asStateFlow()

    private val _quoteData = MutableStateFlow<MarketData?>(null)
    val quoteData: StateFlow<MarketData?> = _quoteData.asStateFlow()

    private val _intradayData = MutableStateFlow<List<MarketData>>(emptyList())
    val intradayData: StateFlow<List<MarketData>> = _intradayData.asStateFlow()

    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _errorMessage = MutableStateFlow<String?>(null)
    val errorMessage: StateFlow<String?> = _errorMessage.asStateFlow()

    private val _accountState = MutableStateFlow(AccountState())
    val accountState: StateFlow<AccountState> = _accountState.asStateFlow()

    private val _strategyRuntimes = MutableStateFlow<List<StrategyRuntime>>(emptyList())
    val strategyRuntimes: StateFlow<List<StrategyRuntime>> = _strategyRuntimes.asStateFlow()

    private val _selectedStrategyType = MutableStateFlow(StrategyType.DoubleMA)
    val selectedStrategyType: StateFlow<StrategyType> = _selectedStrategyType.asStateFlow()

    private val _strategyConfig = MutableStateFlow(StrategyConfig())
    val strategyConfig: StateFlow<StrategyConfig> = _strategyConfig.asStateFlow()

    private val _signals = MutableStateFlow<List<StrategySignal>>(emptyList())
    val signals: StateFlow<List<StrategySignal>> = _signals.asStateFlow()

    private val _tradeNotices = MutableStateFlow<List<TradeNotice>>(emptyList())
    val tradeNotices: StateFlow<List<TradeNotice>> = _tradeNotices.asStateFlow()

    val marketIndexDefinitions = listOf(
        MarketIndexDefinition("000001.SH", "上证指数"),
        MarketIndexDefinition("399001.SZ", "深证成指"),
        MarketIndexDefinition("399006.SZ", "创业板指"),
        MarketIndexDefinition("000688.SH", "科创50")
    )

    private val _marketIndexQuotes = MutableStateFlow<Map<String, MarketData>>(emptyMap())
    val marketIndexQuotes: StateFlow<Map<String, MarketData>> = _marketIndexQuotes.asStateFlow()

    private val _favoriteStocks = MutableStateFlow<Map<String, String>>(
        linkedMapOf(
            "600519.SH" to "贵州茅台",
            "000001.SZ" to "平安银行",
            "600036.SH" to "招商银行",
            "002594.SZ" to "比亚迪",
            "300750.SZ" to "宁德时代"
        )
    )
    val favoriteStocks: StateFlow<Map<String, String>> = _favoriteStocks.asStateFlow()

    private val _favoriteQuotes = MutableStateFlow<Map<String, MarketData>>(emptyMap())
    val favoriteQuotes: StateFlow<Map<String, MarketData>> = _favoriteQuotes.asStateFlow()

    init {
        updateStrategyConfig(StrategyConfig())
    }

    fun getFavoriteStocks(): List<Pair<String, String>> = _favoriteStocks.value.toList()

    fun getScreenerViewModel(): ScreenerViewModel = screenerViewModel

    fun dismissTradeNotice(id: Int) {
        _tradeNotices.value = _tradeNotices.value.filterNot { it.id == id }
    }

    fun login(username: String, password: String): Boolean {
        val name = username.trim()
        if (name.isEmpty() || password.isEmpty()) {
            _loginState.value = _loginState.value.copy(errorMessage = "请输入客户端用户名和密码，或使用游客看盘。")
            return false
        }
        _loginState.value = LoginState(
            isLoggedIn = true,
            isGuestMode = false,
            accountHint = name
        )
        _accountState.value = _accountState.value.copy(name = "$name 的模拟账户")
        return true
    }

    fun enterGuestMode() {
        _loginState.value = LoginState(
            isLoggedIn = true,
            isGuestMode = true,
            accountHint = ""
        )
        _accountState.value = _accountState.value.copy(name = "游客模拟账户")
    }

    fun logout() {
        stopQuoteRefresh()
        _strategyRuntimes.value.forEach { it.job?.cancel() }
        _strategyRuntimes.value = emptyList()
        _loginState.value = LoginState()
    }

    fun setSymbol(symbol: String) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        _currentSymbol.value = normalized
        _strategyConfig.value = _strategyConfig.value.copy(symbol = normalized)
    }

    fun setStrategyType(type: StrategyType) {
        _selectedStrategyType.value = type
        _strategyConfig.value = _strategyConfig.value.copy(strategyType = type)
    }

    fun updateStrategyConfig(config: StrategyConfig) {
        val normalized = SymbolHelper.normalizeSymbol(config.symbol)
        val fixed = config.copy(symbol = normalized)
        _strategyConfig.value = fixed
        _selectedStrategyType.value = fixed.strategyType
        _currentSymbol.value = normalized
    }

    fun addToFavorites(symbol: String, name: String) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        if (normalized.isBlank()) return
        val updated = LinkedHashMap(_favoriteStocks.value)
        updated[normalized] = name.ifBlank { normalized }
        _favoriteStocks.value = updated
        fetchQuoteForSymbol(normalized)
    }

    fun removeFromFavorites(symbol: String) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        val updated = LinkedHashMap(_favoriteStocks.value)
        updated.remove(normalized)
        _favoriteStocks.value = updated
        _favoriteQuotes.value = _favoriteQuotes.value.toMutableMap().also { it.remove(normalized) }
    }

    fun toggleFavorite(symbol: String, name: String) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        if (_favoriteStocks.value.containsKey(normalized)) {
            removeFromFavorites(normalized)
        } else {
            addToFavorites(normalized, name)
        }
    }

    fun isFavorite(symbol: String): Boolean {
        return _favoriteStocks.value.containsKey(SymbolHelper.normalizeSymbol(symbol))
    }

    fun fetchQuote() {
        viewModelScope.launch {
            val normalized = _currentSymbol.value
            _isLoading.value = true
            _errorMessage.value = null
            val data = loadQuoteForSymbol(normalized)
            if (data == null) {
                _errorMessage.value = "行情获取失败：$normalized"
            }
            _isLoading.value = false
        }
    }

    fun fetchIntraday() {
        viewModelScope.launch {
            val normalized = _currentSymbol.value
            _isLoading.value = true
            _errorMessage.value = null
            val data = repository.fetchIntradayForSymbol(normalized, _quoteData.value)
            if (data.isNotEmpty()) {
                _intradayData.value = data
                data.lastOrNull()?.let { updatePositionsFromQuote(it) }
            } else {
                _errorMessage.value = "分时走势获取失败：$normalized"
            }
            _isLoading.value = false
        }
    }

    fun fetchQuoteForSymbol(symbol: String) {
        viewModelScope.launch {
            loadQuoteForSymbol(symbol)
        }
    }

    fun refreshFavoriteQuotes() {
        viewModelScope.launch {
            _favoriteStocks.value.keys.forEach { symbol ->
                loadQuoteForSymbol(symbol)
            }
        }
    }

    fun refreshMarketIndexQuotes() {
        viewModelScope.launch {
            val quotes = repository.fetchMarketIndexQuotes(
                marketIndexDefinitions.map { it.symbol to it.name }
            )
            if (quotes.isNotEmpty()) {
                _marketIndexQuotes.value = quotes
            }
        }
    }

    fun startQuoteRefresh() {
        stopQuoteRefresh()
        quoteJob = viewModelScope.launch {
            while (true) {
                loadQuoteForSymbol(_currentSymbol.value)
                delay(3000)
            }
        }
    }

    fun stopQuoteRefresh() {
        quoteJob?.cancel()
        quoteJob = null
    }

    fun startStrategy() {
        val config = _strategyConfig.value.copy(symbol = SymbolHelper.normalizeSymbol(_strategyConfig.value.symbol))
        val strategy = createStrategy(config)
        val runtime = StrategyRuntime(
            id = runtimeIds.getAndIncrement(),
            config = config,
            strategy = strategy,
            running = true
        )

        strategy.onSignalGenerated = { signal ->
            handleStrategySignal(runtime.id, signal)
        }

        runtime.job = viewModelScope.launch {
            primeStrategyWithIntraday(runtime)
            while (runtime.running) {
                val quote = loadQuoteForSymbol(config.symbol)
                if (quote != null) {
                    syncStrategyPosition(runtime)
                    runtime.strategy.processMarketData(quote)
                }
                delay(if (SymbolHelper.isAShareContinuousTradingTime()) 3000 else 10000)
            }
        }

        _strategyRuntimes.value = _strategyRuntimes.value + runtime
    }

    fun stopStrategy(runtimeId: Int) {
        val runtime = _strategyRuntimes.value.find { it.id == runtimeId }
        runtime?.running = false
        runtime?.job?.cancel()
        _strategyRuntimes.value = _strategyRuntimes.value.filter { it.id != runtimeId }
    }

    fun resetAccount() {
        _accountState.value = AccountState(name = _accountState.value.name)
    }

    fun updatePositionsPrice(symbol: String, price: Double) {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        val account = _accountState.value
        val position = account.positions[normalized] ?: return
        val updated = updatePositionMark(position, price, position.todayOpenPrice)
        val positions = account.positions.toMutableMap()
        positions[normalized] = updated
        _accountState.value = account.copy(positions = positions)
    }

    private fun createStrategy(config: StrategyConfig): StrategyBase {
        return when (config.strategyType) {
            StrategyType.DoubleMA -> MovingAverageStrategy()
            StrategyType.ProsperityGrowth -> ProsperityGrowthStrategy()
        }.also { it.setConfig(config) }
    }

    private suspend fun primeStrategyWithIntraday(runtime: StrategyRuntime) {
        val callback = runtime.strategy.onSignalGenerated
        runtime.strategy.onSignalGenerated = null
        syncStrategyPosition(runtime)
        val points = repository.fetchIntradayForSymbol(runtime.config.symbol, _quoteData.value)
        points.takeLast(240).forEach { runtime.strategy.processMarketData(it) }
        runtime.strategy.onSignalGenerated = callback
    }

    private fun syncStrategyPosition(runtime: StrategyRuntime) {
        val position = _accountState.value.positions[runtime.config.symbol]
        runtime.strategy.syncPositionState(
            hasPosition = position != null && position.quantity > 0.0,
            entryPrice = position?.avgCost ?: 0.0
        )
    }

    private fun handleStrategySignal(runtimeId: Int, signal: StrategySignal) {
        val runtime = _strategyRuntimes.value.find { it.id == runtimeId } ?: return
        val normalizedSignal = signal.copy(symbol = SymbolHelper.normalizeSymbol(signal.symbol))
        val updatedSignals = (listOf(normalizedSignal) + runtime.signals).take(50).toMutableList()
        val updatedRuntime = runtime.copy(signals = updatedSignals)
        _strategyRuntimes.value = _strategyRuntimes.value.map {
            if (it.id == runtimeId) updatedRuntime else it
        }
        _signals.value = (listOf(normalizedSignal) + _signals.value).take(100)

        viewModelScope.launch {
            executeOrder(runtime.config, normalizedSignal)
        }
    }

    private suspend fun loadQuoteForSymbol(symbol: String): MarketData? {
        val normalized = SymbolHelper.normalizeSymbol(symbol)
        val data = repository.fetchQuoteForSymbol(normalized) ?: return null
        if (normalized == _currentSymbol.value) {
            _quoteData.value = data
        }
        if (_favoriteStocks.value.containsKey(normalized)) {
            _favoriteQuotes.value = _favoriteQuotes.value + (normalized to data)
        }
        updatePositionsFromQuote(data)
        return data
    }

    private suspend fun executeOrder(config: StrategyConfig, signal: StrategySignal) {
        val account = _accountState.value
        val normalized = SymbolHelper.normalizeSymbol(signal.symbol)
        val price = signal.price
        if (normalized.isBlank() || price <= 0.0) return

        when (signal.type) {
            SignalType.BUY -> executeBuy(account, config, normalized, signal, price)
            SignalType.SELL, SignalType.STOP_LOSS, SignalType.TAKE_PROFIT ->
                executeSell(account, config, normalized, signal, price)
            else -> Unit
        }
    }

    private fun executeBuy(
        account: AccountState,
        config: StrategyConfig,
        symbol: String,
        signal: StrategySignal,
        price: Double
    ) {
        val position = account.positions[symbol]
        val totalAssets = account.currentCash + account.positions.values.sumOf { it.marketValue }
        val maxPercent = config.positionConfig.maxSingleTrackPercent.coerceIn(0.0, 100.0)
        val targetValue = totalAssets * maxPercent / 100.0
        val currentPositionValue = position?.let { it.quantity * price } ?: 0.0
        val budget = min(account.currentCash, max(0.0, targetValue - currentPositionValue))
        val desiredLot = roundDownToBoardLot(
            if (signal.volume > 0.0) signal.volume else config.positionConfig.lotSize
        ).takeIf { it >= BoardLotSize } ?: BoardLotSize
        val affordableLot = affordableBoardLot(price, budget)
        val lotSize = min(desiredLot, affordableLot)
        if (lotSize < BoardLotSize) return

        val amount = price * lotSize
        val fees = calculateStockFees(amount, sell = false)
        val cashOut = amount + fees.total
        if (cashOut > account.currentCash) return

        val positions = account.positions.toMutableMap()
        val updatedPosition = if (position != null) {
            val totalQty = position.quantity + lotSize
            val totalCost = position.avgCost * position.quantity + cashOut
            updatePositionMark(
                position.copy(
                    quantity = totalQty,
                    avgCost = totalCost / totalQty,
                    currentPrice = price
                ),
                price,
                position.todayOpenPrice.takeIf { it > 0.0 } ?: price
            )
        } else {
            updatePositionMark(
                PositionInfo(
                    symbol = symbol,
                    name = _favoriteStocks.value[symbol].orEmpty(),
                    quantity = lotSize,
                    avgCost = cashOut / lotSize,
                    todayOpenPrice = price,
                    currentPrice = price
                ),
                price,
                price
            )
        }
        positions[symbol] = updatedPosition

        val record = TradeRecord(
            time = tradeTime(),
            symbol = symbol,
            type = "买入",
            price = price,
            volume = lotSize,
            amount = amount,
            fee = fees.total
        )

        _accountState.value = account.copy(
            currentCash = account.currentCash - cashOut,
            positions = positions,
            tradeRecords = newTradeRecords(account, record)
        )
        enqueueTradeNotice(record)
    }

    private fun executeSell(
        account: AccountState,
        config: StrategyConfig,
        symbol: String,
        signal: StrategySignal,
        price: Double
    ) {
        val position = account.positions[symbol] ?: return
        if (position.quantity <= 0.0) return

        val requestedLot = when {
            signal.type == SignalType.TAKE_PROFIT &&
                config.strategyType == StrategyType.ProsperityGrowth &&
                config.riskConfig.partialTakeProfitEnabled &&
                position.quantity > BoardLotSize -> roundDownToBoardLot(position.quantity / 2.0)
                    .coerceAtLeast(BoardLotSize)
            signal.volume > 0.0 -> roundDownToBoardLot(signal.volume).coerceAtLeast(BoardLotSize)
            else -> position.quantity
        }
        val lotSize = min(requestedLot, position.quantity)
        if (lotSize <= 0.0) return

        val amount = price * lotSize
        val fees = calculateStockFees(amount, sell = true)
        val cashIn = amount - fees.total
        val remainingQty = position.quantity - lotSize
        val positions = account.positions.toMutableMap()

        if (remainingQty <= 0.0) {
            positions.remove(symbol)
        } else {
            positions[symbol] = updatePositionMark(
                position.copy(
                    quantity = remainingQty,
                    currentPrice = price
                ),
                price,
                position.todayOpenPrice
            )
        }

        val type = when (signal.type) {
            SignalType.STOP_LOSS -> "止损"
            SignalType.TAKE_PROFIT -> "止盈"
            else -> "卖出"
        }
        val record = TradeRecord(
            time = tradeTime(),
            symbol = symbol,
            type = type,
            price = price,
            volume = lotSize,
            amount = amount,
            fee = fees.total
        )

        _accountState.value = account.copy(
            currentCash = account.currentCash + cashIn,
            positions = positions,
            tradeRecords = newTradeRecords(account, record)
        )
        enqueueTradeNotice(record)
    }

    private fun updatePositionsFromQuote(data: MarketData) {
        val normalized = SymbolHelper.normalizeSymbol(data.symbol)
        val account = _accountState.value
        val position = account.positions[normalized] ?: return
        val openPrice = data.open.takeIf { it > 0.0 }
            ?: position.todayOpenPrice.takeIf { it > 0.0 }
            ?: position.avgCost
        val updated = updatePositionMark(
            position.copy(
                name = data.name.ifBlank { position.name },
                currentPrice = data.close
            ),
            data.close,
            openPrice
        )
        val positions = account.positions.toMutableMap()
        positions[normalized] = updated
        _accountState.value = account.copy(positions = positions)
    }

    private fun updatePositionMark(position: PositionInfo, price: Double, todayOpenPrice: Double): PositionInfo {
        val marketValue = position.quantity * price
        val profit = (price - position.avgCost) * position.quantity
        val profitPercent = if (position.avgCost > 0.0) (price - position.avgCost) / position.avgCost * 100.0 else 0.0
        return position.copy(
            todayOpenPrice = todayOpenPrice,
            currentPrice = price,
            marketValue = marketValue,
            profit = profit,
            profitPercent = profitPercent
        )
    }

    private fun newTradeRecords(account: AccountState, record: TradeRecord): MutableList<TradeRecord> {
        return (listOf(record) + account.tradeRecords).take(500).toMutableList()
    }

    private fun enqueueTradeNotice(record: TradeRecord) {
        val notice = TradeNotice(
            id = tradeNoticeIds.getAndIncrement(),
            type = record.type,
            symbol = record.symbol,
            name = displayNameForSymbol(record.symbol),
            price = record.price,
            volume = record.volume,
            amount = record.amount,
            fee = record.fee
        )
        _tradeNotices.value = (_tradeNotices.value + notice).takeLast(8)
    }

    private fun displayNameForSymbol(symbol: String): String {
        return _favoriteStocks.value[symbol]
            ?: _favoriteQuotes.value[symbol]?.name
            ?: _quoteData.value?.takeIf { SymbolHelper.normalizeSymbol(it.symbol) == symbol }?.name
            ?: _accountState.value.positions[symbol]?.name
            ?: ""
    }

    private fun roundDownToBoardLot(shares: Double): Double {
        if (shares <= 0.0) return 0.0
        return floor(shares / BoardLotSize) * BoardLotSize
    }

    private fun affordableBoardLot(price: Double, budget: Double): Double {
        if (price <= 0.0 || budget <= 0.0) return 0.0
        var lotSize = roundDownToBoardLot(budget / price)
        while (lotSize >= BoardLotSize) {
            val amount = price * lotSize
            if (amount + calculateStockFees(amount, sell = false).total <= budget) {
                return lotSize
            }
            lotSize -= BoardLotSize
        }
        return 0.0
    }

    private fun calculateStockFees(amount: Double, sell: Boolean): TradeFeeBreakdown {
        if (amount <= 0.0) return TradeFeeBreakdown()
        val commission = max(amount * CommissionRate, MinCommission)
        val stampDuty = if (sell) amount * StampDutyRate else 0.0
        val transferFee = amount * TransferFeeRate
        return TradeFeeBreakdown(
            commission = commission,
            stampDuty = stampDuty,
            transferFee = transferFee,
            total = commission + stampDuty + transferFee
        )
    }

    private fun tradeTime(): String {
        return SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).format(Date())
    }
}
