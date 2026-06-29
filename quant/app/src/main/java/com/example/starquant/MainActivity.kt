package com.example.starquant

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.slideInVertically
import androidx.compose.animation.slideOutVertically
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AccountBalance
import androidx.compose.material.icons.filled.FilterList
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Insights
import androidx.compose.material.icons.filled.Star
import androidx.compose.material.icons.filled.TrendingUp
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.zIndex
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.starquant.ui.screens.AccountScreen
import com.example.starquant.ui.screens.FavoritesScreen
import com.example.starquant.ui.screens.HomeScreen
import com.example.starquant.ui.screens.LoginScreen
import com.example.starquant.ui.screens.MarketScreen
import com.example.starquant.ui.screens.ScreenerScreen
import com.example.starquant.ui.screens.StrategyScreen
import com.example.starquant.ui.theme.StarQuantTheme
import com.example.starquant.ui.theme.BrandPrimary
import com.example.starquant.ui.theme.CardSurface
import com.example.starquant.ui.theme.FallGreen
import com.example.starquant.ui.theme.RiseRed
import com.example.starquant.ui.theme.TextSecondary
import com.example.starquant.ui.theme.TextTertiary
import com.example.starquant.ui.theme.ThinBorder
import com.example.starquant.ui.viewmodel.MainViewModel
import com.example.starquant.ui.viewmodel.TradeNotice
import kotlinx.coroutines.delay
import java.util.Locale

data class BottomNavItem(
    val label: String,
    val icon: ImageVector
)

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            StarQuantTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    StarQuantApp()
                }
            }
        }
    }
}

@Composable
fun StarQuantApp() {
    val viewModel: MainViewModel = viewModel()
    val loginState by viewModel.loginState.collectAsState()
    val tradeNotices by viewModel.tradeNotices.collectAsState()
    val items = listOf(
        BottomNavItem("首页", Icons.Filled.Home),
        BottomNavItem("自选", Icons.Filled.Star),
        BottomNavItem("行情", Icons.Filled.TrendingUp),
        BottomNavItem("选股", Icons.Filled.FilterList),
        BottomNavItem("策略", Icons.Filled.Insights),
        BottomNavItem("我的", Icons.Filled.AccountBalance)
    )
    var selectedIndex by remember { mutableIntStateOf(0) }

    if (!loginState.isLoggedIn) {
        LoginScreen(viewModel = viewModel, modifier = Modifier.fillMaxSize())
        return
    }

    Box(modifier = Modifier.fillMaxSize()) {
        Scaffold(
            modifier = Modifier.fillMaxSize(),
            bottomBar = {
                NavigationBar(
                    modifier = Modifier.fillMaxWidth(),
                    containerColor = CardSurface
                ) {
                    items.forEachIndexed { index, item ->
                        NavigationBarItem(
                            selected = selectedIndex == index,
                            onClick = { selectedIndex = index },
                            icon = { Icon(item.icon, contentDescription = item.label) },
                            label = { Text(item.label) },
                            colors = NavigationBarItemDefaults.colors(
                                selectedIconColor = BrandPrimary,
                                selectedTextColor = BrandPrimary,
                                unselectedIconColor = TextTertiary,
                                unselectedTextColor = TextTertiary,
                                indicatorColor = BrandPrimary.copy(alpha = 0.12f)
                            )
                        )
                    }
                }
            }
        ) { innerPadding ->
            val modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding)

            when (selectedIndex) {
                0 -> HomeScreen(
                    viewModel = viewModel,
                    onStockClick = { symbol ->
                        viewModel.setSymbol(symbol)
                        viewModel.fetchQuote()
                        viewModel.fetchIntraday()
                        selectedIndex = 2
                    },
                    modifier = modifier
                )
                1 -> FavoritesScreen(
                    viewModel = viewModel,
                    onStockClick = { symbol ->
                        viewModel.setSymbol(symbol)
                        viewModel.fetchQuote()
                        viewModel.fetchIntraday()
                        selectedIndex = 2
                    },
                    modifier = modifier
                )
                2 -> MarketScreen(viewModel = viewModel, modifier = modifier)
                3 -> ScreenerScreen(
                    viewModel = viewModel.getScreenerViewModel(),
                    onNavigateToMarket = { symbol ->
                        viewModel.setSymbol(symbol)
                        viewModel.fetchQuote()
                        viewModel.fetchIntraday()
                        selectedIndex = 2
                    },
                    onNavigateToStrategy = { symbol ->
                        viewModel.setSymbol(symbol)
                        selectedIndex = 4
                    },
                    onAddFavorite = { symbol, name ->
                        viewModel.addToFavorites(symbol, name)
                    },
                    modifier = modifier
                )
                4 -> StrategyScreen(viewModel = viewModel, modifier = modifier)
                5 -> AccountScreen(viewModel = viewModel, modifier = modifier)
            }
        }

        tradeNotices.firstOrNull()?.let { notice ->
            var visible by remember(notice.id) { mutableStateOf(true) }
            LaunchedEffect(notice.id, visible) {
                if (visible) {
                    delay(3400)
                    visible = false
                } else {
                    delay(220)
                    viewModel.dismissTradeNotice(notice.id)
                }
            }
            AnimatedVisibility(
                visible = visible,
                enter = slideInVertically(initialOffsetY = { -it }) + fadeIn(),
                exit = slideOutVertically(targetOffsetY = { -it }) + fadeOut(),
                modifier = Modifier
                    .align(Alignment.TopCenter)
                    .zIndex(20f)
            ) {
                TradeTopBanner(
                    notice = notice,
                    onDismiss = { visible = false }
                )
            }
        }
    }
}

@Composable
private fun TradeTopBanner(
    notice: TradeNotice,
    onDismiss: () -> Unit
) {
    val isBuy = notice.type == "买入"
    val accent = if (isBuy) RiseRed else FallGreen
    val title = if (isBuy) "买入成交" else "卖出成交"
    val displayName = notice.name.ifBlank { notice.symbol }

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .statusBarsPadding()
            .padding(horizontal = 16.dp, vertical = 10.dp)
            .clickable(onClick = onDismiss),
        shape = RoundedCornerShape(18.dp),
        border = BorderStroke(1.dp, ThinBorder),
        colors = CardDefaults.cardColors(containerColor = CardSurface),
        elevation = CardDefaults.cardElevation(defaultElevation = 12.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            Box(
                modifier = Modifier
                    .size(38.dp)
                    .clip(CircleShape)
                    .background(accent.copy(alpha = 0.12f)),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = if (isBuy) "买" else "卖",
                    color = accent,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )
            }
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = title,
                    color = accent,
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    text = "$displayName ${notice.symbol} · ${notice.volume.formatShares()}股 @ ${notice.price.formatPrice()}",
                    color = MaterialTheme.colorScheme.onSurface,
                    style = MaterialTheme.typography.bodySmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                Text(
                    text = "成交额 ${notice.amount.formatAmount()} · 费用 ${notice.fee.formatPrice()}",
                    color = TextSecondary,
                    style = MaterialTheme.typography.bodySmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
            }
            if (notice.type != "买入" && notice.type != "卖出") {
                Text(
                    text = notice.type,
                    color = accent,
                    style = MaterialTheme.typography.labelMedium,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier.width(36.dp)
                )
            }
        }
    }
}

private fun Double.formatPrice(): String {
    return String.format(Locale.US, "%.2f", this)
}

private fun Double.formatShares(): String {
    return String.format(Locale.US, "%.0f", this)
}

private fun Double.formatAmount(): String {
    return if (this >= 10000.0) {
        String.format(Locale.US, "%.2f万", this / 10000.0)
    } else {
        String.format(Locale.US, "%.0f", this)
    }
}
