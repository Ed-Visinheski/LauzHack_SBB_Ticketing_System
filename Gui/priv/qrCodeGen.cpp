#include "qrCodeGen.h"
#include <qrencode.h>
#include <cstring>

// Production QR code generation using libqrencode

QImage QRCodeGenerator::generateQRCode(const QString& data, int size)
{
    // Convert QString to C string
    QByteArray dataBytes = data.toUtf8();
    const char* dataStr = dataBytes.constData();
    
    // Generate QR code using libqrencode
    // Parameters: data, version (0=auto), error correction level, encoding mode, case sensitive
    QRcode* qrcode = QRcode_encodeString(dataStr, 0, QR_ECLEVEL_M, QR_MODE_8, 1);
    
    if (!qrcode) {
        // Return a blank white image if generation fails
        QImage fallback(size, size, QImage::Format_RGB32);
        fallback.fill(Qt::white);
        return fallback;
    }
    
    // Get the QR code width (it's a square)
    int qrWidth = qrcode->width;
    
    // Create image from QR code data
    // Each module (bit) in the QR code will be a pixel in our base image
    QImage image(qrWidth, qrWidth, QImage::Format_RGB32);
    image.fill(Qt::white);
    
    // qrcode->data is a byte array where each byte represents one module
    // Bit 0 (LSB) indicates black (1) or white (0)
    unsigned char* qrData = qrcode->data;
    
    for (int y = 0; y < qrWidth; ++y) {
        for (int x = 0; x < qrWidth; ++x) {
            int idx = y * qrWidth + x;
            // Check if the module is black (bit 0 is set)
            bool isBlack = qrData[idx] & 1;
            image.setPixel(x, y, isBlack ? qRgb(0, 0, 0) : qRgb(255, 255, 255));
        }
    }
    
    // Free the QR code structure
    QRcode_free(qrcode);
    
    // Scale to requested size with no smoothing (keep sharp edges)
    return image.scaled(size, size, Qt::KeepAspectRatio, Qt::FastTransformation);
}