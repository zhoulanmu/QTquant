#include "signalsignalpanel.h"
#include <QTableWidgetItem>
#include <QBrush>
#include <QColor>

SignalPanel::SignalPanel(QWidget *parent) :
    QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    auto indicatorGroup = new QGroupBox("实时指标", this);
    auto indicatorLayout = new QGridLayout(indicatorGroup);

    indicatorLayout->addWidget(new QLabel("RSI(14):"), 0, 0);
    m_rsiLabel = new QLabel("52.3");
    indicatorLayout->addWidget(m_rsiLabel, 0, 1);

    indicatorLayout->addWidget(new QLabel("MACD:"), 1, 0);
    m_macdLabel = new QLabel("+0.85");
    indicatorLayout->addWidget(m_macdLabel, 1, 1);

    indicatorLayout->addWidget(new QLabel("布林带:"), 2, 0);
    m_bollStatusLabel = new QLabel("中位");
    indicatorLayout->addWidget(m_bollStatusLabel, 2, 1);

    mainLayout->addWidget(indicatorGroup);

    auto signalGroup = new QGroupBox("最近信号", this);
    auto signalLayout = new QVBoxLayout(signalGroup);

    m_signalTable = new QTableWidget(this);
    m_signalTable->setColumnCount(4);
    m_signalTable->setHorizontalHeaderLabels({"时间", "类型", "依据", "数值"});
    m_signalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_signalTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_signalTable->setRowCount(0);
    m_signalTable->setMaximumHeight(120);

    m_signalTable->insertRow(0);
    m_signalTable->setItem(0, 0, new QTableWidgetItem("10:30:15"));
    auto buyItem = new QTableWidgetItem("买入");
    buyItem->setForeground(QBrush(Qt::red));
    m_signalTable->setItem(0, 1, buyItem);
    m_signalTable->setItem(0, 2, new QTableWidgetItem("MA金叉"));
    m_signalTable->setItem(0, 3, new QTableWidgetItem("MA5>MA20"));

    m_signalTable->insertRow(1);
    m_signalTable->setItem(1, 0, new QTableWidgetItem("11:15:22"));
    auto sellItem = new QTableWidgetItem("卖出");
    sellItem->setForeground(QBrush(Qt::green));
    m_signalTable->setItem(1, 1, sellItem);
    m_signalTable->setItem(1, 2, new QTableWidgetItem("RSI超买"));
    m_signalTable->setItem(1, 3, new QTableWidgetItem("RSI=78.5"));

    m_signalTable->insertRow(2);
    m_signalTable->setItem(2, 0, new QTableWidgetItem("14:20:33"));
    auto buyItem2 = new QTableWidgetItem("买入");
    buyItem2->setForeground(QBrush(Qt::red));
    m_signalTable->setItem(2, 1, buyItem2);
    m_signalTable->setItem(2, 2, new QTableWidgetItem("布林带下轨"));
    m_signalTable->setItem(2, 3, new QTableWidgetItem("接近支撑"));

    m_signalTable->resizeColumnsToContents();
    signalLayout->addWidget(m_signalTable);

    mainLayout->addWidget(signalGroup);

    setLayout(mainLayout);
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
        m_macdLabel->setText("+" + QString("%1").arg(macd, 0, 'f', 2));
        m_macdLabel->setStyleSheet("color: red;");
    } else {
        m_macdLabel->setText(QString("%1").arg(macd, 0, 'f', 2));
        m_macdLabel->setStyleSheet("color: green;");
    }

    double bollMid = (bollUpper + bollLower) / 2;
    if (currentPrice >= bollUpper) {
        m_bollStatusLabel->setText("上轨");
        m_bollStatusLabel->setStyleSheet("color: green;");
    } else if (currentPrice <= bollLower) {
        m_bollStatusLabel->setText("下轨");
        m_bollStatusLabel->setStyleSheet("color: red;");
    } else if (currentPrice >= bollMid) {
        m_bollStatusLabel->setText("上沿");
        m_bollStatusLabel->setStyleSheet("color: darkred;");
    } else {
        m_bollStatusLabel->setText("下沿");
        m_bollStatusLabel->setStyleSheet("color: darkgreen;");
    }
}

void SignalPanel::addSignal(const QString& time, const QString& type, const QString& indicator, double value)
{
    int row = m_signalTable->rowCount();
    m_signalTable->insertRow(row);

    m_signalTable->setItem(row, 0, new QTableWidgetItem(time));

    auto typeItem = new QTableWidgetItem(type);
    if (type == "买入") {
        typeItem->setForeground(QBrush(Qt::red));
    } else {
        typeItem->setForeground(QBrush(Qt::green));
    }
    m_signalTable->setItem(row, 1, typeItem);

    m_signalTable->setItem(row, 2, new QTableWidgetItem(indicator));
    m_signalTable->setItem(row, 3, new QTableWidgetItem(QString("%1").arg(value, 0, 'f', 2)));

    m_signalTable->resizeColumnsToContents();

    if (m_signalTable->rowCount() > 10) {
        m_signalTable->removeRow(0);
    }
}
