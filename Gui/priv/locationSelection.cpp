#include "locationSelection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

LocationSelection::LocationSelection(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: white; border-radius: 8px;");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // Departure
    auto departureLabel = new QLabel("From:", this);
    departureLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: black;");
    departureEdit_ = new QLineEdit(this);
    departureEdit_->setPlaceholderText("Enter departure station");
    departureEdit_->setFixedHeight(40);
    departureEdit_->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; color: black;");

    // Destination
    auto destinationLabel = new QLabel("To:", this);
    destinationLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: black;");
    destinationEdit_ = new QLineEdit(this);
    destinationEdit_->setPlaceholderText("Enter destination station");
    destinationEdit_->setFixedHeight(40);
    destinationEdit_->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; color: black;");

    // Date and Time in one row
    auto dateTimeLayout = new QHBoxLayout();
    dateTimeLayout->setSpacing(12);

    // Date
    auto dateContainer = new QVBoxLayout();
    auto dateLabel = new QLabel("Date:", this);
    dateLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: black;");
    dateEdit_ = new QDateEdit(QDate::currentDate(), this);
    dateEdit_->setCalendarPopup(true);
    dateEdit_->setFixedHeight(40);
    dateEdit_->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; color: black;");
    dateContainer->addWidget(dateLabel);
    dateContainer->addWidget(dateEdit_);

    // Time
    auto timeContainer = new QVBoxLayout();
    auto timeLabel = new QLabel("Time:", this);
    timeLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: black;");
    timeEdit_ = new QTimeEdit(QTime::currentTime(), this);
    timeEdit_->setFixedHeight(40);
    timeEdit_->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px; color: black;");
    timeContainer->addWidget(timeLabel);
    timeContainer->addWidget(timeEdit_);

    dateTimeLayout->addLayout(dateContainer, 1);
    dateTimeLayout->addLayout(timeContainer, 1);

    // Go button
    goButton_ = new QPushButton("Book Train", this);
    goButton_->setFixedHeight(48);
    goButton_->setStyleSheet(
        "background-color: #ec0001; color: white; font-weight: bold; "
        "font-size: 16px; border: none; border-radius: 4px;"
    );

    // Connect button to emit signal with current values
    connect(goButton_, &QPushButton::clicked, this, [this]() {
        emit searchClicked(
            departureEdit_->text(),
            destinationEdit_->text(),
            dateEdit_->date(),
            timeEdit_->time()
        );
    });

    // Add all to main layout
    mainLayout->addWidget(departureLabel);
    mainLayout->addWidget(departureEdit_);
    mainLayout->addWidget(destinationLabel);
    mainLayout->addWidget(destinationEdit_);
    mainLayout->addLayout(dateTimeLayout);
    mainLayout->addWidget(goButton_);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void LocationSelection::clear()
{
    departureEdit_->clear();
    destinationEdit_->clear();
    dateEdit_->setDate(QDate::currentDate());
    timeEdit_->setTime(QTime::currentTime());
}