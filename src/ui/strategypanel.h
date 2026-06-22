#pragma once

#include <QMap>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include "../strategy/strategybase.h"

class QCompleter;
class QListWidget;
class QListWidgetItem;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
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
    void rememberStockName(const QString& symbol, const QString& name);
    void addSystemLog(const QString& message);
    void addSignalLog(const StrategySignal& signal);

private slots:
    void on_paramChanged();
    void onSymbolTextChanged(const QString& text);
    void onSymbolEditingFinished();
    void onSearchTimerTimeout();
    void onSearchReplyFinished();
    void onCompleterActivated(const QString& text);
    void onAddFavoriteClicked();
    void onRemoveFavoriteClicked();
    void onFavoriteActivated(QListWidgetItem* item);
    void onFavoriteSelectionChanged();

private:
    void setupStockSearchUi();
    void loadWatchlist();
    void saveWatchlist() const;
    void refreshWatchlist();
    void selectSymbol(const QString& symbol, const QString& name, bool emitChange);
    void addFavoriteSymbol(const QString& symbol, const QString& name);
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
    QStringListModel* m_searchModel;
    QTimer* m_searchTimer;
    QNetworkAccessManager* m_searchNetwork;
    QNetworkReply* m_searchReply;
    QListWidget* m_watchlistWidget;
    QPushButton* m_addFavoriteBtn;
    QPushButton* m_removeFavoriteBtn;
    QMap<QString, QString> m_stockNames;
    QMap<QString, QString> m_completionSymbols;
    QStringList m_favorites;
    bool m_updatingSymbol;
};
