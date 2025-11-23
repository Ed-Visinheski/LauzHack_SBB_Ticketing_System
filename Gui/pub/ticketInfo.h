#pragma once
#include <QString>
#include <QDate>
#include <QTime>
#include <QDateTime>

class TicketInfo
{
public:
    TicketInfo() = default;
    TicketInfo(const QString& departure, const QString& destination, const QDate& date, const QTime& time);

    QString departure() const { return departure_; }
    QString destination() const { return destination_; }
    QDate date() const { return date_; }
    QTime time() const { return time_; }
    QString bookingReference() const { return bookingReference_; }
    bool isValid() const { return !bookingReference_.isEmpty(); }

private:
    QString departure_;
    QString destination_;
    QDate date_;
    QTime time_;
    QString bookingReference_;
    
    QString generateBookingReference();
};