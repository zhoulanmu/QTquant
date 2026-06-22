#include "logindialog.h"
#include "ui_logindialog.h"

#include <QDesktopServices>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QUrl>

namespace {
const QUrl EastMoneyLoginUrl(QStringLiteral("https://passport2.eastmoney.com/pub/login?backurl=https%3A%2F%2Fwww.eastmoney.com%2F"));
}

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_cookieEdit(nullptr)
    , m_openLoginButton(nullptr)
    , m_guestButton(nullptr)
    , m_authenticated(false)
    , m_guestMode(true)
{
    ui->setupUi(this);

    setMinimumSize(480, 520);
    setWindowTitle(QStringLiteral("QTQuant - 东财账号"));

    ui->verticalLayout->setSpacing(18);
    ui->verticalLayout->setContentsMargins(35, 35, 35, 35);
    ui->inputLayout->setSpacing(12);
    ui->horizontalLayout->setSpacing(12);

    ui->titleLabel->setText(QStringLiteral("QTQuant 看盘"));
    ui->usernameEdit->setPlaceholderText(QStringLiteral("东财账号备注（可选）"));
    ui->passwordEdit->hide();
    ui->loginBtn->setText(QStringLiteral("接入东财会话"));
    ui->cancelBtn->setText(QStringLiteral("取消"));
    ui->tipLabel->setText(QStringLiteral("免费实时行情无需登录；东财 Cookie 只保存在本次程序内存中。"));
    ui->tipLabel->setWordWrap(true);

    m_cookieEdit = new QPlainTextEdit(this);
    m_cookieEdit->setObjectName(QStringLiteral("cookieEdit"));
    m_cookieEdit->setMaximumHeight(96);
    m_cookieEdit->setPlaceholderText(QStringLiteral("粘贴东财网页登录后的 Cookie（可选）"));
    ui->inputLayout->addWidget(m_cookieEdit);

    m_openLoginButton = new QPushButton(QStringLiteral("打开东财官方登录页"), this);
    m_openLoginButton->setObjectName(QStringLiteral("openEastMoneyBtn"));
    ui->inputLayout->addWidget(m_openLoginButton);

    m_guestButton = new QPushButton(QStringLiteral("游客看盘"), this);
    m_guestButton->setObjectName(QStringLiteral("guestBtn"));
    ui->horizontalLayout->insertWidget(1, m_guestButton);

    connect(m_openLoginButton, &QPushButton::clicked, this, &LoginDialog::openEastMoneyLogin);
    connect(m_guestButton, &QPushButton::clicked, this, &LoginDialog::acceptGuest);

    this->setStyleSheet(R"(
        QDialog {
            background-color: #253b6e;
            border-radius: 8px;
        }
        QLineEdit, QPlainTextEdit {
            background-color: #4a5568;
            color: white;
            border: 1px solid #718096;
            border-radius: 6px;
            padding: 10px 12px;
            font-size: 14px;
            selection-background-color: #4299e1;
        }
        QLineEdit:focus, QPlainTextEdit:focus {
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
        QPushButton#loginBtn, QPushButton#openEastMoneyBtn {
            background-color: #2196f3;
        }
        QPushButton#loginBtn:hover, QPushButton#openEastMoneyBtn:hover {
            background-color: #1976d2;
        }
        QPushButton#guestBtn {
            background-color: #2f855a;
        }
        QPushButton#guestBtn:hover {
            background-color: #276749;
        }
        QLabel#titleLabel {
            font-size: 20px;
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

QString LoginDialog::sessionCookie() const
{
    return m_sessionCookie;
}

void LoginDialog::accept()
{
    m_accountHint = ui->usernameEdit->text().trimmed();
    const QString cookie = m_cookieEdit ? m_cookieEdit->toPlainText().trimmed() : QString();

    if (cookie.isEmpty()) {
        acceptGuest();
        return;
    }

    if (!looksLikeCookie(cookie)) {
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("Cookie 格式看起来不完整，至少需要包含 name=value。"));
        return;
    }

    m_sessionCookie = cookie;
    m_authenticated = true;
    m_guestMode = false;
    QDialog::accept();
}

void LoginDialog::reject()
{
    QDialog::reject();
}

void LoginDialog::openEastMoneyLogin()
{
    if (!QDesktopServices::openUrl(EastMoneyLoginUrl)) {
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("无法打开浏览器，请手动访问 passport2.eastmoney.com。"));
    }
}

void LoginDialog::acceptGuest()
{
    m_accountHint.clear();
    m_sessionCookie.clear();
    m_authenticated = true;
    m_guestMode = true;
    QDialog::accept();
}

bool LoginDialog::looksLikeCookie(const QString& cookie) const
{
    return cookie.contains(QLatin1Char('='));
}
