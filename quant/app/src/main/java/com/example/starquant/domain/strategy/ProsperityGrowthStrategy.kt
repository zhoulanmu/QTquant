package com.example.starquant.domain.strategy

import com.example.starquant.data.model.MarketData
import com.example.starquant.data.model.SignalType
import java.util.ArrayDeque

class ProsperityGrowthStrategy : StrategyBase() {
    companion object {
        private const val MaxHistory = 180
        private const val PullbackTolerance = 0.015
        private const val ShrinkVolumeRatio = 0.90
        private const val BreakVolumeRatio = 1.15
    }

    private val priceHistory = ArrayDeque<MarketData>()
    private val closePrices = mutableListOf<Double>()
    private val volumes = mutableListOf<Double>()
    private var partialTaken: Boolean = false

    override fun reset() {
        super.reset()
        priceHistory.clear()
        closePrices.clear()
        volumes.clear()
        partialTaken = false
    }

    fun hasPositionState(): Boolean = hasPosition
    fun getEntryPrice(): Double = positionEntryPrice
    fun getStopLossPrice(): Double = positionStopLossPrice
    fun getTakeProfitPrice(): Double = positionTakeProfitPrice

    override fun processMarketData(data: MarketData) {
        if (data.symbol != symbol || data.close <= 0.0) return

        appendData(data)
        if (closePrices.size < 60) return

        val ma20 = simpleMA(20)
        val ma60 = simpleMA(60)
        val prevMA60 = previousSimpleMA(60)
        val avgVolume20 = averageVolume(20)

        if (hasPosition) {
            checkExitRules(data, ma60, avgVolume20)
            return
        }

        checkEntryRules(data, ma20, ma60, prevMA60, avgVolume20)
    }

    private fun appendData(data: MarketData) {
        priceHistory.addLast(data)
        closePrices.add(data.close)
        volumes.add(data.volume)

        while (priceHistory.size > MaxHistory) {
            priceHistory.removeFirst()
            closePrices.removeAt(0)
            volumes.removeAt(0)
        }
    }

    private fun simpleMA(period: Int): Double {
        if (period <= 0 || closePrices.size < period) return 0.0
        var sum = 0.0
        for (i in closePrices.size - period until closePrices.size) {
            sum += closePrices[i]
        }
        return sum / period
    }

    private fun previousSimpleMA(period: Int): Double {
        if (period <= 0 || closePrices.size <= period) return 0.0
        var sum = 0.0
        for (i in closePrices.size - period - 1 until closePrices.size - 1) {
            sum += closePrices[i]
        }
        return sum / period
    }

    private fun averageVolume(period: Int): Double {
        if (period <= 0 || volumes.size < period) return 0.0
        var sum = 0.0
        for (i in volumes.size - period until volumes.size) {
            sum += volumes[i]
        }
        return sum / period
    }

    private fun recentPullbackCount(): Int {
        var count = 0
        for (i in closePrices.size - 1 downTo 1) {
            if (closePrices[i] < closePrices[i - 1]) {
                count++
                continue
            }
            break
        }
        return count
    }

    private fun externalConditionsConfirmed(): Boolean {
        val growth = strategyConfig.growthBuyConfig
        return growth.profitGrowthConfirmed &&
                growth.orderLandingConfirmed &&
                growth.noPureConceptConfirmed
    }

    private fun hasEnabledTrack(): Boolean {
        return strategyConfig.growthBuyConfig.enabledTracks.isNotEmpty()
    }

    private fun checkExitRules(data: MarketData, ma60: Double, avgVolume20: Double) {
        val risk = strategyConfig.riskConfig
        val lotSize = strategyConfig.positionConfig.lotSize
        val profitPercent =
            if (positionEntryPrice > 0.0) (data.close / positionEntryPrice - 1.0) * 100.0 else 0.0
        val volumeBreak = avgVolume20 <= 0.0 || data.volume >= avgVolume20 * BreakVolumeRatio

        if (risk.breakMA60VolumeStopEnabled && ma60 > 0.0 && data.close < ma60 && volumeBreak) {
            hasPosition = false
            partialTaken = false
            emitSignal(
                SignalType.STOP_LOSS, data.close, lotSize,
                "景气成长：放量跌破 60 均线，模拟离场"
            )
            return
        }

        if (profitPercent <= -risk.stopLossPercent) {
            hasPosition = false
            partialTaken = false
            emitSignal(
                SignalType.STOP_LOSS, data.close, lotSize,
                "景气成长：达到通用止损线，模拟离场"
            )
            return
        }

        if (profitPercent >= risk.surgeTakeProfitPercent && !partialTaken) {
            partialTaken = risk.partialTakeProfitEnabled
            val sellVolume = if (risk.partialTakeProfitEnabled) lotSize / 2.0 else lotSize
            if (!risk.partialTakeProfitEnabled) {
                hasPosition = false
            }
            emitSignal(
                SignalType.TAKE_PROFIT, data.close, sellVolume,
                "景气成长：短期涨幅触发分批止盈"
            )
            return
        }

        if (profitPercent >= risk.takeProfitPercent && !risk.partialTakeProfitEnabled) {
            hasPosition = false
            emitSignal(
                SignalType.TAKE_PROFIT, data.close, lotSize,
                "景气成长：达到通用止盈线，模拟离场"
            )
        }
    }

    private fun checkEntryRules(
        data: MarketData,
        ma20: Double,
        ma60: Double,
        prevMA60: Double,
        avgVolume20: Double
    ) {
        val growth = strategyConfig.growthBuyConfig
        val ma60Up = !growth.requireMA60Up || (ma60 > 0.0 && prevMA60 > 0.0 && ma60 >= prevMA60)
        val nearMA = !growth.requirePullbackToMA20OrMA60 ||
                (ma20 > 0.0 && data.close <= ma20 * (1.0 + PullbackTolerance)) ||
                (ma60 > 0.0 && data.close <= ma60 * (1.0 + PullbackTolerance))
        val shrinkVolume = !growth.requireVolumePullback ||
                avgVolume20 <= 0.0 ||
                data.volume <= avgVolume20 * ShrinkVolumeRatio
        val pullbackCount = recentPullbackCount()
        val pullbackWindow = pullbackCount >= growth.pullbackMinDays &&
                pullbackCount <= growth.pullbackMaxDays

        if (!hasEnabledTrack() || !externalConditionsConfirmed() || !ma60Up ||
            !nearMA || !shrinkVolume || !pullbackWindow
        ) {
            return
        }

        hasPosition = true
        partialTaken = false
        positionEntryPrice = data.close
        entryTime = data.timestamp
        positionStopLossPrice =
            if (ma60 > 0.0) ma60 else data.close * (1.0 - strategyConfig.riskConfig.stopLossPercent / 100.0)
        positionTakeProfitPrice = data.close * (1.0 + strategyConfig.riskConfig.surgeTakeProfitPercent / 100.0)

        emitSignal(
            SignalType.BUY, data.close, strategyConfig.positionConfig.lotSize,
            "景气成长：人工确认项通过，技术回踩条件触发模拟分批低吸"
        )
    }
}
