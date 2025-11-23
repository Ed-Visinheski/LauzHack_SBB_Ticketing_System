# TicketOwnership Verification Class

## Overview
The `TicketOwnership` class provides cryptographic verification that a booking ticket belongs to a specific user by comparing their Personal Identity Token (PIT) with the ticket's QR code data.

## Purpose
In an anonymous ticketing system, we need to verify:
1. **User Identity**: The PIT proves the user owns a specific PGP key pair
2. **Ticket Authenticity**: The ticket was issued by the company
3. **Ownership Link**: The ticket was issued to the same user presenting the PIT

## QR Code Formats

### Personal Identity Token (PIT)
```
PIT:pubKeyHash:timestamp:signature
```
- `pubKeyHash`: First 16 hex chars of SHA-256 hash of user's public key
- `timestamp`: Unix epoch seconds when PIT was generated
- `signature`: User's private key signs (publicKey + timestamp)

### Booking Ticket
```
TICKET:bookingRef:timestamp:userPubKeyHash:companySignature
```
- `bookingRef`: 6-character alphanumeric booking reference
- `timestamp`: Unix epoch seconds when ticket was issued
- `userPubKeyHash`: First 16 hex chars of SHA-256 hash of user's public key
- `companySignature`: Company's private key signs (userPublicKey + bookingRef + timestamp)

## API Usage

### Basic Verification (Hash Comparison Only)
```cpp
#include "ticketOwnership.h"

QString pitQR = "PIT:a3f5d8c2e1b4f7a9:1732368000:9a8b7c6d...";
QString ticketQR = "TICKET:ABC123:1732368000:a3f5d8c2e1b4f7a9:5e4d3c2b...";

auto result = TicketOwnership::verifyOwnership(pitQR, ticketQR);

if (result.keysMatch) {
    qDebug() << "Public key hashes match!";
    qDebug() << "Booking reference:" << result.bookingReference;
} else {
    qWarning() << "Verification failed:" << result.errorMessage;
}
```

### Full Verification (With User's Public Key)
```cpp
QString pitQR = "PIT:a3f5d8c2e1b4f7a9:1732368000:9a8b7c6d...";
QString ticketQR = "TICKET:ABC123:1732368000:a3f5d8c2e1b4f7a9:5e4d3c2b...";
QString userPublicKey = "-----BEGIN PGP PUBLIC KEY BLOCK-----\n...";

auto result = TicketOwnership::verifyOwnership(pitQR, ticketQR, userPublicKey);

if (result.isValid) {
    qDebug() << "PIT signature valid:" << result.pitSignatureValid;
    qDebug() << "Keys match:" << result.keysMatch;
}
```

### Complete Verification (With Company's Public Key)
```cpp
QString pitQR = "PIT:a3f5d8c2e1b4f7a9:1732368000:9a8b7c6d...";
QString ticketQR = "TICKET:ABC123:1732368000:a3f5d8c2e1b4f7a9:5e4d3c2b...";
QString userPublicKey = "-----BEGIN PGP PUBLIC KEY BLOCK-----\n...";
QString companyPublicKey = "-----BEGIN PGP PUBLIC KEY BLOCK-----\n...";

auto result = TicketOwnership::verifyOwnership(pitQR, ticketQR, userPublicKey, companyPublicKey);

if (result.isValid) {
    qDebug() << "Full verification passed!";
    qDebug() << "PIT signature valid:" << result.pitSignatureValid;
    qDebug() << "Ticket signature valid:" << result.ticketSignatureValid;
    qDebug() << "Keys match:" << result.keysMatch;
    qDebug() << "Booking reference:" << result.bookingReference;
} else {
    qWarning() << "Verification failed:" << result.errorMessage;
}
```

## VerificationResult Structure

```cpp
struct VerificationResult {
    bool isValid;                // Overall verification result
    bool pitParsed;              // PIT QR code parsed successfully
    bool ticketParsed;           // Ticket QR code parsed successfully
    bool keysMatch;              // User's public key matches in both tokens
    bool pitSignatureValid;      // PIT signature verified
    bool ticketSignatureValid;   // Ticket signature verified (requires company key)
    QString userPublicKey;       // Extracted or provided user's public key
    QString errorMessage;        // Detailed error or success message
    qint64 pitTimestamp;         // Timestamp from PIT
    qint64 ticketTimestamp;      // Timestamp from ticket
    QString bookingReference;    // Booking reference from ticket
};
```

## Inspector Application Example

```cpp
// Inspector scans user's PIT QR code
QString scannedPIT = scanQRCode();  // From camera

// Inspector scans ticket QR code
QString scannedTicket = scanQRCode();  // From camera

// Get user's public key (from AccountInfo or stored data)
QString userPublicKey = getCurrentUserPublicKey();

// Get company's public key (embedded in inspector app)
CompanyInfo company;
QString companyPublicKey = company.publicKey();

// Verify ownership
auto result = TicketOwnership::verifyOwnership(
    scannedPIT, 
    scannedTicket, 
    userPublicKey, 
    companyPublicKey
);

if (result.isValid) {
    showSuccessScreen(result.bookingReference);
} else {
    showErrorScreen(result.errorMessage);
}
```

## Verification Workflow

### Step 1: Parse QR Codes
```
PIT:     "PIT:a3f5d8c2e1b4f7a9:1732368000:9a8b7c6d..."
         ↓
Parse → pubKeyHash, timestamp, signature

Ticket:  "TICKET:ABC123:1732368000:a3f5d8c2e1b4f7a9:5e4d3c2b..."
         ↓
Parse → bookingRef, timestamp, userPubKeyHash, companySignature
```

### Step 2: Compare Public Key Hashes
```
PIT pubKeyHash:    a3f5d8c2e1b4f7a9
Ticket pubKeyHash: a3f5d8c2e1b4f7a9
                   ↓
            Hashes Match? → YES/NO
```

### Step 3: Verify PIT Signature (if public key provided)
```
User's Public Key + PIT Timestamp → Reconstruct Data
                                    ↓
                          Verify with PIT Signature
                                    ↓
                              Valid/Invalid
```

### Step 4: Verify Ticket Signature (if company key provided)
```
User's Public Key + Booking Ref + Ticket Timestamp → Reconstruct Data
                                                      ↓
                                      Verify with Company's Public Key
                                                      ↓
                                                 Valid/Invalid
```

## Security Properties

### What This Verifies:
1. **PIT Authenticity**: User created PIT with their private key
2. **Ticket Authenticity**: Company issued ticket (requires company's private key)
3. **Ownership Link**: Same public key used in both tokens
4. **Freshness**: Timestamps prevent replay attacks

### What This Doesn't Verify:
- Ticket hasn't been used before (requires database check)
- Ticket is valid for current date/time (requires separate validation)
- User's identity matches photo ID (requires manual inspection)

## Limitations & Future Enhancements

### Current Limitations:
1. **Public Key Hash Only**: QR codes contain hash (16 hex chars), not full public key
2. **Hash Collisions**: Truncated SHA-256 (8 bytes) has ~4 billion possibilities
3. **No Revocation**: Can't check if ticket was canceled

### Proposed Enhancements:

#### 1. Already Implemented ✓
The public key hash is now embedded in both PIT and ticket QR codes:
```
PIT:     "PIT:pubKeyHash:timestamp:signature"
Ticket:  "TICKET:bookingRef:timestamp:userPubKeyHash:companySignature"
```

Benefits:
- **Standalone Verification**: Can compare hashes without external data
- **Compact QR Codes**: 16 hex chars instead of full ~1700 char public key  
- **Privacy**: Only hash is visible, not full key

#### 2. Add Ticket Status Check
```cpp
bool isTicketValid = checkTicketStatus(bookingReference);
if (!isTicketValid) {
    result.errorMessage = "Ticket has been canceled or already used";
    result.isValid = false;
}
```

#### 3. Add Timestamp Validation
```cpp
qint64 now = QDateTime::currentSecsSinceEpoch();
qint64 maxAge = 24 * 60 * 60;  // 24 hours

if (now - result.pitTimestamp > maxAge) {
    result.errorMessage = "PIT has expired - please generate a new one";
    result.isValid = false;
}
```

## Integration with Main Application

### In Window.cpp (when booking):
```cpp
// Store user's public key with ticket
ticketInfo_.setUserPublicKey(accountInfo_.publicKey());
```

### In Inspector App (when verifying):
```cpp
// Get user's public key from their account or from QR code
QString userKey = accountInfo_.publicKey();

// Verify ownership
auto result = TicketOwnership::verifyOwnership(
    pitQRString, 
    ticketQRString, 
    userKey,
    companyInfo_.publicKey()
);
```

## Testing

### Unit Test Example:
```cpp
void testValidOwnership() {
    // Create test user
    PgpKeyManager keyManager("testuser_123456");
    QString publicKey = QString::fromStdString(keyManager.getPublicKey());
    QString privateKey = QString::fromStdString(keyManager.getPrivateKey());
    
    // Generate PIT
    qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    QString dataToSign = publicKey + QString::number(timestamp);
    std::string signature = PgpKeyManager::signData(dataToSign.toStdString(), privateKey.toStdString());
    QString pitQR = QString("PIT:%1:%2").arg(timestamp).arg(QString::fromStdString(signature));
    
    // Generate ticket (simplified - would use CompanyInfo)
    QString bookingRef = "ABC123";
    QString ticketQR = "TICKET:ABC123:1732368000:companySignatureHex...";
    
    // Verify
    auto result = TicketOwnership::verifyOwnership(pitQR, ticketQR, publicKey);
    
    QVERIFY(result.isValid);
    QVERIFY(result.pitSignatureValid);
    QCOMPARE(result.bookingReference, "ABC123");
}
```

## API Reference

### Static Methods

#### `verifyOwnership(pitQR, ticketQR)`
Basic verification without signature validation.
- **Parameters**: QR code strings only
- **Returns**: VerificationResult with parsing results
- **Use Case**: Quick format validation

#### `verifyOwnership(pitQR, ticketQR, userPublicKey)`
Verify PIT signature with user's public key.
- **Parameters**: QR codes + user's public key
- **Returns**: VerificationResult with PIT signature validation
- **Use Case**: User identity verification

#### `verifyOwnership(pitQR, ticketQR, userPublicKey, companyPublicKey)`
Full verification including ticket signature.
- **Parameters**: QR codes + user key + company key
- **Returns**: VerificationResult with full validation
- **Use Case**: Complete ticket ownership proof

#### `parsePIT(pitQRData, outPublicKey, outTimestamp, outSignature)`
Parse PIT QR code string into components.
- **Returns**: `true` if parsing succeeded

#### `parseTicket(ticketQRData, outBookingRef, outTimestamp, outSignature)`
Parse ticket QR code string into components.
- **Returns**: `true` if parsing succeeded

#### `verifyPITSignature(publicKey, timestamp, signature)`
Cryptographically verify PIT signature.
- **Returns**: `true` if signature is valid

#### `verifyTicketSignature(userPublicKey, bookingRef, timestamp, signature, companyPublicKey)`
Cryptographically verify ticket signature.
- **Returns**: `true` if signature is valid

## Notes

- All signature verification uses RNP (OpenPGP implementation)
- Signatures are detached binary signatures (not armored)
- Hex encoding is used for signatures in QR codes
- Timestamps are Unix epoch seconds (qint64)
- All methods are static - no instantiation required
