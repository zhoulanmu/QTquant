package com.example.starquant.ui.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.starquant.data.model.MarketData
import com.example.starquant.ui.viewmodel.MainViewModel
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

@Composable
fun HomeScreen(
    viewModel: MainViewModel,
    onStockClick: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val accountState by viewModel.accountState.collectAsState()
    val signals by viewModel.signals.collectAsState()
    val strategyRuntimes by viewModel.strategyRuntimes.collectAsState()
    val favorites by viewModel.favoriteStocks.collectAsState()
    val favoriteQuotes by viewModel.favoriteQuotes.collectAsState()
    val sdf = SimpleDateFormat("yyyy-MM-dd HH:mm", Locale.getDefault())
    val todayProfit = accountState.positions.values.sumOf { position ->
        val base = position.todayOpenPrice.takeIf { it > 0.0 } ?: position.avgCost
        (position.currentPrice - base) * position.quantity
    }
    val todayBase = accountState.positions.values.sumOf { position ->
        val base = position.todayOpenPrice.takeIf { it > 0.0 } ?: position.avgCost
        base * position.quantity
    }
    val todayProfitPercent = if (todayBase > 0.0) todayProfit / todayBase * 100.0 else 0.0

    LaunchedEffect(favorites) {
        viewModel.refreshFavoriteQuotes()
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = "StarQuant 量化交易",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold
        )

        Text(
            text = sdf.format(Date()),
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )

        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.primaryContainer
            )
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = "账户总资产",
                    style = MaterialTheme.typography.titleMedium
                )
                Text(
                    text = String.format("¥ %.2f", accountState.totalAssets),
                    style = MaterialTheme.typography.headlineLarge,
                    fontWeight = FontWeight.Bold
                )
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Column {
                        Text("总盈亏", style = MaterialTheme.typography.bodySmall)
                        Text(
                            String.format("%.2f (%.2f%%)", accountState.totalProfit, accountState.totalProfitPercent),
                            color = if (accountState.totalProfit >= 0) Color(0xFFE53935) else Color(0xFF43A047),
                            fontWeight = FontWeight.Bold
                        )
                    }
                    Column {
                        Text("可用资金", style = MaterialTheme.typography.bodySmall)
                        Text(
                            String.format("¥ %.2f", accountState.currentCash),
                            fontWeight = FontWeight.SemiBold
                        )
                    }
                    Column {
                        Text("持仓数", style = MaterialTheme.typography.bodySmall)
                        Text(
                            "${accountState.positions.size}",
                            fontWeight = FontWeight.SemiBold
                        )
                    }
                }
            }
        }

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            StatCard(
                title = "今日收益",
                value = String.format("%+.2f", todayProfit),
                percent = String.format("%+.2f%%", todayProfitPercent),
                color = if (todayProfit >= 0) Color(0xFFE53935) else Color(0xFF43A047),
                modifier = Modifier.weight(1f)
            )
            StatCard(
                title = "策略运行",
                value = "${strategyRuntimes.size}",
                percent = "个",
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.weight(1f)
            )
        }

        Text(
            text = "自选行情",
            style = MaterialTheme.typography.titleLarge,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.padding(top = 4.dp)
        )

        LazyColumn(
            modifier = Modifier.weight(1f),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            if (favorites.isEmpty()) {
                item {
                    Text(
                        text = "暂无自选股",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        modifier = Modifier.padding(8.dp)
                    )
                }
            }
            items(favorites.toList(), key = { it.first }) { (symbol, name) ->
                StockItemRow(
                    symbol = symbol,
                    name = name,
                    quote = favoriteQuotes[symbol],
                    onClick = { onStockClick(symbol) },
                    onRemove = { viewModel.removeFromFavorites(symbol) }
                )
            }
        }

        Text(
            text = "最新信号",
            style = MaterialTheme.typography.titleLarge,
            fontWeight = FontWeight.Bold
        )

        if (signals.isEmpty()) {
            Text(
                text = "暂无交易信号",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(8.dp)
            )
        } else {
            LazyColumn(
                modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
                verticalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                items(signals.take(3)) { signal ->
                    SignalRow(signal = signal)
                }
            }
        }
    }
}

@Composable
fun StatCard(
    title: String,
    value: String,
    percent: String,
    color: Color,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier,
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            Text(
                text = title,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Text(
                text = value,
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = color
            )
            Text(
                text = percent,
                style = MaterialTheme.typography.bodySmall,
                color = color
            )
        }
    }
}

@Composable
fun StockItemRow(
    symbol: String,
    name: String,
    quote: MarketData?,
    onClick: () -> Unit,
    onRemove: () -> Unit
) {
    val changePercent = quote?.changePercent ?: 0.0
    val priceColor = when {
        quote == null -> MaterialTheme.colorScheme.onSurfaceVariant
        changePercent >= 0 -> Color(0xFFE53935)
        else -> Color(0xFF43A047)
    }

    Card(
        modifier = Modifier.fillMaxWidth(),
        onClick = onClick
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text(
                    text = name,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    text = symbol,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            Row(
                horizontalArrangement = Arrangement.spacedBy(4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column(horizontalAlignment = Alignment.End) {
                    Text(
                        text = quote?.let { String.format("%.2f", it.close) } ?: "暂无数据",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.SemiBold,
                        color = priceColor
                    )
                    Text(
                        text = quote?.let { String.format("%+.2f%%", it.changePercent) } ?: "--",
                        style = MaterialTheme.typography.bodySmall,
                        color = priceColor
                    )
                }
                IconButton(onClick = onRemove) {
                    Icon(
                        imageVector = Icons.Filled.Delete,
                        contentDescription = "移除自选",
                        tint = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}

@Composable
fun SignalRow(signal: com.example.starquant.data.model.StrategySignal) {
    val color = when (signal.type) {
        com.example.starquant.data.model.SignalType.BUY -> Color(0xFFE53935)
        com.example.starquant.data.model.SignalType.SELL -> Color(0xFF43A047)
        com.example.starquant.data.model.SignalType.STOP_LOSS -> Color(0xFFE64A19)
        com.example.starquant.data.model.SignalType.TAKE_PROFIT -> Color(0xFF43A047)
        else -> Color.Gray
    }
    val typeText = when (signal.type) {
        com.example.starquant.data.model.SignalType.BUY -> "买入"
        com.example.starquant.data.model.SignalType.SELL -> "卖出"
        com.example.starquant.data.model.SignalType.STOP_LOSS -> "止损"
        com.example.starquant.data.model.SignalType.TAKE_PROFIT -> "止盈"
        else -> "信号"
    }

    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = color.copy(alpha = 0.1f)
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(10.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text(
                    text = "$typeText - ${signal.symbol}",
                    fontWeight = FontWeight.Bold,
                    color = color,
                    fontSize = 14.sp
                )
                Text(
                    text = signal.comment,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    fontSize = 11.sp
                )
            }
            Text(
                text = String.format("%.2f", signal.price),
                fontWeight = FontWeight.Bold,
                color = color
            )
        }
    }
}
