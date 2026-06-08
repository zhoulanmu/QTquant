#include "marketpanel.h"
#include "ui_marketpanel.h"

MarketPanel::MarketPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MarketPanel)
{
    ui->setupUi(this);
    ui->label->setBuddy(nullptr);
    ui->label_2->setBuddy(nullptr);
    ui->label_3->setBuddy(nullptr);
    ui->label_4->setBuddy(nullptr);
    ui->label_5->setBuddy(nullptr);
    ui->label_6->setBuddy(nullptr);
    ui->label_7->setBuddy(nullptr);
    ui->label_8->setBuddy(nullptr);
}

MarketPanel::~MarketPanel()
{
    delete ui;
}

void MarketPanel::updateMarketData(const MarketData &data)
{
    ui->symbolLabel->setText(data.symbol);
    ui->timeLabel->setText(data.timestamp.toString("HH:mm:ss"));
    ui->openLabel->setText(QString("%1").arg(data.open, 0, 'f', 2));
    ui->highLabel->setText(QString("%1").arg(data.high, 0, 'f', 2));
    ui->lowLabel->setText(QString("%1").arg(data.low, 0, 'f', 2));
    ui->closeLabel->setText(QString("%1").arg(data.close, 0, 'f', 2));
    ui->volumeLabel->setText(data.volume > 0.0 ? QString("%1").arg(data.volume, 0, 'f', 0) : QStringLiteral("--"));
    ui->turnoverLabel->setText(data.turnover > 0.0
        ? QString("%1").arg(data.turnover / 10000.0, 0, 'f', 2)
        : QStringLiteral("--"));
}
