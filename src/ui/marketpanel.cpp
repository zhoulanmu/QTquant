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

void MarketPanel::setSymbolSelector(QWidget* selector)
{
    if (!selector) {
        return;
    }

    selector->setParent(ui->groupBox);
    selector->setMinimumHeight(42);
    ui->label->setText(QStringLiteral("当前:"));

    ui->gridLayout->addWidget(selector, 0, 0, 1, 4);
    ui->gridLayout->addWidget(ui->label, 1, 0);
    ui->gridLayout->addWidget(ui->symbolLabel, 1, 1);
    ui->gridLayout->addWidget(ui->label_2, 1, 2);
    ui->gridLayout->addWidget(ui->timeLabel, 1, 3);
    ui->gridLayout->addWidget(ui->label_3, 2, 0);
    ui->gridLayout->addWidget(ui->openLabel, 2, 1);
    ui->gridLayout->addWidget(ui->label_4, 2, 2);
    ui->gridLayout->addWidget(ui->closeLabel, 2, 3);
    ui->gridLayout->addWidget(ui->label_5, 3, 0);
    ui->gridLayout->addWidget(ui->highLabel, 3, 1);
    ui->gridLayout->addWidget(ui->label_6, 3, 2);
    ui->gridLayout->addWidget(ui->lowLabel, 3, 3);
    ui->gridLayout->addWidget(ui->label_7, 4, 0);
    ui->gridLayout->addWidget(ui->volumeLabel, 4, 1);
    ui->gridLayout->addWidget(ui->label_8, 4, 2);
    ui->gridLayout->addWidget(ui->turnoverLabel, 4, 3);
}

void MarketPanel::updateMarketData(const MarketData &data)
{
    const QString displaySymbol = data.name.isEmpty()
        ? data.symbol
        : QStringLiteral("%1 %2").arg(data.symbol, data.name);

    ui->symbolLabel->setText(displaySymbol);
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
