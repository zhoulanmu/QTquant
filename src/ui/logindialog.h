#pragma once

#include <QDialog>
#include <QString>

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

protected:
    void accept() override;
    void reject() override;

private slots:
    void acceptGuest();

private:
    Ui::LoginDialog *ui;
    QPushButton* m_guestButton;
    bool m_authenticated;
    bool m_guestMode;
    QString m_accountHint;
};