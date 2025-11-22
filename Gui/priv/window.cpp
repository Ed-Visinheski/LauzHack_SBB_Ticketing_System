#include "window.h"
#include <QPalette>

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
}
