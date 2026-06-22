#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTableWidget>

class SignalPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SignalPanel(QWidget *parent = nullptr);
    ~SignalPanel() override = default;

    void clearIndicators();
    void updateIndicators(double rsi, double macd, double bollUpper, double bollLower, double currentPrice);
    void addSignal(const QString& time, const QString& type, const QString& indicator, double value);

private:
    QLabel *m_rsiLabel;
    QLabel *m_macdLabel;
    QLabel *m_bollStatusLabel;
    QTableWidget *m_signalTable;
};
