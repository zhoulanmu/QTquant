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
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ExtendedFloatingActionButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Switch
import androidx.compose.material3.Tab
import androidx.compose.material3.TabRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.starquant.data.model.StrategyConfig
import com.example.starquant.data.model.StrategyType
import com.example.starquant.ui.components.StarCard
import com.example.starquant.ui.viewmodel.MainViewModel
import kotlinx.coroutines.launch

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun StrategyScreen(
    viewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    val runtimes by viewModel.strategyRuntimes.collectAsState()
    val config by viewModel.strategyConfig.collectAsState()
    val selectedType by viewModel.selectedStrategyType.collectAsState()
    var selectedTab by remember { mutableIntStateOf(0) }
    var showAddDialog by remember { mutableStateOf(false) }
    val snackbarHostState = remember { SnackbarHostState() }
    val scope = rememberCoroutineScope()
    val showSaved: () -> Unit = {
        scope.launch { snackbarHostState.showSnackbar("配置已保存") }
    }

    val tabs = listOf("运行中", "策略配置")

    Scaffold(
        modifier = modifier.fillMaxSize(),
        snackbarHost = { SnackbarHost(snackbarHostState) },
        floatingActionButton = {
            if (selectedTab == 0) {
                ExtendedFloatingActionButton(
                    onClick = { showAddDialog = true },
                    icon = { Icon(Icons.Filled.Add, contentDescription = null) },
                    text = { Text("新建策略") }
                )
            }
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(horizontal = 16.dp, vertical = 12.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = "策略",
                style = MaterialTheme.typography.headlineMedium,
                fontWeight = FontWeight.Bold
            )

            TabRow(selectedTabIndex = selectedTab) {
                tabs.forEachIndexed { index, title ->
                    Tab(
                        selected = selectedTab == index,
                        onClick = { selectedTab = index },
                        text = { Text(title) }
                    )
                }
            }

            when (selectedTab) {
                0 -> {
                    if (runtimes.isEmpty()) {
                        StarCard(
                            modifier = Modifier.fillMaxWidth().padding(vertical = 32.dp)
                        ) {
                            Text(
                                text = "暂无运行中的策略\n点击右下角按钮新建策略",
                                modifier = Modifier.padding(24.dp),
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    } else {
                        LazyColumn(
                            verticalArrangement = Arrangement.spacedBy(8.dp)
                        ) {
                            items(runtimes, key = { it.id }) { runtime ->
                                StrategyRuntimeCard(
                                    runtime = runtime,
                                    onStop = { viewModel.stopStrategy(runtime.id) }
                                )
                            }
                        }
                    }
                }
                1 -> {
                    Column(
                        modifier = Modifier.verticalScroll(rememberScrollState()),
                        verticalArrangement = Arrangement.spacedBy(10.dp)
                    ) {
                        Text(
                            text = "策略类型",
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.SemiBold
                        )
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(8.dp)
                        ) {
                            Button(
                                onClick = { viewModel.setStrategyType(StrategyType.DoubleMA) },
                                modifier = Modifier.weight(1f)
                            ) {
                                Text("双均线策略")
                            }
                            Button(
                                onClick = { viewModel.setStrategyType(StrategyType.ProsperityGrowth) },
                                modifier = Modifier.weight(1f)
                            ) {
                                Text("景气成长策略")
                            }
                        }

                        HorizontalDivider()

                        when (selectedType) {
                            StrategyType.DoubleMA -> DoubleMAConfigSection(viewModel, showSaved)
                            StrategyType.ProsperityGrowth -> ProsperityGrowthConfigSection(viewModel, showSaved)
                        }
                    }
                }
            }
        }
    }

    if (showAddDialog) {
        AlertDialog(
            onDismissRequest = { showAddDialog = false },
            title = { Text("启动策略确认") },
            text = { StrategyStartSummary(config) },
            confirmButton = {
                TextButton(onClick = {
                    viewModel.startStrategy()
                    showAddDialog = false
                }) {
                    Text("启动")
                }
            },
            dismissButton = {
                TextButton(onClick = { showAddDialog = false }) {
                    Text("取消")
                }
            }
        )
    }
}

@Composable
fun StrategyRuntimeCard(
    runtime: com.example.starquant.ui.viewmodel.StrategyRuntime,
    onStop: () -> Unit
) {
    val typeName = when (runtime.config.strategyType) {
        StrategyType.DoubleMA -> "双均线策略"
        StrategyType.ProsperityGrowth -> "景气成长策略"
    }

    StarCard(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        Icons.Filled.PlayArrow,
                        contentDescription = null,
                        tint = Color(0xFF43A047),
                        modifier = Modifier.padding(end = 4.dp)
                    )
                    Text(
                        text = "策略 ${runtime.id} · $typeName",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )
                }
                Text(
                    text = runtime.config.symbol,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Text(
                    text = "信号数: ${runtime.signals.size}",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                runtime.signals.firstOrNull()?.let { signal ->
                    Text(
                        text = "最新: ${signal.comment}",
                        style = MaterialTheme.typography.bodySmall,
                        fontSize = 11.sp
                    )
                }
            }
            IconButton(onClick = onStop) {
                Icon(Icons.Filled.Stop, contentDescription = "停止", tint = Color(0xFFE53935))
            }
        }
    }
}

@Composable
private fun StrategyStartSummary(config: StrategyConfig) {
    val typeName = when (config.strategyType) {
        StrategyType.DoubleMA -> "双均线策略"
        StrategyType.ProsperityGrowth -> "景气成长策略"
    }
    Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("策略类型: $typeName")
        Text("标的: ${config.symbol}")
        if (config.strategyType == StrategyType.DoubleMA) {
            Text("快线/慢线: ${config.doubleMAConfig.fastMA}/${config.doubleMAConfig.slowMA}")
            Text("K线周期: ${config.doubleMAConfig.barPeriodMinutes}分钟")
        } else {
            Text("启用方向: ${config.growthBuyConfig.enabledTracks.joinToString().ifBlank { "景气主线" }}")
            Text("人工确认: ${if (config.growthBuyConfig.profitGrowthConfirmed && config.growthBuyConfig.orderLandingConfirmed && config.growthBuyConfig.noPureConceptConfirmed) "已通过" else "未完全通过"}")
        }
        Text("止损/止盈: ${config.riskConfig.stopLossPercent}% / ${config.riskConfig.takeProfitPercent}%")
        Text("下单: ${config.positionConfig.lotSize.toInt()}股")
        Text("仓位上限: ${config.positionConfig.maxSingleTrackPercent}%")
    }
}

@Composable
fun DoubleMAConfigSection(
    viewModel: MainViewModel,
    onSaved: () -> Unit
) {
    val config by viewModel.strategyConfig.collectAsState()
    val dm = config.doubleMAConfig
    val risk = config.riskConfig
    val pos = config.positionConfig

    var fastMA by remember(dm.fastMA) { mutableStateOf(dm.fastMA.toString()) }
    var slowMA by remember(dm.slowMA) { mutableStateOf(dm.slowMA.toString()) }
    var period by remember(dm.barPeriodMinutes) { mutableStateOf(dm.barPeriodMinutes.toString()) }
    var stopLoss by remember(risk.stopLossPercent) { mutableStateOf(risk.stopLossPercent.toString()) }
    var takeProfit by remember(risk.takeProfitPercent) { mutableStateOf(risk.takeProfitPercent.toString()) }
    var lotSize by remember(pos.lotSize) { mutableStateOf(pos.lotSize.toString()) }
    var symbol by remember(config.symbol) { mutableStateOf(config.symbol) }

    Text(
        text = "双均线策略配置",
        style = MaterialTheme.typography.titleMedium,
        fontWeight = FontWeight.Bold
    )

    OutlinedTextField(
        value = symbol,
        onValueChange = { symbol = it },
        label = { Text("股票代码") },
        modifier = Modifier.fillMaxWidth(),
        singleLine = true
    )

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        OutlinedTextField(
            value = fastMA,
            onValueChange = { fastMA = it },
            label = { Text("快线周期") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
        OutlinedTextField(
            value = slowMA,
            onValueChange = { slowMA = it },
            label = { Text("慢线周期") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
    }

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        OutlinedTextField(
            value = period,
            onValueChange = { period = it },
            label = { Text("K线周期(分钟)") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
        OutlinedTextField(
            value = lotSize,
            onValueChange = { lotSize = it },
            label = { Text("下单股数") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
    }

    Text(
        text = "风险管理",
        style = MaterialTheme.typography.titleMedium,
        fontWeight = FontWeight.SemiBold
    )

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        OutlinedTextField(
            value = stopLoss,
            onValueChange = { stopLoss = it },
            label = { Text("止损%") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
        OutlinedTextField(
            value = takeProfit,
            onValueChange = { takeProfit = it },
            label = { Text("止盈%") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
    }

    Button(
        onClick = {
            val newConfig = config.copy(
                symbol = symbol.uppercase(),
                strategyType = StrategyType.DoubleMA,
                doubleMAConfig = dm.copy(
                    fastMA = fastMA.toIntOrNull() ?: 5,
                    slowMA = slowMA.toIntOrNull() ?: 20,
                    barPeriodMinutes = period.toIntOrNull() ?: 5
                ),
                riskConfig = risk.copy(
                    stopLossPercent = stopLoss.toDoubleOrNull() ?: 2.0,
                    takeProfitPercent = takeProfit.toDoubleOrNull() ?: 5.0
                ),
                positionConfig = pos.copy(
                    lotSize = lotSize.toDoubleOrNull() ?: 100.0
                )
            )
            viewModel.updateStrategyConfig(newConfig)
            onSaved()
        },
        modifier = Modifier.fillMaxWidth()
    ) {
        Text("保存配置")
    }
}

@Composable
fun ProsperityGrowthConfigSection(
    viewModel: MainViewModel,
    onSaved: () -> Unit
) {
    val config by viewModel.strategyConfig.collectAsState()
    val growth = config.growthBuyConfig
    val risk = config.riskConfig

    var symbol by remember(config.symbol) { mutableStateOf(config.symbol) }
    var stopLoss by remember(risk.stopLossPercent) { mutableStateOf(risk.stopLossPercent.toString()) }
    var takeProfit by remember(risk.takeProfitPercent) { mutableStateOf(risk.takeProfitPercent.toString()) }
    var surgeTP by remember(risk.surgeTakeProfitPercent) { mutableStateOf(risk.surgeTakeProfitPercent.toString()) }
    var profitGrowthConfirmed by remember(growth.profitGrowthConfirmed) { mutableStateOf(growth.profitGrowthConfirmed) }
    var orderLandingConfirmed by remember(growth.orderLandingConfirmed) { mutableStateOf(growth.orderLandingConfirmed) }
    var noPureConceptConfirmed by remember(growth.noPureConceptConfirmed) { mutableStateOf(growth.noPureConceptConfirmed) }
    var requireMA60Up by remember(growth.requireMA60Up) { mutableStateOf(growth.requireMA60Up) }
    var requireVolumePullback by remember(growth.requireVolumePullback) { mutableStateOf(growth.requireVolumePullback) }
    var requirePullback by remember(growth.requirePullbackToMA20OrMA60) { mutableStateOf(growth.requirePullbackToMA20OrMA60) }

    Text(
        text = "景气成长策略配置",
        style = MaterialTheme.typography.titleMedium,
        fontWeight = FontWeight.Bold
    )

    OutlinedTextField(
        value = symbol,
        onValueChange = { symbol = it },
        label = { Text("股票代码") },
        modifier = Modifier.fillMaxWidth(),
        singleLine = true
    )

    Text(
        text = "人工确认项",
        style = MaterialTheme.typography.titleMedium,
        fontWeight = FontWeight.SemiBold
    )
    SwitchRow("利润增长确认", profitGrowthConfirmed) { profitGrowthConfirmed = it }
    SwitchRow("订单落地确认", orderLandingConfirmed) { orderLandingConfirmed = it }
    SwitchRow("非纯概念确认", noPureConceptConfirmed) { noPureConceptConfirmed = it }

    Text(
        text = "技术条件",
        style = MaterialTheme.typography.titleMedium,
        fontWeight = FontWeight.SemiBold
    )
    SwitchRow("60均线向上", requireMA60Up) { requireMA60Up = it }
    SwitchRow("缩量回踩", requireVolumePullback) { requireVolumePullback = it }
    SwitchRow("回踩MA20/60", requirePullback) { requirePullback = it }
    Text(text = "回踩天数: ${growth.pullbackMinDays}-${growth.pullbackMaxDays}天")

    Text(
        text = "风险管理",
        style = MaterialTheme.typography.titleMedium,
        fontWeight = FontWeight.SemiBold
    )

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        OutlinedTextField(
            value = stopLoss,
            onValueChange = { stopLoss = it },
            label = { Text("止损%") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
        OutlinedTextField(
            value = takeProfit,
            onValueChange = { takeProfit = it },
            label = { Text("止盈%") },
            modifier = Modifier.weight(1f),
            singleLine = true
        )
    }

    OutlinedTextField(
        value = surgeTP,
        onValueChange = { surgeTP = it },
        label = { Text("暴涨止盈%") },
        modifier = Modifier.fillMaxWidth(),
        singleLine = true
    )

    Button(
        onClick = {
            val newConfig = config.copy(
                symbol = symbol.uppercase(),
                strategyType = StrategyType.ProsperityGrowth,
                growthBuyConfig = growth.copy(
                    enabledTracks = growth.enabledTracks.ifEmpty { listOf("景气主线") },
                    profitGrowthConfirmed = profitGrowthConfirmed,
                    orderLandingConfirmed = orderLandingConfirmed,
                    noPureConceptConfirmed = noPureConceptConfirmed,
                    requireMA60Up = requireMA60Up,
                    requireVolumePullback = requireVolumePullback,
                    requirePullbackToMA20OrMA60 = requirePullback
                ),
                riskConfig = risk.copy(
                    stopLossPercent = stopLoss.toDoubleOrNull() ?: 2.0,
                    takeProfitPercent = takeProfit.toDoubleOrNull() ?: 5.0,
                    surgeTakeProfitPercent = surgeTP.toDoubleOrNull() ?: 25.0
                )
            )
            viewModel.updateStrategyConfig(newConfig)
            onSaved()
        },
        modifier = Modifier.fillMaxWidth()
    ) {
        Text("保存配置")
    }
}

@Composable
private fun SwitchRow(
    label: String,
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(label, style = MaterialTheme.typography.bodyMedium)
        Switch(checked = checked, onCheckedChange = onCheckedChange)
    }
}
