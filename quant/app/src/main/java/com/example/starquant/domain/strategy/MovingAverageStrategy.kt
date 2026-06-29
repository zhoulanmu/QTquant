package com.example.starquant.domain.strategy

import com.example.starquant.data.model.MarketData
import com.example.starquant.data.model.SignalType
import com.example.starquant.domain.indicator.TechnicalIndicators
import java.util.Calendar
import java.util.Date
import kotlin.math.max
import kotlin.math.min

class MovingAverageStrategy : StrategyBase() {
    private var hasCurrentBar: Boolean = false
    private var currentBarStart: Date = Date()
    private var currentBar: MarketData = MarketData()
    private val priceHistory = ArrayDeque<MarketData>()
    private val closePrices = mutableListOf<Double>()
    private val highPrices = mutableListOf<Double>()
    private val lowPrices = mutableListOf<Double>()
    private val volumes = mutableListOf<Double>()

    private var fastMA: Double = 0.0
    private var slowMA: Double = 0.0
    private var rsi: Double = 50.0
    private var macd: Double = 0.0
    private var bollingerUpper: Double = 0.0
    private var bollingerLower: Double = 0.0
    private var buyConfirmBars: Int = 0
    private var sellConfirmBars: Int = 0
    private var cooldownBarsRemaining: Int = 0

    override fun reset() {
        super.reset()
        hasCurrentBar = false
        currentBarStart = Date()
        currentBar = MarketData()
        priceHistory.clear()
        closePrices.clear()
        highPrices.clear()
        lowPrices.clear()
        volumes.clear()
        fastMA = 0.0
        slowMA = 0.0
        rsi = 50.0
        macd = 0.0
        bollingerUpper = 0.0
        bollingerLower = 0.0
        buyConfirmBars = 0
        sellConfirmBars = 0
        cooldownBarsRemaining = 0
    }

    fun closedBarCount(): Int = closePrices.size

    fun getFastMA(): Double = fastMA
    fun getSlowMA(): Double = slowMA
    fun getRSI(): Double = rsi
    fun getMACD(): Double = macd
    fun getBollingerUpper(): Double = bollingerUpper
    fun getBollingerLower(): Double = bollingerLower
    fun getBuyConfirmBars(): Int = buyConfirmBars
    fun getSellConfirmBars(): Int = sellConfirmBars
    fun getCooldownBarsRemaining(): Int = cooldownBarsRemaining
    fun hasPositionState(): Boolean = hasPosition
    fun getEntryPrice(): Double = positionEntryPrice
    fun getStopLossPrice(): Double = positionStopLossPrice
    fun getTakeProfitPrice(): Double = positionTakeProfitPrice

    override fun processMarketData(data: MarketData) {
        if (data.symbol != symbol || data.close <= 0.0) return

        val timestamp = if (data.timestamp.time > 0) data.timestamp else Date()
        val barStart = barStartFor(timestamp)

        if (!hasCurrentBar) {
            startBar(data, barStart)
            return
        }

        if (barStart <= currentBarStart) {
            updateCurrentBar(data)
            return
        }

        processClosedBar(currentBar)
        startBar(data, barStart)
    }

    private fun barPeriodMinutes(): Int {
        val minutes = strategyConfig.doubleMAConfig.barPeriodMinutes
        return if (minutes == 1 || minutes == 3 || minutes == 5) minutes else 5
    }

    private fun barStartFor(timestamp: Date): Date {
        val cal = Calendar.getInstance()
        cal.time = timestamp
        val period = barPeriodMinutes()
        val minute = (cal.get(Calendar.MINUTE) / period) * period
        cal.set(Calendar.MINUTE, minute)
        cal.set(Calendar.SECOND, 0)
        cal.set(Calendar.MILLISECOND, 0)
        return cal.time
    }

    private fun startBar(data: MarketData, barStart: Date) {
        hasCurrentBar = true
        currentBarStart = barStart
        currentBar = data.copy()
        currentBar.timestamp = barStart
        currentBar.open = data.close
        currentBar.high = data.close
        currentBar.low = data.close
        currentBar.close = data.close
    }

    private fun updateCurrentBar(data: MarketData) {
        if (!hasCurrentBar || data.close <= 0.0) return
        currentBar.high = max(currentBar.high, data.close)
        currentBar.low = min(currentBar.low, data.close)
        currentBar.close = data.close
        currentBar.volume = data.volume
        currentBar.turnover = data.turnover
        if (data.averagePrice > 0.0) {
            currentBar.averagePrice = data.averagePrice
        }
        currentBar.tradeCount = data.tradeCount
    }

    private fun processClosedBar(bar: MarketData) {
        priceHistory.addLast(bar)
        closePrices.add(bar.close)
        highPrices.add(bar.high)
        lowPrices.add(bar.low)
        volumes.add(bar.volume)

        val maxHistory = 100
        if (priceHistory.size > maxHistory) {
            priceHistory.removeFirst()
            closePrices.removeAt(0)
            highPrices.removeAt(0)
            lowPrices.removeAt(0)
            volumes.removeAt(0)
        }

        calculateIndicators()

        if (hasPosition) {
            updateStopLossTakeProfit(bar.close)
        }

        if (cooldownBarsRemaining > 0) {
            cooldownBarsRemaining--
        }

        checkTradingSignal(bar)
    }

    private fun calculateIndicators() {
        if (closePrices.size < params.slowMA) {
            fastMA = 0.0
            slowMA = 0.0
            return
        }
        fastMA = TechnicalIndicators.calculateSMA(closePrices, params.fastMA)
        slowMA = TechnicalIndicators.calculateSMA(closePrices, params.slowMA)
        rsi = TechnicalIndicators.calculateRSI(closePrices, 14)
        val macdResult = TechnicalIndicators.calculateMACD(closePrices, 12, 26, 9)
        macd = macdResult.macd
        val bbResult = TechnicalIndicators.calculateBollingerBands(closePrices, 20, 2.0)
        bollingerLower = bbResult.lower
        bollingerUpper = bbResult.upper
    }

    private fun confirmationBars(): Int {
        return strategyConfig.doubleMAConfig.confirmationBars.coerceIn(1, 5)
    }

    private fun cooldownBars(): Int {
        return strategyConfig.doubleMAConfig.cooldownBars.coerceIn(0, 20)
    }

    private fun minSpreadPercent(): Double {
        return strategyConfig.doubleMAConfig.minSpreadPercent.coerceIn(0.0, 5.0)
    }

    private fun minRewardCostMultiple(): Double {
        return strategyConfig.doubleMAConfig.minRewardCostMultiple.coerceIn(0.0, 10.0)
    }

    private fun isLateBuyBlocked(barTime: Date): Boolean {
        if (!strategyConfig.doubleMAConfig.blockLateBuy) return false
        val cal = Calendar.getInstance()
        cal.time = barTime
        return cal.get(Calendar.HOUR_OF_DAY) > 14 ||
                (cal.get(Calendar.HOUR_OF_DAY) == 14 && cal.get(Calendar.MINUTE) >= 45)
    }

    private fun estimatedRoundTripCostPercent(price: Double): Double {
        if (price <= 0.0) return 0.0
        val volume = if (params.lotSize > 0.0) params.lotSize else 100.0
        val amount = price * volume
        if (amount <= 0.0) return 0.0
        val buyCommission = max(amount * 0.0002, 5.0)
        val sellCommission = max(amount * 0.0002, 5.0)
        val buyTransfer = amount * 0.00001
        val sellTransfer = amount * 0.00001
        val stampDuty = amount * 0.001
        return (buyCommission + sellCommission + buyTransfer + sellTransfer + stampDuty) / amount * 100.0
    }

    private fun enterCooldown() {
        buyConfirmBars = 0
        sellConfirmBars = 0
        cooldownBarsRemaining = cooldownBars()
    }

    private fun checkTradingSignal(bar: MarketData) {
        val price = bar.close
        if (closePrices.size <= params.slowMA || price <= 0.0) return

        val previousCloses = closePrices.dropLast(1)
        val prevSlowMA = TechnicalIndicators.calculateSMA(previousCloses, params.slowMA)
        val spreadPercent = if (slowMA > 0.0) (fastMA - slowMA) / slowMA * 100.0 else 0.0
        val spreadEnough = spreadPercent >= minSpreadPercent()
        val trendOk = !strategyConfig.doubleMAConfig.requireSlowMAUp ||
                (prevSlowMA > 0.0 && slowMA >= prevSlowMA && price >= slowMA)
        val costOk = minRewardCostMultiple() <= 0.0 ||
                params.takeProfitPercent >= estimatedRoundTripCostPercent(price) * minRewardCostMultiple()
        val buySetup = fastMA > slowMA &&
                spreadEnough &&
                trendOk &&
                costOk &&
                rsi < 70 &&
                !isLateBuyBlocked(bar.timestamp) &&
                cooldownBarsRemaining <= 0
        val sellSetup = fastMA < slowMA

        if (!hasPosition) {
            sellConfirmBars = 0
            buyConfirmBars = if (buySetup) buyConfirmBars + 1 else 0
            if (buyConfirmBars >= confirmationBars()) {
                hasPosition = true
                positionEntryPrice = price
                entryTime = Date()
                buyConfirmBars = 0
                positionStopLossPrice = positionEntryPrice * (1 - params.stopLossPercent / 100.0)
                positionTakeProfitPrice = positionEntryPrice * (1 + params.takeProfitPercent / 100.0)
                emitSignal(
                    SignalType.BUY, price, params.lotSize,
                    "${barPeriodMinutes()}分钟K线多头确认: 快MA(${params.fastMA})高于慢MA(${params.slowMA})，差值${String.format("%.2f", spreadPercent)}%"
                )
            }
            return
        }

        buyConfirmBars = 0

        if (price <= positionStopLossPrice) {
            hasPosition = false
            enterCooldown()
            emitSignal(
                SignalType.STOP_LOSS, price, params.lotSize,
                "止损触发: 亏损${params.stopLossPercent}%"
            )
            return
        }

        if (price >= positionTakeProfitPrice) {
            hasPosition = false
            enterCooldown()
            emitSignal(
                SignalType.TAKE_PROFIT, price, params.lotSize,
                "止盈触发: 盈利${params.takeProfitPercent}%"
            )
            return
        }

        sellConfirmBars = if (sellSetup) sellConfirmBars + 1 else 0
        if (sellConfirmBars >= confirmationBars()) {
            hasPosition = false
            enterCooldown()
            emitSignal(
                SignalType.SELL, price, params.lotSize,
                "${barPeriodMinutes()}分钟K线空头确认: 快MA(${params.fastMA})低于慢MA(${params.slowMA})"
            )
        }
    }

    private fun updateStopLossTakeProfit(price: Double) {
        val currentProfit = (price - positionEntryPrice) / positionEntryPrice * 100.0
        if (currentProfit >= 3.0) {
            val newSL = positionEntryPrice * (1 + 0.01)
            if (newSL > positionStopLossPrice) {
                positionStopLossPrice = newSL
            }
        }
        if (currentProfit >= 5.0) {
            val newTP = positionEntryPrice * (1 + 0.10)
            if (newTP > positionTakeProfitPrice) {
                positionTakeProfitPrice = newTP
            }
        }
    }
}
