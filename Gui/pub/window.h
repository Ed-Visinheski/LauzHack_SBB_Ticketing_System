#pragma once
#include "definitions.h"
#include <QWidget>
#include <QStackedWidget>
#include "navBar.h"
#include "topBar.h"
#include "locationSelection.h"
#include "ticketInfo.h"
#include "bookingReference.h"
#include "logInPage.h"
#include "accountInfo.h"
#include "identificationToken.h"

class Window : public QWidget
{
public:
    explicit Window(QWidget* parent = nullptr);

private slots:
    void handleLoginSuccess(const AccountInfo& account);

private:
    void setupMainUI();

    NavBar* navBar_ = nullptr;
    TopBar* topBar_ = nullptr;
    QStackedWidget* stacked_ = nullptr;
    QStackedWidget* mainStacked_ = nullptr;  // Login vs Main app
    QWidget* homePage_ = nullptr;
    QWidget* ticketPage_ = nullptr;
    QWidget* idPage_ = nullptr;
    LoginPage* loginPage_ = nullptr;
    LocationSelection* locationSelection_ = nullptr;
    BookingReference* bookingReference_ = nullptr;
    IdentificationToken* identificationToken_ = nullptr;
    TicketInfo ticketInfo_;
    AccountInfo accountInfo_;
    bool isLoggedIn_ = false;
};