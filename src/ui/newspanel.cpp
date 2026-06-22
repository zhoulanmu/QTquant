#include "newspanel.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextBrowser>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QVBoxLayout>

namespace {
constexpr int NewsTimeoutMs = 8000;
constexpr int AutoRefreshMs = 120000;
constexpr auto EastMoneyEndpoint = "https://np-listapi.eastmoney.com/comm/web/getNewsByColumns";
constexpr auto WallStreetCnEndpoint = "https://api-one-wscn.awtmt.com/apiv1/content/lives";

QString valueToString(const QJsonObject& object, const QString& key)
{
    const QJsonValue value = object.value(key);
    if (value.isString()) {
        return value.toString().trimmed();
    }
    if (value.isDouble()) {
        return QString::number(static_cast<qint64>(value.toDouble()));
    }
    return QString();
}

QDateTime parseDateTime(const QString& value)
{
    QString text = value.trimmed();
    if (text.isEmpty()) {
        return QDateTime();
    }

    bool ok = false;
    const qint64 numeric = text.toLongLong(&ok);
    if (ok) {
        if (text.size() >= 13) {
            return QDateTime::fromMSecsSinceEpoch(numeric).toLocalTime();
        }
        return QDateTime::fromSecsSinceEpoch(numeric).toLocalTime();
    }

    QDateTime time = QDateTime::fromString(text, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (time.isValid()) {
        return time;
    }
    time = QDateTime::fromString(text, QStringLiteral("yyyy-MM-dd HH:mm"));
    if (time.isValid()) {
        return time;
    }
    return QDateTime::fromString(text, Qt::ISODate).toLocalTime();
}
}

NewsPanel::NewsPanel(QWidget* parent)
    : QWidget(parent)
    , m_sourceCombo(nullptr)
    , m_refreshButton(nullptr)
    , m_statusLabel(nullptr)
    , m_newsTable(nullptr)
    , m_detailBrowser(nullptr)
    , m_network(new QNetworkAccessManager(this))
    , m_reply(nullptr)
    , m_refreshTimer(new QTimer(this))
{
    setupUi();

    m_refreshTimer->setInterval(AutoRefreshMs);
    connect(m_refreshTimer, &QTimer::timeout, this, &NewsPanel::refreshNews);
    m_refreshTimer->start();

    refreshNews();
}

void NewsPanel::setupUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(12);

    auto* newsGroup = new QGroupBox(QStringLiteral("市场新闻"), this);
    auto* newsLayout = new QVBoxLayout(newsGroup);
    newsLayout->setContentsMargins(10, 12, 10, 10);
    newsLayout->setSpacing(8);

    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(8);

    m_sourceCombo = new QComboBox(newsGroup);
    m_sourceCombo->addItem(QStringLiteral("东方财富财经"), static_cast<int>(NewsSource::EastMoney));
    m_sourceCombo->addItem(QStringLiteral("华尔街见闻快讯"), static_cast<int>(NewsSource::WallStreetCn));
    m_refreshButton = new QPushButton(QStringLiteral("刷新"), newsGroup);
    m_statusLabel = new QLabel(QStringLiteral("--"), newsGroup);
    m_statusLabel->setMinimumWidth(260);

    toolbarLayout->addWidget(m_sourceCombo);
    toolbarLayout->addWidget(m_refreshButton);
    toolbarLayout->addWidget(m_statusLabel, 1);
    newsLayout->addLayout(toolbarLayout);

    m_newsTable = new QTableWidget(newsGroup);
    m_newsTable->setColumnCount(4);
    m_newsTable->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("来源"), QStringLiteral("标题"), QStringLiteral("摘要")});
    m_newsTable->horizontalHeader()->setStretchLastSection(true);
    m_newsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_newsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_newsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_newsTable->verticalHeader()->setVisible(false);
    m_newsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_newsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_newsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_newsTable->setAlternatingRowColors(false);
    newsLayout->addWidget(m_newsTable, 3);

    auto* detailGroup = new QGroupBox(QStringLiteral("新闻详情"), this);
    auto* detailLayout = new QVBoxLayout(detailGroup);
    detailLayout->setContentsMargins(10, 12, 10, 10);
    m_detailBrowser = new QTextBrowser(detailGroup);
    m_detailBrowser->setOpenExternalLinks(true);
    detailLayout->addWidget(m_detailBrowser);

    rootLayout->addWidget(newsGroup, 3);
    rootLayout->addWidget(detailGroup, 2);

    connect(m_refreshButton, &QPushButton::clicked, this, &NewsPanel::onRefreshClicked);
    connect(m_sourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewsPanel::onSourceChanged);
    connect(m_newsTable, &QTableWidget::itemSelectionChanged, this, &NewsPanel::onSelectionChanged);
}

void NewsPanel::refreshNews()
{
    const NewsSource source = static_cast<NewsSource>(m_sourceCombo->currentData().toInt());
    if (source == NewsSource::WallStreetCn) {
        requestWallStreetCnNews();
        return;
    }
    requestEastMoneyNews();
}

void NewsPanel::requestEastMoneyNews()
{
    if (m_reply) {
        disconnect(m_reply, nullptr, this, nullptr);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QUrl url(QString::fromLatin1(EastMoneyEndpoint));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("client"), QStringLiteral("web"));
    query.addQueryItem(QStringLiteral("biz"), QStringLiteral("web_news_col"));
    query.addQueryItem(QStringLiteral("column"), QStringLiteral("345"));
    query.addQueryItem(QStringLiteral("order"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("needInteractData"), QStringLiteral("0"));
    query.addQueryItem(QStringLiteral("page_index"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("page_size"), QStringLiteral("30"));
    query.addQueryItem(QStringLiteral("req_trace"), QUuid::createUuid().toString(QUuid::WithoutBraces));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0 StarQuant/0.1 News"));
    request.setRawHeader("Referer", "https://finance.eastmoney.com/");
    request.setTransferTimeout(NewsTimeoutMs);

    setLoading(true);
    m_reply = m_network->get(request);
    connect(m_reply, &QNetworkReply::finished, this, &NewsPanel::onNewsReplyFinished);
}

void NewsPanel::requestWallStreetCnNews()
{
    if (m_reply) {
        disconnect(m_reply, nullptr, this, nullptr);
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QUrl url(QString::fromLatin1(WallStreetCnEndpoint));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("channel"), QStringLiteral("global-channel"));
    query.addQueryItem(QStringLiteral("client"), QStringLiteral("pc"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("30"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Mozilla/5.0 StarQuant/0.1 News"));
    request.setRawHeader("Referer", "https://wallstreetcn.com/");
    request.setTransferTimeout(NewsTimeoutMs);

    setLoading(true);
    m_reply = m_network->get(request);
    connect(m_reply, &QNetworkReply::finished, this, &NewsPanel::onNewsReplyFinished);
}

void NewsPanel::onNewsReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply == m_reply) {
        m_reply = nullptr;
    }

    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorText = reply->errorString();
    const QByteArray payload = reply->readAll();
    reply->deleteLater();
    setLoading(false);

    if (networkError != QNetworkReply::NoError) {
        m_statusLabel->setText(QStringLiteral("新闻获取失败：%1").arg(networkErrorText));
        return;
    }

    QString errorMessage;
    QList<NewsItem> items;
    const NewsSource source = static_cast<NewsSource>(m_sourceCombo->currentData().toInt());
    if (source == NewsSource::WallStreetCn) {
        items = parseWallStreetCnNews(payload, &errorMessage);
    } else {
        items = parseEastMoneyNews(payload, &errorMessage);
    }

    if (items.isEmpty()) {
        m_statusLabel->setText(errorMessage.isEmpty() ? QStringLiteral("新闻接口暂无数据") : errorMessage);
        return;
    }

    updateNewsList(items);
    m_statusLabel->setText(QStringLiteral("已更新 %1 条 · %2").arg(items.size()).arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))));
}

QList<NewsItem> NewsPanel::parseEastMoneyNews(const QByteArray& payload, QString* errorMessage) const
{
    QList<NewsItem> items;
    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("东方财富新闻返回无效 JSON：%1").arg(jsonError.errorString());
        }
        return items;
    }

    const QJsonObject root = document.object();
    const QString code = valueToString(root, QStringLiteral("code"));
    if (code != QStringLiteral("1")) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("东方财富新闻接口返回：%1").arg(valueToString(root, QStringLiteral("message")));
        }
        return items;
    }

    const QJsonObject data = root.value(QStringLiteral("data")).toObject();
    const QJsonArray list = data.value(QStringLiteral("list")).toArray();
    for (const QJsonValue& value : list) {
        const QJsonObject object = value.toObject();
        NewsItem item;
        item.title = valueToString(object, QStringLiteral("title"));
        item.source = valueToString(object, QStringLiteral("mediaName"));
        item.summary = valueToString(object, QStringLiteral("summary"));
        item.url = valueToString(object, QStringLiteral("uniqueUrl"));
        if (item.url.isEmpty()) {
            item.url = valueToString(object, QStringLiteral("url"));
        }
        item.time = parseDateTime(valueToString(object, QStringLiteral("showTime")));
        if (item.title.isEmpty()) {
            continue;
        }
        if (item.source.isEmpty()) {
            item.source = QStringLiteral("东方财富");
        }
        items.append(item);
    }

    return items;
}

QList<NewsItem> NewsPanel::parseWallStreetCnNews(const QByteArray& payload, QString* errorMessage) const
{
    QList<NewsItem> items;
    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("华尔街见闻返回无效 JSON：%1").arg(jsonError.errorString());
        }
        return items;
    }

    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("code")).toInt() != 20000) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("华尔街见闻接口返回：%1").arg(valueToString(root, QStringLiteral("message")));
        }
        return items;
    }

    const QJsonObject data = root.value(QStringLiteral("data")).toObject();
    const QJsonArray list = data.value(QStringLiteral("items")).toArray();
    for (const QJsonValue& value : list) {
        const QJsonObject object = value.toObject();
        NewsItem item;
        item.summary = valueToString(object, QStringLiteral("content_text"));
        if (item.summary.isEmpty()) {
            item.summary = htmlToPlainText(valueToString(object, QStringLiteral("content")));
        }
        item.title = item.summary.left(42);
        if (item.summary.size() > 42) {
            item.title += QStringLiteral("...");
        }
        const QJsonObject author = object.value(QStringLiteral("author")).toObject();
        item.source = valueToString(author, QStringLiteral("display_name"));
        if (item.source.isEmpty()) {
            item.source = QStringLiteral("华尔街见闻");
        }
        item.url = valueToString(object, QStringLiteral("uri"));
        item.time = parseDateTime(valueToString(object, QStringLiteral("display_time")));
        if (!item.time.isValid()) {
            item.time = parseDateTime(valueToString(object, QStringLiteral("created_at")));
        }
        if (item.summary.isEmpty()) {
            continue;
        }
        items.append(item);
    }

    return items;
}

void NewsPanel::updateNewsList(const QList<NewsItem>& items)
{
    m_items = items;
    m_newsTable->setRowCount(m_items.size());

    for (int row = 0; row < m_items.size(); ++row) {
        const NewsItem& item = m_items.at(row);
        m_newsTable->setItem(row, 0, new QTableWidgetItem(formatNewsTime(item.time)));
        m_newsTable->setItem(row, 1, new QTableWidgetItem(item.source));
        m_newsTable->setItem(row, 2, new QTableWidgetItem(item.title));
        m_newsTable->setItem(row, 3, new QTableWidgetItem(item.summary));
    }

    if (!m_items.isEmpty()) {
        m_newsTable->selectRow(0);
        updateDetail(0);
    } else {
        m_detailBrowser->clear();
    }
}

void NewsPanel::updateDetail(int row)
{
    if (row < 0 || row >= m_items.size()) {
        m_detailBrowser->clear();
        return;
    }

    const NewsItem& item = m_items.at(row);
    QString html;
    html += QStringLiteral("<h3>%1</h3>").arg(item.title.toHtmlEscaped());
    html += QStringLiteral("<p style='color:#8fb3d9;'>%1 · %2</p>")
        .arg(formatNewsTime(item.time).toHtmlEscaped(), item.source.toHtmlEscaped());
    html += QStringLiteral("<p>%1</p>").arg(item.summary.toHtmlEscaped().replace('\n', QStringLiteral("<br>")));
    if (!item.url.isEmpty()) {
        html += QStringLiteral("<p><a href='%1'>打开原文</a></p>").arg(item.url.toHtmlEscaped());
    }
    m_detailBrowser->setHtml(html);
}

void NewsPanel::onRefreshClicked()
{
    refreshNews();
}

void NewsPanel::onSourceChanged(int)
{
    refreshNews();
}

void NewsPanel::onSelectionChanged()
{
    const QList<QTableWidgetItem*> selected = m_newsTable->selectedItems();
    if (selected.isEmpty()) {
        return;
    }
    updateDetail(selected.first()->row());
}

void NewsPanel::setLoading(bool loading)
{
    m_refreshButton->setEnabled(!loading);
    m_sourceCombo->setEnabled(!loading);
    m_statusLabel->setText(loading ? QStringLiteral("正在获取新闻...") : QStringLiteral("--"));
}

QString NewsPanel::htmlToPlainText(const QString& html)
{
    QString text = html;
    text.replace(QRegularExpression(QStringLiteral("<br\\s*/?>"), QRegularExpression::CaseInsensitiveOption), QStringLiteral("\n"));
    text.remove(QRegularExpression(QStringLiteral("<[^>]+>")));
    text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    return text.trimmed();
}

QString NewsPanel::formatNewsTime(const QDateTime& time)
{
    if (!time.isValid()) {
        return QStringLiteral("--:--");
    }

    const QDate today = QDate::currentDate();
    if (time.date() == today) {
        return time.toString(QStringLiteral("HH:mm:ss"));
    }
    return time.toString(QStringLiteral("MM-dd HH:mm"));
}