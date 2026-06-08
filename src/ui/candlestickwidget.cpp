#include "candlestickwidget.h"
#include <QPainter>
#include <limits>

CandlestickWidget::CandlestickWidget(QWidget *parent) :
    QWidget(parent),
    m_minPrice(0.0),
    m_maxPrice(1.0)
{
    setStyleSheet("background-color: black;");
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

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9));

    for (int i = 0; i <= 5; ++i) {
        double price = m_maxPrice - ((m_maxPrice - m_minPrice) / 5) * i;
        int y = padding + (chartHeight / 5) * i;
        painter.drawText(5, y + 4, QString("%1").arg(price, 0, 'f', 2));
    }
}

void CandlestickWidget::drawCandlesticks(QPainter &painter)
{
    int padding = 60;
    int chartWidth = width() - padding;
    int chartHeight = height() - 20;

    if (m_priceHistory.size() < 2) return;

    double scaleY = chartHeight / (m_maxPrice - m_minPrice);
    int step = qMax(1, (int)m_priceHistory.size() - 1);
    int candleWidth = qMax(2, static_cast<int>(chartWidth / step) - 2);

    for (size_t i = 0; i < m_priceHistory.size(); ++i) {
        const MarketData& data = m_priceHistory[i];
        int x = padding + (chartWidth / step) * i;

        int openY = padding + chartHeight - static_cast<int>((data.open - m_minPrice) * scaleY);
        int closeY = padding + chartHeight - static_cast<int>((data.close - m_minPrice) * scaleY);
        int highY = padding + chartHeight - static_cast<int>((data.high - m_minPrice) * scaleY);
        int lowY = padding + chartHeight - static_cast<int>((data.low - m_minPrice) * scaleY);

        QColor color = (data.close >= data.open) ? Qt::red : Qt::green;
        painter.setPen(color);
        painter.setBrush(color);

        painter.drawLine(x, highY, x, lowY);

        int bodyTop = qMin(openY, closeY);
        int bodyBottom = qMax(openY, closeY);
        int bodyHeight = bodyBottom - bodyTop;
        if (bodyHeight < 1) bodyHeight = 1;

        painter.drawRect(x - candleWidth / 2, bodyTop, candleWidth, bodyHeight);
    }
}
