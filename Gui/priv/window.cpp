#include "window.h"
#include <QPalette>
#include <QVBoxLayout>

Window::Window(QWidget* parent)
    : QWidget(parent)
{
    // Fixed window size (width x height)
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    // White background
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    // Create nav bar and place it at the bottom using a layout
    navBar_ = new NavBar(this);
    navBar_->setFixedHeight(64);
    
    topBar_ = new TopBar(this);
    topBar_->setFixedHeight(64);

    auto pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    auto topBarLayout = new QVBoxLayout(this);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(0);
    topBarLayout->addWidget(topBar_);

    auto navBarLayout = new QVBoxLayout(this);
    navBarLayout->setContentsMargins(0, 0, 0, 0);
    navBarLayout->setSpacing(0);
    navBarLayout->addStretch();      // push nav bar to bottom
    navBarLayout->addWidget(navBar_);

    pageLayout->addLayout(topBarLayout);
    pageLayout->addLayout(navBarLayout);


    setLayout(pageLayout);
}