#pragma once
#include "definitions.h"
#include <QWidget>
#include <QStackedWidget>
#include "navBar.h"
#include "topBar.h"
#include "locationSelection.h"
#include "ticketInfo.h"
#include "bookingReference.h"

class Window : public QWidget
{
public:
    explicit Window(QWidget* parent = nullptr);

private:
    NavBar* navBar_ = nullptr;
    TopBar* topBar_ = nullptr;
    QStackedWidget* stacked_ = nullptr;
    QWidget* homePage_ = nullptr;
    QWidget* ticketPage_ = nullptr;
    QWidget* idPage_ = nullptr;
    LocationSelection* locationSelection_ = nullptr;
    BookingReference* bookingReference_ = nullptr;
    TicketInfo ticketInfo_;
};