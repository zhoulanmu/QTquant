#pragma once

#include <QWidget>
#include "../strategy/strategybase.h"

namespace Ui {
class StrategyPanel;
}

class StrategyPanel : public QWidget
{
    Q_OBJECT

signals:
    void startClicked();
    void stopClicked();
    void parametersChanged();

public:
    explicit StrategyPanel(QWidget *parent = nullptr);
    ~StrategyPanel();

    QString getSymbol() const;
    int getFastMA() const;
    int getSlowMA() const;
    double getStopLossPercent() const;
    double getTakeProfitPercent() const;
    double getLotSize() const;

    void setRunningState(bool running);
    void addSystemLog(const QString& message);
    void addSignalLog(const StrategySignal& signal);

private slots:
    void on_paramChanged();

private:
    Ui::StrategyPanel *ui;
};
