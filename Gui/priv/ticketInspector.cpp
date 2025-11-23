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
    setStyleSheet("background-color: #f5f5f5;");
    
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Add SBB header with logo
    auto headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background-color: #eb0000;");
    headerWidget->setFixedHeight(80);
    auto headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    
    auto logoLabel = new QLabel(headerWidget);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedSize(50, 50);
    logoLabel->setStyleSheet("background: transparent; border: none; padding: 0;");
    QPixmap logoPix("icons/SBB_logo.svg");
    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(logoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    headerLayout->addWidget(logoLabel);
    
    titleLabel_ = new QLabel("Ticket Inspector", headerWidget);
    titleLabel_->setStyleSheet("font-size: 18px; font-weight: 600; color: white; background: transparent; border: none;");
    titleLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(titleLabel_);
    headerLayout->addStretch();
    
    mainLayout->addWidget(headerWidget);

    // Content wrapper with scroll area
    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f5f5f5; }");
    
    auto contentWrapper = new QWidget();
    contentWrapper->setStyleSheet("background-color: #f5f5f5;");
    auto contentMainLayout = new QVBoxLayout(contentWrapper);
    contentMainLayout->setContentsMargins(30, 30, 30, 30);
    contentMainLayout->setSpacing(25);

    // Content area with two columns
    auto contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    // Left Column - PIT
    auto pitPanel = new QFrame(contentWrapper);
    pitPanel->setFrameShape(QFrame::NoFrame);
    pitPanel->setStyleSheet("QFrame { background-color: white; border: 1px solid #e0e0e0; border-radius: 12px; }");
    auto pitLayout = new QVBoxLayout(pitPanel);
    pitLayout->setContentsMargins(20, 20, 20, 20);
    pitLayout->setSpacing(15);
    
    pitLabel_ = new QLabel("Personal Identity Token (PIT)", pitPanel);
    pitLabel_->setStyleSheet("font-size: 16px; font-weight: 600; color: #333; background: transparent; border: none;");
    pitLabel_->setAlignment(Qt::AlignCenter);
    pitLayout->addWidget(pitLabel_);
    
    // PIT Image preview
    pitImageLabel_ = new QLabel(pitPanel);
    pitImageLabel_->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
    pitImageLabel_->setAlignment(Qt::AlignCenter);
    pitImageLabel_->setStyleSheet("QLabel { background-color: #fafafa; border: 2px dashed #e0e0e0; border-radius: 8px; color: #666; }");
    pitImageLabel_->setText("No QR Code Loaded");
    pitImageLabel_->setWordWrap(true);
    pitImageLabel_->setScaledContents(false);
    pitLayout->addWidget(pitImageLabel_, 0, Qt::AlignCenter);
    
    loadPITButton_ = new QPushButton("Load PIT QR Code", pitPanel);
    loadPITButton_->setMinimumHeight(46);
    loadPITButton_->setCursor(Qt::PointingHandCursor);
    loadPITButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #eb0000; color: white; font-weight: 600; "
        "  font-size: 14px; border: none; border-radius: 8px; "
        "} "
        "QPushButton:hover { background-color: #c00000; } "
        "QPushButton:pressed { background-color: #a00000; }"
    );
    connect(loadPITButton_, &QPushButton::clicked, this, &TicketInspector::loadPITQRCode);
    pitLayout->addWidget(loadPITButton_);
    
    pitStatusLabel_ = new QLabel("Status: Waiting", pitPanel);
    pitStatusLabel_->setStyleSheet("font-size: 13px; color: #666; background: transparent; border: none;");
    pitStatusLabel_->setAlignment(Qt::AlignCenter);
    pitLayout->addWidget(pitStatusLabel_);
    
    contentLayout->addWidget(pitPanel, 1);

    // Right Column - Ticket
    auto ticketPanel = new QFrame(contentWrapper);
    ticketPanel->setFrameShape(QFrame::NoFrame);
    ticketPanel->setStyleSheet("QFrame { background-color: white; border: 1px solid #e0e0e0; border-radius: 12px; }");
    auto ticketLayout = new QVBoxLayout(ticketPanel);
    ticketLayout->setContentsMargins(20, 20, 20, 20);
    ticketLayout->setSpacing(15);
    
    ticketLabel_ = new QLabel("Booking Ticket", ticketPanel);
    ticketLabel_->setStyleSheet("font-size: 16px; font-weight: 600; color: #333; background: transparent; border: none;");
    ticketLabel_->setAlignment(Qt::AlignCenter);
    ticketLayout->addWidget(ticketLabel_);
    
    // Ticket Image preview
    ticketImageLabel_ = new QLabel(ticketPanel);
    ticketImageLabel_->setFixedSize(IMAGE_SIZE, IMAGE_SIZE);
    ticketImageLabel_->setAlignment(Qt::AlignCenter);
    ticketImageLabel_->setStyleSheet("QLabel { background-color: #fafafa; border: 2px dashed #e0e0e0; border-radius: 8px; color: #666; }");
    ticketImageLabel_->setText("No QR Code Loaded");
    ticketImageLabel_->setWordWrap(true);
    ticketImageLabel_->setScaledContents(false);
    ticketLayout->addWidget(ticketImageLabel_, 0, Qt::AlignCenter);
    
    loadTicketButton_ = new QPushButton("Load Ticket QR Code", ticketPanel);
    loadTicketButton_->setMinimumHeight(46);
    loadTicketButton_->setCursor(Qt::PointingHandCursor);
    loadTicketButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #eb0000; color: white; font-weight: 600; "
        "  font-size: 14px; border: none; border-radius: 8px; "
        "} "
        "QPushButton:hover { background-color: #c00000; } "
        "QPushButton:pressed { background-color: #a00000; }"
    );
    connect(loadTicketButton_, &QPushButton::clicked, this, &TicketInspector::loadTicketQRCode);
    ticketLayout->addWidget(loadTicketButton_);
    
    ticketStatusLabel_ = new QLabel("Status: Waiting", ticketPanel);
    ticketStatusLabel_->setStyleSheet("font-size: 13px; color: #666; background: transparent; border: none;");
    ticketStatusLabel_->setAlignment(Qt::AlignCenter);
    ticketLayout->addWidget(ticketStatusLabel_);
    
    contentLayout->addWidget(ticketPanel, 1);

    contentMainLayout->addLayout(contentLayout);

    // Action Buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    
    clearButton_ = new QPushButton("Clear All", contentWrapper);
    clearButton_->setMinimumHeight(50);
    clearButton_->setCursor(Qt::PointingHandCursor);
    clearButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: white; color: #666; font-weight: 600; "
        "  font-size: 14px; border: 2px solid #e0e0e0; border-radius: 8px; "
        "} "
        "QPushButton:hover { background-color: #f5f5f5; border-color: #ccc; }"
    );
    connect(clearButton_, &QPushButton::clicked, this, &TicketInspector::clearAll);
    buttonLayout->addWidget(clearButton_);
    
    verifyButton_ = new QPushButton("Verify Ownership", contentWrapper);
    verifyButton_->setMinimumHeight(50);
    verifyButton_->setCursor(Qt::PointingHandCursor);
    verifyButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #eb0000; color: white; font-weight: 600; "
        "  font-size: 15px; border: none; border-radius: 8px; "
        "} "
        "QPushButton:hover { background-color: #c00000; } "
        "QPushButton:pressed { background-color: #a00000; } "
        "QPushButton:disabled { background-color: #e0e0e0; color: #999; }"
    );
    verifyButton_->setEnabled(false);
    connect(verifyButton_, &QPushButton::clicked, this, &TicketInspector::verifyOwnership);
    buttonLayout->addWidget(verifyButton_, 1);
    
    contentMainLayout->addLayout(buttonLayout);

    // Result Panel
    resultPanel_ = new QWidget(contentWrapper);
    resultPanel_->setStyleSheet("QWidget { background-color: white; border: 1px solid #e0e0e0; border-radius: 12px; }");
    auto resultLayout = new QVBoxLayout(resultPanel_);
    resultLayout->setContentsMargins(24, 24, 24, 24);
    resultLayout->setSpacing(10);
    
    auto resultTopLayout = new QHBoxLayout();
    resultIconLabel_ = new QLabel(resultPanel_);
    resultIconLabel_->setFixedSize(48, 48);
    resultIconLabel_->setAlignment(Qt::AlignCenter);
    resultIconLabel_->setStyleSheet("background: transparent; border: none;");
    resultTopLayout->addWidget(resultIconLabel_);
    
    resultTextLabel_ = new QLabel("Awaiting verification...", resultPanel_);
    resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: 600; color: #666; background: transparent; border: none;");
    resultTextLabel_->setWordWrap(true);
    resultTopLayout->addWidget(resultTextLabel_, 1);
    
    resultLayout->addLayout(resultTopLayout);
    
    detailsLabel_ = new QLabel("Load both QR codes and click 'Verify Ownership' to check if the ticket belongs to the user.", resultPanel_);
    detailsLabel_->setStyleSheet("font-size: 13px; color: #666; background: transparent; border: none;");
    detailsLabel_->setWordWrap(true);
    resultLayout->addWidget(detailsLabel_);
    
    resultPanel_->setVisible(true);
    contentMainLayout->addWidget(resultPanel_);
    
    scrollArea->setWidget(contentWrapper);
    mainLayout->addWidget(scrollArea);
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
        pitStatusLabel_->setStyleSheet("font-size: 13px; color: #eb0000; font-weight: 600;");
        QMessageBox::warning(this, "Error", "Failed to decode QR code from image.\n\nMake sure the image contains a valid QR code.");
    } else {
        pitStatusLabel_->setText("Status: QR Code Loaded ✓");
        pitStatusLabel_->setStyleSheet("font-size: 13px; color: #27ae60; font-weight: 600;");
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
        ticketStatusLabel_->setStyleSheet("font-size: 13px; color: #eb0000; font-weight: 600;");
        QMessageBox::warning(this, "Error", "Failed to decode QR code from image.\n\nMake sure the image contains a valid QR code.");
    } else {
        ticketStatusLabel_->setText("Status: QR Code Loaded ✓");
        ticketStatusLabel_->setStyleSheet("font-size: 13px; color: #27ae60; font-weight: 600;");
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
    pitStatusLabel_->setStyleSheet("font-size: 13px; color: #666;");
    ticketStatusLabel_->setText("Status: Waiting");
    ticketStatusLabel_->setStyleSheet("font-size: 13px; color: #666;");
    
    verifyButton_->setEnabled(false);
    
    resultIconLabel_->clear();
    resultTextLabel_->setText("Awaiting verification...");
    resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: 600; color: #666;");
    detailsLabel_->setText("Load both QR codes and click 'Verify Ownership' to check if the ticket belongs to the user.");
    detailsLabel_->setStyleSheet("font-size: 13px; color: #666; margin-top: 10px;");
    resultPanel_->setStyleSheet("QWidget { background-color: white; border: 1px solid #e0e0e0; border-radius: 12px; padding: 24px; }");
}

void TicketInspector::updateVerificationStatus(const TicketOwnership::VerificationResult& result)
{
    if (result.keysMatch && result.pitParsed && result.ticketParsed) {
        // Success - Keys Match
        resultPanel_->setStyleSheet("QWidget { background-color: #d5f4e6; border: 2px solid #27ae60; border-radius: 12px; padding: 24px; }");
        resultIconLabel_->setText("✓");
        resultIconLabel_->setStyleSheet("font-size: 36px; color: #27ae60; font-weight: 600;");
        resultTextLabel_->setText("VALID: Ticket Belongs to User");
        resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: 600; color: #27ae60;");
        
        QString details = QString("• Booking Reference: %1\n"
                                  "• PIT Parsed: ✓\n"
                                  "• Ticket Parsed: ✓\n"
                                  "• Public Key Hashes Match: ✓\n"
                                  "• Verification: PASSED");
        details = details.arg(result.bookingReference);
        
        detailsLabel_->setText(details);
        detailsLabel_->setStyleSheet("font-size: 13px; color: #27ae60; margin-top: 10px; font-weight: 600;");
        
    } else {
        // Failure - Keys Don't Match or Parse Error
        resultPanel_->setStyleSheet("QWidget { background-color: #ffe6e6; border: 2px solid #eb0000; border-radius: 12px; padding: 24px; }");
        resultIconLabel_->setText("✗");
        resultIconLabel_->setStyleSheet("font-size: 36px; color: #eb0000; font-weight: 600;");
        resultTextLabel_->setText("INVALID: Ticket Does NOT Belong to User");
        resultTextLabel_->setStyleSheet("font-size: 18px; font-weight: 600; color: #eb0000;");
        
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
        detailsLabel_->setStyleSheet("font-size: 13px; color: #eb0000; margin-top: 10px; font-weight: 600;");
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