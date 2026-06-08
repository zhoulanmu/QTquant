#pragma once

#include <QMainWindow>
#include <QMap>
#include "market/marketdata.h"
#include "strategy/strategybase.h"
#include "strategy/movingaveragestrategy.h"
#include "ui/accountpanel.h"
#include "ui/statisticspanel.h"
#include "ui/signalsignalpanel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMarketDataUpdated(const MarketData& data);
    void onStrategySignal(const StrategySignal& signal);
    void onStartStrategy();
    void onStopStrategy();
    void onUpdateParameters();

private:
    void updateAccountInfo();
    void executeOrder(const StrategySignal& signal);

private:
    Ui::MainWindow *ui;
    MarketDataSimulator* m_marketData;
    MovingAverageStrategy* m_strategy;
    AccountPanel* m_accountPanel;
    StatisticsPanel* m_statisticsPanel;
    SignalPanel* m_signalPanel;
    bool m_isRunning;

    double m_initialCapital;
    double m_currentCash;
    QMap<QString, PositionInfo> m_positions;
    double m_currentPrice;
};
