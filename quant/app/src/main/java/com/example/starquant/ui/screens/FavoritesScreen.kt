package com.example.starquant.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Search
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
import com.example.starquant.data.model.MarketData
import com.example.starquant.ui.components.ChangePill
import com.example.starquant.ui.components.SectionHeader
import com.example.starquant.ui.components.StarCard
import com.example.starquant.ui.components.StarPrimaryButton
import com.example.starquant.ui.theme.FallGreen
import com.example.starquant.ui.theme.RiseRed
import com.example.starquant.ui.theme.TextSecondary
import com.example.starquant.ui.theme.TextTertiary
import com.example.starquant.ui.viewmodel.MainViewModel

@Composable
fun FavoritesScreen(
    viewModel: MainViewModel,
    onStockClick: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val favorites by viewModel.favoriteStocks.collectAsState()
    val favoriteQuotes by viewModel.favoriteQuotes.collectAsState()
    var searchText by remember { mutableStateOf("") }

    LaunchedEffect(favorites) {
        viewModel.refreshFavoriteQuotes()
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
                text = "自选",
                style = MaterialTheme.typography.headlineMedium,
                fontWeight = FontWeight.Bold
            )
            Box(
                modifier = Modifier
                    .background(Color.White, CircleShape)
                    .padding(10.dp),
                contentAlignment = Alignment.Center
            ) {
                Icon(Icons.Filled.Add, contentDescription = "添加", tint = MaterialTheme.colorScheme.onSurface)
            }
        }

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            OutlinedTextField(
                value = searchText,
                onValueChange = { searchText = it },
                modifier = Modifier.weight(1f),
                singleLine = true,
                placeholder = { Text("搜索添加股票") },
                leadingIcon = { Icon(Icons.Filled.Search, contentDescription = null) }
            )
            StarPrimaryButton(
                text = "添加",
                onClick = {
                    if (searchText.isNotBlank()) {
                        viewModel.addToFavorites(searchText, searchText.uppercase())
                        searchText = ""
                    }
                }
            )
        }

        StarCard(modifier = Modifier.fillMaxWidth().weight(1f)) {
            Column(
                modifier = Modifier.padding(14.dp),
                verticalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                SectionHeader(title = "自选股", action = "编辑 ${favorites.size}只")
                if (favorites.isEmpty()) {
                    Text(
                        text = "暂无自选股",
                        style = MaterialTheme.typography.bodyMedium,
                        color = TextSecondary,
                        modifier = Modifier.padding(vertical = 24.dp)
                    )
                } else {
                    LazyColumn(
                        verticalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        items(favorites.toList(), key = { it.first }) { (symbol, name) ->
                            FavoriteStockRow(
                                symbol = symbol,
                                name = name,
                                quote = favoriteQuotes[symbol],
                                onClick = { onStockClick(symbol) },
                                onRemove = { viewModel.removeFromFavorites(symbol) }
                            )
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun FavoriteStockRow(
    symbol: String,
    name: String,
    quote: MarketData?,
    onClick: () -> Unit,
    onRemove: () -> Unit
) {
    val change = quote?.changePercent ?: 0.0
    val color = if (change >= 0.0) RiseRed else FallGreen
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(vertical = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Row(
            horizontalArrangement = Arrangement.spacedBy(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .width(3.dp)
                    .height(54.dp)
                    .background(if (quote == null) TextTertiary else color)
            )
            Column {
                Text(
                    text = name,
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    text = symbol,
                    style = MaterialTheme.typography.bodyMedium,
                    color = TextSecondary
                )
            }
        }
        Row(
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(horizontalAlignment = Alignment.End) {
                Text(
                    text = quote?.let { String.format("%.2f", it.close) } ?: "--",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold,
                    color = if (quote == null) TextTertiary else color
                )
                if (quote != null) {
                    ChangePill(change = change, text = String.format("%+.2f%%", change))
                } else {
                    Text("--", style = MaterialTheme.typography.bodySmall, color = TextTertiary)
                }
            }
            IconButton(onClick = onRemove) {
                Icon(
                    imageVector = Icons.Filled.Delete,
                    contentDescription = "删除",
                    tint = TextSecondary
                )
            }
        }
    }
}
