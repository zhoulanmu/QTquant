#include "candlestickwidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QPolygonF>
#include <cmath>

CandlestickWidget::CandlestickWidget(QWidget *parent) :
    QWidget(parent),
    m_minPrice(0.0),
    m_maxPrice(1.0),
    m_zoomFactor(1.0)
{
    setStyleSheet("background-color: #1a1a2e;");
    setMouseTracking(true);
}

void CandlestickWidget::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        m_zoomFactor = qMin(m_zoomFactor * 1.1, 3.0);
    } else {
        m_zoomFactor = qMax(m_zoomFactor / 1.1, 0.5);
    }
    update();
    event->accept();
}

void CandlestickWidget::updateData(const std::deque<MarketData> &history, double minPrice, double maxPrice)
{
    m_priceHistory = history;
    m_minPrice = minPrice;
    m_maxPrice = maxPrice;
    update();
}

void CandlestickWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), QColor(26, 26, 46));
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_priceHistory.empty()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("等待行情数据..."));
        return;
    }

    drawGrid(painter);
    drawPriceLabels(painter);
    drawCandlesticks(painter);
}

QRectF CandlestickWidget::chartArea() const
{
    constexpr qreal left = 64.0;
    constexpr qreal top = 16.0;
    constexpr qreal right = 12.0;
    constexpr qreal bottom = 30.0;

    return QRectF(left,
                  top,
                  qMax<qreal>(1.0, width() - left - right),
                  qMax<qreal>(1.0, height() - top - bottom));
}

int CandlestickWidget::priceToY(double price, const QRectF &area) const
{
    if (m_maxPrice <= m_minPrice) {
        return qRound(area.center().y());
    }

    const qreal ratio = (price - m_minPrice) / (m_maxPrice - m_minPrice);
    const qreal y = area.bottom() - ratio * area.height();
    return qRound(qBound(area.top(), y, area.bottom()));
}

void CandlestickWidget::drawGrid(QPainter &painter)
{
    const QRectF area = chartArea();
    painter.setPen(QColor(45, 45, 55));

    for (int i = 0; i <= 5; ++i) {
        const qreal y = area.top() + area.height() / 5.0 * i;
        painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
    }

    for (int i = 0; i <= 8; ++i) {
        const qreal x = area.left() + area.width() / 8.0 * i;
        painter.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
    }
}

void CandlestickWidget::drawPriceLabels(QPainter &painter)
{
    const QRectF area = chartArea();

    painter.setPen(QColor(255, 215, 0));
    painter.setFont(QFont("Arial", 9));

    for (int i = 0; i <= 5; ++i) {
        const double price = m_maxPrice - ((m_maxPrice - m_minPrice) / 5.0) * i;
        const qreal y = area.top() + area.height() / 5.0 * i;
        painter.drawText(QRectF(2, y - 8, area.left() - 6, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString("%1").arg(price, 0, 'f', 2));
    }

    const int dataCount = static_cast<int>(m_priceHistory.size());
    const int visibleCount = qMax(1, qMin(static_cast<int>(dataCount / m_zoomFactor), dataCount));
    const int startIdx = qMax(0, dataCount - visibleCount);
    drawTimeLabels(painter, area, startIdx, visibleCount);
}

void CandlestickWidget::drawTimeLabels(QPainter &painter, const QRectF &area, int startIdx, int visibleCount)
{
    if (m_priceHistory.empty() || visibleCount <= 0) {
        return;
    }

    painter.setPen(QColor(255, 215, 0));
    painter.setFont(QFont("Arial", 8));

    const int labelCount = qMin(6, visibleCount);
    for (int i = 0; i < labelCount; ++i) {
        const int relativeIdx = labelCount == 1
            ? 0
            : qRound(static_cast<double>(i) * (visibleCount - 1) / (labelCount - 1));
        const int dataIdx = qMin(startIdx + relativeIdx, static_cast<int>(m_priceHistory.size()) - 1);
        const qreal x = visibleCount == 1
            ? area.center().x()
            : area.left() + area.width() * relativeIdx / (visibleCount - 1);

        const QString timeStr = m_priceHistory[dataIdx].timestamp.toString("HH:mm:ss");
        painter.drawText(QRectF(x - 34, area.bottom() + 4, 68, 18), Qt::AlignCenter, timeStr);
    }
}

void CandlestickWidget::drawCandlesticks(QPainter &painter)
{
    const QRectF area = chartArea();
    const int dataCount = static_cast<int>(m_priceHistory.size());
    if (dataCount <= 0 || m_maxPrice <= m_minPrice) {
        return;
    }

    const int visibleCount = qMax(1, qMin(static_cast<int>(dataCount / m_zoomFactor), dataCount));
    const int startIdx = qMax(0, dataCount - visibleCount);

    if (visibleCount == 1) {
        const MarketData& data = m_priceHistory.back();
        const int y = priceToY(data.close, area);
        painter.setPen(QPen(QColor(0, 229, 255), 2));
        painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
        painter.setBrush(QColor(0, 229, 255));
        painter.drawEllipse(QPointF(area.center().x(), y), 4.0, 4.0);
        return;
    }

    const qreal stepX = area.width() / (visibleCount - 1);
    QPolygonF closeLine;

    for (int i = startIdx; i < dataCount; ++i) {
        const MarketData& data = m_priceHistory[i];
        if (data.close <= 0.0) {
            continue;
        }

        const int relativeIdx = i - startIdx;
        const qreal x = area.left() + stepX * relativeIdx;
        closeLine << QPointF(x, priceToY(data.close, area));
    }

    if (closeLine.isEmpty()) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 229, 255), 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPolyline(closeLine);

    const QPointF lastPoint = closeLine.last();
    painter.setPen(QPen(QColor(255, 255, 255, 180), 1));
    painter.setBrush(QColor(0, 229, 255));
    painter.drawEllipse(lastPoint, 4.5, 4.5);

    painter.setPen(QPen(QColor(99, 179, 237, 120), 1, Qt::DashLine));
    painter.drawLine(QPointF(area.left(), lastPoint.y()), QPointF(area.right(), lastPoint.y()));
}
