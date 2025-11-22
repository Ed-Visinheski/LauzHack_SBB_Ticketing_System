#include "navBar.h"
#include <QPushButton>
#include <QHBoxLayout>

NavBar::NavBar(QWidget* parent)
    : QWidget(parent)
{
    // Light grey background
    setStyleSheet("background-color: #f0f0f0;");

    // Buttons A, B, C
    auto btnA = new QPushButton("A", this);
    auto btnB = new QPushButton("B", this);
    auto btnC = new QPushButton("C", this);

    // Optional: set uniform size for buttons
    btnA->setFixedSize(80, 40);
    btnB->setFixedSize(80, 40);
    btnC->setFixedSize(80, 40);

    // Layout with equal spacing: stretch segments around and between buttons
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12); // left, top, right, bottom padding
    layout->setSpacing(0);

    layout->addStretch(1);
    layout->addWidget(btnA, 0, Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(btnB, 0, Qt::AlignCenter);
    layout->addStretch(1);
    layout->addWidget(btnC, 0, Qt::AlignCenter);
    layout->addStretch(1);

    setLayout(layout);
}