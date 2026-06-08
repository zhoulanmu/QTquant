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
    setStyleSheet("background-color: black;");
    ui->label->setStyleSheet("color: white; font-size: 14px;");

    m_candlestickWidget = new CandlestickWidget(this);
    ui->verticalLayout->removeWidget(ui->chartWidget);
    delete ui->chartWidget;
    ui->chartWidget = m_candlestickWidget;
    ui->verticalLayout->addWidget(m_candlestickWidget);
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
