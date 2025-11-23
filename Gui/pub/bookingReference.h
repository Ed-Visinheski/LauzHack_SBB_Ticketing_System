#pragma once
#include <QWidget>
#include <QLabel>
#include "ticketInfo.h"

class BookingReference : public QWidget
{
    Q_OBJECT
public:
    explicit BookingReference(QWidget* parent = nullptr);
    void updateTicket(const TicketInfo& ticket);

private:
    QLabel* referenceLabel_ = nullptr;
    QLabel* departureLabel_ = nullptr;
    QLabel* destinationLabel_ = nullptr;
    QLabel* dateLabel_ = nullptr;
    QLabel* timeLabel_ = nullptr;
    QLabel* noTicketLabel_ = nullptr;
};