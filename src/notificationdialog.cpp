#include "notificationdialog.h"
#include "utils/colors.h"
#include "config.h"
#include "materialicons.h"

#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

NotificationDialog::NotificationDialog(const QString& currentUsername, QNetworkAccessManager* netMgr, QWidget* parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog),
      m_currentUsername(currentUsername), m_netMgr(netMgr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(420, 500);

    setupUI();
    fetchPendingRequests();
}

void NotificationDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_container = new QWidget(this);
    m_container->setObjectName("notifContainer");
    m_container->setStyleSheet(
        "QWidget#notifContainer {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 rgba(22, 27, 34, 250),"
        "    stop:1 rgba(13, 17, 23, 250));"
        "  border-radius: 24px;"
        "  border: 1px solid rgba(255, 255, 255, 0.08);"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(28, 32, 28, 28);
    layout->setSpacing(16);

    // Header row: title + close button
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(4);

    QLabel* titleLabel = new QLabel("NOTIFICATIONS");
    titleLabel->setStyleSheet(
        "color: white; font-size: 18px; font-weight: 900; font-family: 'Segoe UI Black';"
        "letter-spacing: 2px; background: transparent; border: none;"
    );
    titleCol->addWidget(titleLabel);

    m_countLabel = new QLabel("FRIEND REQUESTS (0)");
    m_countLabel->setStyleSheet(
        "color: rgba(120, 160, 200, 0.7); font-size: 11px; font-weight: 700;"
        "letter-spacing: 1.5px; font-family: 'Segoe UI';"
        "background: transparent; border: none;"
    );
    titleCol->addWidget(m_countLabel);

    headerLayout->addLayout(titleCol);
    headerLayout->addStretch();

    m_closeBtn = new QPushButton("✕");
    m_closeBtn->setFixedSize(36, 36);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: rgba(255, 255, 255, 0.5);"
        "  font-size: 18px;"
        "  border: none;"
        "  border-radius: 18px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255, 255, 255, 0.1);"
        "  color: white;"
        "}"
    );
    connect(m_closeBtn, &QPushButton::clicked, this, &NotificationDialog::onClose);
    headerLayout->addWidget(m_closeBtn, 0, Qt::AlignTop);

    layout->addLayout(headerLayout);

    // Scrollable requests area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "  width: 4px; background: transparent;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: rgba(255, 255, 255, 0.15);"
        "  border-radius: 2px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    m_requestsLayout = new QVBoxLayout(scrollContent);
    m_requestsLayout->setContentsMargins(0, 0, 0, 0);
    m_requestsLayout->setSpacing(10);
    m_requestsLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    layout->addWidget(m_scrollArea);

    // Empty state label (hidden by default)
    m_emptyLabel = new QLabel("No pending requests");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(
        "color: rgba(255, 255, 255, 0.3); font-size: 13px;"
        "font-family: 'Segoe UI'; background: transparent; border: none;"
        "padding: 40px;"
    );
    m_emptyLabel->hide();
    layout->addWidget(m_emptyLabel);

    mainLayout->addWidget(m_container);
}

QWidget* NotificationDialog::createRequestCard(const QString& username) {
    QWidget* card = new QWidget();
    card->setObjectName("requestCard");
    card->setFixedHeight(72);
    card->setStyleSheet(
        "QWidget#requestCard {"
        "  background: rgba(255, 255, 255, 0.04);"
        "  border-radius: 14px;"
        "  border: 1px solid rgba(255, 255, 255, 0.06);"
        "}"
        "QWidget#requestCard:hover {"
        "  background: rgba(255, 255, 255, 0.07);"
        "}"
    );

    QHBoxLayout* cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(14, 10, 14, 10);
    cardLayout->setSpacing(12);

    // Avatar circle placeholder
    QLabel* avatar = new QLabel();
    avatar->setFixedSize(44, 44);
    avatar->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "  stop:0 rgba(80, 100, 140, 0.6),"
        "  stop:1 rgba(50, 70, 100, 0.6));"
        "border-radius: 22px;"
        "border: 2px solid rgba(255, 255, 255, 0.1);"
    );
    avatar->setAlignment(Qt::AlignCenter);

    // First letter of username as avatar text
    QFont avatarFont("Segoe UI", 16, QFont::Bold);
    avatar->setFont(avatarFont);
    avatar->setText(username.left(1).toUpper());
    cardLayout->addWidget(avatar);

    // Name + subtitle column
    QVBoxLayout* infoCol = new QVBoxLayout();
    infoCol->setSpacing(2);

    QLabel* nameLabel = new QLabel(username);
    nameLabel->setStyleSheet(
        "color: white; font-size: 13px; font-weight: 700;"
        "font-family: 'Segoe UI'; background: transparent; border: none;"
    );
    infoCol->addWidget(nameLabel);

    QLabel* subtitleLabel = new QLabel("WANTS TO JOIN YOUR LOBBY");
    subtitleLabel->setStyleSheet(
        "color: rgba(100, 180, 255, 0.6); font-size: 9px; font-weight: 600;"
        "letter-spacing: 1px; font-family: 'Segoe UI';"
        "background: transparent; border: none;"
    );
    infoCol->addWidget(subtitleLabel);

    cardLayout->addLayout(infoCol);
    cardLayout->addStretch();

    // Accept button (green tick)
    QPushButton* acceptBtn = new QPushButton("✓");
    acceptBtn->setFixedSize(38, 38);
    acceptBtn->setCursor(Qt::PointingHandCursor);
    acceptBtn->setStyleSheet(
        "QPushButton {"
        "  background: rgba(46, 204, 113, 0.15);"
        "  color: rgba(46, 204, 113, 0.8);"
        "  font-size: 18px; font-weight: bold;"
        "  border: 1px solid rgba(46, 204, 113, 0.3);"
        "  border-radius: 19px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(46, 204, 113, 0.3);"
        "  color: rgba(46, 204, 113, 1.0);"
        "  border: 1px solid rgba(46, 204, 113, 0.5);"
        "}"
    );
    connect(acceptBtn, &QPushButton::clicked, this, [this, username, card]() {
        acceptRequest(username, card);
    });
    cardLayout->addWidget(acceptBtn);

    // Reject button (red cross)
    QPushButton* rejectBtn = new QPushButton("✕");
    rejectBtn->setFixedSize(38, 38);
    rejectBtn->setCursor(Qt::PointingHandCursor);
    rejectBtn->setStyleSheet(
        "QPushButton {"
        "  background: rgba(231, 76, 60, 0.15);"
        "  color: rgba(231, 76, 60, 0.8);"
        "  font-size: 16px; font-weight: bold;"
        "  border: 1px solid rgba(231, 76, 60, 0.3);"
        "  border-radius: 19px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(231, 76, 60, 0.3);"
        "  color: rgba(231, 76, 60, 1.0);"
        "  border: 1px solid rgba(231, 76, 60, 0.5);"
        "}"
    );
    connect(rejectBtn, &QPushButton::clicked, this, [this, username, card]() {
        rejectRequest(username, card);
    });
    cardLayout->addWidget(rejectBtn);

    return card;
}

void NotificationDialog::fetchPendingRequests() {
    if (!m_netMgr || m_currentUsername.isEmpty()) return;

    QString url = QString("%1/api/social/requests/pending?username=%2")
        .arg(Config::WEBSERVER_BASE_URL).arg(m_currentUsername);

    QNetworkReply* reply = m_netMgr->get(QNetworkRequest{QUrl(url)});
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isArray()) return;

        QJsonArray arr = doc.array();
        m_pendingCount = arr.size();

        // Clear existing cards (except the stretch at the end)
        while (m_requestsLayout->count() > 1) {
            QLayoutItem* item = m_requestsLayout->takeAt(0);
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }

        if (arr.isEmpty()) {
            m_scrollArea->hide();
            m_emptyLabel->show();
        } else {
            m_emptyLabel->hide();
            m_scrollArea->show();
            for (const QJsonValue& val : arr) {
                QJsonObject obj = val.toObject();
                QString username = obj["username"].toString();
                if (username.isEmpty()) continue;

                QWidget* card = createRequestCard(username);
                // Insert before the stretch
                m_requestsLayout->insertWidget(m_requestsLayout->count() - 1, card);
            }
        }

        updateCountLabel();
    });
}

void NotificationDialog::acceptRequest(const QString& username, QWidget* card) {
    if (!m_netMgr) return;

    QJsonObject payload;
    payload["username"] = m_currentUsername;
    payload["friend_username"] = username;

    QUrl url(Config::WEBSERVER_BASE_URL + "/api/social/request/accept");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_netMgr->post(request, QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, card]() {
        reply->deleteLater();
        if (card) {
            card->hide();
            card->deleteLater();
            m_pendingCount = qMax(0, m_pendingCount - 1);
            updateCountLabel();
            emit requestHandled();
        }
        if (m_pendingCount == 0) {
            m_scrollArea->hide();
            m_emptyLabel->show();
        }
    });
}

void NotificationDialog::rejectRequest(const QString& username, QWidget* card) {
    if (!m_netMgr) return;

    QJsonObject payload;
    payload["username"] = m_currentUsername;
    payload["friend_username"] = username;

    QUrl url(Config::WEBSERVER_BASE_URL + "/api/social/request/reject");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_netMgr->post(request, QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, card]() {
        reply->deleteLater();
        if (card) {
            card->hide();
            card->deleteLater();
            m_pendingCount = qMax(0, m_pendingCount - 1);
            updateCountLabel();
            emit requestHandled();
        }
        if (m_pendingCount == 0) {
            m_scrollArea->hide();
            m_emptyLabel->show();
        }
    });
}

void NotificationDialog::updateCountLabel() {
    m_countLabel->setText(QString("FRIEND REQUESTS (%1)").arg(m_pendingCount));
}

void NotificationDialog::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    // Transparent — MainWindow handles blur overlay
}

void NotificationDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);

    QPropertyAnimation* anim = new QPropertyAnimation(m_container, "geometry", this);
    anim->setDuration(300);
    anim->setEasingCurve(QEasingCurve::OutBack);

    QRect endGeom = m_container->geometry();
    QRect startGeom = endGeom;
    startGeom.translate(0, 20);
    startGeom.adjust(10, 10, -10, -10);

    anim->setStartValue(startGeom);
    anim->setEndValue(endGeom);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationDialog::onClose() {
    close();
}
