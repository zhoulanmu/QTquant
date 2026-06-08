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
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MarketPanel
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
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
        MarketPanel->resize(300, 180);
        MarketPanel->setMinimumSize(QSize(0, 150));
        verticalLayout_2 = new QVBoxLayout(MarketPanel);
        verticalLayout_2->setObjectName("verticalLayout_2");
        groupBox = new QGroupBox(MarketPanel);
        groupBox->setObjectName("groupBox");
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName("formLayout");
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label = new QLabel(groupBox);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        symbolLabel = new QLabel(groupBox);
        symbolLabel->setObjectName("symbolLabel");

        formLayout->setWidget(0, QFormLayout::FieldRole, symbolLabel);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        formLayout->setWidget(0, QFormLayout::FieldRole, label_2);

        timeLabel = new QLabel(groupBox);
        timeLabel->setObjectName("timeLabel");

        formLayout->setWidget(0, QFormLayout::FieldRole, timeLabel);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");

        formLayout->setWidget(1, QFormLayout::LabelRole, label_3);

        openLabel = new QLabel(groupBox);
        openLabel->setObjectName("openLabel");

        formLayout->setWidget(1, QFormLayout::FieldRole, openLabel);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName("label_4");

        formLayout->setWidget(1, QFormLayout::FieldRole, label_4);

        closeLabel = new QLabel(groupBox);
        closeLabel->setObjectName("closeLabel");

        formLayout->setWidget(1, QFormLayout::FieldRole, closeLabel);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        formLayout->setWidget(2, QFormLayout::LabelRole, label_5);

        highLabel = new QLabel(groupBox);
        highLabel->setObjectName("highLabel");

        formLayout->setWidget(2, QFormLayout::FieldRole, highLabel);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        formLayout->setWidget(2, QFormLayout::FieldRole, label_6);

        lowLabel = new QLabel(groupBox);
        lowLabel->setObjectName("lowLabel");

        formLayout->setWidget(2, QFormLayout::FieldRole, lowLabel);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName("label_7");

        formLayout->setWidget(3, QFormLayout::LabelRole, label_7);

        volumeLabel = new QLabel(groupBox);
        volumeLabel->setObjectName("volumeLabel");

        formLayout->setWidget(3, QFormLayout::FieldRole, volumeLabel);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName("label_8");

        formLayout->setWidget(3, QFormLayout::FieldRole, label_8);

        turnoverLabel = new QLabel(groupBox);
        turnoverLabel->setObjectName("turnoverLabel");

        formLayout->setWidget(3, QFormLayout::FieldRole, turnoverLabel);


        verticalLayout_2->addWidget(groupBox);


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
        label_8->setText(QCoreApplication::translate("MarketPanel", "\346\210\220\344\272\244\351\242\235(\344\270\207):", nullptr));
        turnoverLabel->setText(QCoreApplication::translate("MarketPanel", "--", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MarketPanel: public Ui_MarketPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MARKETPANEL_H
