#pragma once
#include <QString>

class AccountInfo
{
public:
    AccountInfo() = default;
    AccountInfo(const QString& email, const QString& publicKey, const QString& privateKey);

    QString email() const { return email_; }
    QString publicKey() const { return publicKey_; }
    QString privateKey() const { return privateKey_; }
    bool isValid() const { return !email_.isEmpty() && !publicKey_.isEmpty(); }

    void setKeys(const QString& email, const QString& publicKey, const QString& privateKey);
    void clear();

private:
    QString email_;
    QString publicKey_;
    QString privateKey_;
};