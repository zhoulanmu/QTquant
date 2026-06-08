#pragma once

#include <QWidget>
#include "../market/marketdata.h"

namespace Ui {
class MarketPanel;
}

class MarketPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MarketPanel(QWidget *parent = nullptr);
    ~MarketPanel();

    void updateMarketData(const MarketData& data);

private:
    Ui::MarketPanel *ui;
};
