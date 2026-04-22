#include "addfrienddialog.h"
#include "utils/colors.h"
#include "config.h"
#include "materialicons.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

AddFriendDialog::AddFriendDialog(const QString& currentUsername, QNetworkAccessManager* netMgr, QWidget* parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog), 
      m_currentUsername(currentUsername), m_netMgr(netMgr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(380, 360);
    
    setupUI();
}

void AddFriendDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_container = new QWidget(this);
    m_container->setObjectName("modalContainer");
    m_container->setStyleSheet(
        "QWidget#modalContainer {"
        "  background: #000000;"
        "  border-radius: 24px;"
        "  border: 1px solid rgba(255, 255, 255, 0.1);"
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(24, 32, 24, 32);
    layout->setSpacing(20);
    
    // Header Stack
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(16);
    
    QWidget* iconBox = new QWidget();
    iconBox->setFixedSize(48, 48);
    iconBox->setStyleSheet("background: rgba(255, 255, 255, 0.1); border-radius: 12px; border: none;");
    QVBoxLayout* iconBoxLayout = new QVBoxLayout(iconBox);
    iconBoxLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* iconLabel = new QLabel();
    QPixmap iconPix(24, 24);
    iconPix.fill(Qt::transparent);
    QPainter p(&iconPix);
    MaterialIcons::draw(p, QRectF(0, 0, 24, 24), Qt::white, MaterialIcons::PersonAdd);
    p.end();
    iconLabel->setPixmap(iconPix);
    iconLabel->setStyleSheet("background: transparent; border: none;");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconBoxLayout->addWidget(iconLabel);
    headerLayout->addWidget(iconBox);
    
    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    QLabel* titleLabel = new QLabel("ADD NEW CONTACT");
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: 900; letter-spacing: 1.6px; background: transparent; border: none;");
    QLabel* subtitleLabel = new QLabel("SEARCH BY SECURITY HANDLE");
    subtitleLabel->setStyleSheet("color: #7A7A7A; font-size: 10px; font-weight: bold; letter-spacing: 1px; background: transparent; border: none;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addStretch();
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    
    // Close Button
    m_closeBtn = new QPushButton("✕");
    m_closeBtn->setFont(QFont("Segoe UI", 16, QFont::Bold));
    m_closeBtn->setFixedSize(32, 32);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(
        "QPushButton { color: rgba(255,255,255,0.5); background: transparent; border: none; border-radius: 16px; }"
        "QPushButton:hover { color: white; background: rgba(255,255,255,0.1); }"
    );
    connect(m_closeBtn, &QPushButton::clicked, this, &AddFriendDialog::onClose);
    headerLayout->addWidget(m_closeBtn, 0, Qt::AlignTop);
    
    layout->addLayout(headerLayout);
    
    layout->addSpacing(10);
    
    // Functional Input Area
    QWidget* inputWrapper = new QWidget();
    inputWrapper->setFixedHeight(48);
    inputWrapper->setStyleSheet(
        "background: rgba(0, 0, 0, 0.4);"
        "border: 1px solid rgba(255, 255, 255, 0.1);"
        "border-radius: 8px;"
    );
    QHBoxLayout* inputLayout = new QHBoxLayout(inputWrapper);
    inputLayout->setContentsMargins(12, 0, 12, 0);
    inputLayout->setSpacing(10);
    
    QLabel* searchIcon = new QLabel("@");
    searchIcon->setFont(QFont("Consolas", 14, QFont::Bold));
    searchIcon->setStyleSheet("color: rgba(255, 255, 255, 0.4); background: transparent; border: none;");
    inputLayout->addWidget(searchIcon);
    
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("username");
    m_searchInput->setFont(QFont("Consolas", 13));
    m_searchInput->setStyleSheet("background: transparent; color: white; border: none;");
    inputLayout->addWidget(m_searchInput);
    
    layout->addWidget(inputWrapper);
    
    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color: #E74C3C; font-size: 11px; background: transparent; border: none;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);
    
    layout->addStretch();
    
    // Primary Button
    m_sendBtn = new QPushButton("SEND INVITATION");
    m_sendBtn->setFixedHeight(48);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton {"
        "  background: #EFECE3;"
        "  color: black;"
        "  font-size: 14px;"
        "  font-weight: 900;"
        "  letter-spacing: 1px;"
        "  border-radius: 8px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background: white;"
        "}"
    );
    
    // Luminous Glow
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(this);
    glow->setBlurRadius(20);
    glow->setColor(QColor("#EFECE3"));
    glow->setOffset(0, 0);
    m_sendBtn->setGraphicsEffect(glow);
    
    connect(m_sendBtn, &QPushButton::clicked, this, &AddFriendDialog::sendInvitation);
    layout->addWidget(m_sendBtn);
    
    mainLayout->addWidget(m_container);
}

void AddFriendDialog::sendInvitation() {
    QString target = m_searchInput->text().trimmed();
    if (target.isEmpty()) {
        m_statusLabel->setText("HANDLE CANNOT BE EMPTY");
        m_statusLabel->show();
        return;
    }
    
    m_sendBtn->setEnabled(false);
    m_sendBtn->setText("SENDING...");
    
    QJsonObject payload;
    payload["from_username"] = m_currentUsername;
    payload["to_username"] = target;
    
    QUrl url(Config::WEBSERVER_BASE_URL + "/api/social/request/send");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_netMgr->post(request, QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_sendBtn->setEnabled(true);
        m_sendBtn->setText("SEND INVITATION");
        
        if (reply->error() == QNetworkReply::NoError) {
            m_statusLabel->setStyleSheet("color: #2ECC71; font-size: 11px; background: transparent; border: none;");
            m_statusLabel->setText("INVITATION SENT SUCCESSFULLY");
            m_statusLabel->show();
            m_searchInput->clear();
        } else {
            m_statusLabel->setStyleSheet("color: #E74C3C; font-size: 11px; background: transparent; border: none;");
            m_statusLabel->setText("USER NOT FOUND OR ALREADY INVITED");
            m_statusLabel->show();
        }
    });
}

void AddFriendDialog::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 150)); // Dark semi-transparent overlay
}

void AddFriendDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    
    // Animation: Scale from 0.95 to 1.0 and slide up slightly
    QPropertyAnimation* animScale = new QPropertyAnimation(m_container, "geometry", this);
    animScale->setDuration(300);
    animScale->setEasingCurve(QEasingCurve::OutBack);
    
    QRect endGeom = m_container->geometry();
    QRect startGeom = endGeom;
    startGeom.translate(0, 20); // Slide up by 20px
    
    // Simulate scale by adjusting size and moving to keep centered
    startGeom.adjust(10, 10, -10, -10);
    
    animScale->setStartValue(startGeom);
    animScale->setEndValue(endGeom);
    animScale->start(QAbstractAnimation::DeleteWhenStopped);
}

void AddFriendDialog::onClose() {
    close();
}
