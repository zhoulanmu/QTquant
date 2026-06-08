/********************************************************************************
** Form generated from reading UI file 'accountpanel.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ACCOUNTPANEL_H
#define UI_ACCOUNTPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AccountPanel
{
public:
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout_2;
    QLabel *label_9;
    QLabel *totalAssetsLabel;
    QLabel *label_10;
    QLabel *availableCashLabel;
    QLabel *label_11;
    QLabel *marketValueLabel;
    QLabel *label_12;
    QLabel *totalProfitLabel;
    QLabel *label_13;
    QLabel *profitPercentLabel;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_4;
    QTableWidget *positionsTable;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_5;
    QTableWidget *tradesTable;

    void setupUi(QWidget *AccountPanel)
    {
        if (AccountPanel->objectName().isEmpty())
            AccountPanel->setObjectName("AccountPanel");
        AccountPanel->resize(300, 250);
        verticalLayout_3 = new QVBoxLayout(AccountPanel);
        verticalLayout_3->setObjectName("verticalLayout_3");
        groupBox_3 = new QGroupBox(AccountPanel);
        groupBox_3->setObjectName("groupBox_3");
        formLayout_2 = new QFormLayout(groupBox_3);
        formLayout_2->setObjectName("formLayout_2");
        label_9 = new QLabel(groupBox_3);
        label_9->setObjectName("label_9");

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_9);

        totalAssetsLabel = new QLabel(groupBox_3);
        totalAssetsLabel->setObjectName("totalAssetsLabel");

        formLayout_2->setWidget(0, QFormLayout::FieldRole, totalAssetsLabel);

        label_10 = new QLabel(groupBox_3);
        label_10->setObjectName("label_10");

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_10);

        availableCashLabel = new QLabel(groupBox_3);
        availableCashLabel->setObjectName("availableCashLabel");

        formLayout_2->setWidget(1, QFormLayout::FieldRole, availableCashLabel);

        label_11 = new QLabel(groupBox_3);
        label_11->setObjectName("label_11");

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_11);

        marketValueLabel = new QLabel(groupBox_3);
        marketValueLabel->setObjectName("marketValueLabel");

        formLayout_2->setWidget(2, QFormLayout::FieldRole, marketValueLabel);

        label_12 = new QLabel(groupBox_3);
        label_12->setObjectName("label_12");

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_12);

        totalProfitLabel = new QLabel(groupBox_3);
        totalProfitLabel->setObjectName("totalProfitLabel");

        formLayout_2->setWidget(3, QFormLayout::FieldRole, totalProfitLabel);

        label_13 = new QLabel(groupBox_3);
        label_13->setObjectName("label_13");

        formLayout_2->setWidget(4, QFormLayout::LabelRole, label_13);

        profitPercentLabel = new QLabel(groupBox_3);
        profitPercentLabel->setObjectName("profitPercentLabel");

        formLayout_2->setWidget(4, QFormLayout::FieldRole, profitPercentLabel);


        verticalLayout_3->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(AccountPanel);
        groupBox_4->setObjectName("groupBox_4");
        verticalLayout_4 = new QVBoxLayout(groupBox_4);
        verticalLayout_4->setObjectName("verticalLayout_4");
        positionsTable = new QTableWidget(groupBox_4);
        positionsTable->setObjectName("positionsTable");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(positionsTable->sizePolicy().hasHeightForWidth());
        positionsTable->setSizePolicy(sizePolicy);
        positionsTable->setMinimumSize(QSize(0, 80));

        verticalLayout_4->addWidget(positionsTable);


        verticalLayout_3->addWidget(groupBox_4);

        groupBox_5 = new QGroupBox(AccountPanel);
        groupBox_5->setObjectName("groupBox_5");
        verticalLayout_5 = new QVBoxLayout(groupBox_5);
        verticalLayout_5->setObjectName("verticalLayout_5");
        tradesTable = new QTableWidget(groupBox_5);
        tradesTable->setObjectName("tradesTable");
        sizePolicy.setHeightForWidth(tradesTable->sizePolicy().hasHeightForWidth());
        tradesTable->setSizePolicy(sizePolicy);
        tradesTable->setMinimumSize(QSize(0, 60));

        verticalLayout_5->addWidget(tradesTable);


        verticalLayout_3->addWidget(groupBox_5);


        retranslateUi(AccountPanel);

        QMetaObject::connectSlotsByName(AccountPanel);
    } // setupUi

    void retranslateUi(QWidget *AccountPanel)
    {
        AccountPanel->setWindowTitle(QCoreApplication::translate("AccountPanel", "Form", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("AccountPanel", "\350\264\246\346\210\267\350\265\204\344\272\247", nullptr));
        label_9->setText(QCoreApplication::translate("AccountPanel", "\346\200\273\350\265\204\344\272\247:", nullptr));
        totalAssetsLabel->setText(QCoreApplication::translate("AccountPanel", "100000.00", nullptr));
        label_10->setText(QCoreApplication::translate("AccountPanel", "\345\217\257\347\224\250\350\265\204\351\207\221:", nullptr));
        availableCashLabel->setText(QCoreApplication::translate("AccountPanel", "100000.00", nullptr));
        label_11->setText(QCoreApplication::translate("AccountPanel", "\346\214\201\344\273\223\345\270\202\345\200\274:", nullptr));
        marketValueLabel->setText(QCoreApplication::translate("AccountPanel", "0.00", nullptr));
        label_12->setText(QCoreApplication::translate("AccountPanel", "\346\200\273\347\233\210\344\272\217:", nullptr));
        totalProfitLabel->setText(QCoreApplication::translate("AccountPanel", "0.00", nullptr));
        label_13->setText(QCoreApplication::translate("AccountPanel", "\347\233\210\344\272\217\346\257\224:", nullptr));
        profitPercentLabel->setText(QCoreApplication::translate("AccountPanel", "0.00%", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("AccountPanel", "\346\214\201\344\273\223\346\230\216\347\273\206", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("AccountPanel", "\346\210\220\344\272\244\350\256\260\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AccountPanel: public Ui_AccountPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ACCOUNTPANEL_H
