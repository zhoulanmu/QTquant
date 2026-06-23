#include "chartpanel.h"
#include "ui_chartpanel.h"

#include <QPainter>
#include <limits>

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

    if (!m_priceHistory.empty()
        && m_priceHistory.back().timestamp.date() == data.timestamp.date()
        && m_priceHistory.back().timestamp.time().hour() == data.timestamp.time().hour()
        && m_priceHistory.back().timestamp.time().minute() == data.timestamp.time().minute()) {
        m_priceHistory.back() = data;
    } else {
        m_priceHistory.push_back(data);
    }

    while (m_priceHistory.size() > MAX_POINTS) {
        m_priceHistory.pop_front();
    }

    updatePriceRange();
    m_candlestickWidget->updateData(m_priceHistory, m_minPrice, m_maxPrice);
}

void ChartPanel::updateIntradayData(const QVector<MarketData>& data)
{
    m_priceHistory.clear();
    for (const MarketData& point : data) {
        if (point.close > 0.0 && point.timestamp.isValid()) {
            m_priceHistory.push_back(point);
        }
    }

    while (m_priceHistory.size() > MAX_POINTS) {
        m_priceHistory.pop_front();
    }

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

    if (previousClose > 0.0) {
        const double upperDistance = qAbs(m_maxPrice - previousClose);
        const double lowerDistance = qAbs(previousClose - m_minPrice);
        const double distance = qMax(qMax(upperDistance, lowerDistance), previousClose * 0.003);
        m_minPrice = qMax(0.0, previousClose - distance);
        m_maxPrice = previousClose + distance;
    }

    if (m_minPrice == std::numeric_limits<double>::max() || m_maxPrice == std::numeric_limits<double>::lowest()) {
        m_minPrice = 0.0;
        m_maxPrice = 1.0;
        return;
    }

    const double range = m_maxPrice - m_minPrice;
    const double padding = range < 0.001
        ? qMax(0.01, m_maxPrice * 0.0005)
        : qMax(0.01, range * 0.25);

    m_minPrice = qMax(0.0, m_minPrice - padding);
    m_maxPrice += padding;
}
