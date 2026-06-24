#include "candlestickwidget.h"
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTime>
#include <QWheelEvent>
#include <cmath>

namespace {
constexpr int TotalTradingMinutes = 240;

int tradingMinuteOfDay(const QTime& time)
{
    if (!time.isValid()) {
        return -1;
    }

    const QTime morningStart(9, 30);
    const QTime lunchStart(11, 30);
    const QTime afternoonStart(13, 0);
    const QTime marketClose(15, 0);

    if (time < morningStart) {
        return 0;
    }
    if (time <= lunchStart) {
        return qBound(0, morningStart.secsTo(time) / 60, 120);
    }
    if (time < afternoonStart) {
        return 120;
    }
    if (time <= marketClose) {
        return qBound(120, 120 + afternoonStart.secsTo(time) / 60, TotalTradingMinutes);
    }
    return TotalTradingMinutes;
}

double previousCloseFor(const std::deque<MarketData>& history)
{
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (it->previousClose > 0.0) {
            return it->previousClose;
        }
    }
    return 0.0;
}

QColor labelColorForPrice(double price, double previousClose)
{
    if (previousClose <= 0.0) {
        return QColor(255, 215, 0);
    }
    if (price > previousClose) {
        return QColor(255, 77, 79);
    }
    if (price < previousClose) {
        return QColor(72, 187, 120);
    }
    return QColor(210, 218, 230);
}
}

CandlestickWidget::CandlestickWidget(QWidget *parent) :
    QWidget(parent),
    m_autoMinPrice(0.0),
    m_autoMaxPrice(1.0),
    m_minPrice(0.0),
    m_maxPrice(1.0),
    m_priceZoomFactor(1.0),
    m_timeStartMinute(0),
    m_timeEndMinute(TotalTradingMinutes),
    m_timeZoomFactor(1.0),
    m_emptyMessage(QStringLiteral("正在加载行情数据..."))
{
    setStyleSheet("background-color: #1a1a2e;");
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setMouseTracking(true);
}

void CandlestickWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_priceHistory.empty()) {
        const QRectF area = chartArea();
        const QPointF pos = event->position();
        const bool timeZoom = event->modifiers().testFlag(Qt::ControlModifier)
            || event->modifiers().testFlag(Qt::ShiftModifier);
        const double factor = event->angleDelta().y() > 0 ? 1.18 : (1.0 / 1.18);
        if (timeZoom) {
            m_timeZoomFactor = qBound(1.0, m_timeZoomFactor * factor, 8.0);
            applyTimeZoom(xToMinute(pos.x(), area));
        } else {
            m_priceZoomFactor = qBound(1.0, m_priceZoomFactor * factor, 24.0);
            applyPriceZoom(priceAtY(pos.y(), area));
        }
        update();
    }
    event->accept();
}

void CandlestickWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    resetZoom();
    update();
    event->accept();
}

void CandlestickWidget::updateData(const std::deque<MarketData> &history, double minPrice, double maxPrice)
{
    m_priceHistory = history;
    m_autoMinPrice = minPrice;
    m_autoMaxPrice = maxPrice;
    applyPriceZoom((m_minPrice + m_maxPrice) / 2.0);
    applyTimeZoom((m_timeStartMinute + m_timeEndMinute) / 2);
    update();
}

void CandlestickWidget::setEmptyMessage(const QString& message)
{
    m_emptyMessage = message.isEmpty() ? QStringLiteral("正在加载行情数据...") : message;
    if (m_priceHistory.empty()) {
        update();
    }
}

void CandlestickWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), QColor(26, 26, 46));
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_priceHistory.empty()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, m_emptyMessage);
        return;
    }

    drawGrid(painter);
    drawPriceLabels(painter);
    drawCandlesticks(painter);
}

QRectF CandlestickWidget::chartArea() const
{
    constexpr qreal left = 72.0;
    constexpr qreal top = 18.0;
    constexpr qreal right = 24.0;
    constexpr qreal bottom = 34.0;

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

double CandlestickWidget::priceAtY(qreal y, const QRectF& area) const
{
    if (m_maxPrice <= m_minPrice || area.height() <= 0.0) {
        return (m_minPrice + m_maxPrice) / 2.0;
    }

    const qreal ratio = qBound(0.0, (area.bottom() - y) / area.height(), 1.0);
    return m_minPrice + ratio * (m_maxPrice - m_minPrice);
}

double CandlestickWidget::visibleTimeSpan() const
{
    return qMax(1.0, static_cast<double>(m_timeEndMinute - m_timeStartMinute));
}

int CandlestickWidget::xToMinute(qreal x, const QRectF& area) const
{
    if (area.width() <= 0.0) {
        return (m_timeStartMinute + m_timeEndMinute) / 2;
    }

    const double ratio = qBound(0.0, (x - area.left()) / area.width(), 1.0);
    return qBound(0, qRound(m_timeStartMinute + ratio * visibleTimeSpan()), TotalTradingMinutes);
}

qreal CandlestickWidget::minuteToVisibleX(int minute, const QRectF& area) const
{
    const double ratio = qBound(0.0,
                                (static_cast<double>(minute) - m_timeStartMinute) / visibleTimeSpan(),
                                1.0);
    return area.left() + area.width() * ratio;
}

void CandlestickWidget::applyPriceZoom(double centerPrice)
{
    if (m_autoMaxPrice <= m_autoMinPrice) {
        m_minPrice = m_autoMinPrice;
        m_maxPrice = m_autoMaxPrice;
        return;
    }

    const double autoRange = m_autoMaxPrice - m_autoMinPrice;
    const double viewRange = autoRange / qMax(1.0, m_priceZoomFactor);
    const double minCenter = m_autoMinPrice + viewRange / 2.0;
    const double maxCenter = m_autoMaxPrice - viewRange / 2.0;
    const double center = minCenter <= maxCenter
        ? qBound(minCenter, centerPrice, maxCenter)
        : (m_autoMinPrice + m_autoMaxPrice) / 2.0;

    m_minPrice = qMax(0.0, center - viewRange / 2.0);
    m_maxPrice = center + viewRange / 2.0;
}

void CandlestickWidget::applyTimeZoom(int centerMinute)
{
    const double viewSpan = TotalTradingMinutes / qMax(1.0, m_timeZoomFactor);
    double start = centerMinute - viewSpan / 2.0;
    double end = centerMinute + viewSpan / 2.0;
    if (start < 0.0) {
        end -= start;
        start = 0.0;
    }
    if (end > TotalTradingMinutes) {
        start -= end - TotalTradingMinutes;
        end = TotalTradingMinutes;
    }

    m_timeStartMinute = qBound(0, qRound(start), TotalTradingMinutes - 1);
    m_timeEndMinute = qBound(m_timeStartMinute + 1, qRound(end), TotalTradingMinutes);
}

void CandlestickWidget::resetZoom()
{
    m_priceZoomFactor = 1.0;
    m_timeZoomFactor = 1.0;
    m_timeStartMinute = 0;
    m_timeEndMinute = TotalTradingMinutes;
    m_minPrice = m_autoMinPrice;
    m_maxPrice = m_autoMaxPrice;
}

void CandlestickWidget::drawGrid(QPainter &painter)
{
    const QRectF area = chartArea();
    painter.setPen(QPen(QColor(46, 52, 78), 1));

    for (int i = 0; i <= 5; ++i) {
        const qreal y = area.top() + area.height() / 5.0 * i;
        painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
    }

    const QList<int> minutes = {0, 60, 120, 180, TotalTradingMinutes};
    for (int minute : minutes) {
        if (minute < m_timeStartMinute || minute > m_timeEndMinute) {
            continue;
        }
        const qreal x = minuteToVisibleX(minute, area);
        painter.drawLine(QPointF(x, area.top()), QPointF(x, area.bottom()));
    }
}

void CandlestickWidget::drawPriceLabels(QPainter &painter)
{
    const QRectF area = chartArea();
    const double previousClose = previousCloseFor(m_priceHistory);

    painter.setFont(QFont("Arial", 9));
    for (int i = 0; i <= 5; ++i) {
        const double price = m_maxPrice - ((m_maxPrice - m_minPrice) / 5.0) * i;
        const qreal y = area.top() + area.height() / 5.0 * i;
        painter.setPen(labelColorForPrice(price, previousClose));
        painter.drawText(QRectF(2, y - 8, area.left() - 8, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString("%1").arg(price, 0, 'f', 2));
    }

    if (previousClose > 0.0 && previousClose >= m_minPrice && previousClose <= m_maxPrice) {
        const int y = priceToY(previousClose, area);
        painter.setPen(QPen(QColor(160, 170, 190, 120), 1, Qt::DashLine));
        painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
    }

    drawTimeLabels(painter, area, 0, 0);
}

void CandlestickWidget::drawTimeLabels(QPainter &painter, const QRectF &area, int startIdx, int visibleCount)
{
    Q_UNUSED(startIdx);
    Q_UNUSED(visibleCount);

    painter.setPen(QColor(210, 218, 230));
    painter.setFont(QFont("Arial", 8));

    const QList<QPair<int, QString>> labels = {
        {0, QStringLiteral("9:30")},
        {60, QStringLiteral("10:30")},
        {120, QStringLiteral("11:30/13:00")},
        {180, QStringLiteral("14:00")},
        {TotalTradingMinutes, QStringLiteral("15:00")}
    };

    for (const auto& label : labels) {
        if (label.first < m_timeStartMinute || label.first > m_timeEndMinute) {
            continue;
        }
        const qreal x = minuteToVisibleX(label.first, area);
        painter.drawText(QRectF(x - 42, area.bottom() + 5, 84, 18), Qt::AlignCenter, label.second);
    }
}

void CandlestickWidget::drawCandlesticks(QPainter &painter)
{
    const QRectF area = chartArea();
    if (m_priceHistory.empty() || m_maxPrice <= m_minPrice) {
        return;
    }

    QPolygonF priceLine;
    QPolygonF averageLine;

    for (const MarketData& data : m_priceHistory) {
        if (data.close <= 0.0 || !data.timestamp.isValid()) {
            continue;
        }

        const int minute = tradingMinuteOfDay(data.timestamp.time());
        if (minute < 0 || minute < m_timeStartMinute || minute > m_timeEndMinute) {
            continue;
        }

        const qreal x = minuteToVisibleX(minute, area);
        priceLine << QPointF(x, priceToY(data.close, area));

        if (data.averagePrice > 0.0) {
            averageLine << QPointF(x, priceToY(data.averagePrice, area));
        }
    }

    if (priceLine.isEmpty()) {
        return;
    }

    QPainterPath fillPath;
    fillPath.moveTo(priceLine.first().x(), area.bottom());
    fillPath.lineTo(priceLine.first());
    for (int i = 1; i < priceLine.size(); ++i) {
        fillPath.lineTo(priceLine.at(i));
    }
    fillPath.lineTo(priceLine.last().x(), area.bottom());
    fillPath.closeSubpath();

    QLinearGradient fillGradient(area.topLeft(), area.bottomLeft());
    fillGradient.setColorAt(0.0, QColor(66, 153, 225, 64));
    fillGradient.setColorAt(1.0, QColor(66, 153, 225, 12));
    painter.fillPath(fillPath, fillGradient);

    if (averageLine.size() > 1) {
        painter.setPen(QPen(QColor(246, 173, 85), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPolyline(averageLine);
    }

    const bool hasVisibleTimeSpan = priceLine.size() > 1
        && std::abs(priceLine.first().x() - priceLine.last().x()) > 0.5;

    painter.setPen(QPen(QColor(0, 229, 255), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (priceLine.size() == 1) {
        painter.drawPoint(priceLine.first());
    } else {
        painter.drawPolyline(priceLine);
    }

    const QPointF lastPoint = priceLine.last();
    if (hasVisibleTimeSpan) {
        painter.setPen(QPen(QColor(99, 179, 237, 120), 1, Qt::DashLine));
        painter.drawLine(QPointF(area.left(), lastPoint.y()), QPointF(area.right(), lastPoint.y()));
    }

    painter.setPen(QPen(QColor(255, 255, 255, 200), 1));
    painter.setBrush(QColor(0, 229, 255));
    painter.drawEllipse(lastPoint, 4.5, 4.5);
}
