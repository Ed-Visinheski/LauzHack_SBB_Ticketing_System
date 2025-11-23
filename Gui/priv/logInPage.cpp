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
    mainLayout->setContentsMargins(40, 60, 40, 60);
    mainLayout->setSpacing(20);

    mainLayout->addStretch(1);

    // Title
    auto titleLabel = new QLabel("SBB Login", this);
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #ec0001;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(20);

    // Login container
    auto loginContainer = new QWidget(this);
    loginContainer->setStyleSheet("background-color: white; border-radius: 12px;");
    loginContainer->setMaximumWidth(360);
    
    auto containerLayout = new QVBoxLayout(loginContainer);
    containerLayout->setContentsMargins(30, 30, 30, 30);
    containerLayout->setSpacing(16);

    // Email
    auto emailLabel = new QLabel("Email:", loginContainer);
    emailLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
    emailEdit_ = new QLineEdit(loginContainer);
    emailEdit_->setPlaceholderText("Enter your email");
    emailEdit_->setFixedHeight(44);
    emailEdit_->setStyleSheet(
        "QLineEdit { "
        "  padding: 10px; border: 2px solid #ddd; border-radius: 6px; "
        "  font-size: 14px; color: #333; "
        "} "
        "QLineEdit:focus { "
        "  border-color: #ec0001; "
        "}"
    );

    // Password
    auto passwordLabel = new QLabel("Password:", loginContainer);
    passwordLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333;");
    passwordEdit_ = new QLineEdit(loginContainer);
    passwordEdit_->setPlaceholderText("Enter your password");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setFixedHeight(44);
    passwordEdit_->setStyleSheet(
        "QLineEdit { "
        "  padding: 10px; border: 2px solid #ddd; border-radius: 6px; "
        "  font-size: 14px; color: #333; "
        "} "
        "QLineEdit:focus { "
        "  border-color: #ec0001; "
        "}"
    );

    // Status label
    statusLabel_ = new QLabel("", loginContainer);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setWordWrap(true);
    statusLabel_->setStyleSheet("font-size: 12px; min-height: 20px;");
    statusLabel_->hide();

    // Login button
    loginButton_ = new QPushButton("Login", loginContainer);
    loginButton_->setFixedHeight(48);
    loginButton_->setStyleSheet(
        "QPushButton { "
        "  background-color: #ec0001; color: white; font-weight: bold; "
        "  font-size: 16px; border: none; border-radius: 6px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #d00001; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #b00001; "
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

    // Center container
    auto centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(loginContainer);
    centerLayout->addStretch();
    mainLayout->addLayout(centerLayout);

    mainLayout->addStretch(2);

    setLayout(mainLayout);
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