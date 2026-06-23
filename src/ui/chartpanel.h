#pragma once

#include <QWidget>
#include <QPainter>
#include <QString>
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
    void updateIntradayData(const QVector<MarketData>& data);
    void clearData();
    void setEmptyMessage(const QString& message);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Ui::ChartPanel *ui;
    std::deque<MarketData> m_priceHistory;
    double m_minPrice;
    double m_maxPrice;
    const int MAX_POINTS = 300;
    CandlestickWidget* m_candlestickWidget;

    void updatePriceRange();
};
