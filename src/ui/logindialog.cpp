#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 窗口基础设置
    setMinimumSize(420, 380); // 稍微加高一点，给布局留出空间
    setWindowTitle("用户登录 - QTQuant");

    // ========== 1. 先给所有布局设置间距（用 ui 指针直接访问，避免 findChild 失败） ==========
    // 主布局
    ui->verticalLayout->setSpacing(25);           // 控件之间的垂直间距
    ui->verticalLayout->setContentsMargins(35, 35, 35, 35); // 窗口四周的边距
    // 输入框布局
    ui->inputLayout->setSpacing(18);              // 用户名和密码之间的间距
    // 按钮布局
    ui->horizontalLayout->setSpacing(20);          // 登录和取消按钮之间的间距

    // ========== 2. 统一样式表，避免挤压布局 ==========
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
            padding: 12px 14px;
            font-size: 14px;
            min-height: 42px; /* 不要设太大，避免挤掉间距 */
        }
        QLineEdit:focus {
            border-color: #63b3ed;
        }
        QPushButton {
            border: none;
            border-radius: 6px;
            padding: 12px 32px;
            font-size: 14px;
            min-height: 44px;
        }
        QPushButton#loginBtn {
            background-color: #2196f3;
            color: white;
        }
        QPushButton#loginBtn:hover {
            background-color: #1976d2;
        }
        QPushButton#cancelBtn {
            background-color: #718096;
            color: white;
        }
        QPushButton#cancelBtn:hover {
            background-color: #5a6678;
        }
        QLabel#titleLabel {
            font-size: 20px;
            font-weight: bold;
            color: #63b3ed;
            margin-bottom: 5px; /* 标题和输入框之间额外加一点间距 */
        }
        QLabel#tipLabel {
            font-size: 12px;
            color: #a0aec0;
            margin-top: 5px; /* 提示文字和按钮之间额外加一点间距 */
        }
    )");

    // 密码框安全设置（双重保险）
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::isAuthenticated() const
{
    return m_authenticated;
}

void LoginDialog::accept()
{
    QString user = ui->usernameEdit->text().trimmed();
    QString pwd = ui->passwordEdit->text();
    
    if (user.isEmpty() || pwd.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码");
        return;
    }
    
    if ((user == "admin" && pwd == "123456") || 
        (user == "user" && pwd == "654321")) {
        m_authenticated = true;
        QDialog::accept();
    } else {
        QMessageBox::critical(this, "错误", "用户名或密码错误");
        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();
    }
}

void LoginDialog::reject()
{
    QDialog::reject();
}
