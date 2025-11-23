#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QVBoxLayout>
#include "ticketInfo.h"
#include "companyInfo.h"

// Individual ticket card widget
class TicketCard : public QWidget
{
    Q_OBJECT
public:
    explicit TicketCard(const TicketInfo& ticket, QWidget* parent = nullptr);

signals:
    void qrCodeRequested(const TicketInfo& ticket);

private:
    TicketInfo ticket_;
};

// Container for multiple tickets with scroll
class BookingReference : public QWidget
{
    Q_OBJECT
public:
    explicit BookingReference(QWidget* parent = nullptr);
    void addTicket(const TicketInfo& ticket);
    void clearTickets();
    void setCompanyInfo(const CompanyInfo* companyInfo) { companyInfo_ = companyInfo; }

private slots:
    void showQRCode(const TicketInfo& ticket);
    void hideQRCode();
    void downloadCurrentQRCode();

private:
    void setupQROverlay();

    QScrollArea* scrollArea_ = nullptr;
    QWidget* ticketContainer_ = nullptr;
    QVBoxLayout* ticketLayout_ = nullptr;
    QLabel* noTicketLabel_ = nullptr;
    
    // QR Code overlay (centered on page)
    QWidget* qrOverlay_ = nullptr;
    QLabel* qrImageLabel_ = nullptr;
    QLabel* qrTitleLabel_ = nullptr;
    QPushButton* downloadQRButton_ = nullptr;
    
    // Company info for signing tickets
    const CompanyInfo* companyInfo_ = nullptr;
    
    // Current ticket being displayed
    TicketInfo currentTicket_;
};