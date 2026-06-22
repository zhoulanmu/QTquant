#include "signalsignalpanel.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QTableWidgetItem>

SignalPanel::SignalPanel(QWidget *parent) :
    QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    auto indicatorGroup = new QGroupBox(QStringLiteral("实时指标"), this);
    auto indicatorLayout = new QGridLayout(indicatorGroup);

    indicatorLayout->addWidget(new QLabel(QStringLiteral("RSI(14):")), 0, 0);
    m_rsiLabel = new QLabel(QStringLiteral("--"));
    indicatorLayout->addWidget(m_rsiLabel, 0, 1);

    indicatorLayout->addWidget(new QLabel(QStringLiteral("MACD:")), 1, 0);
    m_macdLabel = new QLabel(QStringLiteral("--"));
    indicatorLayout->addWidget(m_macdLabel, 1, 1);

    indicatorLayout->addWidget(new QLabel(QStringLiteral("布林带:")), 2, 0);
    m_bollStatusLabel = new QLabel(QStringLiteral("--"));
    indicatorLayout->addWidget(m_bollStatusLabel, 2, 1);

    mainLayout->addWidget(indicatorGroup);

    auto signalGroup = new QGroupBox(QStringLiteral("最近信号"), this);
    auto signalLayout = new QVBoxLayout(signalGroup);

    m_signalTable = new QTableWidget(this);
    m_signalTable->setColumnCount(4);
    m_signalTable->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("类型"), QStringLiteral("依据"), QStringLiteral("数值")});
    m_signalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_signalTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_signalTable->setRowCount(0);
    m_signalTable->setMaximumHeight(120);
    m_signalTable->resizeColumnsToContents();

    signalLayout->addWidget(m_signalTable);
    mainLayout->addWidget(signalGroup);
    setLayout(mainLayout);
}

void SignalPanel::clearIndicators()
{
    m_rsiLabel->setText(QStringLiteral("--"));
    m_rsiLabel->setStyleSheet("");
    m_macdLabel->setText(QStringLiteral("--"));
    m_macdLabel->setStyleSheet("");
    m_bollStatusLabel->setText(QStringLiteral("--"));
    m_bollStatusLabel->setStyleSheet("");
}

void SignalPanel::updateIndicators(double rsi, double macd, double bollUpper, double bollLower, double currentPrice)
{
    m_rsiLabel->setText(QString("%1").arg(rsi, 0, 'f', 1));

    if (rsi > 70) {
        m_rsiLabel->setStyleSheet("color: green; font-weight: bold;");
    } else if (rsi < 30) {
        m_rsiLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        m_rsiLabel->setStyleSheet("");
    }

    if (macd >= 0) {
        m_macdLabel->setText(QStringLiteral("+") + QString("%1").arg(macd, 0, 'f', 2));
        m_macdLabel->setStyleSheet("color: red;");
    } else {
        m_macdLabel->setText(QString("%1").arg(macd, 0, 'f', 2));
        m_macdLabel->setStyleSheet("color: green;");
    }

    const double bollMid = (bollUpper + bollLower) / 2;
    if (currentPrice >= bollUpper) {
        m_bollStatusLabel->setText(QStringLiteral("上轨"));
        m_bollStatusLabel->setStyleSheet("color: green;");
    } else if (currentPrice <= bollLower) {
        m_bollStatusLabel->setText(QStringLiteral("下轨"));
        m_bollStatusLabel->setStyleSheet("color: red;");
    } else if (currentPrice >= bollMid) {
        m_bollStatusLabel->setText(QStringLiteral("上沿"));
        m_bollStatusLabel->setStyleSheet("color: darkred;");
    } else {
        m_bollStatusLabel->setText(QStringLiteral("下沿"));
        m_bollStatusLabel->setStyleSheet("color: darkgreen;");
    }
}

void SignalPanel::addSignal(const QString& time, const QString& type, const QString& indicator, double value)
{
    const int row = m_signalTable->rowCount();
    m_signalTable->insertRow(row);

    m_signalTable->setItem(row, 0, new QTableWidgetItem(time));

    auto typeItem = new QTableWidgetItem(type);
    typeItem->setForeground(QBrush(type == QStringLiteral("买入") ? Qt::red : Qt::green));
    m_signalTable->setItem(row, 1, typeItem);

    m_signalTable->setItem(row, 2, new QTableWidgetItem(indicator));
    m_signalTable->setItem(row, 3, new QTableWidgetItem(QString("%1").arg(value, 0, 'f', 2)));

    m_signalTable->resizeColumnsToContents();

    if (m_signalTable->rowCount() > 10) {
        m_signalTable->removeRow(0);
    }
}
