#include "bookingReference.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

BookingReference::BookingReference(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: white; border-radius: 8px;");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // Title
    auto titleLabel = new QLabel("Your Ticket", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #ec0001;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Booking Reference (large, centered)
    referenceLabel_ = new QLabel(this);
    referenceLabel_->setStyleSheet(
        "font-size: 32px; font-weight: bold; color: black; "
        "background-color: #f5f5f5; padding: 20px; border-radius: 8px;"
    );
    referenceLabel_->setAlignment(Qt::AlignCenter);
    referenceLabel_->hide();

    // Separator line
    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #ccc;");

    // Trip details container
    auto detailsLayout = new QVBoxLayout();
    detailsLayout->setSpacing(12);

    // Departure
    auto depContainer = new QVBoxLayout();
    auto depTitle = new QLabel("From:", this);
    depTitle->setStyleSheet("font-size: 12px; color: #666;");
    departureLabel_ = new QLabel(this);
    departureLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: black;");
    depContainer->addWidget(depTitle);
    depContainer->addWidget(departureLabel_);

    // Destination
    auto destContainer = new QVBoxLayout();
    auto destTitle = new QLabel("To:", this);
    destTitle->setStyleSheet("font-size: 12px; color: #666;");
    destinationLabel_ = new QLabel(this);
    destinationLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: black;");
    destContainer->addWidget(destTitle);
    destContainer->addWidget(destinationLabel_);

    // Date and time in one row
    auto dateTimeRow = new QHBoxLayout();
    dateTimeRow->setSpacing(20);

    auto dateContainer = new QVBoxLayout();
    auto dateTitle = new QLabel("Date:", this);
    dateTitle->setStyleSheet("font-size: 12px; color: #666;");
    dateLabel_ = new QLabel(this);
    dateLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: black;");
    dateContainer->addWidget(dateTitle);
    dateContainer->addWidget(dateLabel_);

    auto timeContainer = new QVBoxLayout();
    auto timeTitle = new QLabel("Time:", this);
    timeTitle->setStyleSheet("font-size: 12px; color: #666;");
    timeLabel_ = new QLabel(this);
    timeLabel_->setStyleSheet("font-size: 16px; font-weight: bold; color: black;");
    timeContainer->addWidget(timeTitle);
    timeContainer->addWidget(timeLabel_);

    dateTimeRow->addLayout(dateContainer);
    dateTimeRow->addLayout(timeContainer);

    detailsLayout->addLayout(depContainer);
    detailsLayout->addLayout(destContainer);
    detailsLayout->addLayout(dateTimeRow);

    // No ticket message (shown when no booking)
    noTicketLabel_ = new QLabel("No active booking.\nBook a ticket from the Home page.", this);
    noTicketLabel_->setStyleSheet("font-size: 16px; color: #666;");
    noTicketLabel_->setAlignment(Qt::AlignCenter);

    // Add to main layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(referenceLabel_);
    mainLayout->addWidget(separator);
    mainLayout->addLayout(detailsLayout);
    mainLayout->addWidget(noTicketLabel_);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Initially hide ticket details
    referenceLabel_->hide();
    separator->hide();
    departureLabel_->hide();
    destinationLabel_->hide();
    dateLabel_->hide();
    timeLabel_->hide();
}

void BookingReference::updateTicket(const TicketInfo& ticket)
{
    if (!ticket.isValid()) {
        // No valid ticket, show placeholder
        referenceLabel_->hide();
        departureLabel_->hide();
        destinationLabel_->hide();
        dateLabel_->hide();
        timeLabel_->hide();
        noTicketLabel_->show();
        return;
    }

    // Update labels with ticket data
    referenceLabel_->setText(ticket.bookingReference());
    departureLabel_->setText(ticket.departure());
    destinationLabel_->setText(ticket.destination());
    dateLabel_->setText(ticket.date().toString("dddd, MMMM d, yyyy"));
    timeLabel_->setText(ticket.time().toString("hh:mm"));

    // Show ticket details, hide placeholder
    referenceLabel_->show();
    departureLabel_->show();
    destinationLabel_->show();
    dateLabel_->show();
    timeLabel_->show();
    noTicketLabel_->hide();
}