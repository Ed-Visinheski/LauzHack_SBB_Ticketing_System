#include "ticketInfo.h"
#include <QRandomGenerator>

TicketInfo::TicketInfo(const QString& departure, const QString& destination, const QDate& date, const QTime& time)
    : departure_(departure)
    , destination_(destination)
    , date_(date)
    , time_(time)
{
    bookingReference_ = generateBookingReference();
}

QString TicketInfo::generateBookingReference()
{
    // Generate a random 6-character alphanumeric booking reference
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString ref;
    for (int i = 0; i < 6; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        ref += chars[index];
    }
    return ref;
}