#pragma once
#include "definitions.h"
#include <QWidget>
#include "navBar.h"
#include "topBar.h"

class Window : public QWidget
{
public:
    explicit Window(QWidget* parent = nullptr);

private:
    NavBar* navBar_ = nullptr; //Bottom bar
    TopBar* topBar_ = nullptr; //top bar
}; 