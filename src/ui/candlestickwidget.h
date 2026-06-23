#pragma once

#include <QWidget>
#include <QRectF>
#include <QString>
#include <deque>
#include "../market/marketdata.h"

class CandlestickWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CandlestickWidget(QWidget *parent = nullptr);
    ~CandlestickWidget() override = default;

    void updateData(const std::deque<MarketData>& history, double minPrice, double maxPrice);
    void setEmptyMessage(const QString& message);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    std::deque<MarketData> m_priceHistory;
    double m_minPrice;
    double m_maxPrice;
    double m_zoomFactor;
    QString m_emptyMessage;
    const int MAX_POINTS = 50;

    QRectF chartArea() const;
    int priceToY(double price, const QRectF& area) const;
    void drawCandlesticks(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawPriceLabels(QPainter& painter);
    void drawTimeLabels(QPainter& painter, const QRectF& area, int startIdx, int visibleCount);
};
