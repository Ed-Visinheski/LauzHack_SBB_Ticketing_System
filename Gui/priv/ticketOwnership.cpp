#include "ticketOwnership.h"
#include "PgpKeyManager.h"
#include <QStringList>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <rnp/rnp.h>
#include <rnp/rnp_err.h>

TicketOwnership::VerificationResult TicketOwnership::verifyOwnership(const QString& pitQRData, const QString& ticketQRData)
{
    // Call overload with empty public keys - limited verification only
    return verifyOwnership(pitQRData, ticketQRData, QString(), QString());
}

TicketOwnership::VerificationResult TicketOwnership::verifyOwnership(const QString& pitQRData, const QString& ticketQRData,
                                                                     const QString& userPublicKey)
{
    // Call overload with company key empty
    return verifyOwnership(pitQRData, ticketQRData, userPublicKey, QString());
}

TicketOwnership::VerificationResult TicketOwnership::verifyOwnership(const QString& pitQRData, const QString& ticketQRData,
                                                                     const QString& userPublicKey, const QString& companyPublicKey)
{
    VerificationResult result;

    // Parse PIT: "PIT:pubKeyHash:timestamp:signature"
    QString pitPubKeyHash;
    qint64 pitTimestamp = 0;
    QString pitSignature;
    
    result.pitParsed = parsePIT(pitQRData, pitPubKeyHash, pitTimestamp, pitSignature);
    if (!result.pitParsed) {
        result.errorMessage = "Failed to parse PIT QR code";
        return result;
    }
    
    result.pitTimestamp = pitTimestamp;

    // Parse Ticket: "TICKET:bookingRef:timestamp:userPubKeyHash:companySignature"
    QString bookingRefAndHash;
    qint64 ticketTimestamp = 0;
    QString ticketSignature;
    
    result.ticketParsed = parseTicket(ticketQRData, bookingRefAndHash, ticketTimestamp, ticketSignature);
    if (!result.ticketParsed) {
        result.errorMessage = "Failed to parse ticket QR code";
        return result;
    }
    
    // Split bookingRef and userPubKeyHash
    QStringList refParts = bookingRefAndHash.split(':');
    if (refParts.size() != 2) {
        result.errorMessage = "Failed to extract booking reference and key hash from ticket";
        return result;
    }
    
    QString bookingRef = refParts[0];
    QString ticketPubKeyHash = refParts[1];
    
    result.bookingReference = bookingRef;
    result.ticketTimestamp = ticketTimestamp;

    // Compare public key hashes from PIT and ticket
    result.keysMatch = (pitPubKeyHash.toLower() == ticketPubKeyHash.toLower());
    if (!result.keysMatch) {
        result.errorMessage = "Public key hashes do not match - ticket does not belong to this user";
        qDebug() << "PIT hash:" << pitPubKeyHash << "Ticket hash:" << ticketPubKeyHash;
        return result;
    }
    
    // If userPublicKey is provided, verify it matches the hash and use for signature verification
    if (!userPublicKey.isEmpty()) {
        // Compute hash of provided public key
        QByteArray publicKeyBytes = userPublicKey.toUtf8();
        QByteArray computedHash = QCryptographicHash::hash(publicKeyBytes, QCryptographicHash::Sha256);
        QString computedHashStr = computedHash.toHex().left(16);
        
        if (computedHashStr.toLower() != pitPubKeyHash.toLower()) {
            result.errorMessage = "Provided public key does not match hash in PIT";
            result.isValid = false;
            return result;
        }
        
        result.userPublicKey = userPublicKey;
    } else {
        // No public key provided - can verify hashes match but not signatures
        result.userPublicKey = "";
        result.errorMessage = "Public key hashes match, but signature verification requires full public key";
    }

    // Verify PIT signature (user signed their own public key + timestamp)
    if (!userPublicKey.isEmpty()) {
        result.pitSignatureValid = verifyPITSignature(userPublicKey, pitTimestamp, pitSignature);
        if (!result.pitSignatureValid) {
            result.errorMessage = "PIT signature verification failed - invalid identity token";
            return result;
        }
    }

    // Verify ticket signature if company public key is provided
    if (!companyPublicKey.isEmpty()) {
        result.ticketSignatureValid = verifyTicketSignature(userPublicKey, bookingRef, 
                                                           ticketTimestamp, ticketSignature, 
                                                           companyPublicKey);
        if (!result.ticketSignatureValid) {
            result.errorMessage = "Ticket signature verification failed - invalid or forged ticket";
            return result;
        }
    } else {
        // Can't verify ticket signature without company key
        result.ticketSignatureValid = false;
        result.errorMessage = "Cannot verify ticket signature: company public key not provided";
    }

    // Final validation: Both signatures valid and keys match
    result.isValid = result.pitParsed && result.ticketParsed && 
                     result.pitSignatureValid && result.keysMatch;
    
    // If we verified ticket signature, include that too
    if (!companyPublicKey.isEmpty()) {
        result.isValid = result.isValid && result.ticketSignatureValid;
    }

    if (result.isValid) {
        result.errorMessage = "Verification successful - ticket belongs to user";
    } else if (result.errorMessage.isEmpty()) {
        result.errorMessage = "Verification failed";
    }

    return result;
}

bool TicketOwnership::parsePIT(const QString& pitQRData, QString& outPublicKey, 
                               qint64& outTimestamp, QString& outSignature)
{
    // Expected format: "PIT:pubKeyHash:timestamp:signature"
    QStringList parts = pitQRData.split(':');
    
    if (parts.size() != 4 || parts[0] != "PIT") {
        qWarning() << "Invalid PIT format. Expected 'PIT:pubKeyHash:timestamp:signature', got:" << pitQRData;
        return false;
    }

    // Extract public key hash (16 hex chars = 8 bytes of SHA-256)
    outPublicKey = parts[1]; // Store hash temporarily in outPublicKey
    
    bool ok = false;
    outTimestamp = parts[2].toLongLong(&ok);
    if (!ok) {
        qWarning() << "Invalid timestamp in PIT:" << parts[2];
        return false;
    }

    // Check if PIT is older than 20 seconds
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    qint64 age = currentTime - outTimestamp;
    
    if (age > 20) {
        qWarning() << "PIT expired: Age is" << age << "seconds (max 20 seconds allowed)";
        return false;
    }
    
    if (age < 0) {
        qWarning() << "PIT timestamp is in the future (clock skew)";
        return false;
    }

    outSignature = parts[3];
    
    return true;
}

bool TicketOwnership::parseTicket(const QString& ticketQRData, QString& outBookingRef, 
                                  qint64& outTimestamp, QString& outSignature)
{
    // Expected format: "TICKET:bookingRef:timestamp:userPubKeyHash:companySignature"
    QStringList parts = ticketQRData.split(':');
    
    if (parts.size() != 5 || parts[0] != "TICKET") {
        qWarning() << "Invalid ticket format. Expected 'TICKET:bookingRef:timestamp:userPubKeyHash:signature', got:" << ticketQRData;
        return false;
    }

    outBookingRef = parts[1];
    
    bool ok = false;
    outTimestamp = parts[2].toLongLong(&ok);
    if (!ok) {
        qWarning() << "Invalid timestamp in ticket:" << parts[2];
        return false;
    }
    
    // parts[3] contains userPubKeyHash - store it in outBookingRef temporarily
    // We'll handle it in the calling function
    outBookingRef = parts[1] + ":" + parts[3]; // bookingRef:userPubKeyHash

    outSignature = parts[4];
    
    return true;
}

bool TicketOwnership::verifyPITSignature(const QString& publicKey, qint64 timestamp, 
                                         const QString& signature)
{
    if (publicKey.isEmpty() || signature.isEmpty()) {
        qWarning() << "Cannot verify PIT signature: missing public key or signature";
        return false;
    }

    try {
        // Create RNP FFI for verification
        rnp_ffi_t ffi = nullptr;
        if (rnp_ffi_create(&ffi, "GPG", "GPG") != RNP_SUCCESS || !ffi) {
            qWarning() << "Failed to create RNP FFI for verification";
            return false;
        }

        // Import public key
        QByteArray publicKeyBytes = publicKey.toUtf8();
        rnp_input_t key_input = nullptr;
        if (rnp_input_from_memory(&key_input,
                                  reinterpret_cast<const uint8_t*>(publicKeyBytes.constData()),
                                  publicKeyBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create input for public key";
            return false;
        }

        if (rnp_import_keys(ffi, key_input, RNP_LOAD_SAVE_PUBLIC_KEYS, nullptr) != RNP_SUCCESS) {
            rnp_input_destroy(key_input);
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to import public key";
            return false;
        }
        rnp_input_destroy(key_input);

        // Reconstruct the signed data: publicKey + timestamp
        QString dataToVerify = publicKey + QString::number(timestamp);
        QByteArray dataBytes = dataToVerify.toUtf8();

        // Convert hex signature back to binary
        QByteArray signatureBytes = QByteArray::fromHex(signature.toUtf8());

        // Create inputs for verification
        rnp_input_t data_input = nullptr;
        if (rnp_input_from_memory(&data_input,
                                  reinterpret_cast<const uint8_t*>(dataBytes.constData()),
                                  dataBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create data input";
            return false;
        }

        rnp_input_t sig_input = nullptr;
        if (rnp_input_from_memory(&sig_input,
                                  reinterpret_cast<const uint8_t*>(signatureBytes.constData()),
                                  signatureBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create signature input";
            return false;
        }

        // Verify the signature
        rnp_op_verify_t verify_op = nullptr;
        if (rnp_op_verify_detached_create(&verify_op, ffi, data_input, sig_input) != RNP_SUCCESS) {
            rnp_input_destroy(sig_input);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create verify operation";
            return false;
        }

        rnp_result_t result = rnp_op_verify_execute(verify_op);
        bool isValid = (result == RNP_SUCCESS);

        // Cleanup
        rnp_op_verify_destroy(verify_op);
        rnp_input_destroy(sig_input);
        rnp_input_destroy(data_input);
        rnp_ffi_destroy(ffi);

        return isValid;

    } catch (const std::exception& e) {
        qWarning() << "Exception during PIT signature verification:" << e.what();
        return false;
    }
}

bool TicketOwnership::verifyTicketSignature(const QString& userPublicKey, const QString& bookingRef,
                                            qint64 timestamp, const QString& signature,
                                            const QString& companyPublicKey)
{
    if (companyPublicKey.isEmpty() || signature.isEmpty()) {
        qWarning() << "Cannot verify ticket signature: missing company public key or signature";
        return false;
    }

    try {
        // Create RNP FFI for verification
        rnp_ffi_t ffi = nullptr;
        if (rnp_ffi_create(&ffi, "GPG", "GPG") != RNP_SUCCESS || !ffi) {
            qWarning() << "Failed to create RNP FFI for ticket verification";
            return false;
        }

        // Import company's public key
        QByteArray publicKeyBytes = companyPublicKey.toUtf8();
        rnp_input_t key_input = nullptr;
        if (rnp_input_from_memory(&key_input,
                                  reinterpret_cast<const uint8_t*>(publicKeyBytes.constData()),
                                  publicKeyBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create input for company public key";
            return false;
        }

        if (rnp_import_keys(ffi, key_input, RNP_LOAD_SAVE_PUBLIC_KEYS, nullptr) != RNP_SUCCESS) {
            rnp_input_destroy(key_input);
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to import company public key";
            return false;
        }
        rnp_input_destroy(key_input);

        // Reconstruct the signed data: userPublicKey + bookingRef + timestamp
        QString dataToVerify = userPublicKey + bookingRef + QString::number(timestamp);
        QByteArray dataBytes = dataToVerify.toUtf8();

        // Convert hex signature back to binary
        QByteArray signatureBytes = QByteArray::fromHex(signature.toUtf8());

        // Create inputs for verification
        rnp_input_t data_input = nullptr;
        if (rnp_input_from_memory(&data_input,
                                  reinterpret_cast<const uint8_t*>(dataBytes.constData()),
                                  dataBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create data input for ticket verification";
            return false;
        }

        rnp_input_t sig_input = nullptr;
        if (rnp_input_from_memory(&sig_input,
                                  reinterpret_cast<const uint8_t*>(signatureBytes.constData()),
                                  signatureBytes.size(),
                                  false) != RNP_SUCCESS) {
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create signature input for ticket verification";
            return false;
        }

        // Verify the company's signature
        rnp_op_verify_t verify_op = nullptr;
        if (rnp_op_verify_detached_create(&verify_op, ffi, data_input, sig_input) != RNP_SUCCESS) {
            rnp_input_destroy(sig_input);
            rnp_input_destroy(data_input);
            rnp_ffi_destroy(ffi);
            qWarning() << "Failed to create verify operation for ticket";
            return false;
        }

        rnp_result_t result = rnp_op_verify_execute(verify_op);
        bool isValid = (result == RNP_SUCCESS);

        // Cleanup
        rnp_op_verify_destroy(verify_op);
        rnp_input_destroy(sig_input);
        rnp_input_destroy(data_input);
        rnp_ffi_destroy(ffi);

        return isValid;

    } catch (const std::exception& e) {
        qWarning() << "Exception during ticket signature verification:" << e.what();
        return false;
    }
}

QString TicketOwnership::extractUserPublicKeyFromTicket(const QString& ticketQRData)
{
    // New format: "TICKET:bookingRef:timestamp:userPubKeyHash:companySignature"
    QStringList parts = ticketQRData.split(':');
    
    if (parts.size() != 5 || parts[0] != "TICKET") {
        qWarning() << "Invalid ticket format for key extraction";
        return QString();
    }
    
    // Return the user's public key hash (16 hex chars)
    // Note: This returns the HASH, not the full public key
    // The full key would need to be provided separately for signature verification
    return parts[3];
}