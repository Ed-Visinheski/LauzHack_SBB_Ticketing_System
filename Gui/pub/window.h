#pragma once
#include "definitions.h"
#include <QWidget>
#include "topBar.h"


class Window : public QWidget
{
public:
    explicit Window(QWidget* parent = nullptr);

private:
    NavBar* navBar_ = nullptr;
    TopBar* topBar_ = nullptr;
};