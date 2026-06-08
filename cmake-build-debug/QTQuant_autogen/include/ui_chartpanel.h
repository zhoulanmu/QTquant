/********************************************************************************
** Form generated from reading UI file 'chartpanel.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHARTPANEL_H
#define UI_CHARTPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChartPanel
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QWidget *chartWidget;

    void setupUi(QWidget *ChartPanel)
    {
        if (ChartPanel->objectName().isEmpty())
            ChartPanel->setObjectName("ChartPanel");
        ChartPanel->resize(600, 400);
        verticalLayout = new QVBoxLayout(ChartPanel);
        verticalLayout->setObjectName("verticalLayout");
        label = new QLabel(ChartPanel);
        label->setObjectName("label");
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        chartWidget = new QWidget(ChartPanel);
        chartWidget->setObjectName("chartWidget");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(chartWidget->sizePolicy().hasHeightForWidth());
        chartWidget->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(chartWidget);


        retranslateUi(ChartPanel);

        QMetaObject::connectSlotsByName(ChartPanel);
    } // setupUi

    void retranslateUi(QWidget *ChartPanel)
    {
        ChartPanel->setWindowTitle(QCoreApplication::translate("ChartPanel", "Form", nullptr));
        label->setText(QCoreApplication::translate("ChartPanel", "\344\273\267\346\240\274\350\265\260\345\212\277\345\233\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChartPanel: public Ui_ChartPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHARTPANEL_H
