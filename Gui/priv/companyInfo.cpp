#include "companyInfo.h"
#include "PgpKeyManager.h"
#include <QCryptographicHash>
#include <stdexcept>

CompanyInfo::CompanyInfo()
{
    generateKeys();
}

void CompanyInfo::generateKeys()
{
    try {
        // Create deterministic ID for the company using the constexpr email
        // This ensures the same keys are always generated for the company
        std::string companyEmail(COMPANY_EMAIL);
        
        // Use a fixed "password" for deterministic key generation
        // In production, this could be derived from a secure company secret
        std::string companySeed = "SBB_COMPANY_MASTER_KEY_2025";
        
        QByteArray combined = QString::fromStdString(companyEmail + companySeed).toUtf8();
        QByteArray hash = QCryptographicHash::hash(combined, QCryptographicHash::Sha256);
        std::string deterministicId = companyEmail + "_" + hash.toHex().left(16).toStdString();
        
        // Generate PGP keys using PgpKeyManager
        PgpKeyManager keyManager(deterministicId);
        
        // Export keys
        publicKey_ = QString::fromStdString(keyManager.exportPublicKeyArmored());
        privateKey_ = QString::fromStdString(keyManager.exportSecretKeyArmored());
        
    } catch (const std::exception& e) {
        // qWarning() << "Failed to generate company keys:" << e.what();
        publicKey_.clear();
        privateKey_.clear();
    }
}