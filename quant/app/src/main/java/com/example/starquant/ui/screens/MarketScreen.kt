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
import androidx.compose.material3.FilterChip
import androidx.compose.material3.FilterChipDefaults
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
import com.example.starquant.ui.components.StarCard
import com.example.starquant.ui.components.StarPrimaryButton
import com.example.starquant.ui.theme.FallGreen
import com.example.starquant.ui.theme.MA10Color
import com.example.starquant.ui.theme.MA20Color
import com.example.starquant.ui.theme.MA5Color
import com.example.starquant.ui.theme.RiseRed
import com.example.starquant.ui.theme.TextSecondary
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
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = "行情",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold
        )

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
            StarPrimaryButton(
                text = "查询",
                onClick = {
                    if (searchText.isNotBlank()) {
                        viewModel.setSymbol(searchText.uppercase())
                        viewModel.fetchQuote()
                        viewModel.fetchIntraday()
                    }
                }
            )
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
            StarCard(modifier = Modifier.fillMaxWidth()) {
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
                        val upColor = RiseRed
                        val downColor = FallGreen
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
            StarCard(modifier = Modifier.fillMaxWidth().padding(vertical = 24.dp)) {
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
                label = { Text("MA5", fontSize = 11.sp) },
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = MA5Color.copy(alpha = 0.28f),
                    selectedLabelColor = MaterialTheme.colorScheme.onSurface
                )
            )
            FilterChip(
                selected = showMA10,
                onClick = { showMA10 = !showMA10 },
                label = { Text("MA10", fontSize = 11.sp) },
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = MA10Color.copy(alpha = 0.22f),
                    selectedLabelColor = MaterialTheme.colorScheme.onSurface
                )
            )
            FilterChip(
                selected = showMA20,
                onClick = { showMA20 = !showMA20 },
                label = { Text("MA20", fontSize = 11.sp) },
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = MA20Color.copy(alpha = 0.24f),
                    selectedLabelColor = MaterialTheme.colorScheme.onSurface
                )
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
            StarCard(modifier = Modifier.fillMaxWidth()) {
                CandlestickChart(
                    data = chartData,
                    modifier = Modifier.fillMaxWidth().padding(8.dp)
                )
            }
        } else {
            StarCard(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 24.dp)
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
        isUp == true -> RiseRed
        isUp == false -> FallGreen
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
            color = TextSecondary,
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
