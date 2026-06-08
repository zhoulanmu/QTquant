#include "accountpanel.h"
#include "ui_accountpanel.h"
#include <QTableWidgetItem>
#include <QBrush>
#include <QColor>

AccountPanel::AccountPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountPanel)
{
    ui->setupUi(this);

    ui->positionsTable->setColumnCount(6);
    ui->positionsTable->setHorizontalHeaderLabels({"股票代码", "持仓数量", "成本价", "现价", "市值", "盈亏(%)"});
    ui->positionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->positionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->positionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tradesTable->setColumnCount(6);
    ui->tradesTable->setHorizontalHeaderLabels({"时间", "股票代码", "方向", "成交价格", "成交数量", "成交金额"});
    ui->tradesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tradesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tradesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    initMockData();
}

void AccountPanel::initMockData()
{
    QMap<QString, PositionInfo> mockPositions;

    PositionInfo pos1;
    pos1.symbol = "000001.SH";
    pos1.quantity = 500;
    pos1.avgCost = 10.50;
    pos1.currentPrice = 10.78;
    pos1.marketValue = pos1.quantity * pos1.currentPrice;
    pos1.profit = (pos1.currentPrice - pos1.avgCost) * pos1.quantity;
    pos1.profitPercent = (pos1.currentPrice / pos1.avgCost - 1) * 100;
    mockPositions["000001.SH"] = pos1;

    PositionInfo pos2;
    pos2.symbol = "600519.SH";
    pos2.quantity = 100;
    pos2.avgCost = 1850.00;
    pos2.currentPrice = 1920.50;
    pos2.marketValue = pos2.quantity * pos2.currentPrice;
    pos2.profit = (pos2.currentPrice - pos2.avgCost) * pos2.quantity;
    pos2.profitPercent = (pos2.currentPrice / pos2.avgCost - 1) * 100;
    mockPositions["600519.SH"] = pos2;

    updatePositions(mockPositions);

    addTradeRecord("000001.SH", "买入", 10.50, 500, 5250.00, "10:30:15");
    addTradeRecord("600519.SH", "买入", 1850.00, 100, 185000.00, "11:15:22");
    addTradeRecord("000001.SH", "卖出", 10.65, 200, 2130.00, "14:20:33");
    addTradeRecord("000001.SH", "买入", 10.48, 300, 3144.00, "14:45:18");
}

AccountPanel::~AccountPanel()
{
    delete ui;
}

void AccountPanel::updateAccount(double totalAssets, double availableCash, double marketValue, double totalProfit, double profitPercent)
{
    ui->totalAssetsLabel->setText(QString::number(totalAssets, 'f', 2));
    ui->availableCashLabel->setText(QString::number(availableCash, 'f', 2));
    ui->marketValueLabel->setText(QString::number(marketValue, 'f', 2));
    ui->totalProfitLabel->setText(QString::number(totalProfit, 'f', 2));
    ui->profitPercentLabel->setText(QString::number(profitPercent, 'f', 2) + "%");

    if (totalProfit >= 0) {
        ui->totalProfitLabel->setStyleSheet("color: red;");
        ui->profitPercentLabel->setStyleSheet("color: red;");
    } else {
        ui->totalProfitLabel->setStyleSheet("color: green;");
        ui->profitPercentLabel->setStyleSheet("color: green;");
    }
}

void AccountPanel::updatePositions(const QMap<QString, PositionInfo> &positions)
{
    ui->positionsTable->setRowCount(0);

    for (auto it = positions.begin(); it != positions.end(); ++it) {
        const PositionInfo& pos = it.value();
        int row = ui->positionsTable->rowCount();
        ui->positionsTable->insertRow(row);

        ui->positionsTable->setItem(row, 0, new QTableWidgetItem(pos.symbol));
        ui->positionsTable->setItem(row, 1, new QTableWidgetItem(QString::number(pos.quantity, 'f', 0)));
        ui->positionsTable->setItem(row, 2, new QTableWidgetItem(QString::number(pos.avgCost, 'f', 2)));
        ui->positionsTable->setItem(row, 3, new QTableWidgetItem(QString::number(pos.currentPrice, 'f', 2)));
        ui->positionsTable->setItem(row, 4, new QTableWidgetItem(QString::number(pos.marketValue, 'f', 2)));

        auto profitItem = new QTableWidgetItem(QString::number(pos.profitPercent, 'f', 2));
        if (pos.profitPercent >= 0) {
            profitItem->setForeground(QBrush(Qt::red));
        } else {
            profitItem->setForeground(QBrush(Qt::green));
        }
        ui->positionsTable->setItem(row, 5, profitItem);
    }

    ui->positionsTable->resizeColumnsToContents();
}

void AccountPanel::addTradeRecord(const QString &symbol, const QString &type, double price, double volume, double amount, const QString &time)
{
    int row = ui->tradesTable->rowCount();
    ui->tradesTable->insertRow(row);

    ui->tradesTable->setItem(row, 0, new QTableWidgetItem(time));
    ui->tradesTable->setItem(row, 1, new QTableWidgetItem(symbol));

    auto typeItem = new QTableWidgetItem(type);
    if (type == "买入") {
        typeItem->setForeground(QBrush(Qt::red));
    } else {
        typeItem->setForeground(QBrush(Qt::green));
    }
    ui->tradesTable->setItem(row, 2, typeItem);

    ui->tradesTable->setItem(row, 3, new QTableWidgetItem(QString("%.2f").arg(price)));
    ui->tradesTable->setItem(row, 4, new QTableWidgetItem(QString("%.0f").arg(volume)));
    ui->tradesTable->setItem(row, 5, new QTableWidgetItem(QString("%.2f").arg(amount)));

    ui->tradesTable->resizeColumnsToContents();

    if (ui->tradesTable->rowCount() > 50) {
        ui->tradesTable->removeRow(0);
    }
}
