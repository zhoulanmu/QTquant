package com.example.starquant

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AccountBalance
import androidx.compose.material.icons.filled.FilterList
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Insights
import androidx.compose.material.icons.filled.Star
import androidx.compose.material.icons.filled.TrendingUp
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
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
import com.example.starquant.ui.theme.TextTertiary
import com.example.starquant.ui.viewmodel.MainViewModel

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
}
