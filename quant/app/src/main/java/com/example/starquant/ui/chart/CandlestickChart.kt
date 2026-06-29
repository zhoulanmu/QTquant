package com.example.starquant.ui.chart

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.drawText
import androidx.compose.ui.text.rememberTextMeasurer
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.starquant.data.model.MarketData
import com.example.starquant.domain.indicator.TechnicalIndicators
import java.text.SimpleDateFormat
import java.util.Locale
import kotlin.math.abs

data class ChartData(
    val klines: List<MarketData> = emptyList(),
    val showMA5: Boolean = true,
    val showMA10: Boolean = true,
    val showMA20: Boolean = true,
    val showMA60: Boolean = false,
    val showVolume: Boolean = true
)

@Composable
fun CandlestickChart(
    data: ChartData,
    modifier: Modifier = Modifier
) {
    val textMeasurer = rememberTextMeasurer()
    val upColor = Color(0xFFE53935)
    val downColor = Color(0xFF43A047)
    val gridColor = Color(0xFFE0E0E0)
    val textColor = Color(0xFF757575)

    Column(modifier = modifier) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(250.dp)
                .background(Color.White)
        ) {
            Canvas(modifier = Modifier.fillMaxSize().padding(start = 45.dp, end = 10.dp, top = 10.dp, bottom = 10.dp)) {
                val klines = data.klines
                if (klines.isEmpty()) return@Canvas

                val count = klines.size.coerceAtMost(120)
                val visibleKlines = if (klines.size > count) klines.takeLast(count) else klines

                val width = size.width
                val height = size.height
                val candleWidth = (width / count) * 0.7f
                val gap = (width / count) * 0.3f

                var maxPrice = Double.MIN_VALUE
                var minPrice = Double.MAX_VALUE
                for (k in visibleKlines) {
                    if (k.high > maxPrice) maxPrice = k.high
                    if (k.low < minPrice) minPrice = k.low
                }

                val priceRange = maxPrice - minPrice
                if (priceRange <= 0) return@Canvas

                val priceTop = maxPrice + priceRange * 0.05
                val priceBottom = minPrice - priceRange * 0.05
                val priceTotalRange = priceTop - priceBottom

                for (i in 0..4) {
                    val y = height * i / 4f
                    drawLine(
                        color = gridColor,
                        start = Offset(0f, y),
                        end = Offset(width, y),
                        strokeWidth = 0.5f
                    )
                    val price = priceTop - priceTotalRange * i / 4f
                    val priceText = String.format("%.2f", price)
                    val textLayoutResult = textMeasurer.measure(
                        text = priceText,
                        style = TextStyle(fontSize = 10.sp, color = textColor)
                    )
                    drawText(
                        textLayoutResult = textLayoutResult,
                        topLeft = Offset(-40f, y - textLayoutResult.size.height / 2f)
                    )
                }

                val ma5Prices = visibleKlines.map { it.close }
                val ma10Prices = visibleKlines.map { it.close }
                val ma20Prices = visibleKlines.map { it.close }

                for ((index, kline) in visibleKlines.withIndex()) {
                    val x = index * (candleWidth + gap) + gap / 2f
                    val openY = height - ((kline.open - priceBottom) / priceTotalRange * height).toFloat()
                    val closeY = height - ((kline.close - priceBottom) / priceTotalRange * height).toFloat()
                    val highY = height - ((kline.high - priceBottom) / priceTotalRange * height).toFloat()
                    val lowY = height - ((kline.low - priceBottom) / priceTotalRange * height).toFloat()

                    val isUp = kline.close >= kline.open
                    val color = if (isUp) upColor else downColor

                    drawLine(
                        color = color,
                        start = Offset(x + candleWidth / 2, highY),
                        end = Offset(x + candleWidth / 2, lowY),
                        strokeWidth = 1f
                    )

                    val bodyTop = minOf(openY, closeY)
                    val bodyHeight = abs(openY - closeY).coerceAtLeast(1f)
                    drawRect(
                        color = color,
                        topLeft = Offset(x, bodyTop),
                        size = androidx.compose.ui.geometry.Size(candleWidth, bodyHeight)
                    )
                }

                fun drawMALine(period: Int, color: Color) {
                    if (visibleKlines.size < period) return
                    val path = Path()
                    for (i in period - 1 until visibleKlines.size) {
                        val prices = visibleKlines.subList(i - period + 1, i + 1).map { it.close }
                        val ma = TechnicalIndicators.calculateSMA(prices, period)
                        val x = i * (candleWidth + gap) + candleWidth / 2f + gap / 2f
                        val y = height - ((ma - priceBottom) / priceTotalRange * height).toFloat()
                        if (i == period - 1) {
                            path.moveTo(x, y)
                        } else {
                            path.lineTo(x, y)
                        }
                    }
                    drawPath(path = path, color = color, style = Stroke(width = 1f))
                }

                if (data.showMA5) drawMALine(5, Color(0xFFFFA000))
                if (data.showMA10) drawMALine(10, Color(0xFFE040FB))
                if (data.showMA20) drawMALine(20, Color(0xFF29B6F6))
                if (data.showMA60) drawMALine(60, Color(0xFFFF7043))
            }
        }

        if (data.showVolume) {
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(60.dp)
                    .background(Color.White)
            ) {
                Canvas(modifier = Modifier.fillMaxSize().padding(start = 45.dp, end = 10.dp, top = 5.dp, bottom = 5.dp)) {
                    val klines = data.klines
                    if (klines.isEmpty()) return@Canvas

                    val count = klines.size.coerceAtMost(120)
                    val visibleKlines = if (klines.size > count) klines.takeLast(count) else klines

                    val width = size.width
                    val height = size.height
                    val candleWidth = (width / count) * 0.7f
                    val gap = (width / count) * 0.3f

                    var maxVolume = Double.MIN_VALUE
                    for (k in visibleKlines) {
                        if (k.volume > maxVolume) maxVolume = k.volume
                    }
                    if (maxVolume <= 0) return@Canvas

                    for ((index, kline) in visibleKlines.withIndex()) {
                        val x = index * (candleWidth + gap) + gap / 2f
                        val volHeight = (kline.volume / maxVolume * height).toFloat()
                        val isUp = kline.close >= kline.open
                        val color = if (isUp) upColor else downColor

                        drawRect(
                            color = color,
                            topLeft = Offset(x, height - volHeight),
                            size = androidx.compose.ui.geometry.Size(candleWidth, volHeight)
                        )
                    }

                    drawLine(
                        color = gridColor,
                        start = Offset(0f, 0f),
                        end = Offset(width, 0f),
                        strokeWidth = 0.5f
                    )
                }
            }
        }

        if (data.klines.isNotEmpty()) {
            val last = data.klines.last()
            val sdf = SimpleDateFormat("yyyy-MM-dd HH:mm", Locale.getDefault())
            Text(
                text = "时间: ${sdf.format(last.timestamp)}  开:${String.format("%.2f", last.open)}  高:${String.format("%.2f", last.high)}  低:${String.format("%.2f", last.low)}  收:${String.format("%.2f", last.close)}",
                style = TextStyle(fontSize = 11.sp, color = textColor),
                modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
            )
        }
    }
}

@Composable
fun LineChart(
    dataPoints: List<Double>,
    modifier: Modifier = Modifier,
    color: Color = Color(0xFF1976D2),
    fillColor: Color = Color(0x331976D2)
) {
    Box(modifier = modifier.background(Color.White)) {
        Canvas(modifier = Modifier.fillMaxSize().padding(8.dp)) {
            if (dataPoints.isEmpty()) return@Canvas

            val width = size.width
            val height = size.height
            val count = dataPoints.size

            var maxVal = Double.MIN_VALUE
            var minVal = Double.MAX_VALUE
            for (v in dataPoints) {
                if (v > maxVal) maxVal = v
                if (v < minVal) minVal = v
            }
            val range = maxVal - minVal
            if (range <= 0) return@Canvas

            val path = Path()
            val fillPath = Path()

            for ((i, value) in dataPoints.withIndex()) {
                val x = (i.toFloat() / (count - 1)) * width
                val y = height - ((value - minVal) / range * height).toFloat()
                if (i == 0) {
                    path.moveTo(x, y)
                    fillPath.moveTo(x, height)
                    fillPath.lineTo(x, y)
                } else {
                    path.lineTo(x, y)
                    fillPath.lineTo(x, y)
                }
            }
            fillPath.lineTo(width, height)
            fillPath.close()

            drawPath(path = fillPath, color = fillColor)
            drawPath(path = path, color = color, style = Stroke(width = 1.5f))
        }
    }
}
