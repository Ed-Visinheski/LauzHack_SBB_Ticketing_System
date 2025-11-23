# Ticket Inspector Application

## Overview
The Ticket Inspector is a standalone application for SBB train inspectors to verify ticket ownership by scanning QR codes.

## Launching Inspector Mode

### Launch Modes

#### Inspector Only
```bash
./build/bin/main --inspector
# Or short flag
./build/bin/main -i
```

#### User Application Only (Default)
```bash
./build/bin/main
# Or explicit flag
./build/bin/main --user
./build/bin/main -u
```

#### Both Windows Simultaneously
```bash
./build/bin/main --both
# Or short flag
./build/bin/main -b
```

This launches both the user window and inspector window side by side, perfect for:
- **Testing**: Create tickets in user window, verify them in inspector window
- **Demonstrations**: Show the complete flow in one screen
- **Development**: Work on both interfaces simultaneously

## Usage Workflow

### 1. Launch Inspector Application
Run the application with the `--inspector` flag to open the inspector interface.

### 2. Load PIT QR Code
- Click "Load PIT QR Code" button
- Select an image file containing the user's Personal Identity Token QR code
- Supported formats: PNG, JPG, JPEG, BMP
- The QR code will be decoded automatically

### 3. Load Ticket QR Code
- Click "Load Ticket QR Code" button
- Select an image file containing the booking ticket QR code
- The QR code will be decoded automatically

### 4. Verify Ownership
- Once both QR codes are loaded, the "Verify Ownership" button becomes enabled
- Click "Verify Ownership" to check if the ticket belongs to the user
- Results are displayed in the panel below

### 5. Interpret Results

**✓ VALID (Green)**
- The ticket belongs to the user
- Public key hashes match between PIT and ticket
- Booking reference is displayed
- All checks passed

**✗ INVALID (Red)**
- The ticket does NOT belong to the user
- Public key hashes do not match
- Error details are shown
- Ticket is fraudulent or belongs to someone else

### 6. Clear and Restart
- Click "Clear All" to reset and verify another passenger

## QR Code Requirements

### PIT QR Code Format
```
PIT:pubKeyHash:timestamp:signature
```
Example: `PIT:a3f5d8c2e1b4f7a9:1732368000:9a8b7c6d5e4f3a2b...`

### Ticket QR Code Format
```
TICKET:bookingRef:timestamp:userPubKeyHash:companySignature
```
Example: `TICKET:ABC123:1732368000:a3f5d8c2e1b4f7a9:5e4d3c2b1a9f8e7d...`

## Technical Details

### QR Code Decoding
The inspector uses **zbar** to decode QR codes from image files:
- Must be installed: `brew install zbar` (macOS) or `sudo apt install zbar-tools` (Ubuntu)
- Command: `zbarimg --quiet --raw <image_file>`
- Automatic extraction from uploaded images

### Verification Process
1. **Parse QR Codes**: Extract public key hashes, timestamps, signatures
2. **Compare Hashes**: Check if PIT hash == Ticket hash
3. **Result**: Match = Valid, No Match = Invalid

### What Gets Verified
- ✓ Public key hash from PIT matches ticket's user public key hash
- ✓ Both QR codes parsed successfully
- ✓ Booking reference extracted

### What Doesn't Get Verified (Yet)
- Cryptographic signatures (requires public keys, not just hashes)
- Ticket expiration/validity dates
- Database check for canceled tickets
- Company signature verification

## Troubleshooting

### "Failed to decode QR code from image"
**Cause**: QR code not found or image quality too poor

**Solutions**:
- Ensure image contains a clear, visible QR code
- Try a higher resolution image
- Ensure zbar is installed: `which zbarimg`
- Test manually: `zbarimg --raw <your_image.png>`

### "Public key hashes do not match"
**Cause**: Ticket belongs to a different user

**Action**: 
- This is a fraud attempt or wrong ticket
- Request additional identification
- Do not allow boarding

### zbarimg command not found
**Cause**: zbar not installed

**Solutions**:
- macOS: `brew install zbar`
- Ubuntu: `sudo apt-get install zbar-tools`
- Verify: `which zbarimg`

### Image file won't load
**Cause**: Unsupported format or corrupted file

**Solutions**:
- Use PNG, JPG, JPEG, or BMP formats
- Ensure file is not corrupted
- Try converting to PNG format

## Screenshot Generation (For Testing)

### Save QR Code from User App
1. Launch user app: `./build/bin/main`
2. Login and create a booking
3. Navigate to ID page to see PIT QR code
4. Navigate to Tickets page and click ticket to see booking QR code
5. Take screenshots of both QR codes

### Test with Inspector
1. Launch inspector: `./build/bin/main --inspector`
2. Load PIT screenshot
3. Load ticket screenshot
4. Click verify - should show VALID if from same user

## Security Features

### What Inspector Checks
- **Identity Proof**: User has the PIT matching the ticket
- **Ownership Link**: Same public key in both QR codes
- **Format Validation**: QR codes are properly formatted

### What Inspector Doesn't Check
- **Signature Validity**: Not verifying cryptographic signatures (would need full public keys)
- **Ticket Status**: Not checking if ticket was canceled or already used
- **Date/Time**: Not validating journey date/time
- **Route**: Not checking if train matches ticket route

### Recommended Manual Checks
- Photo ID matches passenger
- Journey date is today
- Departure/destination match train route
- Ticket hasn't been visually tampered with

## Advanced Usage

### Batch Verification
For multiple passengers:
1. Verify first passenger
2. Click "Clear All"
3. Load next passenger's QR codes
4. Repeat

### Debugging Mode
Check decoded QR code data:
```bash
# Manually decode QR code
zbarimg --raw pit_qr.png
zbarimg --raw ticket_qr.png

# Launch with debug output
./build/bin/main --inspector 2>&1 | grep "Decoded QR"
```

### Integration with Camera
Future enhancement: Real-time camera scanning instead of file upload

## Command Line Options

```bash
# User application only (default)
./build/bin/main
./build/bin/main --user
./build/bin/main -u

# Inspector application only
./build/bin/main --inspector
./build/bin/main -i

# Both windows simultaneously
./build/bin/main --both
./build/bin/main -b
```

### Testing Workflow with Both Windows

1. **Launch both**: `./build/bin/main --both`
2. **User window** (left): Login and create a booking
3. **Take screenshots**: 
   - ID page: PIT QR code
   - Tickets page: Booking QR code
4. **Inspector window** (right): Load both screenshots
5. **Verify**: Click "Verify Ownership" → Should show ✓ VALID

## System Requirements

### Software Dependencies
- Qt 6.x
- zbar (QR code decoder)
- RNP (PGP cryptography)
- libqrencode (for generation, not used in inspector)

### Operating Systems
- macOS 10.15+
- Ubuntu 20.04+
- Any Linux with Qt6 support

### Hardware
- Any modern PC
- Display: 800x700 minimum resolution
- Camera (future): For real-time QR scanning

## Best Practices

### For Inspectors
1. **Always verify both QR codes** - Never accept just one
2. **Check photo ID** - Match with physical identification
3. **Visual inspection** - Look for obvious forgeries
4. **Clear between checks** - Don't mix up passengers
5. **Report anomalies** - Flag suspicious patterns

### For Testing
1. **Use real QR codes** - Don't mock data
2. **Test edge cases** - Try mismatched QR codes
3. **Check error messages** - Ensure they're clear
4. **Performance** - Should verify in under 2 seconds

## FAQ

**Q: Can I verify without internet?**
A: Yes, verification is completely offline.

**Q: How accurate is hash matching?**
A: Hash collision probability is ~1 in 4 billion (2^32) for 8-byte truncated SHA-256.

**Q: What if QR code is damaged?**
A: QR codes have error correction. Minor damage is OK, severe damage will fail to decode.

**Q: Can I verify multiple tickets per PIT?**
A: Yes, load the same PIT and different tickets to verify multiple bookings.

**Q: Does this work with e-tickets?**
A: Yes, as long as they display the correct QR code format.

## Support

For issues or questions:
- Check troubleshooting section above
- Review QR code format requirements
- Ensure zbar is installed correctly
- Check application logs for detailed errors
