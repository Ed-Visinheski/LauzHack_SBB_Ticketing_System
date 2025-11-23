#include "bookingReference.h"
#include "qrCodeGen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QScrollArea>
#include <QDateTime>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QMessageBox>
#include <rnp/rnp.h>
#include <rnp/rnp_err.h>

// ============ TicketCard Implementation ============

TicketCard::TicketCard(const TicketInfo& ticket, QWidget* parent)
    : QWidget(parent)
    , ticket_(ticket)
{

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);
    
    // Modern card styling with rounded corners and shadow
    setStyleSheet(
        "TicketCard { "
        "  background-color: white; "
        "  border-radius: 12px; "
        "  border: 1px solid #e0e0e0; "
        "}"
    );

    // Header: Booking reference and status
    auto headerLayout = new QHBoxLayout();
    auto refLabel = new QLabel("Ref: " + ticket_.bookingReference(), this);
    refLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #666; border: none; background: transparent;");
    
    headerLayout->addWidget(refLabel);
    headerLayout->addStretch();

    // Route info with arrow
    auto routeLayout = new QHBoxLayout();
    routeLayout->setSpacing(12);
    
    auto fromLabel = new QLabel(ticket_.departure(), this);
    fromLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #000; border: none; background: transparent;");
    
    auto arrowLabel = new QLabel("→", this);
    arrowLabel->setStyleSheet("font-size: 20px; color: #eb0000; border: none; background: transparent;");
    
    auto toLabel = new QLabel(ticket_.destination(), this);
    toLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #000; border: none; background: transparent;");
    
    routeLayout->addWidget(fromLabel);
    routeLayout->addWidget(arrowLabel);
    routeLayout->addWidget(toLabel);
    routeLayout->addStretch();

    // Date and time row with modern icons
    auto dateTimeLayout = new QHBoxLayout();
    dateTimeLayout->setSpacing(20);
    
    auto dateIcon = new QLabel("📅", this);
    dateIcon->setStyleSheet("font-size: 16px; border: none; background: transparent;");
    auto dateLabel = new QLabel(ticket_.date().toString("MMM d, yyyy"), this);
    dateLabel->setStyleSheet("font-size: 15px; color: #666; border: none; background: transparent;");
    
    auto timeIcon = new QLabel("🕐", this);
    timeIcon->setStyleSheet("font-size: 16px; border: none; background: transparent;");
    auto timeLabel = new QLabel(ticket_.time().toString("hh:mm"), this);
    timeLabel->setStyleSheet("font-size: 15px; color: #666; border: none; background: transparent;");
    
    dateTimeLayout->addWidget(dateIcon);
    dateTimeLayout->addWidget(dateLabel);
    dateTimeLayout->addSpacing(10);
    dateTimeLayout->addWidget(timeIcon);
    dateTimeLayout->addWidget(timeLabel);
    dateTimeLayout->addStretch();

    // Divider line
    auto divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #e0e0e0; border: none;");
    divider->setFixedHeight(1);

    // QR Code button with modern SBB styling
    auto qrButton = new QPushButton("View QR Code", this);
    qrButton->setFixedHeight(44);
    qrButton->setCursor(Qt::PointingHandCursor);
    qrButton->setStyleSheet(
        "QPushButton { "
        "  background-color: #eb0000; color: white; font-weight: 600; "
        "  font-size: 14px; border: none; border-radius: 8px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #c00000; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #a00000; "
        "}"
    );
    connect(qrButton, &QPushButton::clicked, this, [this]() {
        emit qrCodeRequested(ticket_);
    });

    // Add to main layout
    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(routeLayout);
    mainLayout->addLayout(dateTimeLayout);
    mainLayout->addWidget(divider);
    mainLayout->addWidget(qrButton);

    setLayout(mainLayout);
}

// ============ BookingReference Implementation ============

BookingReference::BookingReference(QWidget* parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Scroll area for tickets with modern styling
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setStyleSheet(
        "QScrollArea { border: none; background-color: transparent; } "
        "QScrollBar:vertical { "
        "  border: none; background: #e5e5e5; width: 10px; border-radius: 5px; "
        "} "
        "QScrollBar::handle:vertical { "
        "  background: #bbb; border-radius: 5px; min-height: 30px; "
        "} "
        "QScrollBar::handle:vertical:hover { "
        "  background: #999; "
        "} "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "  height: 0px; "
        "}"
    );

    // Container widget inside scroll area
    ticketContainer_ = new QWidget();
    ticketLayout_ = new QVBoxLayout(ticketContainer_);
    ticketLayout_->setContentsMargins(20, 20, 20, 20);
    ticketLayout_->setSpacing(16);
    ticketLayout_->addStretch();
    ticketContainer_->setLayout(ticketLayout_);
    
    scrollArea_->setWidget(ticketContainer_);

    // No tickets message with modern styling
    noTicketLabel_ = new QLabel("No tickets yet\n\nBook your first ticket from the Home page", this);
    noTicketLabel_->setStyleSheet("font-size: 16px; color: #999; padding: 60px; line-height: 1.6;");
    noTicketLabel_->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(scrollArea_, 1);
    mainLayout->addWidget(noTicketLabel_);

    setLayout(mainLayout);
    
    scrollArea_->hide();
    
    // Setup centered QR overlay
    setupQROverlay();
}

void BookingReference::setupQROverlay()
{
    // Create centered QR overlay
    qrOverlay_ = new QWidget(this);
    qrOverlay_->setStyleSheet("background-color: rgba(0, 0, 0, 0.95);");
    qrOverlay_->hide();
    
    auto overlayLayout = new QVBoxLayout(qrOverlay_);
    overlayLayout->setContentsMargins(40, 60, 40, 60);
    overlayLayout->setSpacing(24);
    
    overlayLayout->addStretch(1);
    
    // Title
    qrTitleLabel_ = new QLabel("Scan Ticket", qrOverlay_);
    qrTitleLabel_->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");
    qrTitleLabel_->setAlignment(Qt::AlignCenter);
    overlayLayout->addWidget(qrTitleLabel_);
    
    overlayLayout->addSpacing(20);
    
    // QR Code image (will be generated when ticket is shown)
    qrImageLabel_ = new QLabel(qrOverlay_);
    qrImageLabel_->setAlignment(Qt::AlignCenter);
    qrImageLabel_->setStyleSheet("background-color: white; padding: 20px; border-radius: 12px;");
    qrImageLabel_->setFixedSize(320, 320);
    
    overlayLayout->addWidget(qrImageLabel_, 0, Qt::AlignCenter);
    
    overlayLayout->addSpacing(20);
    
    // Download button
    downloadQRButton_ = new QPushButton("Download QR Code", qrOverlay_);
    downloadQRButton_->setFixedSize(200, 50);
    downloadQRButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #3498db; color: white; font-weight: bold; "
        "  font-size: 16px; border: none; border-radius: 8px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #2980b9; "
        "}"
    );
    connect(downloadQRButton_, &QPushButton::clicked, this, &BookingReference::downloadCurrentQRCode);
    
    // Close button
    auto closeButton = new QPushButton("Close", qrOverlay_);
    closeButton->setFixedSize(200, 50);
    closeButton->setStyleSheet(
        "QPushButton { "
        "  background-color: white; color: black; font-weight: bold; "
        "  font-size: 16px; border: none; border-radius: 8px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #f5f5f5; "
        "}"
    );
    connect(closeButton, &QPushButton::clicked, this, &BookingReference::hideQRCode);
    
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(downloadQRButton_);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    overlayLayout->addLayout(buttonLayout);
    
    overlayLayout->addStretch(1);
}

void BookingReference::showQRCode(const TicketInfo& ticket)
{
    // Store current ticket for download
    currentTicket_ = ticket;
    
    // Update title with booking reference
    qrTitleLabel_->setText("Ticket: " + ticket.bookingReference());
    
    try {
        if (!companyInfo_ || !companyInfo_->isValid()) {
            throw std::runtime_error("Company info not available");
        }
        
        // Get or generate timestamp for this ticket
        qint64 timestamp = ticket.timestamp();
        if (timestamp == 0) {
            timestamp = QDateTime::currentSecsSinceEpoch();
        }
        
        // Create data to sign: userPublicKey + bookingReference + timestamp
        QString dataToSign = ticket.userPublicKey() + ticket.bookingReference() + QString::number(timestamp);
        
        // If ticket already has signed data, use it; otherwise generate signature
        QString signatureHex = ticket.signedData();
        
        if (signatureHex.isEmpty()) {
            // Sign using company's private key
            rnp_ffi_t ffi = nullptr;
            if (rnp_ffi_create(&ffi, "GPG", "GPG") != RNP_SUCCESS || !ffi) {
                throw std::runtime_error("Failed to create RNP FFI for signing");
            }
            
            // Import company's private key
            QByteArray privateKeyBytes = companyInfo_->privateKey().toUtf8();
            rnp_input_t key_input = nullptr;
            if (rnp_input_from_memory(&key_input,
                                      reinterpret_cast<const uint8_t*>(privateKeyBytes.constData()),
                                      privateKeyBytes.size(),
                                      false) != RNP_SUCCESS) {
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("Failed to create input for company private key");
            }
            
            if (rnp_import_keys(ffi, key_input, RNP_LOAD_SAVE_SECRET_KEYS, nullptr) != RNP_SUCCESS) {
                rnp_input_destroy(key_input);
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("Failed to import company private key");
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
            
            rnp_op_sign_t sign_op = nullptr;
            if (rnp_op_sign_detached_create(&sign_op, ffi, data_input, sig_output) != RNP_SUCCESS) {
                rnp_output_destroy(sig_output);
                rnp_input_destroy(data_input);
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("Failed to create signing operation");
            }
            
            // Get company key
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
                    throw std::runtime_error("Failed to locate company key");
                }
            }
            rnp_identifier_iterator_destroy(it);
            
            if (!key) {
                rnp_op_sign_destroy(sign_op);
                rnp_output_destroy(sig_output);
                rnp_input_destroy(data_input);
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("No company key found");
            }
            
            if (rnp_op_sign_add_signature(sign_op, key, nullptr) != RNP_SUCCESS) {
                rnp_key_handle_destroy(key);
                rnp_op_sign_destroy(sign_op);
                rnp_output_destroy(sig_output);
                rnp_input_destroy(data_input);
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("Failed to add company signer");
            }
            
            if (rnp_op_sign_execute(sign_op) != RNP_SUCCESS) {
                rnp_key_handle_destroy(key);
                rnp_op_sign_destroy(sign_op);
                rnp_output_destroy(sig_output);
                rnp_input_destroy(data_input);
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("Company signing failed");
            }
            
            uint8_t* sig_buf = nullptr;
            size_t sig_len = 0;
            if (rnp_output_memory_get_buf(sig_output, &sig_buf, &sig_len, false) != RNP_SUCCESS) {
                rnp_key_handle_destroy(key);
                rnp_op_sign_destroy(sign_op);
                rnp_output_destroy(sig_output);
                rnp_input_destroy(data_input);
                rnp_ffi_destroy(ffi);
                throw std::runtime_error("Failed to get company signature");
            }
            
            QByteArray signature(reinterpret_cast<const char*>(sig_buf), sig_len);
            signatureHex = signature.toHex();
            
            rnp_key_handle_destroy(key);
            rnp_op_sign_destroy(sign_op);
            rnp_output_destroy(sig_output);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
        }
        
        // Create user's public key hash (first 16 chars of SHA-256)
        QByteArray userPublicKeyBytes = ticket.userPublicKey().toUtf8();
        QByteArray userKeyHash = QCryptographicHash::hash(userPublicKeyBytes, QCryptographicHash::Sha256);
        QString userPublicKeyHash = userKeyHash.toHex().left(16);
        
        // Create ticket QR code: TICKET:bookingRef:timestamp:userPubKeyHash:companySignature
        QString ticketData = QString("TICKET:%1:%2:%3:%4")
            .arg(ticket.bookingReference())
            .arg(timestamp)
            .arg(userPublicKeyHash)
            .arg(signatureHex);
        
        QImage qrImage = QRCodeGenerator::generateQRCode(ticketData, 280);
        QPixmap qrPixmap = QPixmap::fromImage(qrImage);
        qrImageLabel_->setPixmap(qrPixmap);
        
    } catch (const std::exception& e) {
        qWarning() << "Failed to create signed ticket QR:" << e.what();
        
        // Fallback to simple booking reference QR
        QImage qrImage = QRCodeGenerator::generateQRCode(ticket.bookingReference(), 280);
        QPixmap qrPixmap = QPixmap::fromImage(qrImage);
        qrImageLabel_->setPixmap(qrPixmap);
    }
    
    // Show overlay centered on the page
    qrOverlay_->setGeometry(0, 0, width(), height());
    qrOverlay_->raise();
    qrOverlay_->show();
}

void BookingReference::hideQRCode()
{
    qrOverlay_->hide();
}

void BookingReference::downloadCurrentQRCode()
{
    if (qrImageLabel_->pixmap().isNull()) {
        QMessageBox::warning(this, "No QR Code", "No QR code is currently displayed.");
        return;
    }
    
    if (!currentTicket_.isValid()) {
        QMessageBox::warning(this, "Error", "Invalid ticket information.");
        return;
    }
    
    // Generate filename with booking reference and timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString defaultFileName = QString("Ticket_%1_%2.png")
        .arg(currentTicket_.bookingReference())
        .arg(timestamp);
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Ticket QR Code",
        defaultFileName,
        "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Save the QR code pixmap
    QPixmap pixmap = qrImageLabel_->pixmap();
    if (pixmap.save(fileName)) {
        QMessageBox::information(this, "Success", 
            QString("Ticket QR code saved successfully!\nBooking: %1")
            .arg(currentTicket_.bookingReference()));
    } else {
        QMessageBox::warning(this, "Error", "Failed to save QR code image.");
    }
}

void BookingReference::addTicket(const TicketInfo& ticket)
{
    if (!ticket.isValid()) {
        return;
    }

    // Hide no ticket message, show scroll area
    noTicketLabel_->hide();
    scrollArea_->show();

    // Create new ticket card
    auto ticketCard = new TicketCard(ticket, ticketContainer_);
    
    // Connect QR code signal
    connect(ticketCard, &TicketCard::qrCodeRequested, this, &BookingReference::showQRCode);
    
    // Insert at the top (before the stretch)
    ticketLayout_->insertWidget(0, ticketCard);
}

void BookingReference::clearTickets()
{
    // Remove all ticket cards
    while (ticketLayout_->count() > 1) { // Keep the stretch
        auto item = ticketLayout_->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // Show no ticket message
    scrollArea_->hide();
    noTicketLabel_->show();
}