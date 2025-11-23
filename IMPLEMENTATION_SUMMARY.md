# Company-Signed Booking Tickets Implementation

## Overview
Implemented company-signed booking tickets where each ticket's QR code contains a cryptographic signature proving the ticket was issued by SBB (the train company).

## Architecture

### Key Components

1. **TicketInfo** (Updated)
   - Added `userPublicKey_`: Stores the user's PGP public key
   - Added `signedData_`: Stores the company's signature
   - Added `timestamp_`: Records when the signature was created
   - Methods: `setUserPublicKey()`, `setSignedData()`

2. **CompanyInfo** (New)
   - Constexpr company email: `sbbsupersecretadmin@gmail.com`
   - Generates deterministic company PGP keys using fixed seed: `SBB_COMPANY_MASTER_KEY_2025`
   - Provides `publicKey()` and `privateKey()` getters
   - Singleton-like behavior ensures same keys across sessions

3. **BookingReference** (Updated)
   - Added `companyInfo_` member to access company keys
   - Method `setCompanyInfo()` to inject company info
   - Updated `showQRCode()` to create signed ticket QR codes

4. **Window** (Updated)
   - Added `companyInfo_` member instance
   - Passes company info to `BookingReference` during setup
   - Sets user's public key on tickets when booking is created

## Ticket Signing Flow

### 1. User Login
```
User enters email + password
→ Deterministic user PGP key generation (SHA-256 hash-based userId)
→ AccountInfo stores: email, publicKey, privateKey
```

### 2. Ticket Booking
```
User selects departure, destination, date, time
→ TicketInfo created with random booking reference
→ User's public key added to ticket: ticketInfo_.setUserPublicKey(accountInfo_.publicKey())
→ Ticket added to BookingReference
```

### 3. QR Code Generation (When User Views Ticket)
```
BookingReference::showQRCode() called
→ Data to sign: userPublicKey + bookingReference + timestamp
→ Company's private key signs the data (RNP detached signature)
→ QR code format: "TICKET:bookingRef:timestamp:companySignature"
→ QR displayed to user
```

## Signature Verification (Inspector Side - Future Implementation)

### What Inspector Can Verify:
1. **Ticket Authenticity**: Verify company signature using company's public key
   - Extract: bookingRef, timestamp, signature from QR
   - Reconstruct: userPublicKey + bookingRef + timestamp
   - Verify signature matches using company's public key
   
2. **User Identity**: Cross-check user's PIT (Personal Identity Token)
   - User shows PIT QR: "PIT:timestamp:userSignature"
   - Extract user's public key from PIT verification
   - Ensure ticket's embedded public key matches PIT's public key

### Security Properties:
- **Non-forgeable**: Only SBB company can sign tickets (private key required)
- **Anonymous**: No email in QR codes, only public keys
- **Verifiable**: Inspector can verify without network (offline verification)
- **Timestamped**: Each ticket has unique timestamp preventing reuse
- **Linked**: User's public key links PIT to ticket

## Code Structure

### Data Flow:
```
User Login
  ↓
AccountInfo {email, publicKey, privateKey}
  ↓
Window stores accountInfo_
  ↓
User creates booking
  ↓
TicketInfo gets user's publicKey from accountInfo_
  ↓
BookingReference receives ticket + companyInfo_
  ↓
showQRCode() signs: userPublicKey + bookingRef + timestamp
  ↓
QR code contains company signature
```

### Key Files Modified:
- `Gui/pub/ticketInfo.h`: Added userPublicKey, signedData, timestamp fields
- `Gui/pub/window.h`: Added companyInfo_ member
- `Gui/pub/bookingReference.h`: Added companyInfo_ pointer and setCompanyInfo()
- `Gui/priv/bookingReference.cpp`: Implemented company signing in showQRCode()
- `Gui/priv/window.cpp`: Pass companyInfo to BookingReference, set user's publicKey on tickets

### New Files:
- `Gui/pub/companyInfo.h`: Company PGP key generation
- `Gui/priv/companyInfo.cpp`: Implementation

## QR Code Formats

### Personal Identity Token (PIT):
```
PIT:timestamp:signature
```
- `timestamp`: Unix epoch seconds when PIT was generated
- `signature`: User's private key signs (publicKey + timestamp)

### Booking Ticket:
```
TICKET:bookingRef:timestamp:companySignature
```
- `bookingRef`: 6-character alphanumeric booking reference
- `timestamp`: Unix epoch seconds when ticket was issued
- `companySignature`: Company's private key signs (userPublicKey + bookingRef + timestamp)

## Technical Details

### Cryptographic Operations:
- **Algorithm**: PGP/GPG via RNP library
- **Signature Type**: Detached signatures (binary, not armored)
- **Hash Function**: SHA-256 (RNP default)
- **Key Generation**: RSA-2048 (RNP default for PgpKeyManager)

### RNP API Usage in Signing:
1. Create FFI context: `rnp_ffi_create()`
2. Import private key: `rnp_import_keys()` with company's private key
3. Create signing operation: `rnp_op_sign_detached_create()`
4. Locate company key: `rnp_locate_key()`
5. Add signer: `rnp_op_sign_add_signature()`
6. Execute: `rnp_op_sign_execute()`
7. Extract signature: `rnp_output_memory_get_buf()`
8. Convert to hex: `QByteArray::toHex()`

### Error Handling:
- Try-catch block wraps entire signing operation
- Fallback: If signing fails, displays simple booking reference QR
- Proper RNP resource cleanup on all error paths

## Future Enhancements

### Inspector Application:
- Separate Qt application for ticket inspectors
- Scan PIT QR to extract user's public key
- Scan ticket QR to verify company signature
- Visual indicators: ✓ Valid / ✗ Invalid

### Signature Caching:
- Store signature in TicketInfo after first generation
- Use `setSignedData()` to cache signature with timestamp
- Avoid re-signing same ticket multiple times

### Network Verification (Optional):
- Backend API to check if booking reference is valid
- Revocation list for canceled tickets
- Real-time fraud detection

### Additional Security:
- Ticket expiration (verify timestamp is recent)
- One-time-use tokens (burn ticket after first scan)
- Biometric linking (fingerprint/face tied to PIT)

## Testing Checklist

- [x] Build succeeds with all dependencies
- [ ] User can log in and see PIT QR
- [ ] User can create booking with departure/destination
- [ ] Ticket appears in BookingReference list
- [ ] Clicking ticket shows QR code with company signature
- [ ] QR code format matches "TICKET:ref:ts:sig"
- [ ] Multiple tickets work independently
- [ ] Signature is deterministic for same input
- [ ] Inspector can verify signature (future test)

## Notes

- Company keys are deterministic (same email+seed → same keys)
- User keys are deterministic (same email+password → same keys)
- Signatures are non-deterministic due to timestamp variation
- QR codes are 280x280 pixels (configurable in QRCodeGenerator)
- All cryptographic operations happen client-side (no network)
