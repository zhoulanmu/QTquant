#include "logindialog.h"
#include "ui_logindialog.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_guestButton(nullptr)
    , m_authenticated(false)
    , m_guestMode(true)
{
    ui->setupUi(this);

    setMinimumSize(480, 360);
    setWindowTitle(QStringLiteral("星策 StarQuant - 登录"));

    ui->verticalLayout->setSpacing(18);
    ui->verticalLayout->setContentsMargins(35, 35, 35, 35);
    ui->inputLayout->setSpacing(12);
    ui->horizontalLayout->setSpacing(12);

    ui->titleLabel->setText(QStringLiteral("星策 StarQuant"));
    ui->usernameEdit->setPlaceholderText(QStringLiteral("客户端用户名"));
    ui->passwordEdit->setPlaceholderText(QStringLiteral("客户端密码"));
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->passwordEdit->show();
    ui->loginBtn->setText(QStringLiteral("客户端登录"));
    ui->cancelBtn->setText(QStringLiteral("取消"));
    ui->tipLabel->setText(QStringLiteral("请输入客户端用户名和密码，或使用游客看盘直接进入。"));
    ui->tipLabel->setWordWrap(true);

    m_guestButton = new QPushButton(QStringLiteral("游客看盘"), this);
    m_guestButton->setObjectName(QStringLiteral("guestBtn"));
    ui->horizontalLayout->insertWidget(1, m_guestButton);

    connect(m_guestButton, &QPushButton::clicked, this, &LoginDialog::acceptGuest);

    this->setStyleSheet(R"(
        QDialog {
            background-color: #253b6e;
            border-radius: 8px;
        }
        QLineEdit {
            background-color: #4a5568;
            color: white;
            border: 1px solid #718096;
            border-radius: 6px;
            padding: 10px 12px;
            font-size: 14px;
            selection-background-color: #4299e1;
        }
        QLineEdit:focus {
            border-color: #63b3ed;
        }
        QPushButton {
            border: none;
            border-radius: 6px;
            padding: 10px 18px;
            font-size: 14px;
            min-height: 38px;
            background-color: #718096;
            color: white;
        }
        QPushButton:hover {
            background-color: #5a6678;
        }
        QPushButton#loginBtn {
            background-color: #2196f3;
        }
        QPushButton#loginBtn:hover {
            background-color: #1976d2;
        }
        QPushButton#guestBtn {
            background-color: #2f855a;
        }
        QPushButton#guestBtn:hover {
            background-color: #276749;
        }
        QLabel#titleLabel {
            font-size: 22px;
            font-weight: bold;
            color: #63b3ed;
        }
        QLabel#tipLabel {
            font-size: 12px;
            color: #a0aec0;
        }
    )");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::isAuthenticated() const
{
    return m_authenticated;
}

bool LoginDialog::isGuestMode() const
{
    return m_guestMode;
}

QString LoginDialog::accountHint() const
{
    return m_accountHint;
}

void LoginDialog::accept()
{
    const QString username = ui->usernameEdit->text().trimmed();
    const QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("请输入客户端用户名和密码，或点击游客看盘。"));
        return;
    }

    m_accountHint = username;
    m_authenticated = true;
    m_guestMode = false;
    QDialog::accept();
}

void LoginDialog::reject()
{
    QDialog::reject();
}

void LoginDialog::acceptGuest()
{
    m_accountHint.clear();
    m_authenticated = true;
    m_guestMode = true;
    QDialog::accept();
}