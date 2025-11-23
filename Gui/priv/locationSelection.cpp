#include "locationSelection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

LocationSelection::LocationSelection(QWidget* parent)
    : QWidget(parent)
{
    // Remove default styling - parent will handle it
    setStyleSheet("");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(0);

    // Departure with modern styling
    auto departureLabel = new QLabel("From:", this);
    departureLabel->setStyleSheet("font-weight: 600; font-size: 13px; color: #333; margin-bottom: 4px;");
    departureEdit_ = new QLineEdit(this);
    departureEdit_->setPlaceholderText("Enter departure station");
    departureEdit_->setMinimumHeight(22);
    departureEdit_->setStyleSheet(
        "QLineEdit { "
        "  padding: 10px 14px; border: 2px solid #e0e0e0; border-radius: 6px; "
        "  color: #000; font-size: 14px; background-color: #fafafa; "
        "} "
        "QLineEdit:focus { "
        "  border: 2px solid #eb0000; background-color: white; "
        "}"
    );

    // Destination with modern styling
    auto destinationLabel = new QLabel("To:", this);
    destinationLabel->setStyleSheet("font-weight: 600; font-size: 13px; color: #333; margin-top: 12px; margin-bottom: 4px;");
    destinationEdit_ = new QLineEdit(this);
    destinationEdit_->setPlaceholderText("Enter destination station");
    destinationEdit_->setMinimumHeight(22);
    destinationEdit_->setStyleSheet(
        "QLineEdit { "
        "  padding: 10px 14px; border: 2px solid #e0e0e0; border-radius: 6px; "
        "  color: #000; font-size: 14px; background-color: #fafafa; "
        "} "
        "QLineEdit:focus { "
        "  border: 2px solid #eb0000; background-color: white; "
        "}"
    );

    // Date and Time in one row
    auto dateTimeLayout = new QHBoxLayout();
    dateTimeLayout->setSpacing(12);
    dateTimeLayout->setContentsMargins(0, 12, 0, 0);

    // Date
    auto dateContainer = new QVBoxLayout();
    dateContainer->setSpacing(0);
    dateContainer->setContentsMargins(0, 0, 0, 0);
    auto dateLabel = new QLabel("Date:", this);
    dateLabel->setStyleSheet("font-weight: 600; font-size: 13px; color: #333; margin-bottom: 4px;");
    dateEdit_ = new QDateEdit(QDate::currentDate(), this);
    dateEdit_->setCalendarPopup(true);
    dateEdit_->setDisplayFormat("dd/MM/yyyy");
    dateEdit_->setMinimumHeight(42);
    dateEdit_->setMinimumWidth(140);
    dateEdit_->setStyleSheet(
        "QDateEdit { "
        "  padding: 10px 30px 10px 14px; border: 2px solid #e0e0e0; border-radius: 6px; "
        "  color: #000; font-size: 14px; background-color: #fafafa; "
        "} "
        "QDateEdit:focus { "
        "  border: 2px solid #eb0000; background-color: white; "
        "} "
        "QDateEdit::down-arrow { "
        "  width: 12px; "
        "  height: 8px; "
        "  image: none; "
        "  border-left: 5px solid transparent; "
        "  border-right: 5px solid transparent; "
        "  border-top: 6px solid #666; "
        "} "
        "QDateEdit::down-arrow:hover { "
        "  border-top: 6px solid #eb0000; "
        "} "
        "QCalendarWidget QToolButton { "
        "  color: black; "
        "  background-color: white; "
        "  font-size: 14px; "
        "  font-weight: 600; "
        "} "
        "QCalendarWidget QMenu { "
        "  color: black; "
        "  background-color: white; "
        "} "
        "QCalendarWidget QSpinBox { "
        "  color: black; "
        "  background-color: white; "
        "  selection-background-color: #eb0000; "
        "  selection-color: white; "
        "} "
        "QCalendarWidget QWidget#qt_calendar_navigationbar { "
        "  background-color: #f5f5f5; "
        "} "
        "QCalendarWidget QTableView { "
        "  selection-background-color: #eb0000; "
        "  selection-color: white; "
        "} "
        "QCalendarWidget QWidget { "
        "  alternate-background-color: #f5f5f5; "
        "} "
        "QCalendarWidget QAbstractItemView:enabled { "
        "  color: black; "
        "  background-color: white; "
        "} "
        "QCalendarWidget QAbstractItemView:disabled { "
        "  color: #999; "
        "}"
    );
    dateContainer->addWidget(dateLabel);
    dateContainer->addWidget(dateEdit_);

    // Time with modern styling
    auto timeContainer = new QVBoxLayout();
    timeContainer->setSpacing(0);
    timeContainer->setContentsMargins(0, 0, 0, 0);
    auto timeLabel = new QLabel("Time:", this);
    timeLabel->setStyleSheet("font-weight: 600; font-size: 13px; color: #333; margin-bottom: 4px;");
    timeEdit_ = new QTimeEdit(QTime::currentTime(), this);
    timeEdit_->setDisplayFormat("hh:mm");
    timeEdit_->setMinimumHeight(42);
    timeEdit_->setMinimumWidth(100);
    timeEdit_->setStyleSheet(
        "QTimeEdit { "
        "  padding: 10px 14px; border: 2px solid #e0e0e0; border-radius: 6px; "
        "  color: #000; font-size: 14px; background-color: #fafafa; "
        "} "
        "QTimeEdit:focus { "
        "  border: 2px solid #eb0000; background-color: white; "
        "} "
        "QTimeEdit::drop-down { "
        "  border: none; width: 25px; "
        "}"
    );
    timeContainer->addWidget(timeLabel);
    timeContainer->addWidget(timeEdit_);

    dateTimeLayout->addLayout(dateContainer, 1);
    dateTimeLayout->addLayout(timeContainer, 1);

    // Go button with modern SBB styling
    goButton_ = new QPushButton("Book Train", this);
    goButton_->setMinimumHeight(65);
    goButton_->setCursor(Qt::PointingHandCursor);
    goButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #eb0000; color: white; font-weight: 600; "
        "  font-size: 15px; border: none; border-radius: 6px; margin-top: 16px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #c00000; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #a00000; "
        "}"
    );

    // Connect button to emit signal with current values
    connect(goButton_, &QPushButton::clicked, this, [this]() {
        QString departure = departureEdit_->text().trimmed();
        QString destination = destinationEdit_->text().trimmed();
        
        // Validate inputs
        if (departure.isEmpty()) {
            departureEdit_->setStyleSheet(
                "QLineEdit { "
                "  padding: 10px 14px; border: 2px solid #eb0000; border-radius: 6px; "
                "  color: #000; font-size: 14px; background-color: #fafafa; "
                "} "
                "QLineEdit:focus { "
                "  border: 2px solid #eb0000; background-color: white; "
                "}"
            );
            return;
        }
        
        if (destination.isEmpty()) {
            destinationEdit_->setStyleSheet(
                "QLineEdit { "
                "  padding: 10px 14px; border: 2px solid #eb0000; border-radius: 6px; "
                "  color: #000; font-size: 14px; background-color: #fafafa; "
                "} "
                "QLineEdit:focus { "
                "  border: 2px solid #eb0000; background-color: white; "
                "}"
            );
            return;
        }
        
        // Reset styling to normal
        QString normalStyle = 
            "QLineEdit { "
            "  padding: 10px 14px; border: 2px solid #e0e0e0; border-radius: 6px; "
            "  color: #000; font-size: 14px; background-color: #fafafa; "
            "} "
            "QLineEdit:focus { "
            "  border: 2px solid #eb0000; background-color: white; "
            "}";
        departureEdit_->setStyleSheet(normalStyle);
        destinationEdit_->setStyleSheet(normalStyle);
        
        emit searchClicked(
            departure,
            destination,
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