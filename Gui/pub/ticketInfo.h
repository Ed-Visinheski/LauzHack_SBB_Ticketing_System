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
    QString userPublicKey() const { return userPublicKey_; }
    QString signedData() const { return signedData_; }
    qint64 timestamp() const { return timestamp_; }
    bool isValid() const { return !bookingReference_.isEmpty(); }
    
    void setUserPublicKey(const QString& publicKey) { userPublicKey_ = publicKey; }
    void setSignedData(const QString& signature, qint64 ts) { signedData_ = signature; timestamp_ = ts; }

private:
    QString departure_;
    QString destination_;
    QDate date_;
    QTime time_;
    QString bookingReference_;
    QString userPublicKey_;
    QString signedData_;
    qint64 timestamp_ = 0;
    
    QString generateBookingReference();
};