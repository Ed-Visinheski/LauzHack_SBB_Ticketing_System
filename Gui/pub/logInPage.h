#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "accountInfo.h"

class LoginPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(QWidget* parent = nullptr);

signals:
    void loginSuccessful(const AccountInfo& account);

private slots:
    void handleLogin();

private:
    QLineEdit* emailEdit_ = nullptr;
    QLineEdit* passwordEdit_ = nullptr;
    QPushButton* loginButton_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    
    void showStatus(const QString& message, bool isError);
};