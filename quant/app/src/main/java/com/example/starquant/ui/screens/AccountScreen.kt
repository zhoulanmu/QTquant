package com.example.starquant.ui.screens

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
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
import com.example.starquant.data.model.PositionInfo
import com.example.starquant.data.model.TradeRecord
import com.example.starquant.ui.components.StarCard
import com.example.starquant.ui.theme.FallGreen
import com.example.starquant.ui.theme.RiseRed
import com.example.starquant.ui.theme.TextSecondary
import com.example.starquant.ui.theme.WarningRed
import com.example.starquant.ui.viewmodel.MainViewModel

@Composable
fun AccountScreen(
    viewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    val accountState by viewModel.accountState.collectAsState()
    val positions = accountState.positions.values.toList()
    val records = accountState.tradeRecords
    var showResetConfirm by remember { mutableStateOf(false) }
    var showAllRecords by remember { mutableStateOf(false) }
    val displayRecords = if (showAllRecords) records else records.take(10)

    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = "我的",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold
        )

        StarCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = accountState.name,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
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
                    Text(
                        "总盈亏: ${String.format("%.2f", accountState.totalProfit)} (${String.format("%.2f", accountState.totalProfitPercent)}%)",
                        color = if (accountState.totalProfit >= 0) RiseRed else FallGreen,
                        fontWeight = FontWeight.SemiBold
                    )
                    Text(
                        "可用: ${String.format("%.2f", accountState.currentCash)}",
                        fontWeight = FontWeight.SemiBold
                    )
                }
            }
        }

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            StatMiniCard(
                label = "总资产",
                value = String.format("%.2f万", accountState.totalAssets / 10000),
                modifier = Modifier.weight(1f)
            )
            StatMiniCard(
                label = "持仓市值",
                value = String.format("%.2f万", (accountState.totalAssets - accountState.currentCash) / 10000),
                modifier = Modifier.weight(1f)
            )
            StatMiniCard(
                label = "持仓数",
                value = "${positions.size}",
                modifier = Modifier.weight(1f)
            )
        }

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = { showResetConfirm = true },
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.buttonColors(containerColor = WarningRed)
            ) {
                Text("重置账户")
            }
        }

        Text(
            text = "持仓列表",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold
        )

        if (positions.isEmpty()) {
            StarCard(modifier = Modifier.fillMaxWidth()) {
                Text(
                    text = "暂无持仓",
                    modifier = Modifier.padding(16.dp),
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        } else {
            LazyColumn(
                modifier = Modifier.fillMaxWidth().weight(1f),
                verticalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                items(positions) { position ->
                    PositionItem(position = position)
                }
            }
        }

        Text(
            text = "交易记录",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold
        )

        if (records.isEmpty()) {
            StarCard(modifier = Modifier.fillMaxWidth()) {
                Text(
                    text = "暂无交易记录",
                    modifier = Modifier.padding(16.dp),
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        } else {
            LazyColumn(
                modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
                verticalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                items(displayRecords) { record ->
                    TradeRecordItem(record = record)
                }
                if (records.size > 10 && !showAllRecords) {
                    item {
                        TextButton(
                            onClick = { showAllRecords = true },
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Text("查看全部 ${records.size} 条记录")
                        }
                    }
                }
            }
        }
    }

    if (showResetConfirm) {
        AlertDialog(
            onDismissRequest = { showResetConfirm = false },
            title = { Text("确认重置") },
            text = { Text("重置后所有持仓和交易记录将被清除，此操作不可撤销。") },
            confirmButton = {
                TextButton(
                    onClick = {
                        viewModel.resetAccount()
                        showResetConfirm = false
                        showAllRecords = false
                    }
                ) {
                    Text("确认重置", color = Color(0xFFE53935))
                }
            },
            dismissButton = {
                TextButton(onClick = { showResetConfirm = false }) {
                    Text("取消")
                }
            }
        )
    }
}

@Composable
fun StatMiniCard(
    label: String,
    value: String,
    modifier: Modifier = Modifier
) {
    StarCard(modifier = modifier) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(10.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text(
                text = label,
                style = MaterialTheme.typography.bodySmall,
                color = TextSecondary,
                fontSize = 11.sp
            )
            Text(
                text = value,
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold
            )
        }
    }
}

@Composable
fun PositionItem(position: PositionInfo) {
    StarCard(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(10.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text(
                    text = position.name.ifBlank { position.symbol },
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    text = "${position.quantity.toInt()}股 @ ${String.format("%.2f", position.avgCost)}",
                    style = MaterialTheme.typography.bodySmall,
                    color = TextSecondary,
                    fontSize = 11.sp
                )
            }
            Column(horizontalAlignment = Alignment.End) {
                val profitColor = if (position.profit >= 0) RiseRed else FallGreen
                Text(
                    text = String.format("%.2f", position.currentPrice),
                    fontWeight = FontWeight.SemiBold,
                    color = profitColor
                )
                Text(
                    text = "${String.format("%+.2f", position.profit)} (${String.format("%+.2f%%", position.profitPercent)})",
                    fontSize = 12.sp,
                    color = profitColor
                )
            }
        }
    }
}

@Composable
fun TradeRecordItem(record: TradeRecord) {
    val isBuy = record.type == "买入"
    val color = if (isBuy) RiseRed else FallGreen

    StarCard(modifier = Modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(8.dp),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Column {
                Text(
                    text = "${record.type} ${record.symbol}",
                    fontWeight = FontWeight.SemiBold,
                    color = color,
                    fontSize = 13.sp
                )
                Text(
                    text = record.time,
                    style = MaterialTheme.typography.bodySmall,
                    fontSize = 11.sp,
                    color = TextSecondary
                )
            }
            Column(horizontalAlignment = Alignment.End) {
                Text(
                    text = String.format("%.2f", record.price),
                    fontWeight = FontWeight.SemiBold,
                    fontSize = 13.sp
                )
                Text(
                    text = "${record.volume.toInt()}股",
                    fontSize = 11.sp,
                    color = TextSecondary
                )
            }
        }
    }
}
