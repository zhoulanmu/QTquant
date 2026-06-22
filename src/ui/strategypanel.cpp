#include "strategypanel.h"
#include "ui_strategypanel.h"
#include <QScrollBar>
#include <QTime>

StrategyPanel::StrategyPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StrategyPanel)
{
    ui->setupUi(this);
    ui->logTextEdit->setReadOnly(true);

    ui->label->setBuddy(nullptr);
    ui->label_2->setBuddy(nullptr);
    ui->label_3->setBuddy(nullptr);
    ui->label_4->setBuddy(nullptr);
    ui->label_5->setBuddy(nullptr);
    ui->label_6->setBuddy(nullptr);

    addSystemLog(QStringLiteral("行情连接中，等待策略启动。"));

    connect(ui->startBtn, &QPushButton::clicked, this, &StrategyPanel::startClicked);
    connect(ui->stopBtn, &QPushButton::clicked, this, &StrategyPanel::stopClicked);
    connect(ui->symbolEdit, &QLineEdit::textChanged, this, &StrategyPanel::on_paramChanged);
    connect(ui->fastMASpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->slowMASpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->stopLossSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->takeProfitSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
    connect(ui->lotSizeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrategyPanel::on_paramChanged);
}

StrategyPanel::~StrategyPanel()
{
    delete ui;
}

QString StrategyPanel::getSymbol() const
{
    return ui->symbolEdit->text();
}

int StrategyPanel::getFastMA() const
{
    return ui->fastMASpin->value();
}

int StrategyPanel::getSlowMA() const
{
    return ui->slowMASpin->value();
}

double StrategyPanel::getStopLossPercent() const
{
    return ui->stopLossSpin->value();
}

double StrategyPanel::getTakeProfitPercent() const
{
    return ui->takeProfitSpin->value();
}

double StrategyPanel::getLotSize() const
{
    return ui->lotSizeSpin->value();
}

void StrategyPanel::setRunningState(bool running)
{
    ui->startBtn->setEnabled(!running);
    ui->stopBtn->setEnabled(running);
    ui->symbolEdit->setEnabled(!running);
    ui->fastMASpin->setEnabled(!running);
    ui->slowMASpin->setEnabled(!running);
    ui->stopLossSpin->setEnabled(!running);
    ui->takeProfitSpin->setEnabled(!running);
    ui->lotSizeSpin->setEnabled(!running);
}

void StrategyPanel::addSystemLog(const QString &message)
{
    const QString logLine = QStringLiteral("%1 [系统] %2")
            .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), message);

    ui->logTextEdit->append(logLine);

    QScrollBar *scrollBar = ui->logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void StrategyPanel::addSignalLog(const StrategySignal &signal)
{
    QString timeStr = signal.timestamp.toString("HH:mm:ss");
    QString typeStr;

    switch (signal.type) {
    case SignalType::BUY:
        typeStr = QStringLiteral("[买入]");
        break;
    case SignalType::SELL:
        typeStr = QStringLiteral("[卖出]");
        break;
    case SignalType::STOP_LOSS:
        typeStr = QStringLiteral("[止损]");
        break;
    case SignalType::TAKE_PROFIT:
        typeStr = QStringLiteral("[止盈]");
        break;
    default:
        typeStr = QStringLiteral("[未知]");
    }

    QString logLine = QString("%1 %2 价格: %3 数量: %4 %5\n")
            .arg(timeStr)
            .arg(typeStr)
            .arg(signal.price, 0, 'f', 2)
            .arg(signal.volume, 0, 'f', 0)
            .arg(signal.comment);

    ui->logTextEdit->append(logLine);

    QScrollBar *scrollBar = ui->logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}


void StrategyPanel::on_paramChanged()
{
    emit parametersChanged();
}
