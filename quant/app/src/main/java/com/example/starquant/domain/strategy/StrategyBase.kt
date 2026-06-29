package com.example.starquant.domain.strategy

import com.example.starquant.data.model.MarketData
import com.example.starquant.data.model.SignalType
import com.example.starquant.data.model.StrategyConfig
import com.example.starquant.data.model.StrategyParameters
import com.example.starquant.data.model.StrategySignal
import java.util.Date

abstract class StrategyBase {
    protected var strategyConfig: StrategyConfig = StrategyConfig()
    protected var params: StrategyParameters = StrategyParameters()
    protected var symbol: String = ""
    protected var hasPosition: Boolean = false
    protected var positionEntryPrice: Double = 0.0
    protected var positionStopLossPrice: Double = 0.0
    protected var positionTakeProfitPrice: Double = 0.0
    protected var entryTime: Date = Date()

    var onSignalGenerated: ((StrategySignal) -> Unit)? = null

    open fun setParameters(params: StrategyParameters) {
        this.params = params
        this.symbol = params.symbol
        strategyConfig.symbol = params.symbol
        strategyConfig.doubleMAConfig.fastMA = params.fastMA
        strategyConfig.doubleMAConfig.slowMA = params.slowMA
        strategyConfig.doubleMAConfig.barPeriodMinutes = params.barPeriodMinutes
        strategyConfig.riskConfig.stopLossPercent = params.stopLossPercent
        strategyConfig.riskConfig.takeProfitPercent = params.takeProfitPercent
        strategyConfig.positionConfig.lotSize = params.lotSize
    }

    open fun setConfig(config: StrategyConfig) {
        this.strategyConfig = config
        this.symbol = config.symbol
        params.symbol = config.symbol
        params.fastMA = config.doubleMAConfig.fastMA
        params.slowMA = config.doubleMAConfig.slowMA
        params.barPeriodMinutes = config.doubleMAConfig.barPeriodMinutes
        params.stopLossPercent = config.riskConfig.stopLossPercent
        params.takeProfitPercent = config.riskConfig.takeProfitPercent
        params.lotSize = config.positionConfig.lotSize
    }

    fun getConfig(): StrategyConfig = strategyConfig

    fun syncPositionState(hasPosition: Boolean, entryPrice: Double) {
        if (!hasPosition) {
            this.hasPosition = false
            this.positionEntryPrice = 0.0
            this.positionStopLossPrice = 0.0
            this.positionTakeProfitPrice = 0.0
            return
        }
        if (this.hasPosition && this.positionEntryPrice > 0.0) {
            return
        }
        this.hasPosition = true
        this.positionEntryPrice = if (entryPrice > 0.0) entryPrice else this.positionEntryPrice
        if (this.positionEntryPrice > 0.0) {
            this.positionStopLossPrice =
                this.positionEntryPrice * (1.0 - strategyConfig.riskConfig.stopLossPercent / 100.0)
            this.positionTakeProfitPrice =
                this.positionEntryPrice * (1.0 + strategyConfig.riskConfig.takeProfitPercent / 100.0)
        }
    }

    open fun reset() {
        hasPosition = false
        positionEntryPrice = 0.0
        positionStopLossPrice = 0.0
        positionTakeProfitPrice = 0.0
    }

    protected fun emitSignal(type: SignalType, price: Double, volume: Double, comment: String = "") {
        val signal = StrategySignal(
            type = type,
            symbol = symbol,
            price = price,
            volume = volume,
            timestamp = Date(),
            comment = comment
        )
        onSignalGenerated?.invoke(signal)
    }

    abstract fun processMarketData(data: MarketData)
}
