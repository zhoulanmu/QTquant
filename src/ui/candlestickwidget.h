#pragma once

#include <QWidget>
#include <deque>
#include "../market/marketdata.h"

class CandlestickWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CandlestickWidget(QWidget *parent = nullptr);
    ~CandlestickWidget() override = default;

    void updateData(const std::deque<MarketData>& history, double minPrice, double maxPrice);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    std::deque<MarketData> m_priceHistory;
    double m_minPrice;
    double m_maxPrice;
    double m_zoomFactor;
    const int MAX_POINTS = 50;

    void drawCandlesticks(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawPriceLabels(QPainter& painter);
    void drawTimeLabels(QPainter& painter, int chartWidth, int chartHeight, int padding);
};
