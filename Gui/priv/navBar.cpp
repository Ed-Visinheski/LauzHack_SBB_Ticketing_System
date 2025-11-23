#include "navBar.h"
#include <QPushButton>
#include <QHBoxLayout>

#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

NavBar::NavBar(QWidget* parent)
    : QWidget(parent)
{
    // White background with top border
    setStyleSheet("background-color: white; border-top: 1px solid #e0e0e0;");

    homeButton_ = new QPushButton(QIcon(QString("Icons/magnifying-glass_24px.png")), QString{}, this);
    ticketButton_ = new QPushButton(QIcon("Icons/ticket_24px.png"), QString{}, this);
    idButton_ = new QPushButton(QIcon("Icons/user_24px.png"), QString{}, this);

    // Modern button styling with better sizing
    QString buttonStyle = 
        "QPushButton { "
        "  background-color: transparent; "
        "  border: none; "
        "  border-radius: 8px; "
        "  padding: 8px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #f5f5f5; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #e0e0e0; "
        "}";
    
    homeButton_->setStyleSheet(buttonStyle);
    ticketButton_->setStyleSheet(buttonStyle);
    idButton_->setStyleSheet(buttonStyle);
    
    homeButton_->setFixedSize(60, 50);
    ticketButton_->setFixedSize(60, 50);
    idButton_->setFixedSize(60, 50);
    
    homeButton_->setCursor(Qt::PointingHandCursor);
    ticketButton_->setCursor(Qt::PointingHandCursor);
    idButton_->setCursor(Qt::PointingHandCursor);

    
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 8, 20, 8);
    layout->setSpacing(0);

    layout->addStretch(1);
    layout->addWidget(homeButton_, 0, Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(ticketButton_, 0, Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(idButton_, 0, Qt::AlignCenter);
    layout->addStretch(1);

    setLayout(layout);

    // Forward button clicks as signals
    connect(homeButton_, &QPushButton::clicked, this, &NavBar::homeClicked);
    connect(ticketButton_, &QPushButton::clicked, this, &NavBar::ticketClicked);
    connect(idButton_, &QPushButton::clicked, this, &NavBar::idClicked);
}