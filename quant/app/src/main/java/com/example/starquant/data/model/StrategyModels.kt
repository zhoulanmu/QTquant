package com.example.starquant.data.model

import java.util.Date

enum class SignalType {
    NONE, BUY, SELL, STOP_LOSS, TAKE_PROFIT
}

enum class StrategyType {
    DoubleMA, ProsperityGrowth
}

data class StrategySignal(
    var type: SignalType = SignalType.NONE,
    var symbol: String = "",
    var price: Double = 0.0,
    var volume: Double = 0.0,
    var timestamp: Date = Date(),
    var comment: String = ""
)

data class DoubleMAConfig(
    var fastMA: Int = 5,
    var slowMA: Int = 20,
    var barPeriodMinutes: Int = 5,
    var minSpreadPercent: Double = 0.25,
    var confirmationBars: Int = 2,
    var cooldownBars: Int = 5,
    var requireSlowMAUp: Boolean = true,
    var blockLateBuy: Boolean = true,
    var minRewardCostMultiple: Double = 2.0
)

data class GrowthBuyConfig(
    var enabledTracks: List<String> = listOf("景气主线"),
    var requireMA60Up: Boolean = true,
    var requireVolumePullback: Boolean = true,
    var requirePullbackToMA20OrMA60: Boolean = true,
    var profitGrowthConfirmed: Boolean = true,
    var orderLandingConfirmed: Boolean = true,
    var noPureConceptConfirmed: Boolean = true,
    var pullbackMinDays: Int = 3,
    var pullbackMaxDays: Int = 5
)

data class RiskConfig(
    var stopLossPercent: Double = 2.0,
    var takeProfitPercent: Double = 5.0,
    var surgeTakeProfitPercent: Double = 25.0,
    var partialTakeProfitEnabled: Boolean = true,
    var breakMA60VolumeStopEnabled: Boolean = true
)

data class PositionConfig(
    var lotSize: Double = 100.0,
    var maxSingleTrackPercent: Double = 20.0,
    var diversifyTrackCount: Int = 3
)

data class StrategyConfig(
    var strategyType: StrategyType = StrategyType.DoubleMA,
    var symbol: String = "600519.SH",
    var doubleMAConfig: DoubleMAConfig = DoubleMAConfig(),
    var growthBuyConfig: GrowthBuyConfig = GrowthBuyConfig(),
    var riskConfig: RiskConfig = RiskConfig(),
    var positionConfig: PositionConfig = PositionConfig()
)

data class StrategyParameters(
    var symbol: String = "",
    var fastMA: Int = 5,
    var slowMA: Int = 20,
    var barPeriodMinutes: Int = 5,
    var stopLossPercent: Double = 2.0,
    var takeProfitPercent: Double = 5.0,
    var lotSize: Double = 100.0
)
