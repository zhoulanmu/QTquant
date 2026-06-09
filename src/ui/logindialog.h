#pragma once

#include <QDialog>

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

protected:
    void accept() override;
    void reject() override;

private:
    Ui::LoginDialog *ui;
    bool m_authenticated;
};
