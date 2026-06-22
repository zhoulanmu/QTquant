#pragma once

#include <QMap>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include "../strategy/strategybase.h"

class QCheckBox;
class QComboBox;
class QCompleter;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QSpinBox;
class QStringListModel;
class QTimer;

struct StockCandidate
{
    QString symbol;
    QString name;
};

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
    void viewSymbolChanged(const QString& symbol);
    void favoriteSelected(const QString& symbol);
    void favoriteBuyRequested(const QString& symbol, double price, double volume);
    void favoriteSellRequested(const QString& symbol, double price, double volume);

public:
    explicit StrategyPanel(QWidget *parent = nullptr);
    ~StrategyPanel();

    QString getSymbol() const;
    QString getViewSymbol() const;
    QString getStrategySymbol() const;
    StrategyConfig strategyConfig() const;
    int getFastMA() const;
    int getSlowMA() const;
    double getStopLossPercent() const;
    double getTakeProfitPercent() const;
    double getLotSize() const;
    QString currentStrategyName() const;
    QString currentStrategyConfigurationSummary() const;

    void setRunningState(bool running);
    void rememberStockName(const QString& symbol, const QString& name);
    void addSystemLog(const QString& message);
    void addSignalLog(const StrategySignal& signal);
    void setStrategyRuntimeStatus(const QString& viewSymbol, const QString& strategySymbol, const QString& quoteStatus, const QString& runStatus, const QString& lastQuoteTime);
    QWidget* takeSymbolSelectorWidget(QWidget* parent);
    QWidget* takeStrategyControlWidget(QWidget* parent);
    QWidget* takeTradeLogWidget(QWidget* parent);
    QWidget* takeWatchlistWidget(QWidget* parent);
    void setManualTradePrice(const QString& symbol, double price);

private slots:
    void on_paramChanged();
    void onSymbolTextChanged(const QString& text);
    void onSymbolEditingFinished();
    void onStrategySymbolTextChanged(const QString& text);
    void onStrategySymbolEditingFinished();
    void onFavoriteSearchTextChanged(const QString& text);
    void onFavoriteSearchEditingFinished();
    void onSearchTimerTimeout();
    void onSearchReplyFinished();
    void onCompleterActivated(const QString& text);
    void onStrategyCompleterActivated(const QString& text);
    void onFavoriteCompleterActivated(const QString& text);
    void onStrategyPresetChanged(int index);
    void onApplyStrategyPresetClicked();
    void onStrategyConfigChanged();
    void onAddFavoriteClicked();
    void onRemoveFavoriteClicked();
    void onFavoriteActivated(QListWidgetItem* item);
    void onFavoriteSelectionChanged();
    void onFavoriteBuyClicked();
    void onFavoriteSellClicked();

private:
    void setupStockSearchUi();
    void setupCommonStrategyUi();
    void setupCurrentStrategyConfigUi();
    void updateCurrentStrategyConfigUi();
    void setGrowthConfigEnabled(bool enabled);
    QString selectedGrowthTracks() const;
    void loadViewSymbol();
    void saveViewSymbol() const;
    void loadStrategySymbol();
    void saveStrategySymbol() const;
    void saveCurrentStrategyType() const;
    void loadStrategySettings();
    void saveStrategySettings() const;
    void loadPersonalSettings();
    void savePersonalSettings() const;
    void loadWatchlist();
    void saveWatchlist() const;
    void refreshWatchlist();
    void selectSymbol(const QString& symbol, const QString& name, bool emitChange);
    void selectStrategySymbol(const QString& symbol, const QString& name, bool emitChange);
    void addFavoriteSymbol(const QString& symbol, const QString& name);
    QString selectedFavoriteSymbol() const;
    QString resolveSymbolText(const QString& text) const;
    QString stockNameForSymbol(const QString& symbol) const;
    QVector<StockCandidate> localStockCatalog() const;
    QVector<StockCandidate> searchLocalStocks(const QString& keyword) const;
    QVector<StockCandidate> parseSearchResponse(const QByteArray& payload) const;
    void updateSearchSuggestions(const QString& keyword, const QVector<StockCandidate>& remoteCandidates = QVector<StockCandidate>());
    void showSearchCandidates(const QVector<StockCandidate>& candidates);
    void abortSearchReply();
    bool isValidMarketSymbol(const QString& symbol) const;
    bool containsChinese(const QString& text) const;
    static QString displayTextForCandidate(const StockCandidate& candidate);
    static QString symbolFromQuoteId(const QString& quoteId, const QString& code);

private:
    Ui::StrategyPanel *ui;
    QCompleter* m_symbolCompleter;
    QCompleter* m_strategySymbolCompleter;
    QCompleter* m_watchlistSymbolCompleter;
    QStringListModel* m_searchModel;
    QStringListModel* m_strategySearchModel;
    QTimer* m_searchTimer;
    QNetworkAccessManager* m_searchNetwork;
    QNetworkReply* m_searchReply;
    QLineEdit* m_strategySymbolEdit;
    QLineEdit* m_watchlistSymbolEdit;
    QLineEdit* m_activeSearchEdit;
    QComboBox* m_strategyPresetCombo;
    QLabel* m_strategyPresetDescLabel;
    QLabel* m_viewSymbolStatusLabel;
    QLabel* m_strategySymbolStatusLabel;
    QLabel* m_strategyQuoteStatusLabel;
    QLabel* m_strategyRunStatusLabel;
    QLabel* m_strategyQuoteTimeLabel;
    QPushButton* m_strategyDetailButton;
    QPushButton* m_applyStrategyPresetBtn;
    QGroupBox* m_strategyConfigGroup;
    QLabel* m_strategyConfigHintLabel;
    QVector<QWidget*> m_growthConfigWidgets;
    QVector<QCheckBox*> m_growthTrackChecks;
    QCheckBox* m_ma60UpCheck;
    QCheckBox* m_profitGrowthCheck;
    QCheckBox* m_orderLandingCheck;
    QCheckBox* m_noPureConceptCheck;
    QCheckBox* m_volumePullbackCheck;
    QSpinBox* m_pullbackMinSpin;
    QSpinBox* m_pullbackMaxSpin;
    QDoubleSpinBox* m_sectorCapSpin;
    QSpinBox* m_diversifyMainlineSpin;
    QDoubleSpinBox* m_surgeTakeProfitSpin;
    QCheckBox* m_partialTakeProfitCheck;
    QCheckBox* m_breakMA60VolumeStopCheck;
    QWidget* m_symbolSelectorWidget;
    QWidget* m_personalSidebarWidget;
    QPushButton* m_symbolAddFavoriteBtn;
    QGroupBox* m_strategyControlGroup;
    QGroupBox* m_watchlistGroup;
    QGroupBox* m_manualTradeGroup;
    QListWidget* m_watchlistWidget;
    QPushButton* m_addFavoriteBtn;
    QPushButton* m_removeFavoriteBtn;
    QDoubleSpinBox* m_manualPriceSpin;
    QDoubleSpinBox* m_manualVolumeSpin;
    QPushButton* m_buyFavoriteBtn;
    QPushButton* m_sellFavoriteBtn;
    QMap<QString, QString> m_stockNames;
    QMap<QString, QString> m_completionSymbols;
    QStringList m_favorites;
    bool m_updatingSymbol;
    bool m_loadingSettings;
};