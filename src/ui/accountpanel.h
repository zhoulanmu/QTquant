#pragma once

#include <QWidget>
#include <QMap>

namespace Ui {
class AccountPanel;
}

struct PositionInfo {
    QString symbol;
    double quantity;
    double avgCost;
    double currentPrice;
    double marketValue;
    double profit;
    double profitPercent;
};

class AccountPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AccountPanel(QWidget *parent = nullptr);
    ~AccountPanel();

    void updateAccount(double totalAssets, double availableCash, double marketValue, double totalProfit, double profitPercent);
    void updatePositions(const QMap<QString, PositionInfo>& positions);
    void addTradeRecord(const QString& symbol, const QString& type, double price, double volume, double amount, double fee, const QString& time);
    void insertFundsControlWidget(QWidget* widget);

private:
    Ui::AccountPanel *ui;

    void initMockData();
};
