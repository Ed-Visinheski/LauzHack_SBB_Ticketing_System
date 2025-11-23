#include "ticketInspector.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QProcess>
#include <QTemporaryFile>
#include <QDebug>

TicketInspector::TicketInspector(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("SBB Ticket Inspector");
    setMinimumSize(800, 700);
    setupUI();
}

void TicketInspector::setupUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Title
    titleLabel_ = new QLabel("Ticket Inspector", this);
    titleLabel_->setStyleSheet("font-size: 28px; font-weight: bold; color: #2c3e50;");
    titleLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel_);

    mainLayout->addSpacing(10);

    // Content area with two columns
    auto contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    // Left Column - PIT
    auto pitPanel = new QFrame(this);
    pitPanel->setFrameShape(QFrame::StyledPanel);
    pitPanel->setStyleSheet("QFrame { background-color: #ecf0f1; border-radius: 8px; padding: 15px; }");
    auto pitLayout = new QVBoxLayout(pitPanel);
    
    pitLabel_ = new QLabel("Personal Identity Token (PIT)", pitPanel);
    pitLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: #34495e;");
    pitLabel_->setAlignment(Qt::AlignCenter);
    pitLayout->addWidget(pitLabel_);
    
    pitLayout->addSpacing(10);
    
    // PIT Image preview
    pitImageLabel_ = new QLabel(pitPanel);
    pitImageLabel_->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
    pitImageLabel_->setAlignment(Qt::AlignCenter);
    pitImageLabel_->setStyleSheet("QLabel { background-color: white; border: 2px dashed #95a5a6; border-radius: 8px; }");
    pitImageLabel_->setText("No QR Code Loaded");
    pitImageLabel_->setWordWrap(true);
    pitLayout->addWidget(pitImageLabel_, 0, Qt::AlignCenter);
    
    pitLayout->addSpacing(10);
    
    loadPITButton_ = new QPushButton("Load PIT QR Code", pitPanel);
    loadPITButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #3498db; color: white; font-weight: bold; "
        "  font-size: 14px; padding: 10px 20px; border: none; border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:pressed { background-color: #21618c; }"
    );
    connect(loadPITButton_, &QPushButton::clicked, this, &TicketInspector::loadPITQRCode);
    pitLayout->addWidget(loadPITButton_);
    
    pitLayout->addSpacing(5);
    
    pitStatusLabel_ = new QLabel("Status: Waiting", pitPanel);
    pitStatusLabel_->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    pitStatusLabel_->setAlignment(Qt::AlignCenter);
    pitLayout->addWidget(pitStatusLabel_);
    
    pitLayout->addStretch();
    contentLayout->addWidget(pitPanel);

    // Right Column - Ticket
    auto ticketPanel = new QFrame(this);
    ticketPanel->setFrameShape(QFrame::StyledPanel);
    ticketPanel->setStyleSheet("QFrame { background-color: #ecf0f1; border-radius: 8px; padding: 15px; }");
    auto ticketLayout = new QVBoxLayout(ticketPanel);
    
    ticketLabel_ = new QLabel("Booking Ticket", ticketPanel);
    ticketLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: #34495e;");
    ticketLabel_->setAlignment(Qt::AlignCenter);
    ticketLayout->addWidget(ticketLabel_);
    
    ticketLayout->addSpacing(10);
    
    // Ticket Image preview
    ticketImageLabel_ = new QLabel(ticketPanel);
    ticketImageLabel_->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
    ticketImageLabel_->setAlignment(Qt::AlignCenter);
    ticketImageLabel_->setStyleSheet("QLabel { background-color: white; border: 2px dashed #95a5a6; border-radius: 8px; }");
    ticketImageLabel_->setText("No QR Code Loaded");
    ticketImageLabel_->setWordWrap(true);
    ticketLayout->addWidget(ticketImageLabel_, 0, Qt::AlignCenter);
    
    ticketLayout->addSpacing(10);
    
    loadTicketButton_ = new QPushButton("Load Ticket QR Code", ticketPanel);
    loadTicketButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #3498db; color: white; font-weight: bold; "
        "  font-size: 14px; padding: 10px 20px; border: none; border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:pressed { background-color: #21618c; }"
    );
    connect(loadTicketButton_, &QPushButton::clicked, this, &TicketInspector::loadTicketQRCode);
    ticketLayout->addWidget(loadTicketButton_);
    
    ticketLayout->addSpacing(5);
    
    ticketStatusLabel_ = new QLabel("Status: Waiting", ticketPanel);
    ticketStatusLabel_->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    ticketStatusLabel_->setAlignment(Qt::AlignCenter);
    ticketLayout->addWidget(ticketStatusLabel_);
    
    ticketLayout->addStretch();
    contentLayout->addWidget(ticketPanel);

    mainLayout->addLayout(contentLayout);

    // Action Buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    
    clearButton_ = new QPushButton("Clear All", this);
    clearButton_->setFixedHeight(45);
    clearButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #95a5a6; color: white; font-weight: bold; "
        "  font-size: 15px; padding: 10px 30px; border: none; border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    connect(clearButton_, &QPushButton::clicked, this, &TicketInspector::clearAll);
    buttonLayout->addWidget(clearButton_);
    
    verifyButton_ = new QPushButton("Verify Ownership", this);
    verifyButton_->setFixedHeight(45);
    verifyButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #27ae60; color: white; font-weight: bold; "
        "  font-size: 16px; padding: 10px 40px; border: none; border-radius: 6px; "
        "} "
        "QPushButton:hover { background-color: #229954; } "
        "QPushButton:pressed { background-color: #1e8449; } "
        "QPushButton:disabled { background-color: #bdc3c7; }"
    );
    verifyButton_->setEnabled(false);
    connect(verifyButton_, &QPushButton::clicked, this, &TicketInspector::verifyOwnership);
    buttonLayout->addWidget(verifyButton_, 1);
    
    mainLayout->addLayout(buttonLayout);

    // Result Panel
    resultPanel_ = new QWidget(this);
    resultPanel_->setStyleSheet("QWidget { background-color: #f8f9fa; border-radius: 8px; padding: 20px; }");
    auto resultLayout = new QVBoxLayout(resultPanel_);
    
    auto resultTopLayout = new QHBoxLayout();
    resultIconLabel_ = new QLabel(resultPanel_);
    resultIconLabel_->setFixedSize(48, 48);
    resultIconLabel_->setAlignment(Qt::AlignCenter);
    resultTopLayout->addWidget(resultIconLabel_);
    
    resultTextLabel_ = new QLabel("Awaiting verification...", resultPanel_);
    resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #7f8c8d;");
    resultTextLabel_->setWordWrap(true);
    resultTopLayout->addWidget(resultTextLabel_, 1);
    
    resultLayout->addLayout(resultTopLayout);
    
    detailsLabel_ = new QLabel("Load both QR codes and click 'Verify Ownership' to check if the ticket belongs to the user.", resultPanel_);
    detailsLabel_->setStyleSheet("font-size: 13px; color: #7f8c8d; margin-top: 10px;");
    detailsLabel_->setWordWrap(true);
    resultLayout->addWidget(detailsLabel_);
    
    resultPanel_->setVisible(true);
    mainLayout->addWidget(resultPanel_);
    
    setLayout(mainLayout);
}

void TicketInspector::loadPITQRCode()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Select PIT QR Code Image",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Load and display image
    pitImage_.load(fileName);
    if (pitImage_.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to load image file.");
        return;
    }
    
    pitImageLabel_->setPixmap(pitImage_.scaled(IMAGE_SIZE, IMAGE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // Decode QR code
    pitQRData_ = decodeQRFromImage(fileName);
    
    if (pitQRData_.isEmpty()) {
        pitStatusLabel_->setText("Status: Failed to decode QR");
        pitStatusLabel_->setStyleSheet("font-size: 12px; color: #e74c3c;");
        QMessageBox::warning(this, "Error", "Failed to decode QR code from image.\n\nMake sure the image contains a valid QR code.");
    } else {
        pitStatusLabel_->setText("Status: QR Code Loaded ✓");
        pitStatusLabel_->setStyleSheet("font-size: 12px; color: #27ae60; font-weight: bold;");
    }
    
    // Enable verify button if both QR codes are loaded
    verifyButton_->setEnabled(!pitQRData_.isEmpty() && !ticketQRData_.isEmpty());
}

void TicketInspector::loadTicketQRCode()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Select Ticket QR Code Image",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Load and display image
    ticketImage_.load(fileName);
    if (ticketImage_.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to load image file.");
        return;
    }
    
    ticketImageLabel_->setPixmap(ticketImage_.scaled(IMAGE_SIZE, IMAGE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // Decode QR code
    ticketQRData_ = decodeQRFromImage(fileName);
    
    if (ticketQRData_.isEmpty()) {
        ticketStatusLabel_->setText("Status: Failed to decode QR");
        ticketStatusLabel_->setStyleSheet("font-size: 12px; color: #e74c3c;");
        QMessageBox::warning(this, "Error", "Failed to decode QR code from image.\n\nMake sure the image contains a valid QR code.");
    } else {
        ticketStatusLabel_->setText("Status: QR Code Loaded ✓");
        ticketStatusLabel_->setStyleSheet("font-size: 12px; color: #27ae60; font-weight: bold;");
    }
    
    // Enable verify button if both QR codes are loaded
    verifyButton_->setEnabled(!pitQRData_.isEmpty() && !ticketQRData_.isEmpty());
}

void TicketInspector::verifyOwnership()
{
    if (pitQRData_.isEmpty() || ticketQRData_.isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Please load both PIT and Ticket QR codes first.");
        return;
    }
    
    // Perform verification
    auto result = TicketOwnership::verifyOwnership(pitQRData_, ticketQRData_);
    
    // Update UI with result
    updateVerificationStatus(result);
}

void TicketInspector::clearAll()
{
    pitQRData_.clear();
    ticketQRData_.clear();
    pitImage_ = QPixmap();
    ticketImage_ = QPixmap();
    
    pitImageLabel_->clear();
    pitImageLabel_->setText("No QR Code Loaded");
    ticketImageLabel_->clear();
    ticketImageLabel_->setText("No QR Code Loaded");
    
    pitStatusLabel_->setText("Status: Waiting");
    pitStatusLabel_->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    ticketStatusLabel_->setText("Status: Waiting");
    ticketStatusLabel_->setStyleSheet("font-size: 12px; color: #7f8c8d;");
    
    verifyButton_->setEnabled(false);
    
    resultIconLabel_->clear();
    resultTextLabel_->setText("Awaiting verification...");
    resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #7f8c8d;");
    detailsLabel_->setText("Load both QR codes and click 'Verify Ownership' to check if the ticket belongs to the user.");
    detailsLabel_->setStyleSheet("font-size: 13px; color: #7f8c8d; margin-top: 10px;");
    resultPanel_->setStyleSheet("QWidget { background-color: #f8f9fa; border-radius: 8px; padding: 20px; }");
}

void TicketInspector::updateVerificationStatus(const TicketOwnership::VerificationResult& result)
{
    if (result.keysMatch && result.pitParsed && result.ticketParsed) {
        // Success - Keys Match
        resultPanel_->setStyleSheet("QWidget { background-color: #d5f4e6; border: 2px solid #27ae60; border-radius: 8px; padding: 20px; }");
        resultIconLabel_->setText("✓");
        resultIconLabel_->setStyleSheet("font-size: 36px; color: #27ae60; font-weight: bold;");
        resultTextLabel_->setText("VALID: Ticket Belongs to User");
        resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #27ae60;");
        
        QString details = QString("• Booking Reference: %1\n"
                                  "• PIT Parsed: ✓\n"
                                  "• Ticket Parsed: ✓\n"
                                  "• Public Key Hashes Match: ✓\n"
                                  "• Verification: PASSED");
        details = details.arg(result.bookingReference);
        
        detailsLabel_->setText(details);
        detailsLabel_->setStyleSheet("font-size: 13px; color: #27ae60; margin-top: 10px; font-weight: bold;");
        
    } else {
        // Failure - Keys Don't Match or Parse Error
        resultPanel_->setStyleSheet("QWidget { background-color: #fadbd8; border: 2px solid #e74c3c; border-radius: 8px; padding: 20px; }");
        resultIconLabel_->setText("✗");
        resultIconLabel_->setStyleSheet("font-size: 36px; color: #e74c3c; font-weight: bold;");
        resultTextLabel_->setText("INVALID: Ticket Does NOT Belong to User");
        resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");
        
        QString details = QString("• Error: %1\n"
                                  "• PIT Parsed: %2\n"
                                  "• Ticket Parsed: %3\n"
                                  "• Public Key Hashes Match: %4\n"
                                  "• Verification: FAILED");
        details = details.arg(result.errorMessage)
                        .arg(result.pitParsed ? "✓" : "✗")
                        .arg(result.ticketParsed ? "✓" : "✗")
                        .arg(result.keysMatch ? "✓" : "✗");
        
        detailsLabel_->setText(details);
        detailsLabel_->setStyleSheet("font-size: 13px; color: #e74c3c; margin-top: 10px; font-weight: bold;");
    }
}

QString TicketInspector::decodeQRFromImage(const QString& imagePath)
{
    // Use zbarimg command-line tool to decode QR code
    // This requires zbar to be installed: brew install zbar (macOS) or apt install zbar-tools (Linux)
    
    QProcess process;
    process.start("zbarimg", QStringList() << "--quiet" << "--raw" << imagePath);
    
    if (!process.waitForFinished(5000)) {
        qWarning() << "QR decode timeout or zbarimg not found";
        return QString();
    }
    
    if (process.exitCode() != 0) {
        qWarning() << "QR decode failed:" << process.readAllStandardError();
        return QString();
    }
    
    QString qrData = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    qDebug() << "Decoded QR:" << qrData;
    
    return qrData;
}