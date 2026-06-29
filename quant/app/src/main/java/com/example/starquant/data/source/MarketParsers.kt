package com.example.starquant.data.source

import com.example.starquant.data.model.MarketData
import com.google.gson.JsonArray
import com.google.gson.JsonElement
import com.google.gson.JsonObject
import com.google.gson.JsonParser
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min
import kotlin.math.pow

object SymbolHelper {
    fun normalizeSymbol(symbol: String): String {
        val value = symbol.trim().uppercase(Locale.ROOT).replace('_', '.').replace(" ", "")
        if (value.isEmpty()) return "000001.SH"

        Regex("^([01])\\.(\\d{6})$").matchEntire(value)?.let {
            return "${it.groupValues[2]}.${if (it.groupValues[1] == "1") "SH" else "SZ"}"
        }

        val code: String
        val exchange: String
        when {
            value.startsWith("SH") && value.length >= 8 -> {
                code = value.substring(2, 8)
                exchange = "SH"
            }
            value.startsWith("SZ") && value.length >= 8 -> {
                code = value.substring(2, 8)
                exchange = "SZ"
            }
            value.contains('.') -> {
                val parts = value.split('.')
                code = parts.firstOrNull().orEmpty()
                exchange = when (parts.getOrNull(1).orEmpty()) {
                    "SH", "SS", "SSE", "XSHG" -> "SH"
                    "SZ", "SHE", "SZSE", "XSHE" -> "SZ"
                    else -> ""
                }
            }
            value.length == 6 && value.all { it.isDigit() } -> {
                code = value
                exchange = if (value.startsWith("6") || value.startsWith("5") || value.startsWith("9")) "SH" else "SZ"
            }
            else -> return value
        }

        return if (code.length == 6 && exchange.isNotEmpty()) "$code.$exchange" else value
    }

    fun prefixedMarketSymbol(symbol: String): String {
        val normalized = normalizeSymbol(symbol)
        val parts = normalized.split('.')
        if (parts.size != 2 || parts[0].length != 6) return ""
        return when (parts[1]) {
            "SH" -> "sh${parts[0]}"
            "SZ" -> "sz${parts[0]}"
            else -> ""
        }
    }

    fun secIdForSymbol(symbol: String): String {
        val normalized = normalizeSymbol(symbol)
        val parts = normalized.split('.')
        if (parts.size != 2 || parts[0].length != 6) return ""
        return when (parts[1]) {
            "SH" -> "1.${parts[0]}"
            "SZ" -> "0.${parts[0]}"
            else -> ""
        }
    }

    fun isAShareContinuousTradingTime(now: Date = Date()): Boolean {
        val cal = Calendar.getInstance().apply { time = now }
        val day = cal.get(Calendar.DAY_OF_WEEK)
        if (day == Calendar.SATURDAY || day == Calendar.SUNDAY) return false
        val minuteOfDay = cal.get(Calendar.HOUR_OF_DAY) * 60 + cal.get(Calendar.MINUTE)
        return minuteOfDay in (9 * 60 + 30)..(11 * 60 + 30) ||
            minuteOfDay in (13 * 60)..(15 * 60)
    }

    fun trendTargetDate(now: Date = Date()): Date {
        val cal = Calendar.getInstance().apply { time = now }
        val day = cal.get(Calendar.DAY_OF_WEEK)
        val minuteOfDay = cal.get(Calendar.HOUR_OF_DAY) * 60 + cal.get(Calendar.MINUTE)
        if (day != Calendar.SATURDAY && day != Calendar.SUNDAY && minuteOfDay >= 9 * 60 + 30) {
            return startOfDay(cal.time)
        }
        do {
            cal.add(Calendar.DAY_OF_MONTH, -1)
        } while (cal.get(Calendar.DAY_OF_WEEK) == Calendar.SATURDAY || cal.get(Calendar.DAY_OF_WEEK) == Calendar.SUNDAY)
        return startOfDay(cal.time)
    }

    fun sameDay(left: Date, right: Date): Boolean {
        val a = Calendar.getInstance().apply { time = left }
        val b = Calendar.getInstance().apply { time = right }
        return a.get(Calendar.YEAR) == b.get(Calendar.YEAR) &&
            a.get(Calendar.DAY_OF_YEAR) == b.get(Calendar.DAY_OF_YEAR)
    }

    private fun startOfDay(date: Date): Date {
        return Calendar.getInstance().apply {
            time = date
            set(Calendar.HOUR_OF_DAY, 0)
            set(Calendar.MINUTE, 0)
            set(Calendar.SECOND, 0)
            set(Calendar.MILLISECOND, 0)
        }.time
    }
}

class QuoteParser {
    companion object {
        fun textToDouble(text: String?): Double {
            if (text.isNullOrBlank()) return 0.0
            val value = text.trim().replace(",", "")
            if (value == "-" || value == "--") return 0.0
            return value.toDoubleOrNull() ?: 0.0
        }
    }

    fun parseEastMoneyQuote(payload: String, symbol: String, lastPrice: Double = 0.0): MarketData? {
        return runCatching {
            val root = JsonParser.parseString(extractJsonText(payload)).asJsonObject
            val data = root.getAsJsonObject("data") ?: return null
            val normalized = SymbolHelper.normalizeSymbol(symbol)
            val decimal = data.number("decimal").takeIf { it > 0 }?.toInt() ?: 2
            val name = data.string("f58").ifBlank { data.string("name") }
            val previousClose = data.price("f60", decimal).takeIf { it > 0.0 }
                ?: data.number("preClose").takeIf { it > 0.0 }
                ?: data.number("prePrice")
            var close = data.price("f43", decimal).takeIf { it > 0.0 }
                ?: data.number("current").takeIf { it > 0.0 }
                ?: previousClose.takeIf { it > 0.0 }
                ?: lastPrice
            if (close <= 0.0) return null

            var open = data.price("f46", decimal).takeIf { it > 0.0 } ?: data.number("open")
            var high = data.price("f44", decimal).takeIf { it > 0.0 } ?: data.number("high")
            var low = data.price("f45", decimal).takeIf { it > 0.0 } ?: data.number("low")
            if (open <= 0.0) open = close
            if (high <= 0.0) high = max(open, close)
            if (low <= 0.0) low = min(open, close)

            val volume = data.number("f47").takeIf { it > 0.0 } ?: data.number("volume")
            val turnover = data.number("f48").takeIf { it > 0.0 } ?: data.number("amount")
            MarketData(
                symbol = normalized,
                name = name,
                timestamp = Date(),
                open = open,
                high = high,
                low = low,
                close = close,
                volume = volume,
                turnover = turnover,
                previousClose = previousClose,
                averagePrice = if (volume > 0.0 && turnover > 0.0) turnover / (volume * 100.0) else close,
                tradeCount = if (volume > 0.0) volume.toInt() else 0
            )
        }.getOrNull()
    }

    fun parseSinaQuote(payload: String, symbol: String, lastPrice: Double = 0.0): MarketData? {
        return runCatching {
            val valueStart = payload.indexOf('"')
            val valueEnd = payload.lastIndexOf('"')
            if (valueStart < 0 || valueEnd <= valueStart) return null
            val parts = payload.substring(valueStart + 1, valueEnd).split(',')
            if (parts.size < 32 || parts[0].isBlank()) return null

            val previousClose = textToDouble(parts[2])
            var close = textToDouble(parts[3])
            var open = textToDouble(parts[1])
            var high = textToDouble(parts[4])
            var low = textToDouble(parts[5])
            if (close <= 0.0) close = previousClose.takeIf { it > 0.0 } ?: lastPrice
            if (close <= 0.0) return null
            if (open <= 0.0) open = close
            if (high <= 0.0) high = max(open, close)
            if (low <= 0.0) low = min(open, close)

            val volumeShares = textToDouble(parts[8])
            val volumeLots = if (volumeShares > 0.0) volumeShares / 100.0 else 0.0
            val turnover = textToDouble(parts[9])
            val timestamp = parseDate("${parts[30].trim()} ${parts[31].trim()}", "yyyy-MM-dd HH:mm:ss") ?: Date()

            MarketData(
                symbol = SymbolHelper.normalizeSymbol(symbol),
                name = parts[0].trim(),
                timestamp = timestamp,
                open = open,
                high = high,
                low = low,
                close = close,
                volume = volumeLots,
                turnover = turnover,
                previousClose = previousClose,
                averagePrice = if (volumeLots > 0.0 && turnover > 0.0) turnover / (volumeLots * 100.0) else close,
                tradeCount = if (volumeLots > 0.0) volumeLots.toInt() else 0
            )
        }.getOrNull()
    }

    fun parseTencentQuote(payload: String, symbol: String, lastPrice: Double = 0.0): MarketData? {
        return runCatching {
            val valueStart = payload.indexOf('"')
            val valueEnd = payload.lastIndexOf('"')
            if (valueStart < 0 || valueEnd <= valueStart) return null
            val parts = payload.substring(valueStart + 1, valueEnd).split('~')
            if (parts.size < 35 || parts[3].isBlank()) return null

            val previousClose = textToDouble(parts[4])
            var close = textToDouble(parts[3])
            var open = textToDouble(parts[5])
            var high = textToDouble(parts[33])
            var low = textToDouble(parts[34])
            if (close <= 0.0) close = previousClose.takeIf { it > 0.0 } ?: lastPrice
            if (close <= 0.0) return null
            if (open <= 0.0) open = close
            if (high <= 0.0) high = max(open, close)
            if (low <= 0.0) low = min(open, close)

            val volumeLots = textToDouble(parts[6])
            var turnover = parts.getOrNull(35)?.split('/')?.getOrNull(2)?.let { textToDouble(it) } ?: 0.0
            if (turnover <= 0.0) turnover = textToDouble(parts.getOrNull(37)) * 10000.0
            val timestamp = parseDate(parts[30].trim(), "yyyyMMddHHmmss") ?: Date()

            MarketData(
                symbol = SymbolHelper.normalizeSymbol(symbol),
                name = parts[1].trim(),
                timestamp = timestamp,
                open = open,
                high = high,
                low = low,
                close = close,
                volume = volumeLots,
                turnover = turnover,
                previousClose = previousClose,
                averagePrice = if (volumeLots > 0.0 && turnover > 0.0) turnover / (volumeLots * 100.0) else close,
                tradeCount = if (volumeLots > 0.0) volumeLots.toInt() else 0
            )
        }.getOrNull()
    }
}

class TrendParser {
    fun parseEastMoneyTrend(
        payload: String,
        symbol: String,
        targetDate: Date? = null,
        lastQuoteData: MarketData? = null
    ): List<MarketData> {
        return runCatching {
            val root = JsonParser.parseString(extractJsonText(payload)).asJsonObject
            val data = root.getAsJsonObject("data") ?: return emptyList()
            val trends = data.getAsJsonArray("trends") ?: return emptyList()
            val previousClose = data.number("preClose").takeIf { it > 0.0 } ?: data.number("prePrice")
            val name = data.string("name")
            val points = mutableListOf<MarketData>()

            for (item in trends) {
                val row = item.asString.trim()
                if (row.isEmpty()) continue
                val parts = row.split(',')
                if (parts.size < 3) continue
                val timestamp = parseTrendTimestamp(parts[0], targetDate) ?: continue

                val first = textNumber(parts.getOrNull(1))
                val second = textNumber(parts.getOrNull(2))
                val third = textNumber(parts.getOrNull(3))
                val fourth = textNumber(parts.getOrNull(4))
                val fifth = textNumber(parts.getOrNull(5))
                val sixth = textNumber(parts.getOrNull(6))
                val seventh = textNumber(parts.getOrNull(7))
                val priceBase = max(abs(first), abs(second))
                val looksLikeOhlc = parts.size >= 7 && priceBase > 0.0 &&
                    third > 0.0 && fourth > 0.0 && third <= priceBase * 3.0 && fourth <= priceBase * 3.0

                val open: Double
                val close: Double
                val high: Double
                val low: Double
                val volume: Double
                val turnover: Double
                val averagePrice: Double
                if (looksLikeOhlc) {
                    open = first
                    close = second
                    high = third
                    low = fourth
                    volume = fifth
                    turnover = sixth
                    averagePrice = seventh
                } else {
                    close = first
                    open = close
                    high = close
                    low = close
                    averagePrice = second
                    volume = third
                    turnover = fourth
                }
                if (close <= 0.0) continue

                points.add(
                    MarketData(
                        symbol = SymbolHelper.normalizeSymbol(symbol),
                        name = name,
                        timestamp = timestamp,
                        open = if (open > 0.0) open else close,
                        high = if (high > 0.0) high else close,
                        low = if (low > 0.0) low else close,
                        close = close,
                        volume = volume,
                        turnover = turnover,
                        previousClose = previousClose,
                        averagePrice = if (averagePrice > 0.0) averagePrice else close,
                        tradeCount = if (volume > 0.0) volume.toInt() else 0
                    )
                )
            }
            selectTrendSession(points, targetDate, lastQuoteData)
        }.getOrElse { emptyList() }
    }

    fun parseSinaTrend(
        payload: String,
        symbol: String,
        targetDate: Date? = null,
        lastQuoteData: MarketData? = null
    ): List<MarketData> = parseGenericTrend(payload, symbol, targetDate, lastQuoteData)

    fun parseTencentTrend(
        payload: String,
        symbol: String,
        targetDate: Date? = null,
        lastQuoteData: MarketData? = null
    ): List<MarketData> = parseGenericTrend(payload, symbol, targetDate, lastQuoteData)

    private fun parseGenericTrend(
        payload: String,
        symbol: String,
        targetDate: Date?,
        lastQuoteData: MarketData?
    ): List<MarketData> {
        return runCatching {
            val root = JsonParser.parseString(extractJsonText(payload))
            val points = mutableListOf<MarketData>()
            collectTrendPoints(root, targetDate, SymbolHelper.normalizeSymbol(symbol), "", points)
            selectTrendSession(points, targetDate, lastQuoteData)
        }.getOrElse { emptyList() }
    }

    private fun collectTrendPoints(
        element: JsonElement?,
        targetDate: Date?,
        symbol: String,
        name: String,
        points: MutableList<MarketData>
    ) {
        if (element == null || element.isJsonNull) return
        when {
            element.isJsonObject -> {
                val obj = element.asJsonObject
                if (appendObjectTrendPoint(obj, targetDate, symbol, name, points)) return
                obj.entrySet().forEach { collectTrendPoints(it.value, targetDate, symbol, name, points) }
            }
            element.isJsonArray -> {
                val arr = element.asJsonArray
                if (appendArrayTrendPoint(arr, targetDate, symbol, name, points)) return
                arr.forEach { collectTrendPoints(it, targetDate, symbol, name, points) }
            }
            element.isJsonPrimitive && element.asJsonPrimitive.isString -> {
                appendDelimitedTrendPoint(element.asString, targetDate, symbol, name, points)
            }
        }
    }

    private fun appendObjectTrendPoint(
        obj: JsonObject,
        targetDate: Date?,
        symbol: String,
        name: String,
        points: MutableList<MarketData>
    ): Boolean {
        val timestampText = obj.string("day").ifBlank { obj.string("datetime") }
            .ifBlank {
                val date = obj.string("date")
                val time = obj.string("time")
                if (time.isBlank()) date else "$date $time"
            }
            .ifBlank { obj.string("t") }
        val timestamp = parseTrendTimestamp(timestampText, targetDate) ?: return false
        var close = obj.number("close").takeIf { it > 0.0 } ?: obj.number("price").takeIf { it > 0.0 } ?: obj.number("c")
        if (close <= 0.0) return false
        var open = obj.number("open")
        var high = obj.number("high")
        var low = obj.number("low")
        if (open <= 0.0) open = close
        if (high <= 0.0) high = max(open, close)
        if (low <= 0.0) low = min(open, close)
        val volume = obj.number("volume")
        val turnover = obj.number("amount").takeIf { it > 0.0 } ?: obj.number("turnover")

        points.add(
            MarketData(
                symbol = symbol,
                name = name,
                timestamp = timestamp,
                open = open,
                high = high,
                low = low,
                close = close,
                volume = volume,
                turnover = turnover,
                averagePrice = close,
                tradeCount = if (volume > 0.0) volume.toInt() else 0
            )
        )
        return true
    }

    private fun appendArrayTrendPoint(
        arr: JsonArray,
        targetDate: Date?,
        symbol: String,
        name: String,
        points: MutableList<MarketData>
    ): Boolean {
        if (arr.size() < 2) return false
        val timestampText = if (arr[0].isJsonPrimitive) arr[0].asString else return false
        val timestamp = parseTrendTimestamp(timestampText, targetDate) ?: return false
        val nums = (1 until arr.size()).map { arr[it].asNumberOrZero() }
        if (nums.isEmpty()) return false
        var open = nums.getOrElse(0) { 0.0 }
        var close = if (nums.size >= 4) nums.getOrElse(1) { open } else nums.getOrElse(0) { 0.0 }
        var high = if (nums.size >= 4) nums.getOrElse(2) { close } else close
        var low = if (nums.size >= 4) nums.getOrElse(3) { close } else close
        val volume = nums.getOrElse(4) { 0.0 }
        val turnover = nums.getOrElse(5) { 0.0 }
        val averagePrice = if (nums.size in 2..3) nums[1] else 0.0
        if (close <= 0.0) return false
        if (open <= 0.0) open = close
        if (high <= 0.0) high = max(open, close)
        if (low <= 0.0) low = min(open, close)

        points.add(
            MarketData(
                symbol = symbol,
                name = name,
                timestamp = timestamp,
                open = open,
                high = high,
                low = low,
                close = close,
                volume = volume,
                turnover = turnover,
                averagePrice = if (averagePrice > 0.0) averagePrice else close,
                tradeCount = if (volume > 0.0) volume.toInt() else 0
            )
        )
        return true
    }

    private fun appendDelimitedTrendPoint(
        row: String,
        targetDate: Date?,
        symbol: String,
        name: String,
        points: MutableList<MarketData>
    ): Boolean {
        val parts = row.trim().split(Regex("[,\\s]+")).filter { it.isNotBlank() }
        if (parts.size < 2) return false
        val timestampText = if (parts.size > 2 && parts[0].contains('-') && parts[1].contains(':')) "${parts[0]} ${parts[1]}" else parts[0]
        val numberStart = if (timestampText.contains(' ')) 2 else 1
        val timestamp = parseTrendTimestamp(timestampText, targetDate) ?: return false
        val nums = parts.drop(numberStart).map { textNumber(it) }
        if (nums.isEmpty()) return false
        var open = nums.getOrElse(0) { 0.0 }
        var close = if (nums.size >= 4) nums.getOrElse(1) { open } else open
        var high = if (nums.size >= 4) nums.getOrElse(2) { close } else close
        var low = if (nums.size >= 4) nums.getOrElse(3) { close } else close
        val volume = nums.getOrElse(4) { 0.0 }
        val turnover = nums.getOrElse(5) { 0.0 }
        if (close <= 0.0) return false
        if (open <= 0.0) open = close
        if (high <= 0.0) high = max(open, close)
        if (low <= 0.0) low = min(open, close)
        points.add(MarketData(symbol, name, timestamp, open, high, low, close, volume, turnover, averagePrice = close))
        return true
    }

    private fun selectTrendSession(
        allPoints: List<MarketData>,
        targetDate: Date?,
        lastQuoteData: MarketData?
    ): List<MarketData> {
        if (allPoints.isEmpty()) return emptyList()
        val sorted = allPoints.filter { it.close > 0.0 }.sortedBy { it.timestamp }
        if (sorted.isEmpty()) return emptyList()
        val target = targetDate ?: sorted.last().timestamp
        val selectedDate = sorted
            .map { it.timestamp }
            .filter { !it.after(endOfDay(target)) }
            .maxByOrNull { it.time }
            ?: return emptyList()

        val selected = sorted.filter { SymbolHelper.sameDay(it.timestamp, selectedDate) }.toMutableList()
        if (selected.isEmpty()) return emptyList()
        var previousClose = sorted.lastOrNull { it.timestamp.before(startOfDay(selectedDate)) }?.close ?: 0.0
        if (previousClose <= 0.0 && lastQuoteData != null && SymbolHelper.sameDay(selectedDate, lastQuoteData.timestamp)) {
            previousClose = lastQuoteData.previousClose
        }
        if (previousClose <= 0.0) previousClose = selected.first().open.takeIf { it > 0.0 } ?: selected.first().close

        var volumeSum = 0.0
        var weightedPriceSum = 0.0
        selected.forEach { point ->
            point.previousClose = previousClose
            if (point.volume > 0.0) {
                volumeSum += point.volume
                weightedPriceSum += point.close * point.volume
            }
            if (point.averagePrice <= 0.0) {
                point.averagePrice = if (volumeSum > 0.0) weightedPriceSum / volumeSum else point.close
            }
        }
        return selected
    }
}

private fun JsonObject.string(name: String): String = get(name)?.takeIf { !it.isJsonNull }?.asString?.trim().orEmpty()

private fun JsonObject.number(name: String): Double = get(name)?.asNumberOrZero() ?: 0.0

private fun JsonObject.price(name: String, decimal: Int): Double {
    val raw = number(name)
    if (raw <= 0.0) return 0.0
    val scale = 10.0.pow(decimal)
    return if (raw >= scale * 100.0) raw / scale else raw
}

private fun JsonElement.asNumberOrZero(): Double {
    return runCatching {
        if (isJsonPrimitive && asJsonPrimitive.isString) {
            QuoteParser.textToDouble(asString)
        } else {
            asDouble
        }
    }.getOrDefault(0.0)
}

private fun extractJsonText(payload: String): String {
    val objectStart = payload.indexOf('{')
    val arrayStart = payload.indexOf('[')
    val start = when {
        objectStart >= 0 && arrayStart >= 0 -> min(objectStart, arrayStart)
        objectStart >= 0 -> objectStart
        arrayStart >= 0 -> arrayStart
        else -> -1
    }
    if (start < 0) return payload.trim()
    val opener = payload[start]
    val closer = if (opener == '{') '}' else ']'
    val end = payload.lastIndexOf(closer)
    return if (end >= start) payload.substring(start, end + 1) else payload.substring(start)
}

private fun parseDate(text: String, format: String): Date? =
    runCatching { SimpleDateFormat(format, Locale.getDefault()).parse(text) }.getOrNull()

private fun parseTrendTimestamp(text: String, targetDate: Date?): Date? {
    val value = text.trim()
    if (value.isEmpty()) return null
    val formats = listOf(
        "yyyy-MM-dd HH:mm:ss",
        "yyyy-MM-dd HH:mm",
        "yyyyMMddHHmmss",
        "yyyyMMddHHmm",
        "yyyyMMdd HH:mm:ss",
        "yyyyMMdd HH:mm"
    )
    formats.forEach { format -> parseDate(value, format)?.let { return it } }
    if (targetDate != null && Regex("^\\d{2}:\\d{2}(:\\d{2})?$").matches(value)) {
        val time = value.split(':').map { it.toIntOrNull() ?: 0 }
        return Calendar.getInstance().apply {
            this.time = targetDate
            set(Calendar.HOUR_OF_DAY, time.getOrElse(0) { 0 })
            set(Calendar.MINUTE, time.getOrElse(1) { 0 })
            set(Calendar.SECOND, time.getOrElse(2) { 0 })
            set(Calendar.MILLISECOND, 0)
        }.time
    }
    if (targetDate != null && Regex("^\\d{4}$").matches(value)) {
        val compact = value.toInt()
        return Calendar.getInstance().apply {
            time = targetDate
            set(Calendar.HOUR_OF_DAY, compact / 100)
            set(Calendar.MINUTE, compact % 100)
            set(Calendar.SECOND, 0)
            set(Calendar.MILLISECOND, 0)
        }.time
    }
    return null
}

private fun textNumber(text: String?): Double = QuoteParser.textToDouble(text)

private fun startOfDay(date: Date): Date = Calendar.getInstance().apply {
    time = date
    set(Calendar.HOUR_OF_DAY, 0)
    set(Calendar.MINUTE, 0)
    set(Calendar.SECOND, 0)
    set(Calendar.MILLISECOND, 0)
}.time

private fun endOfDay(date: Date): Date = Calendar.getInstance().apply {
    time = date
    set(Calendar.HOUR_OF_DAY, 23)
    set(Calendar.MINUTE, 59)
    set(Calendar.SECOND, 59)
    set(Calendar.MILLISECOND, 999)
}.time
