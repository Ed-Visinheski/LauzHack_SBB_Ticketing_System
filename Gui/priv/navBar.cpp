#include "navBar.h"
#include <QPushButton>
#include <QHBoxLayout>

#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

NavBar::NavBar(QWidget* parent)
    : QWidget(parent)
{
    // Light grey background
    setStyleSheet("background-color: #f0f0f0;");

    homeButton_ = new QPushButton(QIcon(QString("Icons/magnifying-glass_24px.png")), QString{}, this);
    ticketButton_ = new QPushButton(QIcon("Icons/ticket_24px.png"), QString{}, this);
    idButton_ = new QPushButton(QIcon("Icons/user_24px.png"), QString{}, this);

    // Optional: set uniform size for buttons
    homeButton_->setFixedSize(80, 40);
    ticketButton_->setFixedSize(80, 40);
    idButton_->setFixedSize(80, 40);

    
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
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