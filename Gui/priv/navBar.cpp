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

    // Buttons A, B, C
    auto homeButton = new QPushButton(QIcon(QString("icons/magnifying-glass_24px.png")), QString{}, this);
    auto ticketButton = new QPushButton(QIcon("icons/ticket_24px.png"), QString{}, this);
    auto idButton = new QPushButton(QIcon("icons/user_24px.png"), QString{}, this);

    // Optional: set uniform size for buttons
    homeButton->setFixedSize(80, 40);
    ticketButton->setFixedSize(80, 40);
    idButton->setFixedSize(80, 40);

    // Layout with equal spacing: stretch segments around and between buttons
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12); // left, top, right, bottom padding
    layout->setSpacing(0);

    layout->addStretch(1);
    layout->addWidget(homeButton, 0, Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(ticketButton, 0, Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(idButton, 0, Qt::AlignCenter);
    layout->addStretch(1);

    setLayout(layout);
}