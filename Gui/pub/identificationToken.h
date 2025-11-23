#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>

class IdentificationToken : public QWidget
{
    Q_OBJECT
public:
    explicit IdentificationToken(QWidget* parent = nullptr);
    
    // Set the public and private keys and generate signed token QR code
    void setIdentificationToken(const QString& publicKey, const QString& privateKey);
    void clear();

private:
    QLabel* titleLabel_ = nullptr;
    QLabel* qrImageLabel_ = nullptr;
    QLabel* instructionLabel_ = nullptr;
    QString publicKey_;
};