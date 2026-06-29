# StarQuant 量化交易 App 改良方案

## 一、选股页（最高优先级）

### 1.1 预设策略选中状态可视化

**现状**：`FilterChip(selected = false, ...)` 写死 false，点了哪个预设看不出来。

**改良**：
- 维护一个 `selectedPreset: String?` 状态，记录当前选中的预设名
- `FilterChip(selected = selectedPreset == preset.name)`
- 选中态用蓝色填充 + 白色文字，未选中用灰色边框
- 用户手动修改筛选条件（打开 BottomSheet 并应用）时，清除 `selectedPreset`（因为已经不再是预设条件了）

```
选中"强势股选股"后 Chip 变蓝 -> 筛出结果
用户打开 BottomSheet 手动改了 PE 范围 -> 所有 Chip 恢复未选中
```

### 1.2 BottomSheet 筛选条件回显

**现状**：每次打开 Sheet 看到的都是写死的默认值 `5/200/0/50/10/50`。

**改良**：
- Sheet 内的 `minPrice`/`maxPrice` 等从 ViewModel 的 filter 状态初始化，而非写死
- 打开 Sheet 时读取当前生效的筛选条件并回填到输入框

```kotlin
// 打开 Sheet 时初始化
val currentFilters = viewModel.getFilters()
var minPrice by remember(currentFilters) {
    mutableStateOf(
        currentFilters.find { it.type == ScreenerFilterType.PriceRange }?.minValue?.toString() ?: "5"
    )
}
```

### 1.3 筛选结果支持操作（核心链路打通）

**现状**：`StockResultItem` 的 onClick 是空实现，选股结果是一个终点。

**改良**：点击选股结果弹出 BottomSheet 或长按菜单，提供三个操作：

| 操作 | 行为 |
|------|------|
| 查看详情 | 跳转行情页并自动查询该股票 |
| 加入自选 | 添加到首页自选列表（持久化） |
| 创建策略 | 跳转策略页并预填该股票代码 |

实现方式：

```kotlin
StockResultItem(stock.symbol, stock.name, stock.price, stock.changePercent) {
    showStockActionSheet = true
    selectedStock = stock
}

// BottomSheet 内
Row(horizontalArrangement = Arrangement.SpaceEvenly) {
    Button(onClick = {
        viewModel.setSymbol(selectedStock.symbol)
        // 通知 MainActivity 切换到行情 Tab
        onNavigateToMarket(selectedStock.symbol)
        showStockActionSheet = false
    }) { Text("查看行情") }

    Button(onClick = {
        viewModel.addToFavorites(selectedStock.symbol, selectedStock.name)
        showStockActionSheet = false
    }) { Text("加入自选") }

    Button(onClick = {
        viewModel.setSymbol(selectedStock.symbol)
        onNavigateToStrategy(selectedStock.symbol)
        showStockActionSheet = false
    }) { Text("创建策略") }
}
```

### 1.4 筛选条件摘要展示

**现状**：用户不知道当前生效的是什么条件。

**改良**：在预设策略区域下方、结果列表上方，增加一行条件摘要：

```
当前条件: 价格 5-200 | PE 0-50 | ROE 10-50% | 均线多头 ✓
```

当有筛选条件生效时才显示，无自定义条件时显示"默认（全部股票）"。

### 1.5 "重置"按钮逻辑修正

**现状**：Sheet 内的"重置"只调用了 `viewModel.resetFilters()`，但 Sheet 内的本地变量不更新。

**改良**：重置时同步清空本地变量：

```kotlin
Button(onClick = {
    viewModel.resetFilters()
    // 同步重置本地状态
    minPrice = "5"; maxPrice = "200"
    minPE = "0"; maxPE = "50"
    minROE = "10"; maxROE = "50"
}) { Text("重置") }
```

---

## 二、页面间联动（高优先级）

### 2.1 首页自选股票点击 -> 跳转行情页

**现状**：只 `setSymbol`，不切 Tab。

**改良**：通过回调通知 MainActivity 切换 Tab：

```kotlin
// HomeScreen 增加参数
fun HomeScreen(
    viewModel: MainViewModel,
    onStockClick: (String) -> Unit,  // 新增
    modifier: Modifier
)

// MainActivity 中传递
0 -> HomeScreen(
    viewModel = viewModel,
    onStockClick = { symbol ->
        viewModel.setSymbol(symbol)
        selectedIndex = 1  // 切到行情页
    },
    modifier = modifier
)
```

### 2.2 自选列表持久化与管理

**现状**：自选是写死的 `val favoriteStocks = listOf(...)` 不可变列表，行情页的收藏星标是纯 UI 状态。

**改良**：
- 自选列表改为 `MutableStateFlow`，支持增删
- 首页自选列表增加左滑删除或长按移除
- 行情页的星标操作调用 `viewModel.toggleFavorite(symbol, name)`，同步更新首页自选

```kotlin
private val _favoriteStocks = MutableStateFlow(
    mutableMapOf(
        "600519.SH" to "贵州茅台",
        "000001.SZ" to "平安银行",
        "600036.SH" to "招商银行",
        "002594.SZ" to "比亚迪",
        "300750.SZ" to "宁德时代"
    )
)

fun addToFavorites(symbol: String, name: String) {
    _favoriteStocks.value[symbol] = name
}

fun removeFromFavorites(symbol: String) {
    _favoriteStocks.value.remove(symbol)
}

fun toggleFavorite(symbol: String, name: String) {
    if (_favoriteStocks.value.containsKey(symbol)) {
        removeFromFavorites(symbol)
    } else {
        addToFavorites(symbol, name)
    }
}
```

行情页的星标状态从 ViewModel 获取：

```kotlin
val favorites by viewModel.favoriteStocks.collectAsState()
var isFavorite by remember { mutableStateOf(favorites.containsKey(currentSymbol)) }

// 查询成功后同步
LaunchedEffect(currentSymbol) {
    isFavorite = favorites.containsKey(currentSymbol)
}

IconButton(onClick = {
    viewModel.toggleFavorite(currentSymbol, quoteData?.name ?: currentSymbol)
    isFavorite = !isFavorite
})
```

### 2.3 选股结果 <-> 策略页/行情页联动

通过 MainActivity 层的回调统一管理页面跳转：

```kotlin
// ScreenerScreen 增加回调
fun ScreenerScreen(
    viewModel: ScreenerViewModel,
    onNavigateToMarket: (String) -> Unit,
    onNavigateToStrategy: (String) -> Unit,
    modifier: Modifier
)

// MainActivity 传递
2 -> ScreenerScreen(
    viewModel = viewModel.getScreenerViewModel(),
    onNavigateToMarket = { symbol ->
        viewModel.setSymbol(symbol)
        selectedIndex = 1  // 行情
    },
    onNavigateToStrategy = { symbol ->
        viewModel.setSymbol(symbol)
        selectedIndex = 3  // 策略
    },
    modifier = modifier
)
```

---

## 三、策略页（中优先级）

### 3.1 多策略并行运行支持

**现状**：`strategyJob` 是单一 Job，多个策略会互相覆盖。

**改良**：每个 StrategyRuntime 持有独立的 Job：

```kotlin
data class StrategyRuntime(
    val id: Int,
    val config: StrategyConfig,
    val strategy: StrategyBase,
    var running: Boolean = false,
    var signals: MutableList<StrategySignal> = mutableListOf(),
    var job: Job? = null  // 每个策略自己的 Job
)
```

启动时：

```kotlin
runtime.job = viewModelScope.launch {
    var tick = 0
    while (runtime.running) {
        delay(2000)
        val mockData = generateMockMarketData(config.symbol, tick++)
        strategy.processMarketData(mockData)
    }
}
```

停止时只取消对应的 Job：

```kotlin
fun stopStrategy(runtimeId: Int) {
    val runtime = _strategyRuntimes.value.find { it.id == runtimeId }
    runtime?.let {
        it.running = false
        it.job?.cancel()
    }
    _strategyRuntimes.value = _strategyRuntimes.value.filter { it.id != runtimeId }
}
```

### 3.2 启动策略弹窗增强

**现状**：弹窗只显示策略类型和标的，用户无法确认参数。

**改良**：弹窗内展示关键参数摘要：

```
┌─────────────────────────────┐
│        启动策略确认          │
│                             │
│ 策略类型: 双均线策略          │
│ 标的: 600519.SH             │
│ 快线: 5  慢线: 20           │
│ K线周期: 5分钟               │
│ 止损: 2%  止盈: 5%          │
│ 下单: 100股                  │
│                             │
│     [取消]      [启动]       │
└─────────────────────────────┘
```

### 3.3 景气成长策略人工确认项可编辑

**现状**：`profitGrowthConfirmed` 等只读展示。

**改良**：改为 Switch 或 Checkbox：

```kotlin
Row(
    modifier = Modifier.fillMaxWidth(),
    horizontalArrangement = Arrangement.SpaceBetween
) {
    Text("利润增长确认")
    Switch(
        checked = profitGrowthConfirmed,
        onCheckedChange = { profitGrowthConfirmed = it }
    )
}
```

### 3.4 保存配置增加反馈

**现状**：点击"保存配置"无任何反馈。

**改良**：使用 Snackbar：

```kotlin
var showSaved by remember { mutableStateOf(false) }

Button(onClick = {
    viewModel.updateStrategyConfig(newConfig)
    showSaved = true
}) { Text("保存配置") }

Snackbar(
    visible = showSaved,
    message = "配置已保存"
)
```

---

## 四、首页（中优先级）

### 4.1 自选行情使用真实数据

**现状**：价格和涨跌幅是 `indexOf` 算的假数据。

**改良**：维护一个 `favoriteQuotes: MutableMap<String, MarketData>` 缓存，进入首页时批量拉取自选股行情。

简化方案（先用本地缓存）：

```kotlin
// 首页初始化时触发查询
LaunchedEffect(Unit) {
    favorites.forEach { (symbol, _) ->
        viewModel.fetchQuoteForSymbol(symbol)  // 逐个查询并存入缓存
    }
}
```

如果网络不可用，至少用"暂无数据"代替假数据，避免误导用户。

### 4.2 今日收益动态计算

**现状**：硬编码 `+0.00`。

**改良**：

```kotlin
val todayProfit = accountState.positions.values.sumOf {
    (it.currentPrice - it.avgCost) * it.quantity
}

StatCard(
    title = "今日收益",
    value = String.format("%+.2f", todayProfit),
    percent = String.format("%+.2f%%", todayProfitPercent),
    color = if (todayProfit >= 0) Color(0xFFE53935) else Color(0xFF43A047)
)
```

注意：这需要区分"今日收益"和"总盈亏"。当前数据模型没有记录今开价，如果要精确计算今日收益，需要在 `PositionInfo` 中增加 `todayOpenPrice` 字段。

---

## 五、行情页（低优先级）

### 5.1 收藏星标同步自选列表

（已在 2.2 节描述）

### 5.2 MA 筛选 Chip 位置调整

**现状**：MA5/MA10/MA20/成交量 Chip 在图表下方，控制的是图表叠加显示。

**建议**：移到图表上方作为图表工具栏，更符合看盘软件的使用习惯。同时 MA 筛选只影响视觉不改变数据筛选逻辑，当前位置也 OK，不是 bug。

---

## 六、资产页（低优先级）

### 6.1 重置账户增加二次确认

```kotlin
var showResetConfirm by remember { mutableStateOf(false) }

Button(onClick = { showResetConfirm = true }) {
    Text("重置账户")
}

if (showResetConfirm) {
    AlertDialog(
        onDismissRequest = { showResetConfirm = false },
        title = { Text("确认重置") },
        text = { Text("重置后所有持仓和交易记录将被清除，此操作不可撤销。") },
        confirmButton = {
            TextButton(onClick = {
                viewModel.resetAccount()
                showResetConfirm = false
            }) { Text("确认重置", color = Color.Red) }
        },
        dismissButton = {
            TextButton(onClick = { showResetConfirm = false }) { Text("取消") }
        }
    )
}
```

### 6.2 交易记录分页或"查看更多"

```kotlin
var showAllRecords by remember { mutableStateOf(false) }
val displayRecords = if (showAllRecords) records else records.take(10)

// 列表底部
if (records.size > 10 && !showAllRecords) {
    TextButton(
        onClick = { showAllRecords = true },
        modifier = Modifier.fillMaxWidth()
    ) {
        Text("查看全部 ${records.size} 条记录")
    }
}
```

---

## 七、改良优先级总结

| 优先级 | 改良项 | 影响范围 |
|--------|--------|----------|
| P0 | 选股结果点击操作（查看/自选/策略） | 核心链路断裂 |
| P0 | 预设策略选中状态显示 | 用户不知道选了什么 |
| P0 | 筛选条件 BottomSheet 回显 | 用户不知道当前条件 |
| P1 | 首页自选点击跳转行情 | 页面联动 |
| P1 | 自选列表增删 + 行情星标同步 | 数据一致性 |
| P1 | 多策略并行 Job 支持 | 功能正确性 |
| P1 | 重置账户二次确认 | 防误操作 |
| P2 | 筛选条件摘要展示 | 信息透明 |
| P2 | 启动策略弹窗增强 | 用户体验 |
| P2 | 景气成长人工确认项可编辑 | 功能完整性 |
| P2 | 今日收益动态计算 | 数据准确性 |
| P2 | 自选行情真实数据 | 数据准确性 |
| P3 | 保存配置 Snackbar 反馈 | 交互反馈 |
| P3 | 交易记录"查看更多" | 体验优化 |
| P3 | MA Chip 位置微调 | 体验优化 |
