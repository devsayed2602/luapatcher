#include "socialpage.h"
#include "utils/colors.h"
#include "materialicons.h"
#include "config.h"
#include <QPainter>
#include <QGraphicsDropShadowEffect>

FriendCard::FriendCard(const QJsonObject& data, bool isRequest, QWidget* parent)
    : QWidget(parent), m_data(data), m_isRequest(isRequest)
{
    setFixedHeight(80);
    setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(16);
    
    // Avatar
    QLabel* avatar = new QLabel();
    avatar->setFixedSize(48, 48);
    avatar->setStyleSheet("background: rgba(255,255,255,0.1); border-radius: 24px; color: white; font-weight: bold; font-size: 18px;");
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setText(data["username"].toString().at(0).toUpper());
    layout->addWidget(avatar);
    
    // Info
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);
    infoLayout->setAlignment(Qt::AlignVCenter);
    
    QLabel* nameLabel = new QLabel(data["username"].toString());
    nameLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold; background: transparent;");
    infoLayout->addWidget(nameLabel);
    
    QLabel* levelLabel = new QLabel(QString("Level %1").arg(data["level"].toInt(1)));
    levelLabel->setStyleSheet(QString("color: %1; font-size: 12px; background: transparent;").arg(Colors::ON_SURFACE_VARIANT));
    infoLayout->addWidget(levelLabel);
    
    layout->addLayout(infoLayout);
    layout->addStretch();
    
    // Actions
    if (m_isRequest) {
        QPushButton* acceptBtn = new QPushButton("ACCEPT");
        acceptBtn->setFixedSize(80, 32);
        acceptBtn->setCursor(Qt::PointingHandCursor);
        acceptBtn->setStyleSheet(QString(
            "QPushButton { background: %1; color: %2; border-radius: 8px; font-weight: bold; font-size: 11px; border: none; }"
            "QPushButton:hover { background: %3; }"
        ).arg(Colors::PRIMARY).arg(Colors::ON_PRIMARY).arg(Colors::PRIMARY_CONTAINER));
        connect(acceptBtn, &QPushButton::clicked, [this](){ emit acceptClicked(m_data["username"].toString()); });
        layout->addWidget(acceptBtn);
    } else if (data.contains("is_search_result")) {
        QPushButton* addBtn = new QPushButton("+ ADD");
        addBtn->setFixedSize(70, 32);
        addBtn->setCursor(Qt::PointingHandCursor);
        addBtn->setStyleSheet(QString(
            "QPushButton { background: rgba(255,255,255,0.1); color: white; border-radius: 8px; font-weight: bold; font-size: 11px; border: 1px solid rgba(255,255,255,0.2); }"
            "QPushButton:hover { background: rgba(255,255,255,0.15); }"
        ));
        connect(addBtn, &QPushButton::clicked, [this](){ emit addClicked(m_data["username"].toString()); });
        layout->addWidget(addBtn);
    }
}

void FriendCard::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1,1,-1,-1), 16, 16);
    
    p.fillPath(path, QColor(255, 255, 255, 15));
    p.setPen(QPen(QColor(255, 255, 255, 25), 1));
    p.drawPath(path);
}

SocialPage::SocialPage(const QString& myUsername, bool isGuest, QNetworkAccessManager* nm, QWidget* parent)
    : QWidget(parent), m_myUsername(myUsername), m_isGuest(isGuest), m_networkManager(nm)
{
    setupUI();
    
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(600);
    connect(m_searchTimer, &QTimer::timeout, this, &SocialPage::doSearch);
    
    refresh();
}

void SocialPage::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);
    
    // Header
    QLabel* title = new QLabel("Social Hub");
    title->setStyleSheet("font-size: 32px; font-weight: 900; color: white;");
    mainLayout->addWidget(title);
    
    // Search Area
    QWidget* searchBox = new QWidget();
    searchBox->setFixedHeight(60);
    searchBox->setStyleSheet("background: rgba(255,255,255,0.06); border-radius: 20px; border: 1px solid rgba(255,255,255,0.1);");
    QHBoxLayout* searchLayout = new QHBoxLayout(searchBox);
    searchLayout->setContentsMargins(20, 0, 20, 0);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Search users by username...");
    m_searchEdit->setStyleSheet("background: transparent; border: none; color: white; font-size: 16px;");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SocialPage::onSearchChanged);
    
    searchLayout->addWidget(m_searchEdit);
    mainLayout->addWidget(searchBox);

    if (m_isGuest) {
        m_searchEdit->setEnabled(false);
        m_searchEdit->setPlaceholderText("Please log in to use social features.");
        
        QLabel* guestMsg = new QLabel("Social features are only available to registered users. Create an account to add friends, track progress, and compare levels!");
        guestMsg->setWordWrap(true);
        guestMsg->setStyleSheet(QString("color: %1; font-size: 14px; background: rgba(255,255,255,0.05); padding: 20px; border-radius: 12px;").arg(Colors::ON_SURFACE_VARIANT));
        mainLayout->addWidget(guestMsg);
        mainLayout->addStretch();
        return;
    }
    
    // Search Results (Dynamic)
    m_searchResultsWidget = new QWidget();
    m_searchResultsLayout = new QVBoxLayout(m_searchResultsWidget);
    m_searchResultsLayout->setContentsMargins(0, 0, 0, 0);
    m_searchResultsLayout->setSpacing(10);
    mainLayout->addWidget(m_searchResultsWidget);
    m_searchResultsWidget->hide();
    
    // Content Columns
    QHBoxLayout* columns = new QHBoxLayout();
    columns->setSpacing(30);
    
    // Left: Friend List
    QVBoxLayout* friendsCol = new QVBoxLayout();
    m_friendsTitle = new QLabel("FRIENDS");
    m_friendsTitle->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; letter-spacing: 1px;").arg(Colors::ON_SURFACE_VARIANT));
    friendsCol->addWidget(m_friendsTitle);
    
    m_friendsWidget = new QWidget();
    m_friendsLayout = new QVBoxLayout(m_friendsWidget);
    m_friendsLayout->setContentsMargins(0, 0, 0, 0);
    m_friendsLayout->setSpacing(12);
    friendsCol->addWidget(m_friendsWidget);
    friendsCol->addStretch();
    
    // Right: Pending Requests
    QVBoxLayout* pendingCol = new QVBoxLayout();
    m_pendingTitle = new QLabel("PENDING REQUESTS");
    m_pendingTitle->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; letter-spacing: 1px;").arg(Colors::ON_SURFACE_VARIANT));
    pendingCol->addWidget(m_pendingTitle);
    
    m_pendingWidget = new QWidget();
    m_pendingLayout = new QVBoxLayout(m_pendingWidget);
    m_pendingLayout->setContentsMargins(0, 0, 0, 0);
    m_pendingLayout->setSpacing(12);
    pendingCol->addWidget(m_pendingWidget);
    pendingCol->addStretch();
    
    columns->addLayout(friendsCol, 2);
    columns->addLayout(pendingCol, 1);
    mainLayout->addLayout(columns);
    mainLayout->addStretch();
}

void SocialPage::setUserData(const QString& username, bool isGuest) {
    m_myUsername = username;
    m_isGuest = isGuest;
    
    // Clear and rebuild UI
    clearLayout(layout());
    delete layout();
    setupUI();
    refresh();
}

void SocialPage::refresh() {
    if (m_isGuest) return;
    // Fetch friends
    QString friendsUrl = Config::WEBSERVER_BASE_URL + "/api/social/friends?username=" + m_myUsername;
    QNetworkReply* fReply = m_networkManager->get(QNetworkRequest(QUrl(friendsUrl)));
    connect(fReply, &QNetworkReply::finished, this, &SocialPage::onFriendsFetched);
    
    // Fetch pending
    QString pendingUrl = Config::WEBSERVER_BASE_URL + "/api/social/requests/pending?username=" + m_myUsername;
    QNetworkReply* pReply = m_networkManager->get(QNetworkRequest(QUrl(pendingUrl)));
    connect(pReply, &QNetworkReply::finished, this, &SocialPage::onPendingFetched);
}

void SocialPage::onSearchChanged(const QString& text) {
    if (text.trimmed().length() < 2) {
        m_searchResultsWidget->hide();
        clearLayout(m_searchResultsLayout);
        return;
    }
    m_searchTimer->start();
}

void SocialPage::doSearch() {
    QString url = Config::WEBSERVER_BASE_URL + "/api/social/search?query=" + m_searchEdit->text().trimmed();
    QNetworkReply* reply = m_networkManager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, &SocialPage::onSearchFinished);
}

void SocialPage::onSearchFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) return;
    
    QJsonArray users = QJsonDocument::fromJson(reply->readAll()).array();
    clearLayout(m_searchResultsLayout);
    
    if (users.isEmpty()) {
        m_searchResultsWidget->hide();
        return;
    }
    
    m_searchResultsWidget->show();
    for (int i = 0; i < users.size(); ++i) {
        QJsonObject user = users[i].toObject();
        if (user["username"].toString() == m_myUsername) continue;
        
        user["is_search_result"] = true;
        FriendCard* card = new FriendCard(user, false, this);
        connect(card, &FriendCard::addClicked, this, &SocialPage::sendFriendRequest);
        m_searchResultsLayout->addWidget(card);
    }
}

void SocialPage::onFriendsFetched() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();
    
    QJsonArray friends = QJsonDocument::fromJson(reply->readAll()).array();
    clearLayout(m_friendsLayout);
    
    m_friendsTitle->setText(QString("FRIENDS (%1)").arg(friends.size()));
    
    if (friends.isEmpty()) {
        QLabel* empty = new QLabel("You haven't added any friends yet.");
        empty->setStyleSheet(QString("color: %1; font-style: italic;").arg(Colors::ON_SURFACE_VARIANT));
        m_friendsLayout->addWidget(empty);
    } else {
        for (int i = 0; i < friends.size(); ++i) {
            m_friendsLayout->addWidget(new FriendCard(friends[i].toObject(), false, this));
        }
    }
}

void SocialPage::onPendingFetched() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();
    
    QJsonArray pending = QJsonDocument::fromJson(reply->readAll()).array();
    clearLayout(m_pendingLayout);
    
    m_pendingTitle->setVisible(!pending.isEmpty());
    m_pendingWidget->setVisible(!pending.isEmpty());
    
    for (int i = 0; i < pending.size(); ++i) {
        FriendCard* card = new FriendCard(pending[i].toObject(), true, this);
        connect(card, &FriendCard::acceptClicked, this, &SocialPage::acceptFriendRequest);
        m_pendingLayout->addWidget(card);
    }
}

void SocialPage::sendFriendRequest(const QString& targetUser) {
    QString url = Config::WEBSERVER_BASE_URL + "/api/social/request/send";
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject body;
    body["from_username"] = m_myUsername;
    body["to_username"] = targetUser;
    
    QNetworkReply* reply = m_networkManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, &SocialPage::onActionFinished);
    
    m_searchEdit->clear();
    m_searchResultsWidget->hide();
}

void SocialPage::acceptFriendRequest(const QString& friendUser) {
    QString url = Config::WEBSERVER_BASE_URL + "/api/social/request/accept";
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject body;
    body["username"] = m_myUsername;
    body["friend_username"] = friendUser;
    
    QNetworkReply* reply = m_networkManager->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, &SocialPage::onActionFinished);
}

void SocialPage::onActionFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) reply->deleteLater();
    refresh();
}

void SocialPage::clearLayout(QLayout* layout) {
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }
}
