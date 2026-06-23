#pragma once

#include <QWidget>
#include <QRectF>
#include <QString>
#include <deque>
#include "../market/marketdata.h"

class QMouseEvent;

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
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    std::deque<MarketData> m_priceHistory;
    double m_autoMinPrice;
    double m_autoMaxPrice;
    double m_minPrice;
    double m_maxPrice;
    double m_priceZoomFactor;
    int m_timeStartMinute;
    int m_timeEndMinute;
    double m_timeZoomFactor;
    QString m_emptyMessage;
    const int MAX_POINTS = 50;

    double priceAtY(qreal y, const QRectF& area) const;
    double visibleTimeSpan() const;
    int xToMinute(qreal x, const QRectF& area) const;
    qreal minuteToVisibleX(int minute, const QRectF& area) const;
    void applyPriceZoom(double centerPrice);
    void applyTimeZoom(int centerMinute);
    void resetZoom();
    QRectF chartArea() const;
    int priceToY(double price, const QRectF& area) const;
    void drawCandlesticks(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawPriceLabels(QPainter& painter);
    void drawTimeLabels(QPainter& painter, const QRectF& area, int startIdx, int visibleCount);
};