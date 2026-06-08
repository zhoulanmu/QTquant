/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "ui/chartpanel.h"
#include "ui/marketpanel.h"
#include "ui/strategypanel.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QFrame *frame;
    QVBoxLayout *verticalLayout_3;
    StrategyPanel *strategyPanel;
    MarketPanel *marketPanel;
    QWidget *accountPanel;
    QVBoxLayout *verticalLayout_4;
    ChartPanel *chartPanel;
    QHBoxLayout *horizontalLayout_2;
    QWidget *statisticsPanel;
    QWidget *signalPanel;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName("horizontalLayout");
        frame = new QFrame(centralwidget);
        frame->setObjectName("frame");
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_3 = new QVBoxLayout(frame);
        verticalLayout_3->setObjectName("verticalLayout_3");
        strategyPanel = new StrategyPanel(frame);
        strategyPanel->setObjectName("strategyPanel");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(strategyPanel->sizePolicy().hasHeightForWidth());
        strategyPanel->setSizePolicy(sizePolicy);

        verticalLayout_3->addWidget(strategyPanel);

        marketPanel = new MarketPanel(frame);
        marketPanel->setObjectName("marketPanel");

        verticalLayout_3->addWidget(marketPanel);

        accountPanel = new QWidget(frame);
        accountPanel->setObjectName("accountPanel");
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(accountPanel->sizePolicy().hasHeightForWidth());
        accountPanel->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(accountPanel);


        horizontalLayout->addWidget(frame);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName("verticalLayout_4");
        chartPanel = new ChartPanel(centralwidget);
        chartPanel->setObjectName("chartPanel");
        sizePolicy1.setHeightForWidth(chartPanel->sizePolicy().hasHeightForWidth());
        chartPanel->setSizePolicy(sizePolicy1);

        verticalLayout_4->addWidget(chartPanel);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        statisticsPanel = new QWidget(centralwidget);
        statisticsPanel->setObjectName("statisticsPanel");
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(statisticsPanel->sizePolicy().hasHeightForWidth());
        statisticsPanel->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(statisticsPanel);

        signalPanel = new QWidget(centralwidget);
        signalPanel->setObjectName("signalPanel");
        sizePolicy2.setHeightForWidth(signalPanel->sizePolicy().hasHeightForWidth());
        signalPanel->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(signalPanel);


        verticalLayout_4->addLayout(horizontalLayout_2);


        horizontalLayout->addLayout(verticalLayout_4);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "QTQuant", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
