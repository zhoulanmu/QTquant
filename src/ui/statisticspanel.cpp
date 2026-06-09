#include "statisticspanel.h"

StatisticsPanel::StatisticsPanel(QWidget *parent) :
    QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    auto marketGroup = new QGroupBox("今日行情统计", this);
    auto marketLayout = new QGridLayout(marketGroup);

    marketLayout->addWidget(new QLabel("成交额(万):"), 0, 0);
    m_turnoverLabel = new QLabel("2,156.80");
    marketLayout->addWidget(m_turnoverLabel, 0, 1);

    marketLayout->addWidget(new QLabel("成交量(手):"), 1, 0);
    m_volumeLabel = new QLabel("156,230");
    marketLayout->addWidget(m_volumeLabel, 1, 1);

    marketLayout->addWidget(new QLabel("平均成交量:"), 2, 0);
    m_avgVolumeLabel = new QLabel("12,856");
    marketLayout->addWidget(m_avgVolumeLabel, 2, 1);

    mainLayout->addWidget(marketGroup);

    auto strategyGroup = new QGroupBox("策略绩效统计", this);
    auto strategyLayout = new QGridLayout(strategyGroup);

    strategyLayout->addWidget(new QLabel("胜率(%):"), 0, 0);
    m_winRateLabel = new QLabel("68.5%");
    m_winRateLabel->setStyleSheet("color: red; font-weight: bold;");
    strategyLayout->addWidget(m_winRateLabel, 0, 1);

    strategyLayout->addWidget(new QLabel("盈亏比:"), 1, 0);
    m_profitRateLabel = new QLabel("2.3:1");
    m_profitRateLabel->setStyleSheet("color: red; font-weight: bold;");
    strategyLayout->addWidget(m_profitRateLabel, 1, 1);

    strategyLayout->addWidget(new QLabel("交易次数:"), 2, 0);
    m_tradeCountLabel = new QLabel("45");
    strategyLayout->addWidget(m_tradeCountLabel, 2, 1);

    mainLayout->addWidget(strategyGroup);

    setLayout(mainLayout);
}

void StatisticsPanel::setData(double turnover, double volume, double avgVolume,
                              double profitRate, double winRate, int tradeCount)
{
    m_turnoverLabel->setText(QString::number(turnover / 10000, 'f', 2));
    m_volumeLabel->setText(QString::number(volume, 'f', 0));
    m_avgVolumeLabel->setText(QString::number(avgVolume, 'f', 0));
    m_profitRateLabel->setText(QString("%1:1").arg(profitRate, 0, 'f', 1));
    m_winRateLabel->setText(QString("%1%").arg(winRate, 0, 'f', 1));
    m_tradeCountLabel->setText(QString::number(tradeCount));

    if (winRate >= 50) {
        m_winRateLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        m_winRateLabel->setStyleSheet("color: green; font-weight: bold;");
    }
}
