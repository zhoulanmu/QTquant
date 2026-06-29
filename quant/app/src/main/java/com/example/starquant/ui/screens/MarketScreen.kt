package com.example.starquant.ui.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Search
import androidx.compose.material.icons.filled.Star
import androidx.compose.material.icons.filled.StarBorder
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.FilterChip
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.starquant.ui.chart.CandlestickChart
import com.example.starquant.ui.chart.ChartData
import com.example.starquant.ui.viewmodel.MainViewModel

@Composable
fun MarketScreen(
    viewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    val quoteData by viewModel.quoteData.collectAsState()
    val intradayData by viewModel.intradayData.collectAsState()
    val isLoading by viewModel.isLoading.collectAsState()
    val errorMessage by viewModel.errorMessage.collectAsState()
    val currentSymbol by viewModel.currentSymbol.collectAsState()
    val favorites by viewModel.favoriteStocks.collectAsState()
    var searchText by remember { mutableStateOf(currentSymbol) }
    var showMA5 by remember { mutableStateOf(true) }
    var showMA10 by remember { mutableStateOf(false) }
    var showMA20 by remember { mutableStateOf(true) }
    var showVolume by remember { mutableStateOf(true) }
    val isFavorite = favorites.containsKey(currentSymbol)

    LaunchedEffect(currentSymbol) {
        searchText = currentSymbol
    }

    LaunchedEffect(Unit) {
        viewModel.fetchQuote()
        viewModel.fetchIntraday()
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(12.dp),
        verticalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            OutlinedTextField(
                value = searchText,
                onValueChange = { searchText = it },
                modifier = Modifier.weight(1f),
                placeholder = { Text("输入股票代码") },
                leadingIcon = {
                    Icon(Icons.Default.Search, contentDescription = null)
                },
                singleLine = true
            )
            Button(
                onClick = {
                    if (searchText.isNotBlank()) {
                        viewModel.setSymbol(searchText.uppercase())
                        viewModel.fetchQuote()
                        viewModel.fetchIntraday()
                    }
                }
            ) {
                Text("查询")
            }
            IconButton(
                onClick = {
                    viewModel.toggleFavorite(
                        currentSymbol,
                        quoteData?.name?.ifBlank { currentSymbol } ?: currentSymbol
                    )
                }
            ) {
                Icon(
                    imageVector = if (isFavorite) Icons.Filled.Star else Icons.Filled.StarBorder,
                    contentDescription = null,
                    tint = if (isFavorite) Color(0xFFFFD54F) else Color.Gray
                )
            }
        }

        if (errorMessage != null) {
            Text(
                text = errorMessage.orEmpty(),
                color = MaterialTheme.colorScheme.error,
                style = MaterialTheme.typography.bodySmall
            )
        }

        if (quoteData != null) {
            val data = quoteData!!
            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(
                    containerColor = MaterialTheme.colorScheme.surfaceVariant
                )
            ) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Column {
                            Text(
                                text = data.name.ifBlank { currentSymbol },
                                style = MaterialTheme.typography.titleLarge,
                                fontWeight = FontWeight.Bold
                            )
                            Text(
                                text = currentSymbol,
                                style = MaterialTheme.typography.bodyMedium,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                        val upColor = Color(0xFFE53935)
                        val downColor = Color(0xFF43A047)
                        val priceColor = if (data.changePercent >= 0) upColor else downColor
                        Column(horizontalAlignment = Alignment.End) {
                            Text(
                                text = String.format("%.2f", data.close),
                                style = MaterialTheme.typography.headlineMedium,
                                fontWeight = FontWeight.Bold,
                                color = priceColor
                            )
                            Text(
                                text = String.format("%+.2f (%+.2f%%)", data.changeAmount, data.changePercent),
                                color = priceColor,
                                fontSize = 13.sp,
                                fontWeight = FontWeight.SemiBold
                            )
                        }
                    }

                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        PriceInfo("今开", data.open)
                        PriceInfo("最高", data.high, true)
                        PriceInfo("最低", data.low, false)
                        PriceInfo("昨收", data.previousClose)
                    }

                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        PriceInfo("成交量", data.volume, isVolume = true)
                        PriceInfo("成交额", data.turnover, isVolume = true)
                        PriceInfo("均价", data.averagePrice)
                    }
                }
            }
        } else if (!isLoading) {
            Card(
                modifier = Modifier.fillMaxWidth().padding(vertical = 24.dp),
            ) {
                Text(
                    text = "点击查询获取行情数据",
                    modifier = Modifier.padding(16.dp),
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }

        Text(
            text = "分时走势",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold
        )

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            FilterChip(
                selected = showMA5,
                onClick = { showMA5 = !showMA5 },
                label = { Text("MA5", fontSize = 11.sp) }
            )
            FilterChip(
                selected = showMA10,
                onClick = { showMA10 = !showMA10 },
                label = { Text("MA10", fontSize = 11.sp) }
            )
            FilterChip(
                selected = showMA20,
                onClick = { showMA20 = !showMA20 },
                label = { Text("MA20", fontSize = 11.sp) }
            )
            FilterChip(
                selected = showVolume,
                onClick = { showVolume = !showVolume },
                label = { Text("成交量", fontSize = 11.sp) }
            )
        }

        if (intradayData.isNotEmpty()) {
            val chartData = ChartData(
                klines = intradayData,
                showMA5 = showMA5,
                showMA10 = showMA10,
                showMA20 = showMA20,
                showVolume = showVolume
            )
            CandlestickChart(
                data = chartData,
                modifier = Modifier.fillMaxWidth()
            )
        } else {
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 24.dp),
            ) {
                Text(
                    text = "暂无分时数据",
                    modifier = Modifier.padding(16.dp),
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}

@Composable
fun PriceInfo(
    label: String,
    value: Double,
    isUp: Boolean? = null,
    isVolume: Boolean = false
) {
    val color = when {
        isUp == true -> Color(0xFFE53935)
        isUp == false -> Color(0xFF43A047)
        else -> MaterialTheme.colorScheme.onSurface
    }
    val text = if (isVolume) {
        when {
            value >= 100000000 -> String.format("%.2f亿", value / 100000000)
            value >= 10000 -> String.format("%.2f万", value / 10000)
            else -> String.format("%.0f", value)
        }
    } else {
        String.format("%.2f", value)
    }
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(
            text = label,
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            fontSize = 11.sp
        )
        Text(
            text = text,
            style = MaterialTheme.typography.bodyMedium,
            fontWeight = FontWeight.SemiBold,
            color = color,
            fontSize = 13.sp
        )
    }
}
