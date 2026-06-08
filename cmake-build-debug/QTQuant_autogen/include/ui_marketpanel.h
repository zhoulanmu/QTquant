/********************************************************************************
** Form generated from reading UI file 'marketpanel.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MARKETPANEL_H
#define UI_MARKETPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MarketPanel
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *symbolLabel;
    QLabel *label_2;
    QLabel *timeLabel;
    QLabel *label_3;
    QLabel *openLabel;
    QLabel *label_4;
    QLabel *closeLabel;
    QLabel *label_5;
    QLabel *highLabel;
    QLabel *label_6;
    QLabel *lowLabel;
    QLabel *label_7;
    QLabel *volumeLabel;
    QLabel *label_8;
    QLabel *turnoverLabel;

    void setupUi(QWidget *MarketPanel)
    {
        if (MarketPanel->objectName().isEmpty())
            MarketPanel->setObjectName("MarketPanel");
        MarketPanel->resize(280, 140);
        verticalLayout = new QVBoxLayout(MarketPanel);
        verticalLayout->setObjectName("verticalLayout");
        groupBox = new QGroupBox(MarketPanel);
        groupBox->setObjectName("groupBox");
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setHorizontalSpacing(12);
        gridLayout->setVerticalSpacing(8);
        label = new QLabel(groupBox);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);

        symbolLabel = new QLabel(groupBox);
        symbolLabel->setObjectName("symbolLabel");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(symbolLabel->sizePolicy().hasHeightForWidth());
        symbolLabel->setSizePolicy(sizePolicy);

        gridLayout->addWidget(symbolLabel, 0, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 0, 2, 1, 1);

        timeLabel = new QLabel(groupBox);
        timeLabel->setObjectName("timeLabel");

        gridLayout->addWidget(timeLabel, 0, 3, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        openLabel = new QLabel(groupBox);
        openLabel->setObjectName("openLabel");

        gridLayout->addWidget(openLabel, 1, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 1, 2, 1, 1);

        closeLabel = new QLabel(groupBox);
        closeLabel->setObjectName("closeLabel");

        gridLayout->addWidget(closeLabel, 1, 3, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        highLabel = new QLabel(groupBox);
        highLabel->setObjectName("highLabel");

        gridLayout->addWidget(highLabel, 2, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        gridLayout->addWidget(label_6, 2, 2, 1, 1);

        lowLabel = new QLabel(groupBox);
        lowLabel->setObjectName("lowLabel");

        gridLayout->addWidget(lowLabel, 2, 3, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName("label_7");

        gridLayout->addWidget(label_7, 3, 0, 1, 1);

        volumeLabel = new QLabel(groupBox);
        volumeLabel->setObjectName("volumeLabel");

        gridLayout->addWidget(volumeLabel, 3, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName("label_8");

        gridLayout->addWidget(label_8, 3, 2, 1, 1);

        turnoverLabel = new QLabel(groupBox);
        turnoverLabel->setObjectName("turnoverLabel");

        gridLayout->addWidget(turnoverLabel, 3, 3, 1, 1);


        verticalLayout->addWidget(groupBox);


        retranslateUi(MarketPanel);

        QMetaObject::connectSlotsByName(MarketPanel);
    } // setupUi

    void retranslateUi(QWidget *MarketPanel)
    {
        MarketPanel->setWindowTitle(QCoreApplication::translate("MarketPanel", "Form", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MarketPanel", "\345\256\236\346\227\266\350\241\214\346\203\205", nullptr));
        label->setText(QCoreApplication::translate("MarketPanel", "\344\273\243\347\240\201:", nullptr));
        symbolLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
        label_2->setText(QCoreApplication::translate("MarketPanel", "\346\227\266\351\227\264:", nullptr));
        timeLabel->setText(QCoreApplication::translate("MarketPanel", "--:--:--", nullptr));
        label_3->setText(QCoreApplication::translate("MarketPanel", "\345\274\200\347\233\230:", nullptr));
        openLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
        label_4->setText(QCoreApplication::translate("MarketPanel", "\346\224\266\347\233\230:", nullptr));
        closeLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
        label_5->setText(QCoreApplication::translate("MarketPanel", "\346\234\200\351\253\230:", nullptr));
        highLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
        label_6->setText(QCoreApplication::translate("MarketPanel", "\346\234\200\344\275\216:", nullptr));
        lowLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
        label_7->setText(QCoreApplication::translate("MarketPanel", "\346\210\220\344\272\244\351\207\217:", nullptr));
        volumeLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
        label_8->setText(QCoreApplication::translate("MarketPanel", "\346\210\220\344\272\244\351\242\235:", nullptr));
        turnoverLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MarketPanel: public Ui_MarketPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MARKETPANEL_H
