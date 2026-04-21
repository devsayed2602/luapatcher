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
    , m_currentMode(WELCOME)
    , m_isAvailable(false)
    , m_isChecking(false)
    , m_isGuest(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int w = 500;
    int h = 600;
    setFixedSize(w, h);
    move(screenGeometry.center() - QPoint(w/2, h/2));
    
    m_networkManager = new QNetworkAccessManager(this);
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(300); // Faster check
    connect(m_debounceTimer, &QTimer::timeout, this, &OnboardingDialog::checkAvailability);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &OnboardingDialog::onCheckFinished);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(0);

    // ── Welcome View ──
    m_welcomeView = new QWidget(this);
    QVBoxLayout* welcomeLayout = new QVBoxLayout(m_welcomeView);
    welcomeLayout->setContentsMargins(0, 0, 0, 0);
    welcomeLayout->setSpacing(12);

    QLabel* logoLbl = new QLabel("⚙");
    logoLbl->setAlignment(Qt::AlignCenter);
    logoLbl->setStyleSheet("font-size: 64px; color: #D0BCFF; margin-bottom: 20px;");
    welcomeLayout->addWidget(logoLbl);

    QLabel* welcomeTitle = new QLabel("Lua Patcher");
    welcomeTitle->setAlignment(Qt::AlignCenter);
    welcomeTitle->setStyleSheet("font-size: 32px; font-weight: 800; color: #FFFFFF; font-family: 'Segoe UI';");
    welcomeLayout->addWidget(welcomeTitle);

    QLabel* welcomeSub = new QLabel("Experience the next generation of patching");
    welcomeSub->setAlignment(Qt::AlignCenter);
    welcomeSub->setStyleSheet("font-size: 14px; color: rgba(255,255,255,160); margin-bottom: 40px;");
    welcomeLayout->addWidget(welcomeSub);

    auto createMenuBtn = [this](const QString& text, const QString& bg, const QString& fg) {
        QPushButton* btn = new QPushButton(text);
        btn->setFixedHeight(54);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background: %1; color: %2; border-radius: 16px; font-size: 16px; font-weight: bold; border: none;"
            "}"
            "QPushButton:hover { background: rgba(255,255,255,20); }"
        ).arg(bg).arg(fg));
        return btn;
    };

    QPushButton* loginBtn = createMenuBtn("LOGIN", "rgba(255,255,255,15)", "#FFFFFF");
    QPushButton* signupBtn = createMenuBtn("SIGN UP", Colors::PRIMARY, Colors::ON_PRIMARY);
    QPushButton* guestBtn = createMenuBtn("CONTINUE AS GUEST", "transparent", "rgba(255,255,255,120)");
    
    welcomeLayout->addWidget(signupBtn);
    welcomeLayout->addWidget(loginBtn);
    welcomeLayout->addSpacing(10);
    welcomeLayout->addWidget(guestBtn);
    welcomeLayout->addStretch();

    connect(loginBtn, &QPushButton::clicked, this, [this](){ switchMode(LOGIN); });
    connect(signupBtn, &QPushButton::clicked, this, [this](){ switchMode(REGISTER); });
    connect(guestBtn, &QPushButton::clicked, this, &OnboardingDialog::onGuestClicked);

    mainLayout->addWidget(m_welcomeView);

    // ── Form View (Login/Register) ──
    m_formView = new QWidget(this);
    QVBoxLayout* formLayout = new QVBoxLayout(m_formView);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(0);

    m_backBtn = new QPushButton("← BACK");
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setStyleSheet("QPushButton { background: transparent; color: #8FABD4; border: none; font-weight: bold; font-size: 13px; text-align: left; }");
    connect(m_backBtn, &QPushButton::clicked, this, [this](){ switchMode(WELCOME); });
    formLayout->addWidget(m_backBtn);
    formLayout->addSpacing(30);

    m_titleLabel = new QLabel("Create Account");
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #FFFFFF;");
    formLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel("Set up your unique identity");
    m_subtitleLabel->setStyleSheet("font-size: 14px; color: rgba(255,255,255,160); margin-bottom: 30px;");
    formLayout->addWidget(m_subtitleLabel);

    auto createInput = [](const QString& ph, bool password = false) {
        QLineEdit* le = new QLineEdit();
        le->setPlaceholderText(ph);
        le->setFixedHeight(52);
        if (password) le->setEchoMode(QLineEdit::Password);
        le->setStyleSheet(
            "QLineEdit {"
            "  background: rgba(255,255,255,0.08); border: 1px solid rgba(255,255,255,0.15);"
            "  border-radius: 14px; color: white; padding: 0 16px; font-size: 15px;"
            "}"
            "QLineEdit:focus { border-color: #D0BCFF; background: rgba(255,255,255,0.12); }"
        );
        return le;
    };

    m_usernameInput = createInput("Username");
    m_passwordInput = createInput("Password", true);
    
    connect(m_usernameInput, &QLineEdit::returnPressed, this, &OnboardingDialog::onPrimaryClicked);
    connect(m_passwordInput, &QLineEdit::returnPressed, this, &OnboardingDialog::onPrimaryClicked);
    
    formLayout->addWidget(m_usernameInput);
    formLayout->addSpacing(12);
    formLayout->addWidget(m_passwordInput);
    
    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("font-size: 12px; margin-top: 8px; color: #F2B8B5;");
    m_statusLabel->setWordWrap(true);
    formLayout->addWidget(m_statusLabel);
    
    formLayout->addSpacing(30);

    m_continueBtn = new QPushButton("REGISTER");
    m_continueBtn->setFixedHeight(54);
    m_continueBtn->setCursor(Qt::PointingHandCursor);
    m_continueBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background: %1; color: %2; border-radius: 16px; font-size: 16px; font-weight: bold; border: none;"
        "}"
        "QPushButton:hover { background: %3; }"
        "QPushButton:disabled { background: rgba(255,255,255,0.1); color: rgba(255,255,255,0.3); }"
    ).arg(Colors::PRIMARY).arg(Colors::ON_PRIMARY).arg(Colors::PRIMARY_CONTAINER));
    
    connect(m_usernameInput, &QLineEdit::textChanged, this, &OnboardingDialog::onUsernameChanged);
    connect(m_continueBtn, &QPushButton::clicked, this, &OnboardingDialog::onPrimaryClicked);
    formLayout->addWidget(m_continueBtn);
    formLayout->addStretch();

    mainLayout->addWidget(m_formView);
    m_formView->hide();
}

void OnboardingDialog::switchMode(int mode) {
    m_currentMode = mode;
    m_statusLabel->setText("");
    m_usernameInput->clear();
    m_passwordInput->clear();

    if (mode == WELCOME) {
        m_welcomeView->show();
        m_formView->hide();
    } else {
        m_welcomeView->hide();
        m_formView->show();
        m_titleLabel->setText(mode == LOGIN ? "Welcome Back" : "Sign Up");
        m_subtitleLabel->setText(mode == LOGIN ? "Log in to sync your level and avatar" : "Register to track your stats and level up");
        m_continueBtn->setText(mode == LOGIN ? "LOGIN" : "SIGN UP");
        m_continueBtn->setEnabled(mode == LOGIN); // Login always enabled, Register needs check
        m_usernameInput->setFocus();
    }
}

void OnboardingDialog::onGuestClicked() {
    m_isGuest = true;
    accept();
}

void OnboardingDialog::onUsernameChanged(const QString& text) {
    if (m_currentMode == LOGIN) return;
    
    m_continueBtn->setEnabled(false);
    m_isAvailable = false;
    QString trimmed = text.trimmed();
    
    if (trimmed.isEmpty()) { m_statusLabel->setText(""); return; }
    if (trimmed.length() < 3) { m_statusLabel->setText("⚠ Minimum 3 characters"); return; }
    
    m_statusLabel->setStyleSheet("color: rgba(255,255,255,160);");
    m_statusLabel->setText("⏳ Checking...");
    m_debounceTimer->start();
}

void OnboardingDialog::checkAvailability() {
    QString url = Config::WEBSERVER_BASE_URL + "/api/user/check/" + m_usernameInput->text().trimmed();
    QNetworkReply* reply = m_networkManager->get(QNetworkRequest(QUrl(url)));
    reply->setProperty("type", "check");
}

void OnboardingDialog::onCheckFinished(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply->property("type").toString() != "check") return;
    if (reply->error() != QNetworkReply::NoError) {
         m_statusLabel->setText("✗ Connection error");
         return;
    }
    
    QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    m_isAvailable = obj["available"].toBool();
    m_statusLabel->setText(m_isAvailable ? "✓ Username available" : "✗ Already taken");
    m_statusLabel->setStyleSheet(m_isAvailable ? "color: #A8DB8F;" : "color: #F2B8B5;");
    m_continueBtn->setEnabled(m_isAvailable);
}

void OnboardingDialog::onPrimaryClicked() {
    QString user = m_usernameInput->text().trimmed();
    QString pass = m_passwordInput->text();
    
    if (user.isEmpty() || pass.isEmpty()) return;
    
    m_continueBtn->setEnabled(false);
    m_continueBtn->setText("Processing...");
    
    QString endpoint = (m_currentMode == LOGIN) ? "/api/user/login" : "/api/user/register";
    QNetworkRequest req(QUrl(Config::WEBSERVER_BASE_URL + endpoint));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject body;
    body["username"] = user;
    body["password"] = pass;
    
    QNetworkReply* reply = m_networkManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){ onAuthFinished(reply); });
}

void OnboardingDialog::onAuthFinished(QNetworkReply* reply) {
    reply->deleteLater();
    
    QByteArray responseData = reply->readAll();
    QJsonObject obj = QJsonDocument::fromJson(responseData).object();
    
    if (reply->error() == QNetworkReply::NoError && obj["success"].toBool()) {
        m_userData = obj["user"].toObject();
        m_username = m_userData["username"].toString();
        accept();
    } else {
        QString errorMsg = "Auth failed";
        
        if (reply->error() != QNetworkReply::NoError) {
            // If it's a network error but we have a JSON body, use the server's message
            if (!obj["error"].toString().isEmpty()) {
                errorMsg = obj["error"].toString();
            } else {
                // Otherwise use the Qt network error string
                errorMsg = reply->errorString();
            }
        } else {
            // No network error, but success was false
            errorMsg = obj["error"].toString("Auth failed");
        }
        
        m_statusLabel->setText("✗ " + errorMsg);
        m_statusLabel->setStyleSheet("color: #F2B8B5;");
        m_continueBtn->setEnabled(true);
        m_continueBtn->setText(m_currentMode == LOGIN ? "LOGIN" : "SIGN UP");
    }
}

void OnboardingDialog::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 24, 24);
    p.fillPath(path, QColor(15, 18, 28, 250));
    p.setPen(QPen(QColor(255,255,255,30), 1.5));
    p.drawPath(path);
}

QString OnboardingDialog::username() const { return m_username; }
QJsonObject OnboardingDialog::userData() const { return m_userData; }
bool OnboardingDialog::isGuest() const { return m_isGuest; }
