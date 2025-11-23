#pragma once
#include <QString>
#include <string_view>

class CompanyInfo
{
public:
    // Company email as constexpr (compile-time constant)
    static constexpr std::string_view COMPANY_EMAIL = "sbbsupersecretadmin@gmail.com";
    
    // Constructor generates the company's PGP keys
    CompanyInfo();
    
    // Getters for the keys
    QString publicKey() const { return publicKey_; }
    QString privateKey() const { return privateKey_; }
    QString email() const { return QString::fromUtf8(COMPANY_EMAIL.data(), COMPANY_EMAIL.size()); }
    
    // Check if keys are valid
    bool isValid() const { return !publicKey_.isEmpty() && !privateKey_.isEmpty(); }

private:
    QString publicKey_;
    QString privateKey_;
    
    void generateKeys();
};