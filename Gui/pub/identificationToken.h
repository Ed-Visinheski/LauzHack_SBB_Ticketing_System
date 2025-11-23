#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>

class IdentificationToken : public QWidget
{
    Q_OBJECT
public:
    explicit IdentificationToken(QWidget* parent = nullptr);
    
    // Set the public key and generate QR code
    void setPublicKey(const QString& publicKey, const QString& email);
    void clear();

private:
    QLabel* titleLabel_ = nullptr;
    QLabel* emailLabel_ = nullptr;
    QLabel* qrImageLabel_ = nullptr;
    QLabel* instructionLabel_ = nullptr;
    QString publicKey_;
};