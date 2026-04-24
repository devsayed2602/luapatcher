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
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>

AddFriendDialog::AddFriendDialog(const QString& currentUsername, QNetworkAccessManager* netMgr, QWidget* parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog), 
      m_currentUsername(currentUsername), m_netMgr(netMgr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(380, 480); // Increased height for search results
    
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
    
    // Top Row for Close Button
    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->addStretch();
    
    m_closeBtn = new QPushButton("✕");
    m_closeBtn->setFont(QFont("Segoe UI", 16, QFont::Bold));
    m_closeBtn->setFixedSize(32, 32);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(
        "QPushButton { color: rgba(255,255,255,0.5); background: transparent; border: none; border-radius: 16px; }"
        "QPushButton:hover { color: white; background: rgba(255,255,255,0.1); }"
    );
    connect(m_closeBtn, &QPushButton::clicked, this, &AddFriendDialog::onClose);
    topRow->addWidget(m_closeBtn, 0, Qt::AlignTop);
    
    layout->addLayout(topRow);

    // Centered Icon and Titles
    QVBoxLayout* centerLayout = new QVBoxLayout();
    centerLayout->setAlignment(Qt::AlignHCenter);
    centerLayout->setSpacing(12);
    
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
    
    centerLayout->addWidget(iconBox, 0, Qt::AlignHCenter);
    
    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    titleLayout->setAlignment(Qt::AlignHCenter);
    
    QLabel* titleLabel = new QLabel("ADD NEW CONTACT");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: 900; letter-spacing: 1.6px; background: transparent; border: none;");
    
    QLabel* subtitleLabel = new QLabel("SEARCH BY SECURITY HANDLE");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #7A7A7A; font-size: 10px; font-weight: bold; letter-spacing: 1px; background: transparent; border: none;");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    
    centerLayout->addLayout(titleLayout);
    layout->addLayout(centerLayout);
    
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
    
    m_searchResults = new QListWidget();
    m_searchResults->setStyleSheet(
        "QListWidget {"
        "  background: transparent;"
        "  border: none;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  background: rgba(255, 255, 255, 0.05);"
        "  border-radius: 6px;"
        "  margin-bottom: 4px;"
        "  color: white;"
        "  padding: 8px;"
        "}"
        "QListWidget::item:hover {"
        "  background: rgba(255, 255, 255, 0.1);"
        "}"
        "QListWidget::item:selected {"
        "  background: rgba(46, 204, 113, 0.2);"
        "  border: 1px solid rgba(46, 204, 113, 0.5);"
        "}"
    );
    m_searchResults->setCursor(Qt::PointingHandCursor);
    m_searchResults->hide();
    layout->addWidget(m_searchResults);
    
    connect(m_searchInput, &QLineEdit::textChanged, this, &AddFriendDialog::onSearchTextChanged);
    connect(m_searchResults, &QListWidget::itemClicked, this, &AddFriendDialog::onSearchResultClicked);
    
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
    Q_UNUSED(event);
    // Removed black square overlay to fix corner artifacts. 
    // The main window handles background blur and darkening.
}

void AddFriendDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    
    // Animation: Scale from 0.95 to 1.0 and slide up slightly
    QPropertyAnimation* animScale = new QPropertyAnimation(m_container, "geometry", this);
    animScale->setDuration(300);
    animScale->setEasingCurve(QEasingCurve::OutBack);
    
    QPropertyAnimation* animFade = new QPropertyAnimation(m_container, "windowOpacity", this);
    animFade->setDuration(250);
    animFade->setStartValue(0.0);
    animFade->setEndValue(1.0);
    
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

void AddFriendDialog::onSearchTextChanged(const QString& text) {
    if (text.length() < 2) {
        m_searchResults->clear();
        m_searchResults->hide();
        return;
    }
    
    if (m_searchReply) {
        m_searchReply->abort();
        m_searchReply->deleteLater();
        m_searchReply = nullptr;
    }
    
    QString urlStr = QString("%1/api/social/search?query=%2").arg(Config::WEBSERVER_BASE_URL).arg(text);
    QNetworkRequest request{QUrl(urlStr)};
    m_searchReply = m_netMgr->get(request);
    connect(m_searchReply, &QNetworkReply::finished, this, [this]() {
        onSearchFinished(m_searchReply);
    });
}

void AddFriendDialog::onSearchFinished(QNetworkReply* reply) {
    if (reply != m_searchReply) {
        reply->deleteLater();
        return;
    }
    
    m_searchReply = nullptr;
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isArray()) {
            QJsonArray arr = doc.array();
            m_searchResults->clear();
            
            for (const QJsonValue& val : arr) {
                QJsonObject obj = val.toObject();
                QString username = obj["username"].toString();
                
                // Exclude current user from search results
                if (username.compare(m_currentUsername, Qt::CaseInsensitive) == 0) continue;
                
                QListWidgetItem* item = new QListWidgetItem(username);
                item->setData(Qt::UserRole, username); // Store username in UserRole for easy retrieval
                m_searchResults->addItem(item);
            }
            
            if (m_searchResults->count() > 0) {
                m_searchResults->show();
            } else {
                m_searchResults->hide();
            }
        }
    }
    reply->deleteLater();
}

void AddFriendDialog::onSearchResultClicked(QListWidgetItem* item) {
    if (item) {
        m_searchInput->setText(item->data(Qt::UserRole).toString());
        m_searchResults->hide();
    }
}
