#include "identificationToken.h"
#include "qrCodeGen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QCryptographicHash>

IdentificationToken::IdentificationToken(QWidget* parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    mainLayout->addStretch(1);

    // Title
    titleLabel_ = new QLabel("Personal Identification Token", this);
    titleLabel_->setStyleSheet("font-size: 22px; font-weight: bold; color: #333;");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(true);
    mainLayout->addWidget(titleLabel_);

    // Email display
    emailLabel_ = new QLabel("", this);
    emailLabel_->setStyleSheet("font-size: 14px; color: #666;");
    emailLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(emailLabel_);

    mainLayout->addSpacing(10);

    // QR Code container
    auto qrContainer = new QWidget(this);
    qrContainer->setStyleSheet("background-color: white; border-radius: 12px;");
    qrContainer->setMaximumSize(360, 360);
    
    auto qrLayout = new QVBoxLayout(qrContainer);
    qrLayout->setContentsMargins(20, 20, 20, 20);
    qrLayout->setAlignment(Qt::AlignCenter);

    // QR Code image
    qrImageLabel_ = new QLabel(qrContainer);
    qrImageLabel_->setAlignment(Qt::AlignCenter);
    qrImageLabel_->setStyleSheet("background-color: white; padding: 10px;");
    qrImageLabel_->setFixedSize(320, 320);
    qrLayout->addWidget(qrImageLabel_, 0, Qt::AlignCenter);

    // Center QR container
    auto centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(qrContainer);
    centerLayout->addStretch();
    mainLayout->addLayout(centerLayout);

    mainLayout->addSpacing(10);

    // Instructions
    instructionLabel_ = new QLabel(
        "Present this QR code to ticket inspectors\n"
        "to verify your identity and ticket ownership.",
        this
    );
    instructionLabel_->setStyleSheet("font-size: 13px; color: #666;");
    instructionLabel_->setAlignment(Qt::AlignCenter);
    instructionLabel_->setWordWrap(true);
    mainLayout->addWidget(instructionLabel_);

    mainLayout->addStretch(2);

    setLayout(mainLayout);
}

void IdentificationToken::setPublicKey(const QString& publicKey, const QString& email)
{
    publicKey_ = publicKey;
    
    // Update email display
    emailLabel_->setText(email);
    
    // Generate a hash of the public key for more compact QR code
    // In a real system, you might want to encode the full key or a specific format
    QByteArray keyBytes = publicKey.toUtf8();
    QByteArray keyHash = QCryptographicHash::hash(keyBytes, QCryptographicHash::Sha256);
    QString keyHashHex = keyHash.toHex();
    
    // Create a compact identifier: email + key hash
    QString tokenData = QString("PIT:%1:%2").arg(email, keyHashHex);
    
    // Generate QR code from the token data
    QImage qrImage = QRCodeGenerator::generateQRCode(tokenData, 280);
    QPixmap qrPixmap = QPixmap::fromImage(qrImage);
    qrImageLabel_->setPixmap(qrPixmap);
}

void IdentificationToken::clear()
{
    publicKey_.clear();
    emailLabel_->clear();
    qrImageLabel_->clear();
}