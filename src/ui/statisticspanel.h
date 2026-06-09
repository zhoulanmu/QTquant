#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>

class StatisticsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsPanel(QWidget *parent = nullptr);
    ~StatisticsPanel() override = default;

    void setData(double turnover, double volume, double avgVolume, double profitRate,
                 double winRate, int tradeCount);

private:
    QLabel *m_turnoverLabel;
    QLabel *m_volumeLabel;
    QLabel *m_avgVolumeLabel;
    QLabel *m_profitRateLabel;
    QLabel *m_winRateLabel;
    QLabel *m_tradeCountLabel;
};
