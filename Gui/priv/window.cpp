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

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch();      // push nav bar to bottom
    layout->addWidget(navBar_);
    setLayout(layout);
}