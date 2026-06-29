package com.example.starquant.ui.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.FilterList
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilterChip
import androidx.compose.material3.FilterChipDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.rememberModalBottomSheetState
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
import com.example.starquant.data.model.ScreenerFilter
import com.example.starquant.data.model.ScreenerFilterType
import com.example.starquant.data.model.StockInfo
import com.example.starquant.ui.viewmodel.ScreenerViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ScreenerScreen(
    viewModel: ScreenerViewModel,
    onNavigateToMarket: (String) -> Unit,
    onNavigateToStrategy: (String) -> Unit,
    onAddFavorite: (String, String) -> Unit,
    modifier: Modifier = Modifier
) {
    val results by viewModel.results.collectAsState()
    val isRunning by viewModel.isRunning.collectAsState()
    val resultCount by viewModel.resultCount.collectAsState()
    val filters by viewModel.filters.collectAsState()
    val selectedPreset by viewModel.selectedPreset.collectAsState()
    val filterSummary by viewModel.filterSummary.collectAsState()
    val presets = viewModel.getPresetStrategies()

    var showFilterSheet by remember { mutableStateOf(false) }
    var selectedStock by remember { mutableStateOf<StockInfo?>(null) }
    val sheetState = rememberModalBottomSheetState()

    var minPrice by remember { mutableStateOf("5") }
    var maxPrice by remember { mutableStateOf("200") }
    var minPE by remember { mutableStateOf("0") }
    var maxPE by remember { mutableStateOf("50") }
    var minROE by remember { mutableStateOf("10") }
    var maxROE by remember { mutableStateOf("50") }

    fun filter(type: ScreenerFilterType): ScreenerFilter? = filters.find { it.type == type }
    fun refreshLocalInputs() {
        filter(ScreenerFilterType.PriceRange)?.takeIf { it.enabled }?.let {
            minPrice = it.minValue.toPlainText()
            maxPrice = it.maxValue.toPlainText()
        } ?: run {
            minPrice = "5"
            maxPrice = "200"
        }
        filter(ScreenerFilterType.PE)?.takeIf { it.enabled }?.let {
            minPE = it.minValue.toPlainText()
            maxPE = it.maxValue.toPlainText()
        } ?: run {
            minPE = "0"
            maxPE = "50"
        }
        filter(ScreenerFilterType.ROE)?.takeIf { it.enabled }?.let {
            minROE = it.minValue.toPlainText()
            maxROE = it.maxValue.toPlainText()
        } ?: run {
            minROE = "10"
            maxROE = "50"
        }
    }

    LaunchedEffect(showFilterSheet) {
        if (showFilterSheet) refreshLocalInputs()
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(12.dp),
        verticalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "智能选股",
                style = MaterialTheme.typography.headlineSmall,
                fontWeight = FontWeight.Bold
            )
            Row {
                IconButton(onClick = { showFilterSheet = true }) {
                    Icon(Icons.Default.FilterList, contentDescription = "筛选")
                }
                IconButton(onClick = { viewModel.startScreener() }) {
                    Icon(Icons.Default.Refresh, contentDescription = "刷新")
                }
            }
        }

        Text(
            text = "共 $resultCount 只股票",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )

        Text(
            text = "预设策略",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.SemiBold
        )

        Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
            presets.chunked(4).forEach { row ->
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(6.dp)
                ) {
                    row.forEach { preset ->
                        FilterChip(
                            selected = selectedPreset == preset,
                            onClick = {
                                viewModel.applyPreset(preset)
                                viewModel.startScreener()
                            },
                            label = {
                                Text(preset, fontSize = 11.sp)
                            },
                            colors = FilterChipDefaults.filterChipColors(
                                selectedContainerColor = MaterialTheme.colorScheme.primary,
                                selectedLabelColor = MaterialTheme.colorScheme.onPrimary
                            ),
                            modifier = Modifier.weight(1f)
                        )
                    }
                }
            }
        }

        Card(modifier = Modifier.fillMaxWidth()) {
            Text(
                text = "当前条件: $filterSummary",
                modifier = Modifier.padding(10.dp),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }

        Button(
            onClick = { viewModel.startScreener() },
            modifier = Modifier.fillMaxWidth()
        ) {
            Text(if (isRunning) "筛选中..." else "开始选股")
        }

        LazyColumn(
            modifier = Modifier.fillMaxWidth().weight(1f),
            verticalArrangement = Arrangement.spacedBy(6.dp)
        ) {
            items(results, key = { it.symbol }) { stock ->
                StockResultItem(stock.symbol, stock.name, stock.price, stock.changePercent) {
                    selectedStock = stock
                }
            }
        }
    }

    if (showFilterSheet) {
        ModalBottomSheet(
            onDismissRequest = { showFilterSheet = false },
            sheetState = sheetState
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .verticalScroll(rememberScrollState())
                    .padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Text(
                    "筛选条件",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )

                RangeFields(
                    leftValue = minPrice,
                    onLeftChange = { minPrice = it },
                    leftLabel = "最低价格",
                    rightValue = maxPrice,
                    onRightChange = { maxPrice = it },
                    rightLabel = "最高价格"
                )

                RangeFields(
                    leftValue = minPE,
                    onLeftChange = { minPE = it },
                    leftLabel = "最低PE",
                    rightValue = maxPE,
                    onRightChange = { maxPE = it },
                    rightLabel = "最高PE"
                )

                RangeFields(
                    leftValue = minROE,
                    onLeftChange = { minROE = it },
                    leftLabel = "最低ROE%",
                    rightValue = maxROE,
                    onRightChange = { maxROE = it },
                    rightLabel = "最高ROE%"
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    val maUp = filter(ScreenerFilterType.MAUp)?.enabled == true
                    val macd = filter(ScreenerFilterType.MACD)?.enabled == true
                    val breakthrough = filter(ScreenerFilterType.Breakthrough)?.enabled == true
                    FilterChip(
                        selected = maUp,
                        onClick = { viewModel.setMA60UpFilter(!maUp) },
                        label = { Text("均线多头") }
                    )
                    FilterChip(
                        selected = macd,
                        onClick = { viewModel.setMACDGoldCrossFilter(!macd) },
                        label = { Text("MACD金叉") }
                    )
                    FilterChip(
                        selected = breakthrough,
                        onClick = { viewModel.setBreakthroughMA(!breakthrough) },
                        label = { Text("突破形态") }
                    )
                }

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = {
                            viewModel.resetFilters()
                            minPrice = "5"
                            maxPrice = "200"
                            minPE = "0"
                            maxPE = "50"
                            minROE = "10"
                            maxROE = "50"
                        },
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("重置")
                    }
                    Button(
                        onClick = {
                            viewModel.setPriceRange(minPrice.toDoubleOrNull() ?: 0.0, maxPrice.toDoubleOrNull() ?: 999999.0)
                            viewModel.setPERange(minPE.toDoubleOrNull() ?: 0.0, maxPE.toDoubleOrNull() ?: 999999.0)
                            viewModel.setROERange(minROE.toDoubleOrNull() ?: 0.0, maxROE.toDoubleOrNull() ?: 999999.0)
                            viewModel.startScreener()
                            showFilterSheet = false
                        },
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("应用筛选")
                    }
                }
            }
        }
    }

    selectedStock?.let { stock ->
        ModalBottomSheet(
            onDismissRequest = { selectedStock = null }
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                Text(
                    text = "${stock.name} ${stock.symbol}",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )
                Button(
                    onClick = {
                        selectedStock = null
                        onNavigateToMarket(stock.symbol)
                    },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("查看行情")
                }
                Button(
                    onClick = {
                        onAddFavorite(stock.symbol, stock.name)
                        selectedStock = null
                    },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("加入自选")
                }
                Button(
                    onClick = {
                        selectedStock = null
                        onNavigateToStrategy(stock.symbol)
                    },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text("创建策略")
                }
            }
        }
    }
}

@Composable
private fun RangeFields(
    leftValue: String,
    onLeftChange: (String) -> Unit,
    leftLabel: String,
    rightValue: String,
    onRightChange: (String) -> Unit,
    rightLabel: String
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        OutlinedTextField(
            value = leftValue,
            onValueChange = onLeftChange,
            label = { Text(leftLabel) },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
        OutlinedTextField(
            value = rightValue,
            onValueChange = onRightChange,
            label = { Text(rightLabel) },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
    }
}

private fun Double.toPlainText(): String {
    return if (this % 1.0 == 0.0) String.format("%.0f", this) else String.format("%.2f", this)
}

@Composable
fun StockResultItem(
    symbol: String,
    name: String,
    price: Double,
    changePercent: Double,
    onClick: () -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        onClick = onClick
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
                    text = name,
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    text = symbol,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            Column(horizontalAlignment = Alignment.End) {
                Text(
                    text = String.format("%.2f", price),
                    fontWeight = FontWeight.SemiBold,
                    color = if (changePercent >= 0) Color(0xFFE53935) else Color(0xFF43A047)
                )
                Text(
                    text = String.format("%+.2f%%", changePercent),
                    fontSize = 12.sp,
                    color = if (changePercent >= 0) Color(0xFFE53935) else Color(0xFF43A047)
                )
            }
        }
    }
}
