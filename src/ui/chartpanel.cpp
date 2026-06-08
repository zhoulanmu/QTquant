#include "chartpanel.h"
#include "ui_chartpanel.h"
#include <QPainter>
#include <QDebug>
#include <limits>

ChartPanel::ChartPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartPanel),
    m_minPrice(0.0),
    m_maxPrice(1.0)
{
    ui->setupUi(this);
    setStyleSheet("background-color: #1a1a2e; border: 2px solid #4a5568; border-radius: 12px; padding: 8px;");

    m_candlestickWidget = new CandlestickWidget(this);
    m_candlestickWidget->setStyleSheet("background-color: #1a1a2e; border: none; border-radius: 8px;");
    ui->verticalLayout->removeWidget(ui->chartWidget);
    delete ui->chartWidget;
    ui->chartWidget = m_candlestickWidget;
    ui->verticalLayout->addWidget(m_candlestickWidget);
    ui->verticalLayout->setContentsMargins(0, 0, 0, 0);

    initMockData();
}

void ChartPanel::initMockData()
{
    double basePrice = 10.50;
    QDateTime now = QDateTime::currentDateTime();

    for (int i = 20; i >= 0; --i) {
        MarketData data;
        data.symbol = "000001.SH";
        data.timestamp = now.addSecs(-i * 60);
        
        double change = (std::rand() % 100 - 50) * 0.01;
        data.open = basePrice + change;
        data.high = data.open + std::rand() % 50 * 0.01;
        data.low = data.open - std::rand() % 50 * 0.01;
        data.close = data.open + (std::rand() % 60 - 30) * 0.01;
        data.volume = 1000 + std::rand() % 5000;
        data.turnover = data.close * data.volume;

        m_priceHistory.push_back(data);
        basePrice = data.close;
    }

    updatePriceRange();
    m_candlestickWidget->updateData(m_priceHistory, m_minPrice, m_maxPrice);
    m_candlestickWidget->update();
    this->update();
}

ChartPanel::~ChartPanel()
{
    delete ui;
}

void ChartPanel::updateChartData(const MarketData &data)
{
    m_priceHistory.push_back(data);

    if (m_priceHistory.size() > MAX_POINTS) {
        m_priceHistory.pop_front();
    }

    updatePriceRange();
    m_candlestickWidget->updateData(m_priceHistory, m_minPrice, m_maxPrice);
}

void ChartPanel::updatePriceRange()
{
    if (m_priceHistory.empty()) return;

    m_minPrice = std::numeric_limits<double>::max();
    m_maxPrice = std::numeric_limits<double>::min();

    for (const auto& data : m_priceHistory) {
        m_minPrice = qMin(m_minPrice, data.low);
        m_maxPrice = qMax(m_maxPrice, data.high);
    }

    double range = m_maxPrice - m_minPrice;
    if (range < 0.001) {
        m_minPrice -= 0.01;
        m_maxPrice += 0.01;
    } else {
        m_minPrice -= range * 0.1;
        m_maxPrice += range * 0.1;
    }
}
