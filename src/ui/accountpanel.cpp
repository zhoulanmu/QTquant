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
}

AccountPanel::~AccountPanel()
{
    delete ui;
}

void AccountPanel::updateAccount(double totalAssets, double availableCash, double marketValue, double totalProfit, double profitPercent)
{
    ui->totalAssetsLabel->setText(QString("%.2f").arg(totalAssets));
    ui->availableCashLabel->setText(QString("%.2f").arg(availableCash));
    ui->marketValueLabel->setText(QString("%.2f").arg(marketValue));
    ui->totalProfitLabel->setText(QString("%.2f").arg(totalProfit));
    ui->profitPercentLabel->setText(QString("%.2f%%").arg(profitPercent));

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
        ui->positionsTable->setItem(row, 1, new QTableWidgetItem(QString("%.0f").arg(pos.quantity)));
        ui->positionsTable->setItem(row, 2, new QTableWidgetItem(QString("%.2f").arg(pos.avgCost)));
        ui->positionsTable->setItem(row, 3, new QTableWidgetItem(QString("%.2f").arg(pos.currentPrice)));
        ui->positionsTable->setItem(row, 4, new QTableWidgetItem(QString("%.2f").arg(pos.marketValue)));

        auto profitItem = new QTableWidgetItem(QString("%.2f").arg(pos.profitPercent));
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
