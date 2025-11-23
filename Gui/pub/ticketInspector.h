#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QScrollArea>
#include "ticketOwnership.h"
#include "companyInfo.h"

class TicketInspector : public QWidget
{
    Q_OBJECT

public:
    explicit TicketInspector(QWidget* parent = nullptr);
    
private slots:
    void loadPITQRCode();
    void loadTicketQRCode();
    void verifyOwnership();
    void clearAll();
    
private:
    void setupUI();
    void updateVerificationStatus(const TicketOwnership::VerificationResult& result);
    QString decodeQRFromImage(const QString& imagePath);
    
    // UI Components
    QLabel* titleLabel_ = nullptr;
    
    // PIT Section
    QLabel* pitLabel_ = nullptr;
    QLabel* pitImageLabel_ = nullptr;
    QPushButton* loadPITButton_ = nullptr;
    QLabel* pitStatusLabel_ = nullptr;
    
    // Ticket Section
    QLabel* ticketLabel_ = nullptr;
    QLabel* ticketImageLabel_ = nullptr;
    QPushButton* loadTicketButton_ = nullptr;
    QLabel* ticketStatusLabel_ = nullptr;
    
    // Verification Section
    QPushButton* verifyButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;
    QWidget* resultPanel_ = nullptr;
    QLabel* resultIconLabel_ = nullptr;
    QLabel* resultTextLabel_ = nullptr;
    QLabel* detailsLabel_ = nullptr;
    
    // Data
    QString pitQRData_;
    QString ticketQRData_;
    QPixmap pitImage_;
    QPixmap ticketImage_;
    CompanyInfo companyInfo_;
    
    // Constants
    static constexpr int IMAGE_SIZE = 200;
};