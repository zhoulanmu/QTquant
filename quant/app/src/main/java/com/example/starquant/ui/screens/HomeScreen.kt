package com.example.starquant.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.NotificationsNone
import androidx.compose.material.icons.filled.Person
import androidx.compose.material3.Icon
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
import com.example.starquant.data.model.MarketData
import com.example.starquant.ui.components.ChangePill
import com.example.starquant.ui.components.ChangeText
import com.example.starquant.ui.components.RankBadge
import com.example.starquant.ui.components.SectionHeader
import com.example.starquant.ui.components.StarCard
import com.example.starquant.ui.theme.FallGreen
import com.example.starquant.ui.theme.RiseRed
import com.example.starquant.ui.theme.TextSecondary
import com.example.starquant.ui.theme.TextTertiary
import com.example.starquant.ui.theme.TodayAmber
import com.example.starquant.ui.viewmodel.MainViewModel
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

private data class HotStock(
    val rank: Int,
    val name: String,
    val symbol: String,
    val changePercent: Double
)

private data class HotNews(
    val title: String,
    val summary: String,
    val tag: String
)

@Composable
fun HomeScreen(
    viewModel: MainViewModel,
    onStockClick: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val indexQuotes by viewModel.marketIndexQuotes.collectAsState()
    val sdf = SimpleDateFormat("MM-dd HH:mm", Locale.getDefault())
    val hotStocks = listOf(
        HotStock(1, "广钢气体", "688548.SH", 20.01),
        HotStock(2, "正帆科技", "688596.SH", 17.31),
        HotStock(3, "万邦医药", "301520.SZ", 20.00),
        HotStock(4, "宁德时代", "300750.SZ", 3.25),
        HotStock(5, "贵州茅台", "600519.SH", -0.22)
    )
    val hotNews = listOf(
        HotNews("苹果或采购长鑫内存以缓解成本压力", "广钢气体 +20.01% / 正帆科技 +17.31%", "投票"),
        HotNews("韩国巨资加码半导体，中国 CXO 景气上行？", "礼来 -0.22% / 万邦医药 +20.00%", "自选")
    )

    LaunchedEffect(Unit) {
        viewModel.refreshMarketIndexQuotes()
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        HomeHeader()

        MarketIndexPanel(
            quotes = indexQuotes,
            indexNames = viewModel.marketIndexDefinitions.associate { it.symbol to it.name },
            timeText = sdf.format(Date())
        )

        StarCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(14.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                SectionHeader(title = "热股榜", action = "更多 >")
                hotStocks.forEach { stock ->
                    HotStockRow(stock = stock, onClick = { onStockClick(stock.symbol) })
                }
            }
        }

        StarCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(14.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                SectionHeader(title = "热点榜", action = "更多 >")
                hotNews.forEach { news ->
                    HotNewsRow(news = news)
                }
            }
        }
    }
}

@Composable
private fun HomeHeader() {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(
                text = "牛翻天",
                style = MaterialTheme.typography.headlineMedium,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = "量化交易",
                style = MaterialTheme.typography.bodyMedium,
                color = TextSecondary
            )
        }
        Row(
            horizontalArrangement = Arrangement.spacedBy(10.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                imageVector = Icons.Filled.NotificationsNone,
                contentDescription = "通知",
                tint = MaterialTheme.colorScheme.onSurface
            )
            Box(
                modifier = Modifier
                    .size(36.dp)
                    .background(MaterialTheme.colorScheme.primary, CircleShape),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = Icons.Filled.Person,
                    contentDescription = "我的",
                    tint = Color.White,
                    modifier = Modifier.size(22.dp)
                )
            }
        }
    }
}

@Composable
private fun MarketIndexPanel(
    quotes: Map<String, MarketData>,
    indexNames: Map<String, String>,
    timeText: String
) {
    StarCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(14.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Column {
                    Text(
                        text = "大盘指数",
                        style = MaterialTheme.typography.titleMedium,
                        color = TextTertiary
                    )
                    Text(
                        text = "已收盘 $timeText",
                        style = MaterialTheme.typography.bodySmall,
                        color = TextSecondary
                    )
                }
                Text(
                    text = "L1 实时行情",
                    style = MaterialTheme.typography.labelMedium,
                    color = TodayAmber,
                    fontWeight = FontWeight.Bold
                )
            }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .horizontalScroll(rememberScrollState()),
                horizontalArrangement = Arrangement.spacedBy(18.dp)
            ) {
                indexNames.forEach { (symbol, name) ->
                    MarketIndexTile(
                        name = name,
                        quote = quotes[symbol],
                        modifier = Modifier.width(112.dp)
                    )
                }
            }
        }
    }
}

@Composable
private fun MarketIndexTile(
    name: String,
    quote: MarketData?,
    modifier: Modifier = Modifier
) {
    val change = quote?.changePercent ?: 0.0
    val color = if (change >= 0.0) RiseRed else FallGreen
    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(3.dp)
    ) {
        Text(
            text = name,
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold
        )
        Text(
            text = quote?.let { String.format("%.2f", it.close) } ?: "--",
            style = MaterialTheme.typography.headlineMedium,
            color = if (quote == null) TextTertiary else color,
            fontWeight = FontWeight.Bold
        )
        Text(
            text = quote?.let { String.format("%+.2f  %+.2f%%", it.changeAmount, it.changePercent) } ?: "--",
            style = MaterialTheme.typography.bodyMedium,
            color = if (quote == null) TextTertiary else color
        )
    }
}

@Composable
private fun HotStockRow(
    stock: HotStock,
    onClick: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(vertical = 3.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Row(
            horizontalArrangement = Arrangement.spacedBy(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            RankBadge(rank = stock.rank)
            Text(
                text = stock.name,
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = if (stock.rank <= 3) TodayAmber else MaterialTheme.colorScheme.onSurface
            )
        }
        ChangeText(
            change = stock.changePercent,
            text = String.format("%+.2f%%", stock.changePercent)
        )
    }
}

@Composable
private fun HotNewsRow(news: HotNews) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(
            modifier = Modifier.weight(1f),
            verticalArrangement = Arrangement.spacedBy(3.dp)
        ) {
            Text(
                text = news.title,
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = news.summary,
                style = MaterialTheme.typography.bodySmall,
                color = TextSecondary
            )
        }
        Spacer(modifier = Modifier.width(10.dp))
        ChangePill(change = 1.0, text = news.tag)
    }
}
