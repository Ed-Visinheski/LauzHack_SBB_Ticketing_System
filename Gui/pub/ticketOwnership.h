#pragma once
#include <QString>
#include <QDateTime>

class TicketOwnership
{
public:
    struct VerificationResult {
        bool isValid = false;
        bool pitParsed = false;
        bool ticketParsed = false;
        bool keysMatch = false;
        bool pitSignatureValid = false;
        bool ticketSignatureValid = false;
        QString userPublicKey;
        QString errorMessage;
        qint64 pitTimestamp = 0;
        qint64 ticketTimestamp = 0;
        QString bookingReference;
    };

    // Main verification method
    static VerificationResult verifyOwnership(const QString& pitQRData, const QString& ticketQRData);
    
    // Overload that accepts the user's public key directly (since it's not in the QR codes)
    static VerificationResult verifyOwnership(const QString& pitQRData, const QString& ticketQRData, 
                                             const QString& userPublicKey);
    
    // Overload that also accepts company public key for full ticket verification
    static VerificationResult verifyOwnership(const QString& pitQRData, const QString& ticketQRData,
                                             const QString& userPublicKey, const QString& companyPublicKey);

    // Parse QR code strings
    static bool parsePIT(const QString& pitQRData, QString& outPublicKey, qint64& outTimestamp, QString& outSignature);
    static bool parseTicket(const QString& ticketQRData, QString& outBookingRef, qint64& outTimestamp, QString& outSignature);

    // Verify signatures (requires company public key for ticket verification)
    static bool verifyPITSignature(const QString& publicKey, qint64 timestamp, const QString& signature);
    static bool verifyTicketSignature(const QString& userPublicKey, const QString& bookingRef, 
                                      qint64 timestamp, const QString& signature, 
                                      const QString& companyPublicKey);

    // Extract user public key from ticket (needs to be embedded or retrieved)
    static QString extractUserPublicKeyFromTicket(const QString& ticketQRData);

private:
    TicketOwnership() = delete; // Static class only
};