#pragma once

#include <QMainWindow>
#include <QMap>
#include <QString>
#include "market/marketdata.h"
#include "strategy/strategybase.h"
#include "ui/accountpanel.h"
#include "ui/statisticspanel.h"
#include "ui/signalsignalpanel.h"
#include "ui/newspanel.h"

class QTabWidget;
class QWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(bool guestMode = true, const QString& accountName = QString(), QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMarketDataUpdated(const MarketData& data);
    void onIntradayDataUpdated(const QVector<MarketData>& data);
    void onMarketDataError(const QString& message);
    void onStrategyMarketDataUpdated(const MarketData& data);
    void onStrategyIntradayDataUpdated(const QVector<MarketData>& data);
    void onStrategyMarketDataError(const QString& message);
    void onStrategySignal(const StrategySignal& signal);
    void onStartStrategy();
    void onStopStrategy();
    void onUpdateParameters();
    void onViewSymbolChanged(const QString& symbol);
    void onFavoriteSelected(const QString& symbol);
    void onFavoriteBuyRequested(const QString& symbol, double price, double volume);
    void onFavoriteSellRequested(const QString& symbol, double price, double volume);
    void onManualTradeQuoteUpdated(const MarketData& data);
    void onManualTradeQuoteError(const QString& message);

private:
    void updateAccountInfo();
    void executeOrder(const StrategySignal& signal);
    void buildTabbedLayout();
    QWidget* createMainTab();
    QWidget* createStrategyTab();
    QWidget* createPersonalTab();
    QWidget* createNewsTab();
    void configureStrategy(const StrategyConfig& config);
    void updateSignalIndicators();
    double latestPriceForSymbol(const QString& symbol) const;
    void requestManualTrade(const QString& symbol, SignalType type, double price, double volume);
    void executeManualTrade(const QString& symbol, SignalType type, double price, double volume);

private:
    Ui::MainWindow *ui;
    MarketDataSimulator* m_marketData;
    MarketDataSimulator* m_strategyMarketData;
    MarketDataSimulator* m_manualTradeMarketData;
    StrategyBase* m_strategy;
    StrategyType m_activeStrategyType;
    AccountPanel* m_accountPanel;
    StatisticsPanel* m_statisticsPanel;
    SignalPanel* m_signalPanel;
    NewsPanel* m_newsPanel;
    QTabWidget* m_mainTabs;
    bool m_isRunning;

    double m_initialCapital;
    double m_currentCash;
    QMap<QString, PositionInfo> m_positions;
    double m_currentPrice;
    MarketData m_lastMarketData;
    bool m_hasLastMarketData;
    MarketData m_lastStrategyMarketData;
    bool m_hasLastStrategyMarketData;
    QVector<MarketData> m_indicatorHistory;
    QString m_pendingManualTradeSymbol;
    QString m_manualPriceQuoteSymbol;
    SignalType m_pendingManualTradeType;
    double m_pendingManualTradeVolume;
    bool m_guestMode;
    QString m_accountName;
};