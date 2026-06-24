#include "chartpanel.h"
#include "ui_chartpanel.h"

#include <QPainter>
#include <QTime>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
bool sameSymbolDate(const MarketData& left, const MarketData& right)
{
    return left.symbol == right.symbol
        && left.timestamp.isValid()
        && right.timestamp.isValid()
        && left.timestamp.date() == right.timestamp.date();
}

bool sameMinute(const MarketData& left, const MarketData& right)
{
    return sameSymbolDate(left, right)
        && left.timestamp.time().hour() == right.timestamp.time().hour()
        && left.timestamp.time().minute() == right.timestamp.time().minute();
}

bool sameChartValues(const MarketData& left, const MarketData& right)
{
    return sameSymbolDate(left, right)
        && std::abs(left.close - right.close) < 0.0001
        && std::abs(left.averagePrice - right.averagePrice) < 0.0001
        && std::abs(left.previousClose - right.previousClose) < 0.0001;
}

bool samePoint(const MarketData& left, const MarketData& right)
{
    return sameMinute(left, right) && sameChartValues(left, right);
}

bool beforeAShareOpen(const MarketData& data)
{
    return data.timestamp.isValid() && data.timestamp.time() < QTime(9, 30);
}

bool sameHistory(const std::deque<MarketData>& left, const std::deque<MarketData>& right)
{
    if (left.size() != right.size()) {
        return false;
    }

    for (size_t i = 0; i < left.size(); ++i) {
        if (!samePoint(left.at(i), right.at(i))) {
            return false;
        }
    }
    return true;
}

std::deque<MarketData> normalizedHistory(QVector<MarketData> points)
{
    std::sort(points.begin(), points.end(), [](const MarketData& left, const MarketData& right) {
        if (left.symbol != right.symbol) {
            return left.symbol < right.symbol;
        }
        return left.timestamp < right.timestamp;
    });

    std::deque<MarketData> history;
    for (const MarketData& point : points) {
        if (point.close <= 0.0 || !point.timestamp.isValid()) {
            continue;
        }
        if (!history.empty()
            && sameMinute(history.back(), point)) {
            history.back() = point;
        } else {
            history.push_back(point);
        }
    }
    return history;
}
}

ChartPanel::ChartPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartPanel),
    m_minPrice(0.0),
    m_maxPrice(1.0)
{
    ui->setupUi(this);
    setAutoFillBackground(false);

    m_candlestickWidget = new CandlestickWidget(this);
    m_candlestickWidget->setStyleSheet("background-color: transparent; border: none;");
    ui->verticalLayout->removeWidget(ui->chartWidget);
    delete ui->chartWidget;
    ui->chartWidget = m_candlestickWidget;
    ui->verticalLayout->addWidget(m_candlestickWidget);
    ui->verticalLayout->setContentsMargins(10, 10, 10, 10);
}

ChartPanel::~ChartPanel()
{
    delete ui;
}

void ChartPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF frame = rect().adjusted(1.0, 1.0, -1.0, -1.0);
    painter.setPen(QPen(QColor(74, 85, 104), 1.2));
    painter.setBrush(QColor(26, 26, 46));
    painter.drawRoundedRect(frame, 8.0, 8.0);
}

void ChartPanel::clearData()
{
    m_priceHistory.clear();
    m_minPrice = 0.0;
    m_maxPrice = 1.0;
    m_candlestickWidget->setEmptyMessage(QStringLiteral("正在加载行情数据..."));
    m_candlestickWidget->updateData(m_priceHistory, m_minPrice, m_maxPrice);
}

void ChartPanel::setEmptyMessage(const QString& message)
{
    m_candlestickWidget->setEmptyMessage(message);
}
void ChartPanel::updateChartData(const MarketData &data)
{
    if (data.close <= 0.0 || !data.timestamp.isValid()) {
        return;
    }

    if (!m_priceHistory.empty()
        && (m_priceHistory.back().symbol != data.symbol
            || m_priceHistory.back().timestamp.date() != data.timestamp.date())) {
        m_priceHistory.clear();
    }

    const bool sameMinuteBucket = !m_priceHistory.empty() && sameMinute(m_priceHistory.back(), data);
    const bool samePreOpenBucket = !m_priceHistory.empty()
        && sameSymbolDate(m_priceHistory.back(), data)
        && beforeAShareOpen(m_priceHistory.back())
        && beforeAShareOpen(data);
    if (sameMinuteBucket || samePreOpenBucket) {
        if ((sameMinuteBucket && samePoint(m_priceHistory.back(), data))
            || (samePreOpenBucket && sameChartValues(m_priceHistory.back(), data))) {
            return;
        }
        m_priceHistory.back() = data;
    } else {
        m_priceHistory.push_back(data);
    }

    QVector<MarketData> sortedPoints;
    sortedPoints.reserve(static_cast<int>(m_priceHistory.size()));
    for (const MarketData& point : m_priceHistory) {
        sortedPoints.append(point);
    }
    m_priceHistory = normalizedHistory(sortedPoints);

    while (m_priceHistory.size() > MAX_POINTS) {
        m_priceHistory.pop_front();
    }

    updatePriceRange();
    m_candlestickWidget->updateData(m_priceHistory, m_minPrice, m_maxPrice);
}

void ChartPanel::updateIntradayData(const QVector<MarketData>& data)
{
    std::deque<MarketData> nextHistory = normalizedHistory(data);

    if (nextHistory.empty()) {
        return;
    }

    if (!m_priceHistory.empty()
        && m_priceHistory.back().close > 0.0
        && m_priceHistory.back().timestamp.isValid()
        && m_priceHistory.back().symbol == nextHistory.back().symbol
        && m_priceHistory.back().timestamp.date() == nextHistory.back().timestamp.date()
        && m_priceHistory.back().timestamp >= nextHistory.back().timestamp
        && m_priceHistory.back().timestamp.time() >= QTime(9, 30)) {
        if (!nextHistory.empty() && sameMinute(nextHistory.back(), m_priceHistory.back())) {
            nextHistory.back() = m_priceHistory.back();
        } else {
            nextHistory.push_back(m_priceHistory.back());
        }
    }

    QVector<MarketData> sortedPoints;
    sortedPoints.reserve(static_cast<int>(nextHistory.size()));
    for (const MarketData& point : nextHistory) {
        sortedPoints.append(point);
    }
    nextHistory = normalizedHistory(sortedPoints);

    while (nextHistory.size() > MAX_POINTS) {
        nextHistory.pop_front();
    }

    if (sameHistory(m_priceHistory, nextHistory)) {
        return;
    }

    m_priceHistory = nextHistory;
    updatePriceRange();
    m_candlestickWidget->updateData(m_priceHistory, m_minPrice, m_maxPrice);
}

void ChartPanel::updatePriceRange()
{
    if (m_priceHistory.empty()) {
        m_minPrice = 0.0;
        m_maxPrice = 1.0;
        return;
    }

    m_minPrice = std::numeric_limits<double>::max();
    m_maxPrice = std::numeric_limits<double>::lowest();

    double previousClose = 0.0;
    for (const auto& data : m_priceHistory) {
        if (data.previousClose > 0.0) {
            previousClose = data.previousClose;
        }
        if (data.close > 0.0) {
            m_minPrice = qMin(m_minPrice, data.close);
            m_maxPrice = qMax(m_maxPrice, data.close);
        }
        if (data.averagePrice > 0.0) {
            m_minPrice = qMin(m_minPrice, data.averagePrice);
            m_maxPrice = qMax(m_maxPrice, data.averagePrice);
        }
    }

    if (m_minPrice == std::numeric_limits<double>::max() || m_maxPrice == std::numeric_limits<double>::lowest()) {
        m_minPrice = 0.0;
        m_maxPrice = 1.0;
        return;
    }

    if (previousClose > 0.0) {
        m_minPrice = qMin(m_minPrice, previousClose);
        m_maxPrice = qMax(m_maxPrice, previousClose);
    }

    const double range = m_maxPrice - m_minPrice;
    const double padding = range < 0.001
        ? qMax(0.01, m_maxPrice * 0.0005)
        : qMax(0.01, range * 0.12);

    m_minPrice = qMax(0.0, m_minPrice - padding);
    m_maxPrice += padding;
}
