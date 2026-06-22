#pragma once

#include <QDateTime>
#include <QList>
#include <QWidget>

class QComboBox;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;
class QTableWidget;
class QTextBrowser;
class QTimer;

struct NewsItem
{
    QString title;
    QString source;
    QString summary;
    QString url;
    QDateTime time;
};

class NewsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NewsPanel(QWidget* parent = nullptr);
    ~NewsPanel() override = default;

public slots:
    void refreshNews();

private slots:
    void onNewsReplyFinished();
    void onRefreshClicked();
    void onSourceChanged(int index);
    void onSelectionChanged();

private:
    enum class NewsSource {
        EastMoney = 0,
        WallStreetCn = 1
    };

    void setupUi();
    void setLoading(bool loading);
    void requestEastMoneyNews();
    void requestWallStreetCnNews();
    QList<NewsItem> parseEastMoneyNews(const QByteArray& payload, QString* errorMessage) const;
    QList<NewsItem> parseWallStreetCnNews(const QByteArray& payload, QString* errorMessage) const;
    void updateNewsList(const QList<NewsItem>& items);
    void updateDetail(int row);
    static QString htmlToPlainText(const QString& html);
    static QString formatNewsTime(const QDateTime& time);

private:
    QComboBox* m_sourceCombo;
    QPushButton* m_refreshButton;
    QLabel* m_statusLabel;
    QTableWidget* m_newsTable;
    QTextBrowser* m_detailBrowser;
    QNetworkAccessManager* m_network;
    QNetworkReply* m_reply;
    QTimer* m_refreshTimer;
    QList<NewsItem> m_items;
};