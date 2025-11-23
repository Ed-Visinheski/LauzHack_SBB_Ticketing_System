#pragma once
#include <QImage>
#include <QString>

// QR Code generator using libqrencode
class QRCodeGenerator
{
public:
    // Generate a QR code image from text data using libqrencode
    // Returns a QImage scaled to the specified size
    static QImage generateQRCode(const QString& data, int size = 280);
};