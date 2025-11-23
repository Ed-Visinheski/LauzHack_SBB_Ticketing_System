#include "identificationToken.h"
#include "qrCodeGen.h"
#include "PgpKeyManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QCryptographicHash>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <rnp/rnp.h>
#include <rnp/rnp_err.h>

IdentificationToken::IdentificationToken(QWidget* parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    mainLayout->addStretch(1);

    // Title
    mainLayout->addWidget(titleLabel_);

    mainLayout->addSpacing(10);

    // Timer label
    timerLabel_ = new QLabel("Refreshes in: 10s", this);
    timerLabel_->setStyleSheet("font-size: 14px; font-weight: 600; color: #eb0000;");
    timerLabel_->setAlignment(Qt::AlignCenter);
    timerLabel_->setVisible(false);
    mainLayout->addWidget(timerLabel_);

    mainLayout->addSpacing(5);

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

    mainLayout->addSpacing(10);

    // Download button
    downloadButton_ = new QPushButton("Download PIT QR Code", this);
    downloadButton_->setFixedSize(200, 40);
    downloadButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #3498db; color: white; font-weight: bold; "
        "  font-size: 14px; border: none; border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:pressed { background-color: #21618c; }"
    );
    connect(downloadButton_, &QPushButton::clicked, this, &IdentificationToken::downloadPIT);
    
    auto downloadLayout = new QHBoxLayout();
    downloadLayout->addStretch();
    downloadLayout->addWidget(downloadButton_);
    downloadLayout->addStretch();
    mainLayout->addLayout(downloadLayout);

    mainLayout->addStretch(2);

    setLayout(mainLayout);
    
    // Setup timers
    countdownTimer_ = new QTimer(this);
    connect(countdownTimer_, &QTimer::timeout, this, &IdentificationToken::updateCountdown);
    
    refreshTimer_ = new QTimer(this);
    refreshTimer_->setSingleShot(true);
    connect(refreshTimer_, &QTimer::timeout, this, &IdentificationToken::refreshQRCode);
}

void IdentificationToken::setIdentificationToken(const QString& publicKey, const QString& privateKey)
{
    publicKey_ = publicKey;
    privateKey_ = privateKey;
    
    // Generate initial QR code
    generateQRCodeWithTimestamp();
    
    // Show timer label and start countdown
    timerLabel_->setVisible(true);
    countdown_ = 10;
    timerLabel_->setText(QString("Refreshes in: %1s").arg(countdown_));
    countdownTimer_->start(1000); // Update every second
    refreshTimer_->start(10000); // Refresh after 10 seconds
}

void IdentificationToken::generateQRCodeWithTimestamp()
{
    try {
        // Get current Unix timestamp
        qint64 timestamp = QDateTime::currentSecsSinceEpoch();
        
        // Create data to sign: publicKey + timestamp (anonymous, no email)
        QString dataToSign = publicKey_ + QString::number(timestamp);
        
        // Create temporary FFI context to import keys and sign
        rnp_ffi_t ffi = nullptr;
        if (rnp_ffi_create(&ffi, "GPG", "GPG") != RNP_SUCCESS || !ffi) {
            throw std::runtime_error("Failed to create RNP FFI for signing");
        }
        
        // Import the secret key
        rnp_input_t key_input = nullptr;
        QByteArray privateKeyBytes = privateKey_.toUtf8();
        if (rnp_input_from_memory(&key_input, 
                                  reinterpret_cast<const uint8_t*>(privateKeyBytes.constData()),
                                  privateKeyBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to create input for private key");
        }
        
        if (rnp_import_keys(ffi, key_input, RNP_LOAD_SAVE_SECRET_KEYS, nullptr) != RNP_SUCCESS) {
            rnp_input_destroy(key_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to import private key");
        }
        rnp_input_destroy(key_input);
        
        // Sign the data
        QByteArray dataBytes = dataToSign.toUtf8();
        rnp_input_t data_input = nullptr;
        if (rnp_input_from_memory(&data_input,
                                  reinterpret_cast<const uint8_t*>(dataBytes.constData()),
                                  dataBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to create input for data");
        }
        
        rnp_output_t sig_output = nullptr;
        if (rnp_output_to_memory(&sig_output, 0) != RNP_SUCCESS) {
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to create output for signature");
        }
        
        // Create signing operation
        rnp_op_sign_t sign_op = nullptr;
        if (rnp_op_sign_detached_create(&sign_op, ffi, data_input, sig_output) != RNP_SUCCESS) {
            rnp_output_destroy(sig_output);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to create signing operation");
        }
        
        // Get the key handle for signing (first key in keyring)
        rnp_identifier_iterator_t it = nullptr;
        if (rnp_identifier_iterator_create(ffi, &it, "grip") != RNP_SUCCESS) {
            rnp_op_sign_destroy(sign_op);
            rnp_output_destroy(sig_output);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to create key iterator");
        }
        
        const char* grip = nullptr;
        rnp_key_handle_t key = nullptr;
        if (rnp_identifier_iterator_next(it, &grip) == RNP_SUCCESS && grip) {
            if (rnp_locate_key(ffi, "grip", grip, &key) != RNP_SUCCESS || !key) {
                rnp_identifier_iterator_destroy(it);
                rnp_op_sign_destroy(sign_op);
                rnp_output_destroy(sig_output);
                rnp_input_destroy(data_input);
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("Failed to locate key for signing");
            }
        }
        rnp_identifier_iterator_destroy(it);
        
        if (!key) {
            rnp_op_sign_destroy(sign_op);
            rnp_output_destroy(sig_output);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("No key found for signing");
        }
        
        // Add signer
        if (rnp_op_sign_add_signature(sign_op, key, nullptr) != RNP_SUCCESS) {
            rnp_key_handle_destroy(key);
            rnp_op_sign_destroy(sign_op);
            rnp_output_destroy(sig_output);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to add signer");
        }
        
        // Execute signing
        if (rnp_op_sign_execute(sign_op) != RNP_SUCCESS) {
            rnp_key_handle_destroy(key);
            rnp_op_sign_destroy(sign_op);
            rnp_output_destroy(sig_output);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Signing operation failed");
        }
        
        // Get signature
        uint8_t* sig_buf = nullptr;
        size_t sig_len = 0;
        if (rnp_output_memory_get_buf(sig_output, &sig_buf, &sig_len, false) != RNP_SUCCESS) {
            rnp_key_handle_destroy(key);
            rnp_op_sign_destroy(sign_op);
            rnp_output_destroy(sig_output);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            throw std::runtime_error("Failed to get signature buffer");
        }
        
        QByteArray signature(reinterpret_cast<const char*>(sig_buf), sig_len);
        QString signatureHex = signature.toHex();
        
        // Cleanup
        rnp_key_handle_destroy(key);
        rnp_op_sign_destroy(sign_op);
        rnp_output_destroy(sig_output);
        rnp_input_destroy(data_input);
        rnp_ffi_destroy(ffi);
        
        // Create public key hash (first 16 chars of SHA-256 for brevity in QR)
        QByteArray publicKeyBytes = publicKey_.toUtf8();
        QByteArray hash = QCryptographicHash::hash(publicKeyBytes, QCryptographicHash::Sha256);
        QString publicKeyHash = hash.toHex().left(16);
        
        // Create anonymous token format: PIT:pubKeyHash:timestamp:signature
        QString tokenData = QString("PIT:%1:%2:%3")
            .arg(publicKeyHash)
            .arg(timestamp)
            .arg(signatureHex);
        
        // Generate QR code from the signed token
        QImage qrImage = QRCodeGenerator::generateQRCode(tokenData, 280);
        QPixmap qrPixmap = QPixmap::fromImage(qrImage);
        qrImageLabel_->setPixmap(qrPixmap);
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to create signed token:" << e.what();
        
        // Fallback to simple hash-based token (anonymous)
        QByteArray keyBytes = publicKey_.toUtf8();
        QByteArray keyHash = QCryptographicHash::hash(keyBytes, QCryptographicHash::Sha256);
        qint64 timestamp = QDateTime::currentSecsSinceEpoch();
        QString tokenData = QString("PIT:%1:%2").arg(timestamp).arg(keyHash.toHex());
        
        QImage qrImage = QRCodeGenerator::generateQRCode(tokenData, 280);
        QPixmap qrPixmap = QPixmap::fromImage(qrImage);
        qrImageLabel_->setPixmap(qrPixmap);
    }
}

void IdentificationToken::updateCountdown()
{
    countdown_--;
    if (countdown_ <= 0) {
        countdown_ = 10;
    }
    timerLabel_->setText(QString("Refreshes in: %1s").arg(countdown_));
}

void IdentificationToken::refreshQRCode()
{
    // Regenerate QR code with new timestamp
    generateQRCodeWithTimestamp();
    
    // Reset countdown and restart timer
    countdown_ = 10;
    timerLabel_->setText(QString("Refreshes in: %1s").arg(countdown_));
    refreshTimer_->start(10000); // Schedule next refresh
}

void IdentificationToken::clear()
{
    publicKey_.clear();
    privateKey_.clear();
    qrImageLabel_->clear();
    timerLabel_->setVisible(false);
    countdownTimer_->stop();
    refreshTimer_->stop();
    countdown_ = 10;
}

void IdentificationToken::downloadPIT()
{
    if (qrImageLabel_->pixmap().isNull()) {
        QMessageBox::warning(this, "No QR Code", "Please generate your PIT first by logging in.");
        return;
    }
    
    // Generate filename with timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString defaultFileName = QString("PIT_QRCode_%1.png").arg(timestamp);
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save PIT QR Code",
        defaultFileName,
        "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Save the QR code pixmap
    QPixmap pixmap = qrImageLabel_->pixmap();
    if (pixmap.save(fileName)) {
        QMessageBox::information(this, "Success", "PIT QR code saved successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Failed to save QR code image.");
    }
}