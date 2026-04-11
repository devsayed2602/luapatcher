#include "onboardingdialog.h"
#include "utils/colors.h"
#include "config.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGraphicsDropShadowEffect>
#include <QRegularExpression>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>

OnboardingDialog::OnboardingDialog(QWidget* parent)
    : QDialog(parent)
    , m_isAvailable(false)
    , m_isChecking(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    
    // Size to fill most of the screen
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int w = qMin(600, screenGeometry.width() - 100);
    int h = qMin(500, screenGeometry.height() - 100);
    setFixedSize(w, h);
    
    // Center on screen
    move(screenGeometry.center() - QPoint(w/2, h/2));
    
    // Network
    m_networkManager = new QNetworkAccessManager(this);
    
    // Debounce timer for availability check
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(500);
    connect(m_debounceTimer, &QTimer::timeout, this, &OnboardingDialog::checkAvailability);
    
    // ── Layout ──
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 60, 50, 50);
    mainLayout->setSpacing(0);
    
    // App icon area
    QLabel* iconLabel = new QLabel("⚙");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 48px; color: #D0BCFF; margin-bottom: 10px;");
    mainLayout->addWidget(iconLabel);
    
    mainLayout->addSpacing(16);
    
    // Title
    m_titleLabel = new QLabel("Welcome to Lua Patcher");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(QString(
        "font-size: 28px; font-weight: bold; color: %1; font-family: 'Segoe UI', 'Roboto';"
    ).arg(Colors::ON_SURFACE));
    mainLayout->addWidget(m_titleLabel);
    
    mainLayout->addSpacing(8);
    
    // Subtitle
    m_subtitleLabel = new QLabel("Choose a username to get started");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setStyleSheet(QString(
        "font-size: 14px; color: %1; font-family: 'Segoe UI', 'Roboto';"
    ).arg(Colors::ON_SURFACE_VARIANT));
    mainLayout->addWidget(m_subtitleLabel);
    
    mainLayout->addSpacing(40);
    
    // Username input
    m_usernameInput = new QLineEdit();
    m_usernameInput->setPlaceholderText("Enter username...");
    m_usernameInput->setMaxLength(20);
    m_usernameInput->setFixedHeight(52);
    m_usernameInput->setStyleSheet(QString(
        "QLineEdit {"
        "    background: rgba(255, 255, 255, 8);"
        "    border: 2px solid rgba(255, 255, 255, 20);"
        "    border-radius: 16px;"
        "    font-size: 16px;"
        "    color: %1;"
        "    padding: 0 20px;"
        "    font-family: 'Segoe UI', 'Roboto';"
        "}"
        "QLineEdit:focus {"
        "    border-color: %2;"
        "    background: rgba(255, 255, 255, 12);"
        "}"
    ).arg(Colors::ON_SURFACE).arg(Colors::PRIMARY));
    connect(m_usernameInput, &QLineEdit::textChanged, this, &OnboardingDialog::onUsernameChanged);
    mainLayout->addWidget(m_usernameInput);
    
    mainLayout->addSpacing(8);
    
    // Status label (availability indicator)
    m_statusLabel = new QLabel("");
    m_statusLabel->setAlignment(Qt::AlignLeft);
    m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; font-family: 'Segoe UI';");
    m_statusLabel->setFixedHeight(20);
    mainLayout->addWidget(m_statusLabel);
    
    mainLayout->addSpacing(24);
    
    // Continue button
    m_continueBtn = new QPushButton("Continue");
    m_continueBtn->setFixedHeight(52);
    m_continueBtn->setCursor(Qt::PointingHandCursor);
    m_continueBtn->setEnabled(false);
    m_continueBtn->setStyleSheet(QString(
        "QPushButton {"
        "    background: %1;"
        "    border: none;"
        "    border-radius: 16px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: %2;"
        "    font-family: 'Segoe UI', 'Roboto';"
        "}"
        "QPushButton:hover {"
        "    background: %3;"
        "}"
        "QPushButton:disabled {"
        "    background: rgba(255, 255, 255, 10);"
        "    color: rgba(255, 255, 255, 40);"
        "}"
        "QPushButton:pressed {"
        "    background: %4;"
        "}"
    ).arg(Colors::PRIMARY).arg(Colors::ON_PRIMARY)
     .arg(Colors::PRIMARY_CONTAINER).arg(Colors::PRIMARY));
    connect(m_continueBtn, &QPushButton::clicked, this, &OnboardingDialog::onRegisterClicked);
    mainLayout->addWidget(m_continueBtn);
    
    mainLayout->addStretch();
    
    // Version info at bottom
    QLabel* versionLabel = new QLabel(QString("v%1 • by leVI & raxnmint").arg(Config::APP_VERSION));
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(QString(
        "font-size: 11px; color: rgba(255, 255, 255, 60); font-family: 'Segoe UI';"
    ));
    mainLayout->addWidget(versionLabel);
}

void OnboardingDialog::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw glass background
    QRectF r = QRectF(rect());
    QPainterPath path;
    path.addRoundedRect(r, 28, 28);
    
    // Dark glass fill
    painter.fillPath(path, QColor(10, 12, 20, 240));
    
    // Subtle ambient glow at top
    QRadialGradient glow(r.width() * 0.5, r.height() * 0.15, r.width() * 0.6);
    QColor gc = Colors::currentTheme.primary;
    gc.setAlpha(30);
    glow.setColorAt(0, gc);
    glow.setColorAt(1, QColor(0, 0, 0, 0));
    painter.setClipPath(path);
    painter.fillRect(r, glow);
    
    // Glass border
    painter.setClipRect(rect());
    QPen borderPen(QColor(255, 255, 255, 25), 1.5);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(r.adjusted(0.75, 0.75, -0.75, -0.75), 28, 28);
}

QString OnboardingDialog::username() const {
    return m_username;
}

void OnboardingDialog::onUsernameChanged(const QString& text) {
    m_continueBtn->setEnabled(false);
    m_isAvailable = false;
    
    QString trimmed = text.trimmed();
    
    if (trimmed.isEmpty()) {
        m_statusLabel->setText("");
        return;
    }
    
    // Client-side validation
    if (trimmed.length() < 3) {
        m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #F2B8B5;");
        m_statusLabel->setText("⚠ Must be at least 3 characters");
        return;
    }
    
    QRegularExpression validPattern("^[a-zA-Z0-9_]+$");
    if (!validPattern.match(trimmed).hasMatch()) {
        m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #F2B8B5;");
        m_statusLabel->setText("⚠ Only letters, numbers, and underscores");
        return;
    }
    
    // Show checking state
    m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #CAC4D0;");
    m_statusLabel->setText("⏳ Checking availability...");
    m_isChecking = true;
    
    // Debounce the API call
    m_debounceTimer->start();
}

void OnboardingDialog::checkAvailability() {
    QString username = m_usernameInput->text().trimmed();
    if (username.isEmpty() || username.length() < 3) return;
    
    QString url = Config::WEBSERVER_BASE_URL + "/api/user/check/" + username;
    QUrl requestUrl(url);
    QNetworkRequest request{requestUrl};
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onCheckFinished(reply);
    });
}

void OnboardingDialog::onCheckFinished(QNetworkReply* reply) {
    reply->deleteLater();
    m_isChecking = false;
    
    if (reply->error() != QNetworkReply::NoError) {
        m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #F2B8B5;");
        m_statusLabel->setText("⚠ Could not check availability");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    
    bool available = obj.value("available").toBool(false);
    m_isAvailable = available;
    
    if (available) {
        m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #A8DB8F;");
        m_statusLabel->setText("✓ Username is available!");
        m_continueBtn->setEnabled(true);
    } else {
        m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #F2B8B5;");
        m_statusLabel->setText("✗ Username is already taken");
        m_continueBtn->setEnabled(false);
    }
}

void OnboardingDialog::onRegisterClicked() {
    if (!m_isAvailable) return;
    
    m_continueBtn->setEnabled(false);
    m_continueBtn->setText("Creating account...");
    
    QString username = m_usernameInput->text().trimmed();
    QString url = Config::WEBSERVER_BASE_URL + "/api/user/register";
    
    QUrl registerUrl(url);
    QNetworkRequest request{registerUrl};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject body;
    body["username"] = username;
    QJsonDocument doc(body);
    
    QNetworkReply* reply = m_networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onRegisterFinished(reply);
    });
}

void OnboardingDialog::onRegisterFinished(QNetworkReply* reply) {
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString errorMsg = doc.object().value("error").toString("Registration failed");
        
        m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #F2B8B5;");
        m_statusLabel->setText("✗ " + errorMsg);
        m_continueBtn->setEnabled(true);
        m_continueBtn->setText("Continue");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    
    if (obj.value("success").toBool()) {
        m_username = obj.value("username").toString();
        accept(); // Close dialog with QDialog::Accepted
    } else {
        m_statusLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #F2B8B5;");
        m_statusLabel->setText("✗ " + obj.value("error").toString("Unknown error"));
        m_continueBtn->setEnabled(true);
        m_continueBtn->setText("Continue");
    }
}
