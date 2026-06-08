/********************************************************************************
** Form generated from reading UI file 'strategypanel.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STRATEGYPANEL_H
#define UI_STRATEGYPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StrategyPanel
{
public:
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *symbolEdit;
    QLabel *label_2;
    QSpinBox *fastMASpin;
    QLabel *label_3;
    QSpinBox *slowMASpin;
    QLabel *label_4;
    QDoubleSpinBox *stopLossSpin;
    QLabel *label_5;
    QDoubleSpinBox *takeProfitSpin;
    QLabel *label_6;
    QDoubleSpinBox *lotSizeSpin;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QTextEdit *logTextEdit;

    void setupUi(QWidget *StrategyPanel)
    {
        if (StrategyPanel->objectName().isEmpty())
            StrategyPanel->setObjectName("StrategyPanel");
        StrategyPanel->resize(300, 400);
        verticalLayout_4 = new QVBoxLayout(StrategyPanel);
        verticalLayout_4->setObjectName("verticalLayout_4");
        groupBox = new QGroupBox(StrategyPanel);
        groupBox->setObjectName("groupBox");
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName("formLayout");
        label = new QLabel(groupBox);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        symbolEdit = new QLineEdit(groupBox);
        symbolEdit->setObjectName("symbolEdit");

        formLayout->setWidget(0, QFormLayout::FieldRole, symbolEdit);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        fastMASpin = new QSpinBox(groupBox);
        fastMASpin->setObjectName("fastMASpin");
        fastMASpin->setMinimum(1);
        fastMASpin->setMaximum(50);
        fastMASpin->setValue(5);

        formLayout->setWidget(1, QFormLayout::FieldRole, fastMASpin);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");

        formLayout->setWidget(2, QFormLayout::LabelRole, label_3);

        slowMASpin = new QSpinBox(groupBox);
        slowMASpin->setObjectName("slowMASpin");
        slowMASpin->setMinimum(1);
        slowMASpin->setMaximum(200);
        slowMASpin->setValue(20);

        formLayout->setWidget(2, QFormLayout::FieldRole, slowMASpin);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName("label_4");

        formLayout->setWidget(3, QFormLayout::LabelRole, label_4);

        stopLossSpin = new QDoubleSpinBox(groupBox);
        stopLossSpin->setObjectName("stopLossSpin");
        stopLossSpin->setMinimum(0);
        stopLossSpin->setMaximum(20);
        stopLossSpin->setSingleStep(0);
        stopLossSpin->setValue(2);

        formLayout->setWidget(3, QFormLayout::FieldRole, stopLossSpin);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        formLayout->setWidget(4, QFormLayout::LabelRole, label_5);

        takeProfitSpin = new QDoubleSpinBox(groupBox);
        takeProfitSpin->setObjectName("takeProfitSpin");
        takeProfitSpin->setMinimum(0);
        takeProfitSpin->setMaximum(50);
        takeProfitSpin->setSingleStep(0);
        takeProfitSpin->setValue(5);

        formLayout->setWidget(4, QFormLayout::FieldRole, takeProfitSpin);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        formLayout->setWidget(5, QFormLayout::LabelRole, label_6);

        lotSizeSpin = new QDoubleSpinBox(groupBox);
        lotSizeSpin->setObjectName("lotSizeSpin");
        lotSizeSpin->setMinimum(100);
        lotSizeSpin->setMaximum(100000);
        lotSizeSpin->setSingleStep(100);
        lotSizeSpin->setValue(1000);

        formLayout->setWidget(5, QFormLayout::FieldRole, lotSizeSpin);


        verticalLayout_4->addWidget(groupBox);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        startBtn = new QPushButton(StrategyPanel);
        startBtn->setObjectName("startBtn");

        horizontalLayout_2->addWidget(startBtn);

        stopBtn = new QPushButton(StrategyPanel);
        stopBtn->setObjectName("stopBtn");

        horizontalLayout_2->addWidget(stopBtn);


        verticalLayout_4->addLayout(horizontalLayout_2);

        groupBox_2 = new QGroupBox(StrategyPanel);
        groupBox_2->setObjectName("groupBox_2");
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        logTextEdit = new QTextEdit(groupBox_2);
        logTextEdit->setObjectName("logTextEdit");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(logTextEdit->sizePolicy().hasHeightForWidth());
        logTextEdit->setSizePolicy(sizePolicy);
        logTextEdit->setMinimumSize(QSize(0, 150));

        verticalLayout_3->addWidget(logTextEdit);


        verticalLayout_4->addWidget(groupBox_2);


        retranslateUi(StrategyPanel);

        QMetaObject::connectSlotsByName(StrategyPanel);
    } // setupUi

    void retranslateUi(QWidget *StrategyPanel)
    {
        StrategyPanel->setWindowTitle(QCoreApplication::translate("StrategyPanel", "Form", nullptr));
        groupBox->setTitle(QCoreApplication::translate("StrategyPanel", "\347\255\226\347\225\245\345\217\202\346\225\260", nullptr));
        label->setText(QCoreApplication::translate("StrategyPanel", "\350\202\241\347\245\250\344\273\243\347\240\201", nullptr));
        symbolEdit->setText(QCoreApplication::translate("StrategyPanel", "000001.SH", nullptr));
        label_2->setText(QCoreApplication::translate("StrategyPanel", "\345\277\253\351\200\237MA\345\221\250\346\234\237", nullptr));
        label_3->setText(QCoreApplication::translate("StrategyPanel", "\346\205\242\351\200\237MA\345\221\250\346\234\237", nullptr));
        label_4->setText(QCoreApplication::translate("StrategyPanel", "\346\255\242\346\215\237(%)", nullptr));
        label_5->setText(QCoreApplication::translate("StrategyPanel", "\346\255\242\347\233\210(%)", nullptr));
        label_6->setText(QCoreApplication::translate("StrategyPanel", "\346\214\201\344\273\223\346\225\260\351\207\217", nullptr));
        startBtn->setText(QCoreApplication::translate("StrategyPanel", "\345\220\257\345\212\250\347\255\226\347\225\245", nullptr));
        stopBtn->setText(QCoreApplication::translate("StrategyPanel", "\345\201\234\346\255\242\347\255\226\347\225\245", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("StrategyPanel", "\344\272\244\346\230\223\346\227\245\345\277\227", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StrategyPanel: public Ui_StrategyPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STRATEGYPANEL_H
