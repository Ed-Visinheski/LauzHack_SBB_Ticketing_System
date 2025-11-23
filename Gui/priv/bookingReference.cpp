#include "bookingReference.h"
#include "qrCodeGen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QScrollArea>

// ============ TicketCard Implementation ============

TicketCard::TicketCard(const TicketInfo& ticket, QWidget* parent)
    : QWidget(parent)
    , ticket_(ticket)
{

    auto mainLayout = new QVBoxLayout(this);
    // mainLayout->setContentsMargins(16, 16, 16, 16);
    // mainLayout->setSpacing(12);

    // Header: Booking reference and status
    auto headerLayout = new QHBoxLayout();
    auto refLabel = new QLabel("Ref: " + ticket_.bookingReference(), this);
    refLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #ec0001; border: none;");
    
    headerLayout->addWidget(refLabel);
    headerLayout->addStretch();

    // Route info with arrow
    auto routeLayout = new QHBoxLayout();
    routeLayout->setSpacing(12);
    
    auto fromLabel = new QLabel(ticket_.departure(), this);
    fromLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; border: none;");
    
    auto arrowLabel = new QLabel("→", this);
    arrowLabel->setStyleSheet("font-size: 20px; color: #999; border: none;");
    
    auto toLabel = new QLabel(ticket_.destination(), this);
    toLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; border: none;");
    
    routeLayout->addWidget(fromLabel);
    routeLayout->addWidget(arrowLabel);
    routeLayout->addWidget(toLabel);
    routeLayout->addStretch();

    // Date and time row
    auto dateTimeLayout = new QHBoxLayout();
    dateTimeLayout->setSpacing(20);
    
    auto dateIcon = new QLabel("📅", this);
    dateIcon->setStyleSheet("font-size: 14px; border: none;");
    auto dateLabel = new QLabel(ticket_.date().toString("MMM d, yyyy"), this);
    dateLabel->setStyleSheet("font-size: 14px; color: #666; border: none;");
    
    auto timeIcon = new QLabel("🕐", this);
    timeIcon->setStyleSheet("font-size: 14px; border: none;");
    auto timeLabel = new QLabel(ticket_.time().toString("hh:mm"), this);
    timeLabel->setStyleSheet("font-size: 14px; color: #666; border: none;");
    
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

    // QR Code button
    auto qrButton = new QPushButton("View QR Code", this);
    qrButton->setFixedHeight(36);
    qrButton->setStyleSheet(
        "QPushButton { "
        "  background-color: #ec0001; color: white; font-weight: bold; "
        "  font-size: 13px; border: none; border-radius: 6px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #d00001; "
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

    // Title
    auto titleLabel = new QLabel("My Tickets", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333; padding: 16px;");

    // Scroll area for tickets
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setStyleSheet(
        "QScrollArea { border: none; background-color: transparent; } "
        "QScrollBar:vertical { "
        "  border: none; background: #f5f5f5; width: 8px; border-radius: 4px; "
        "} "
        "QScrollBar::handle:vertical { "
        "  background: #ccc; border-radius: 4px; min-height: 20px; "
        "} "
        "QScrollBar::handle:vertical:hover { "
        "  background: #999; "
        "}"
    );

    // Container widget inside scroll area
    ticketContainer_ = new QWidget();
    ticketLayout_ = new QVBoxLayout(ticketContainer_);
    ticketLayout_->setContentsMargins(16, 0, 16, 16);
    ticketLayout_->setSpacing(16);
    ticketLayout_->addStretch();
    ticketContainer_->setLayout(ticketLayout_);
    
    scrollArea_->setWidget(ticketContainer_);

    // No tickets message
    noTicketLabel_ = new QLabel("No tickets yet\n\nBook your first ticket from the Home page", this);
    noTicketLabel_->setStyleSheet("font-size: 15px; color: #999; padding: 40px;");
    noTicketLabel_->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(titleLabel);
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
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    overlayLayout->addLayout(buttonLayout);
    
    overlayLayout->addStretch(1);
}

void BookingReference::showQRCode(const TicketInfo& ticket)
{
    // Update title with booking reference
    qrTitleLabel_->setText("Ticket: " + ticket.bookingReference());
    
    // Generate QR code from booking reference
    QImage qrImage = QRCodeGenerator::generateQRCode(ticket.bookingReference(), 280);
    QPixmap qrPixmap = QPixmap::fromImage(qrImage);
    qrImageLabel_->setPixmap(qrPixmap);
    
    // Show overlay centered on the page
    qrOverlay_->setGeometry(0, 0, width(), height());
    qrOverlay_->raise();
    qrOverlay_->show();
}

void BookingReference::hideQRCode()
{
    qrOverlay_->hide();
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