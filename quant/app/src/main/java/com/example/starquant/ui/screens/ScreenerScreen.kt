package com.example.starquant.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.FilterList
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilterChip
import androidx.compose.material3.FilterChipDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
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
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.starquant.data.model.ScreenerFilter
import com.example.starquant.data.model.ScreenerFilterType
import com.example.starquant.data.model.SimilarStockAnalysis
import com.example.starquant.data.model.StockFeatureTag
import com.example.starquant.data.model.StockInfo
import com.example.starquant.ui.components.StarCard
import com.example.starquant.ui.components.StarPrimaryButton
import com.example.starquant.ui.theme.BrandPrimary
import com.example.starquant.ui.theme.TextSecondary
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
    val similarAnalysis by viewModel.similarAnalysis.collectAsState()
    val similarError by viewModel.similarError.collectAsState()
    val presets = viewModel.getPresetStrategies()

    var showFilterSheet by remember { mutableStateOf(false) }
    var selectedStock by remember { mutableStateOf<StockInfo?>(null) }
    var similarQuery by remember { mutableStateOf("") }
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
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "选股",
                style = MaterialTheme.typography.headlineMedium,
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

        SimilarStockCard(
            query = similarQuery,
            onQueryChange = { similarQuery = it },
            analysis = similarAnalysis,
            error = similarError,
            onAnalyze = { viewModel.analyzeSimilarStocks(similarQuery) }
        )

        Text(
            text = "预设策略",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.SemiBold
        )

        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            presets.chunked(2).forEach { row ->
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    row.forEach { preset ->
                        FilterChip(
                            selected = selectedPreset == preset,
                            onClick = {
                                viewModel.applyPreset(preset)
                                viewModel.startScreener()
                            },
                            label = {
                                Text(
                                    text = preset,
                                    modifier = Modifier.fillMaxWidth(),
                                    fontSize = 12.sp,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis,
                                    textAlign = TextAlign.Center
                                )
                            },
                            colors = FilterChipDefaults.filterChipColors(
                                selectedContainerColor = MaterialTheme.colorScheme.primary,
                                selectedLabelColor = MaterialTheme.colorScheme.onPrimary
                            ),
                            modifier = Modifier
                                .weight(1f)
                                .height(42.dp)
                        )
                    }
                    if (row.size == 1) {
                        Spacer(modifier = Modifier.weight(1f))
                    }
                }
            }
        }

        StarCard(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 12.dp, vertical = 6.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = filterSummary,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.weight(1f)
                )
                TextButton(onClick = { viewModel.resetFilters() }) {
                    Text("清除")
                }
            }
        }

        StarPrimaryButton(
            text = if (isRunning) "筛选中..." else "开始选股",
            onClick = { viewModel.startScreener() },
            modifier = Modifier
                .fillMaxWidth()
                .height(48.dp)
        )

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
                        label = {
                            Text("均线多头", maxLines = 1, overflow = TextOverflow.Ellipsis)
                        },
                        modifier = Modifier
                            .weight(1f)
                            .height(42.dp)
                    )
                    FilterChip(
                        selected = macd,
                        onClick = { viewModel.setMACDGoldCrossFilter(!macd) },
                        label = {
                            Text("MACD金叉", maxLines = 1, overflow = TextOverflow.Ellipsis)
                        },
                        modifier = Modifier
                            .weight(1f)
                            .height(42.dp)
                    )
                    FilterChip(
                        selected = breakthrough,
                        onClick = { viewModel.setBreakthroughMA(!breakthrough) },
                        label = {
                            Text("突破形态", maxLines = 1, overflow = TextOverflow.Ellipsis)
                        },
                        modifier = Modifier
                            .weight(1f)
                            .height(42.dp)
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
                            .height(48.dp)
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
                        modifier = Modifier
                            .weight(1f)
                            .height(48.dp)
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
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(48.dp)
                ) {
                    Text("查看行情")
                }
                Button(
                    onClick = {
                        onAddFavorite(stock.symbol, stock.name)
                        selectedStock = null
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(48.dp)
                ) {
                    Text("加入自选")
                }
                Button(
                    onClick = {
                        selectedStock = null
                        onNavigateToStrategy(stock.symbol)
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(48.dp)
                ) {
                    Text("创建策略")
                }
            }
        }
    }
}

@Composable
private fun SimilarStockCard(
    query: String,
    onQueryChange: (String) -> Unit,
    analysis: SimilarStockAnalysis?,
    error: String?,
    onAnalyze: () -> Unit
) {
    StarCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "相似股票",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold
                )
                analysis?.let {
                    Text(
                        text = "已筛出 ${it.results.size} 只",
                        style = MaterialTheme.typography.bodySmall,
                        color = TextSecondary
                    )
                }
            }
            OutlinedTextField(
                value = query,
                onValueChange = onQueryChange,
                label = { Text("股票代码或名称") },
                singleLine = true,
                modifier = Modifier.fillMaxWidth()
            )
            StarPrimaryButton(
                text = "分析相似",
                onClick = onAnalyze,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(48.dp)
            )
            error?.let {
                Text(
                    text = it,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.error
                )
            }
            analysis?.let {
                Text(
                    text = "${it.seed.name} ${it.seed.symbol}",
                    style = MaterialTheme.typography.bodyMedium,
                    fontWeight = FontWeight.SemiBold
                )
                FeatureTagGrid(tags = it.features)
            }
        }
    }
}

@Composable
private fun FeatureTagGrid(tags: List<StockFeatureTag>) {
    Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
        tags.chunked(2).forEach { row ->
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                row.forEach { tag ->
                    FeatureTagItem(
                        tag = tag,
                        modifier = Modifier
                            .weight(1f)
                            .height(58.dp)
                    )
                }
                if (row.size == 1) {
                    Spacer(modifier = Modifier.weight(1f))
                }
            }
        }
    }
}

@Composable
private fun FeatureTagItem(
    tag: StockFeatureTag,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .background(BrandPrimary.copy(alpha = 0.08f), RoundedCornerShape(10.dp))
            .padding(horizontal = 10.dp, vertical = 8.dp),
        verticalArrangement = Arrangement.Center
    ) {
        Text(
            text = tag.label,
            style = MaterialTheme.typography.labelMedium,
            color = BrandPrimary,
            fontWeight = FontWeight.Bold,
            maxLines = 1
        )
        Text(
            text = tag.detail,
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurface,
            maxLines = 1,
            overflow = TextOverflow.Ellipsis
        )
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
    StarCard(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
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
