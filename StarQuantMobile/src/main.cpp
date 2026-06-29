#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QLocale>
#include <QTranslator>
#include <QQmlContext>

#include "market/marketdata.h"
#include "strategy/strategybase.h"
#include "strategy/movingaveragestrategy.h"
#include "strategy/prosperitygrowthstrategy.h"
#include "screener/stockscreener.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("星策 StarQuant"));
    app.setOrganizationName(QStringLiteral("StarQuant"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    QLocale::setDefault(QLocale(QLocale::Chinese, QLocale::China));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "StarQuantMobile_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    qRegisterMetaType<MarketData>("MarketData");
    qRegisterMetaType<KLineData>("KLineData");
    qRegisterMetaType<QVector<KLineData>>("QVector<KLineData>");
    qRegisterMetaType<TrendPoint>("TrendPoint");
    qRegisterMetaType<QVector<TrendPoint>>("QVector<TrendPoint>");
    qRegisterMetaType<StockInfo>("StockInfo");
    qRegisterMetaType<QVector<StockInfo>>("QVector<StockInfo>");

    qmlRegisterType<MarketDataSimulator>("StarQuant", 1, 0, "MarketDataSimulator");
    qmlRegisterType<MovingAverageStrategy>("StarQuant", 1, 0, "MovingAverageStrategy");
    qmlRegisterType<ProsperityGrowthStrategy>("StarQuant", 1, 0, "ProsperityGrowthStrategy");
    qmlRegisterType<StockScreener>("StarQuant", 1, 0, "StockScreener");

    QQmlApplicationEngine engine;

    StockScreener* screener = new StockScreener(&app);
    engine.rootContext()->setContextProperty("stockScreener", screener);

    const QUrl url(u"qrc:/StarQuant/qml/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
