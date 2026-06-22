#pragma once

#include <QDialog>
#include <QString>

class QPlainTextEdit;
class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog() override;

    bool isAuthenticated() const;
    bool isGuestMode() const;
    QString accountHint() const;
    QString sessionCookie() const;

protected:
    void accept() override;
    void reject() override;

private slots:
    void openEastMoneyLogin();
    void acceptGuest();

private:
    bool looksLikeCookie(const QString& cookie) const;

private:
    Ui::LoginDialog *ui;
    QPlainTextEdit* m_cookieEdit;
    QPushButton* m_openLoginButton;
    QPushButton* m_guestButton;
    bool m_authenticated;
    bool m_guestMode;
    QString m_accountHint;
    QString m_sessionCookie;
};
