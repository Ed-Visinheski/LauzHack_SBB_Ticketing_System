#include "accountInfo.h"

AccountInfo::AccountInfo(const QString& email, const QString& publicKey, const QString& privateKey)
    : email_(email)
    , publicKey_(publicKey)
    , privateKey_(privateKey)
{
}

void AccountInfo::setKeys(const QString& email, const QString& publicKey, const QString& privateKey)
{
    email_ = email;
    publicKey_ = publicKey;
    privateKey_ = privateKey;
}

void AccountInfo::clear()
{
    email_.clear();
    publicKey_.clear();
    privateKey_.clear();
}