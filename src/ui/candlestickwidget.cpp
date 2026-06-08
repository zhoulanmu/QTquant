#include "candlestickwidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <limits>

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

    if (m_priceHistory.empty()) {
        QPainter painter(this);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "等待行情数据...");
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawGrid(painter);
    drawPriceLabels(painter);
    drawCandlesticks(painter);
}

void CandlestickWidget::drawGrid(QPainter &painter)
{
    int padding = 60;
    int chartWidth = width() - padding;
    int chartHeight = height() - 20;

    painter.setPen(QColor(40, 40, 40));

    for (int i = 0; i <= 5; ++i) {
        int y = padding + (chartHeight / 5) * i;
        painter.drawLine(padding, y, width(), y);
    }

    int step = qMax(1, (int)m_priceHistory.size() / 10);
    for (size_t i = 0; i < m_priceHistory.size(); i += step) {
        int x = padding + (chartWidth / qMax(1, (int)m_priceHistory.size() - 1)) * i;
        painter.drawLine(x, padding, x, height() - 20);
    }
}

void CandlestickWidget::drawPriceLabels(QPainter &painter)
{
    int padding = 60;
    int chartHeight = height() - 20;
    int chartWidth = width() - padding;

    painter.setPen(QColor(255, 215, 0));
    painter.setFont(QFont("Arial", 9));

    for (int i = 0; i <= 5; ++i) {
        double price = m_maxPrice - ((m_maxPrice - m_minPrice) / 5) * i;
        int y = padding + (chartHeight / 5) * i;
        painter.drawText(5, y + 4, QString("%1").arg(price, 0, 'f', 2));
    }

    drawTimeLabels(painter, chartWidth, chartHeight, padding);
}

void CandlestickWidget::drawTimeLabels(QPainter &painter, int chartWidth, int chartHeight, int padding)
{
    if (m_priceHistory.empty()) return;

    painter.setPen(QColor(255, 215, 0));
    painter.setFont(QFont("Arial", 8));

    int step = qMax(1, (int)m_priceHistory.size() / 8);
    for (size_t i = 0; i < m_priceHistory.size(); i += step) {
        const MarketData& data = m_priceHistory[i];
        int x = padding + (chartWidth / qMax(1, (int)m_priceHistory.size() - 1)) * i;
        
        QString timeStr = data.timestamp.toString("HH:mm:ss");
        painter.drawText(x - 15, height() - 5, timeStr);
    }
}

void CandlestickWidget::drawCandlesticks(QPainter &painter)
{
    int leftPadding = 60;
    int bottomPadding = 25;
    int chartWidth = width() - leftPadding - 10;
    int chartHeight = height() - bottomPadding;

    if (chartWidth < 0 || chartHeight < 0) return;
    if (m_priceHistory.size() < 2) return;

    double scaleY = chartHeight / (m_maxPrice - m_minPrice);
    int dataCount = static_cast<int>(m_priceHistory.size());
    int visibleCount = static_cast<int>(dataCount / m_zoomFactor);
    visibleCount = qMax(5, qMin(visibleCount, dataCount));
    
    int startIdx = dataCount - visibleCount;
    if (startIdx < 0) startIdx = 0;

    int step = visibleCount - 1;
    if (step < 1) step = 1;
    
    int candleWidth = qMax(3, static_cast<int>((chartWidth / step) * m_zoomFactor) - 2);
    if (candleWidth < 1) candleWidth = 1;

    for (int i = startIdx; i < dataCount; ++i) {
        const MarketData& data = m_priceHistory[i];
        int relativeIdx = i - startIdx;
        int x = leftPadding + (chartWidth / step) * relativeIdx;
        if (x > width() - 5) continue;

        int openY = bottomPadding + chartHeight - static_cast<int>((data.open - m_minPrice) * scaleY);
        int closeY = bottomPadding + chartHeight - static_cast<int>((data.close - m_minPrice) * scaleY);
        int highY = bottomPadding + chartHeight - static_cast<int>((data.high - m_minPrice) * scaleY);
        int lowY = bottomPadding + chartHeight - static_cast<int>((data.low - m_minPrice) * scaleY);

        openY = qMax(1, qMin(openY, height() - bottomPadding - 1));
        closeY = qMax(1, qMin(closeY, height() - bottomPadding - 1));
        highY = qMax(1, qMin(highY, height() - bottomPadding - 1));
        lowY = qMax(1, qMin(lowY, height() - bottomPadding - 1));

        QColor color = (data.close >= data.open) ? Qt::red : Qt::green;
        painter.setPen(color);
        painter.setBrush(color);

        painter.drawLine(x, highY, x, lowY);

        int bodyTop = qMin(openY, closeY);
        int bodyBottom = qMax(openY, closeY);
        int bodyHeight = bodyBottom - bodyTop;
        if (bodyHeight < 1) bodyHeight = 1;

        int rectX = x - candleWidth / 2;
        if (rectX < leftPadding) rectX = leftPadding;
        painter.drawRect(rectX, bodyTop, candleWidth, bodyHeight);
    }
}
