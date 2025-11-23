#include "logInPage.h"
#include "PgpKeyManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCryptographicHash>
#include <QMessageBox>

LoginPage::LoginPage(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: #f5f5f5;");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Add SBB header with logo
    auto headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background-color: #eb0000;");
    headerWidget->setFixedHeight(80);
    auto headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    
    // Add SBB logo
    auto logoLabel = new QLabel(headerWidget);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedSize(50, 50);
    logoLabel->setStyleSheet("background: transparent; border: none; padding: 0;");
    QPixmap logoPix("icons/SBB_logo.svg");
    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(logoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    headerLayout->addWidget(logoLabel);
    headerLayout->addStretch();
    
    mainLayout->addWidget(headerWidget);

    // Content area
    auto contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: #f5f5f5;");
    auto contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 40, 40, 40);
    contentLayout->setSpacing(20);

    contentLayout->addStretch(1);

    // Title
    auto titleLabel = new QLabel("Welcome Back", contentWidget);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 600; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(titleLabel);

    auto subtitleLabel = new QLabel("Log in to access your tickets", contentWidget);
    subtitleLabel->setStyleSheet("font-size: 14px; color: #666;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(subtitleLabel);

    contentLayout->addSpacing(20);

    // Login container
    auto loginContainer = new QWidget(contentWidget);
    loginContainer->setStyleSheet("background-color: white; border-radius: 12px; border: 1px solid #e0e0e0;");
    loginContainer->setMaximumWidth(500);
    
    auto containerLayout = new QVBoxLayout(loginContainer);
    containerLayout->setContentsMargins(32, 32, 32, 32);
    containerLayout->setSpacing(20);

    // Email
    auto emailLabel = new QLabel("Email:", loginContainer);
    emailLabel->setStyleSheet("font-weight: 600; font-size: 13px; color: #333; border: none; background: transparent;");
    emailEdit_ = new QLineEdit(loginContainer);
    emailEdit_->setPlaceholderText("Enter your email");
    emailEdit_->setMinimumHeight(44);
    emailEdit_->setStyleSheet(
        "QLineEdit { "
        "  padding: 12px 16px; border: 2px solid #e0e0e0; border-radius: 8px; "
        "  font-size: 14px; color: #000; background-color: #fafafa; "
        "} "
        "QLineEdit:focus { "
        "  border-color: #eb0000; background-color: white; "
        "}"
    );

    // Password
    auto passwordLabel = new QLabel("Password:", loginContainer);
    passwordLabel->setStyleSheet("font-weight: 600; font-size: 13px; color: #333; border: none; background: transparent;");
    passwordEdit_ = new QLineEdit(loginContainer);
    passwordEdit_->setPlaceholderText("Enter your password");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setMinimumHeight(44);
    passwordEdit_->setMinimumWidth(280);
    passwordEdit_->setStyleSheet(
        "QLineEdit { "
        "  padding: 12px 16px; border: 2px solid #e0e0e0; border-radius: 8px; "
        "  font-size: 14px; color: #000; background-color: #fafafa; "
        "} "
        "QLineEdit:focus { "
        "  border-color: #eb0000; background-color: white; "
        "}"
    );

    // Status label
    statusLabel_ = new QLabel("", loginContainer);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setWordWrap(true);
    statusLabel_->setStyleSheet("font-size: 13px; min-height: 20px;");
    statusLabel_->hide();

    // Login button
    loginButton_ = new QPushButton("Login", loginContainer);
    loginButton_->setMinimumHeight(50);
    loginButton_->setCursor(Qt::PointingHandCursor);
    loginButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #eb0000; color: white; font-weight: 600; "
        "  font-size: 15px; border: none; border-radius: 8px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #c00000; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #a00000; "
        "}"
    );

    connect(loginButton_, &QPushButton::clicked, this, &LoginPage::handleLogin);
    connect(passwordEdit_, &QLineEdit::returnPressed, this, &LoginPage::handleLogin);

    // Add to container
    containerLayout->addWidget(emailLabel);
    containerLayout->addWidget(emailEdit_);
    containerLayout->addWidget(passwordLabel);
    containerLayout->addWidget(passwordEdit_);
    containerLayout->addWidget(statusLabel_);
    containerLayout->addWidget(loginButton_);

    // Center the login container
    auto centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(loginContainer);
    centerLayout->addStretch();
    contentLayout->addLayout(centerLayout);

    contentLayout->addStretch(2);

    mainLayout->addWidget(contentWidget);
}

void LoginPage::handleLogin()
{
    QString email = emailEdit_->text().trimmed();
    QString password = passwordEdit_->text();

    // Validate inputs
    if (email.isEmpty()) {
        showStatus("Please enter your email", true);
        return;
    }
    if (password.isEmpty()) {
        showStatus("Please enter your password", true);
        return;
    }
    if (!email.contains('@')) {
        showStatus("Please enter a valid email address", true);
        return;
    }

    // Show loading state
    loginButton_->setEnabled(false);
    loginButton_->setText("Generating Keys...");
    showStatus("Generating PGP keys, please wait...", false);

    try {
        // Create deterministic user ID from email + password hash
        // This ensures same credentials always generate same keys
        QByteArray combined = (email + password).toUtf8();
        QByteArray hash = QCryptographicHash::hash(combined, QCryptographicHash::Sha256);
        QString deterministicId = email + "_" + hash.toHex().left(16);

        // Generate PGP keys using PgpKeyManager
        PgpKeyManager keyManager(deterministicId.toStdString());

        // Export keys
        QString publicKey = QString::fromStdString(keyManager.exportPublicKeyArmored());
        QString privateKey = QString::fromStdString(keyManager.exportSecretKeyArmored());

        // Create account info
        AccountInfo account(email, publicKey, privateKey);

        // Clear password field for security
        passwordEdit_->clear();

        showStatus("Login successful!", false);

        // Emit signal with account info
        emit loginSuccessful(account);

    } catch (const std::exception& e) {
        showStatus(QString("Key generation failed: %1").arg(e.what()), true);
        loginButton_->setEnabled(true);
        loginButton_->setText("Login");
    }
}

void LoginPage::showStatus(const QString& message, bool isError)
{
    if (message.isEmpty()) {
        statusLabel_->hide();
    } else {
        statusLabel_->setText(message);
        statusLabel_->setStyleSheet(
            QString("font-size: 12px; min-height: 20px; color: %1;")
                .arg(isError ? "#d32f2f" : "#388e3c")
        );
        statusLabel_->show();
    }
}