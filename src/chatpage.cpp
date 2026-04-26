#include "chatpage.h"
#include "utils/colors.h"
#include "materialicons.h"
#include "config.h"
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>

ChatPage::ChatPage(const QString& myUsername, const QString& friendUsername, QNetworkAccessManager* netMgr, QWidget* parent)
    : QWidget(parent), m_myUsername(myUsername), m_friendUsername(friendUsername), m_netMgr(netMgr)
{
    setupUI();

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &ChatPage::fetchHistory);
    m_pollTimer->start(2000); // Poll every 2 seconds

    fetchHistory();
}

ChatPage::~ChatPage() {
    m_pollTimer->stop();
}

void ChatPage::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 20);
    mainLayout->setSpacing(0);

    // Header
    QWidget* header = new QWidget();
    header->setFixedHeight(70);
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(16);

    QPushButton* backBtn = new QPushButton("← Back");
    backBtn->setFixedSize(90, 36);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton {"
        "  font-size: 14px; font-weight: bold; color: #EFECE3;"
        "  background: rgba(0,0,0,0.4); border: 1px solid rgba(255,255,255,0.1);"
        "  border-radius: 18px; padding: 6px 16px;"
        "  font-family: 'Segoe UI';"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255,255,255,0.15); border-color: rgba(255,255,255,0.25);"
        "}"
    );
    connect(backBtn, &QPushButton::clicked, this, &ChatPage::backRequested);
    headerLayout->addWidget(backBtn);

    // Avatar Placeholder
    QLabel* avatar = new QLabel();
    avatar->setFixedSize(44, 44);
    avatar->setStyleSheet("background: #4A6FA5; border-radius: 22px; color: white; font-weight: bold; font-size: 16px;");
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setText(m_friendUsername.left(1).toUpper());
    headerLayout->addWidget(avatar);

    QVBoxLayout* nameCol = new QVBoxLayout();
    nameCol->setSpacing(2);
    m_friendNameLabel = new QLabel(m_friendUsername);
    m_friendNameLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold; border: none; background: transparent;");
    nameCol->addWidget(m_friendNameLabel);

    m_statusLabel = new QLabel("● ACTIVE NOW");
    m_statusLabel->setStyleSheet("color: #2ECC71; font-size: 10px; font-weight: 800; border: none; background: transparent;");
    nameCol->addWidget(m_statusLabel);
    headerLayout->addLayout(nameCol);
    headerLayout->addStretch();

    mainLayout->addWidget(header);

    // Chat Area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    m_chatLayout = new QVBoxLayout(scrollContent);
    m_chatLayout->setContentsMargins(0, 20, 0, 20);
    m_chatLayout->setSpacing(20);
    m_chatLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(m_scrollArea);

    // Input Area
    QWidget* inputContainer = new QWidget();
    inputContainer->setFixedHeight(60);
    inputContainer->setStyleSheet("background: rgba(255, 255, 255, 0.05); border-radius: 30px;");
    
    QHBoxLayout* inputLayout = new QHBoxLayout(inputContainer);
    inputLayout->setContentsMargins(20, 0, 10, 0);

    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Message " + m_friendUsername + "...");
    m_messageInput->setStyleSheet("color: white; font-size: 14px; border: none; background: transparent; padding: 10px;");
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatPage::sendMessage);
    inputLayout->addWidget(m_messageInput);

    QPushButton* sendBtn = new QPushButton();
    sendBtn->setFixedSize(40, 40);
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setStyleSheet("background: white; border-radius: 20px; border: none;");
    
    // Simple arrow icon for send
    QLabel* sendIcon = new QLabel(sendBtn);
    sendIcon->setGeometry(12, 10, 20, 20);
    sendIcon->setStyleSheet("color: #1a1c23; font-size: 16px; font-weight: bold; background: transparent;");
    sendIcon->setText("➤");
    sendIcon->setAlignment(Qt::AlignCenter);

    connect(sendBtn, &QPushButton::clicked, this, &ChatPage::sendMessage);
    inputLayout->addWidget(sendBtn);

    mainLayout->addWidget(inputContainer);
}

void ChatPage::addMessageBubble(const QString& text, bool isMe, const QString& time) {
    QWidget* row = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* bubbleStack = new QVBoxLayout();
    bubbleStack->setSpacing(4);

    QLabel* bubble = new QLabel(text);
    bubble->setWordWrap(true);
    bubble->setMinimumHeight(40);
    bubble->setMaximumWidth(400);

    if (isMe) {
        bubble->setStyleSheet(
            "background: #F0F0F0; color: #1a1c23; border-radius: 20px; "
            "border-bottom-right-radius: 4px; padding: 12px 18px; font-size: 13px;"
        );
        rowLayout->addStretch();
        bubbleStack->addWidget(bubble);
        QLabel* timeLabel = new QLabel(time);
        timeLabel->setStyleSheet("color: rgba(255, 255, 255, 0.4); font-size: 10px;");
        timeLabel->setAlignment(Qt::AlignRight);
        bubbleStack->addWidget(timeLabel);
        rowLayout->addLayout(bubbleStack);
    } else {
        bubble->setStyleSheet(
            "background: rgba(255, 255, 255, 0.1); color: white; border-radius: 20px; "
            "border-bottom-left-radius: 4px; padding: 12px 18px; font-size: 13px;"
        );
        bubbleStack->addWidget(bubble);
        QLabel* timeLabel = new QLabel(time);
        timeLabel->setStyleSheet("color: rgba(255, 255, 255, 0.4); font-size: 10px;");
        timeLabel->setAlignment(Qt::AlignLeft);
        bubbleStack->addWidget(timeLabel);
        rowLayout->addLayout(bubbleStack);
        rowLayout->addStretch();
    }

    m_chatLayout->insertWidget(m_chatLayout->count() - 1, row);
    
    // Auto scroll to bottom
    QTimer::singleShot(50, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
    });
}

void ChatPage::sendMessage() {
    QString msg = m_messageInput->text().trimmed();
    if (msg.isEmpty()) return;

    m_messageInput->clear();

    QJsonObject obj;
    obj["sender"] = m_myUsername;
    obj["receiver"] = m_friendUsername;
    obj["message"] = msg;

    QUrl url(Config::WEBSERVER_BASE_URL + "/api/social/chat/send");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_netMgr->post(request, QJsonDocument(obj).toJson());
    
    // Optimistically add bubble
    addMessageBubble(msg, true, QDateTime::currentDateTime().toString("h:mm AP"));
}

void ChatPage::fetchHistory() {
    QUrl url(Config::WEBSERVER_BASE_URL + "/api/social/chat/history");
    url.setQuery("user1=" + m_myUsername + "&user2=" + m_friendUsername);
    
    QNetworkReply* reply = m_netMgr->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &ChatPage::onHistoryFetched);
}

void ChatPage::onHistoryFetched() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) return;

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isArray()) return;

    QJsonArray arr = doc.array();
    
    // Optimization: Only redraw if count changed or for first load
    if (arr.size() == m_lastMessageCount) return;
    m_lastMessageCount = arr.size();

    // Clear old messages (except stretch)
    while (m_chatLayout->count() > 1) {
        QLayoutItem* item = m_chatLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }

    for (int i = 0; i < arr.size(); ++i) {
        QJsonObject obj = arr[i].toObject();
        QString sender = obj["sender_username"].toString();
        QString text = obj["message_text"].toString();
        QString timeStr = obj["created_at"].toString();
        
        // Parse timestamp for display (e.g. 2024-04-26T14:22:56...)
        QDateTime dt = QDateTime::fromString(timeStr, Qt::ISODate);
        QString displayTime = dt.isNull() ? "" : dt.toLocalTime().toString("h:mm AP");

        addMessageBubble(text, sender == m_myUsername, displayTime);
    }
    
    // Final scroll to bottom after population
    QScrollBar* vBar = m_scrollArea->verticalScrollBar();
    vBar->setValue(vBar->maximum());
}
