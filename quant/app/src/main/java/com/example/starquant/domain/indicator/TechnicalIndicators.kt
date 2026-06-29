package com.example.starquant.domain.indicator

import kotlin.math.abs
import kotlin.math.pow
import kotlin.math.sqrt

object TechnicalIndicators {

    fun calculateSMA(prices: List<Double>, period: Int): Double {
        if (prices.size < period) return 0.0
        var sum = 0.0
        for (i in prices.size - period until prices.size) {
            sum += prices[i]
        }
        return sum / period
    }

    fun calculateEMA(prices: List<Double>, period: Int): Double {
        if (prices.size < period) return 0.0
        val alpha = 2.0 / (period + 1)
        var ema = prices[prices.size - period]
        for (i in prices.size - period + 1 until prices.size) {
            ema = alpha * prices[i] + (1 - alpha) * ema
        }
        return ema
    }

    data class MACDResult(val macd: Double, val signal: Double)

    fun calculateMACD(prices: List<Double>, shortPeriod: Int = 12, longPeriod: Int = 26, signalPeriod: Int = 9): MACDResult {
        val shortEMA = calculateEMA(prices, shortPeriod)
        val longEMA = calculateEMA(prices, longPeriod)
        val macd = shortEMA - longEMA
        return MACDResult(macd, 0.0)
    }

    fun calculateRSI(prices: List<Double>, period: Int = 14): Double {
        if (prices.size < period + 1) return 50.0
        var avgGain = 0.0
        var avgLoss = 0.0
        for (i in prices.size - period until prices.size) {
            val change = prices[i] - prices[i - 1]
            if (change > 0) {
                avgGain += change
            } else {
                avgLoss += abs(change)
            }
        }
        avgGain /= period
        avgLoss /= period
        if (avgLoss == 0.0) return 100.0
        val rs = avgGain / avgLoss
        return 100.0 - (100.0 / (1.0 + rs))
    }

    data class BollingerBandsResult(val lower: Double, val upper: Double, val middle: Double)

    fun calculateBollingerBands(prices: List<Double>, period: Int = 20, stdDev: Double = 2.0): BollingerBandsResult {
        if (prices.size < period) return BollingerBandsResult(0.0, 0.0, 0.0)
        val sma = calculateSMA(prices, period)
        var variance = 0.0
        for (i in prices.size - period until prices.size) {
            variance += (prices[i] - sma).pow(2)
        }
        variance /= period
        val std = sqrt(variance)
        return BollingerBandsResult(sma - stdDev * std, sma + stdDev * std, sma)
    }

    fun calculateATR(highs: List<Double>, lows: List<Double>, closes: List<Double>, period: Int = 14): Double {
        if (highs.size < period || lows.size < period || closes.size < period + 1) return 0.0
        val trueRanges = mutableListOf<Double>()
        for (i in closes.size - period until closes.size) {
            val tr = maxOf(
                highs[i] - lows[i],
                maxOf(abs(highs[i] - closes[i - 1]), abs(lows[i] - closes[i - 1]))
            )
            trueRanges.add(tr)
        }
        return trueRanges.average()
    }

    fun calculateOBV(closes: List<Double>, volumes: List<Double>): Double {
        if (closes.size < 2 || volumes.size < 2) return 0.0
        var obv = 0.0
        for (i in 1 until closes.size) {
            if (closes[i] > closes[i - 1]) {
                obv += volumes[i]
            } else if (closes[i] < closes[i - 1]) {
                obv -= volumes[i]
            }
        }
        return obv
    }

    fun calculateVolumeWeightedMA(prices: List<Double>, volumes: List<Double>, period: Int): Double {
        if (prices.size < period || volumes.size < period) return 0.0
        var sumPriceVolume = 0.0
        var sumVolume = 0.0
        for (i in prices.size - period until prices.size) {
            sumPriceVolume += prices[i] * volumes[i]
            sumVolume += volumes[i]
        }
        return if (sumVolume > 0) sumPriceVolume / sumVolume else 0.0
    }
}
