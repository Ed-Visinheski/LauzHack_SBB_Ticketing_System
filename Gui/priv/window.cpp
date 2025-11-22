#include "window.h"
#include <QPalette>
#include <QVBoxLayout>

Window::Window(QWidget* parent)
    : QWidget(parent)
{
    // Fixed window size (width x height)
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true); 
    setPalette(pal);

    // Top bar (always visible)
    topBar_ = new TopBar(this);
    topBar_->setFixedHeight(64);

    // Create three persistent pages with different background colors
    homePage_ = new QWidget(this);
    homePage_->setStyleSheet("background-color: #e8f5e9;"); // light green

    ticketPage_ = new QWidget(this);
    ticketPage_->setStyleSheet("background-color: #e3f2fd;"); // light blue

    idPage_ = new QWidget(this);
    idPage_->setStyleSheet("background-color: #fff3e0;"); // light orange

    // Stacked widget to hold pages (preserves state when switching)
    stacked_ = new QStackedWidget(this);
    stacked_->addWidget(homePage_);    // index 0
    stacked_->addWidget(ticketPage_);  // index 1
    stacked_->addWidget(idPage_);      // index 2
    stacked_->setCurrentIndex(0);      // start with home

    // Nav bar at bottom
    navBar_ = new NavBar(this);
    navBar_->setFixedHeight(64);

    // Connect nav button signals to switch pages
    connect(navBar_, &NavBar::homeClicked, this, [this]() { stacked_->setCurrentIndex(0); });
    connect(navBar_, &NavBar::ticketClicked, this, [this]() { stacked_->setCurrentIndex(1); });
    connect(navBar_, &NavBar::idClicked, this, [this]() { stacked_->setCurrentIndex(2); });

    // Main layout: topBar, stacked pages (stretch), navBar
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topBar_);
    mainLayout->addWidget(stacked_, 1); // stretch to fill middle
    mainLayout->addWidget(navBar_);

    setLayout(mainLayout);
}