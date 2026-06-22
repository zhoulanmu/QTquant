#pragma once

#include <QWidget>
#include <QPainter>
#include <deque>
#include "../market/marketdata.h"
#include "candlestickwidget.h"

namespace Ui {
class ChartPanel;
}

class ChartPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ChartPanel(QWidget *parent = nullptr);
    ~ChartPanel();

    void updateChartData(const MarketData& data);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Ui::ChartPanel *ui;
    std::deque<MarketData> m_priceHistory;
    double m_minPrice;
    double m_maxPrice;
    const int MAX_POINTS = 50;
    CandlestickWidget* m_candlestickWidget;

    void updatePriceRange();
};
