#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QTimer>

class IdentificationToken : public QWidget
{
    Q_OBJECT
public:
    explicit IdentificationToken(QWidget* parent = nullptr);
    
    // Set the public and private keys and generate signed token QR code
    void setIdentificationToken(const QString& publicKey, const QString& privateKey);
    void clear();

private slots:
    void downloadPIT();
    void updateCountdown();
    void refreshQRCode();

private:
    void generateQRCodeWithTimestamp();
    
    QLabel* titleLabel_ = nullptr;
    QLabel* timerLabel_ = nullptr;
    QLabel* qrImageLabel_ = nullptr;
    QLabel* instructionLabel_ = nullptr;
    QPushButton* downloadButton_ = nullptr;
    QString publicKey_;
    QString privateKey_;
    QTimer* countdownTimer_ = nullptr;
    QTimer* refreshTimer_ = nullptr;
    int countdown_ = 10;
};