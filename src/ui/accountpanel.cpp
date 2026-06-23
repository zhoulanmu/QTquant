#include "accountpanel.h"
#include "ui_accountpanel.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QTableWidgetItem>

AccountPanel::AccountPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountPanel)
{
    ui->setupUi(this);

    ui->positionsTable->setColumnCount(6);
    ui->positionsTable->setHorizontalHeaderLabels({QStringLiteral("代码"), QStringLiteral("持仓"), QStringLiteral("成本"), QStringLiteral("现价"), QStringLiteral("市值"), QStringLiteral("盈亏(%)")});
    ui->positionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->positionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->positionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tradesTable->setColumnCount(7);
    ui->tradesTable->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("代码"), QStringLiteral("方向"), QStringLiteral("成交价"), QStringLiteral("数量"), QStringLiteral("成交额"), QStringLiteral("费用")});
    ui->tradesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tradesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tradesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    initMockData();
}

void AccountPanel::initMockData()
{
    ui->positionsTable->setRowCount(0);
    ui->tradesTable->setRowCount(0);
}

AccountPanel::~AccountPanel()
{
    delete ui;
}

void AccountPanel::insertFundsControlWidget(QWidget* widget)
{
    if (!widget) {
        return;
    }

    widget->setParent(this);
    ui->verticalLayout_3->insertWidget(1, widget, 0);
}

void AccountPanel::updateAccount(double totalAssets, double availableCash, double marketValue, double totalProfit, double profitPercent)
{
    ui->totalAssetsLabel->setText(QString::number(totalAssets, 'f', 2));
    ui->availableCashLabel->setText(QString::number(availableCash, 'f', 2));
    ui->marketValueLabel->setText(QString::number(marketValue, 'f', 2));
    ui->totalProfitLabel->setText(QString::number(totalProfit, 'f', 2));
    ui->profitPercentLabel->setText(QString::number(profitPercent, 'f', 2) + QStringLiteral("%"));

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
        const int row = ui->positionsTable->rowCount();
        ui->positionsTable->insertRow(row);

        ui->positionsTable->setItem(row, 0, new QTableWidgetItem(pos.symbol));
        ui->positionsTable->setItem(row, 1, new QTableWidgetItem(QString::number(pos.quantity, 'f', 0)));
        ui->positionsTable->setItem(row, 2, new QTableWidgetItem(QString::number(pos.avgCost, 'f', 2)));
        ui->positionsTable->setItem(row, 3, new QTableWidgetItem(QString::number(pos.currentPrice, 'f', 2)));
        ui->positionsTable->setItem(row, 4, new QTableWidgetItem(QString::number(pos.marketValue, 'f', 2)));

        auto profitItem = new QTableWidgetItem(QString::number(pos.profitPercent, 'f', 2));
        profitItem->setForeground(QBrush(pos.profitPercent >= 0 ? Qt::red : Qt::green));
        ui->positionsTable->setItem(row, 5, profitItem);
    }

    ui->positionsTable->resizeColumnsToContents();
}

void AccountPanel::addTradeRecord(const QString &symbol, const QString &type, double price, double volume, double amount, double fee, const QString &time)
{
    const int row = ui->tradesTable->rowCount();
    ui->tradesTable->insertRow(row);

    ui->tradesTable->setItem(row, 0, new QTableWidgetItem(time));
    ui->tradesTable->setItem(row, 1, new QTableWidgetItem(symbol));

    auto typeItem = new QTableWidgetItem(type);
    typeItem->setForeground(QBrush(type == QStringLiteral("买入") ? Qt::red : Qt::green));
    ui->tradesTable->setItem(row, 2, typeItem);

    ui->tradesTable->setItem(row, 3, new QTableWidgetItem(QString::number(price, 'f', 2)));
    ui->tradesTable->setItem(row, 4, new QTableWidgetItem(QString::number(volume, 'f', 0)));
    ui->tradesTable->setItem(row, 5, new QTableWidgetItem(QString::number(amount, 'f', 2)));
    ui->tradesTable->setItem(row, 6, new QTableWidgetItem(QString::number(fee, 'f', 2)));

    ui->tradesTable->resizeColumnsToContents();

    if (ui->tradesTable->rowCount() > 50) {
        ui->tradesTable->removeRow(0);
    }
}
