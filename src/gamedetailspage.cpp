#include "gamedetailspage.h"
#include "glassbutton.h"
#include "utils/colors.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QEventLoop>
#include <QScrollBar>

GameDetailsPage::GameDetailsPage(QNetworkAccessManager* networkManager, QWidget* parent)
    : QWidget(parent), m_networkManager(networkManager)
{
    setFocusPolicy(Qt::StrongFocus); // For Escape key
    buildUI();
}

void GameDetailsPage::buildUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Top Bar (Back button)
    QWidget* topBar = new QWidget();
    topBar->setFixedHeight(60);
    topBar->setStyleSheet("background: rgba(18, 19, 23, 180); border-bottom: 1px solid rgba(255, 255, 255, 10);");
    QHBoxLayout* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 0, 20, 0);

    GlassButton* backBtn = new GlassButton(MaterialIcons::Refresh, "Back", "", Colors::ON_SURFACE_VARIANT, this);
    // Let's just use the button with a smaller size and different style natively, 
    // but GlassButton is fine. We will configure it simply.
    // For now, let's just make it a simple text button to be safe if GlassButton is complex
    QPushButton* realBackBtn = new QPushButton("← Back");
    realBackBtn->setStyleSheet("QPushButton { font-size: 16px; color: white; background: transparent; border: none; font-weight: bold; }"
                               "QPushButton:hover { color: #bb86fc; }");
    realBackBtn->setCursor(Qt::PointingHandCursor);
    connect(realBackBtn, &QPushButton::clicked, this, &GameDetailsPage::backClicked);
    
    topBarLayout->addWidget(realBackBtn);
    topBarLayout->addStretch(1);
    mainLayout->addWidget(topBar);

    // Scroll Area
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; } QScrollBar { width: 0px; }");
    
    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background: transparent;");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 40);
    m_contentLayout->setSpacing(30);

    // Hero Banner
    m_heroBanner = new QLabel();
    m_heroBanner->setFixedHeight(250);
    m_heroBanner->setStyleSheet("background: #1e1e1e; border-radius: 12px; margin: 20px 20px 0 20px;");
    m_heroBanner->setScaledContents(true);
    m_heroBanner->setAlignment(Qt::AlignCenter);
    m_contentLayout->addWidget(m_heroBanner);

    // Info Row (Description + Install Context)
    QWidget* infoRow = new QWidget();
    infoRow->setStyleSheet("margin: 0 20px;");
    QHBoxLayout* infoLayout = new QHBoxLayout(infoRow);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(40);

    m_descriptionLabel = new QLabel("Loading description...");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet("font-size: 14px; color: #dddddd; line-height: 1.5;");
    m_descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    infoLayout->addWidget(m_descriptionLabel, 3); // Takes 3/4 space

    // Install Button Widget
    QWidget* actionWidget = new QWidget();
    QVBoxLayout* actionLayout = new QVBoxLayout(actionWidget);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setAlignment(Qt::AlignTop | Qt::AlignRight);
    
    m_installButton = new GlassButton(MaterialIcons::Download, "Add to Library", "Download patch", Colors::PRIMARY);
    // GlassButton has a fixed width in its class usually, but we'll add it
    connect(m_installButton, &QPushButton::clicked, this, [this]() {
        emit addToLibraryClicked(m_appId, m_gameName, m_hasFix);
    });
    actionLayout->addWidget(m_installButton);
    infoLayout->addWidget(actionWidget, 1); // Takes 1/4 space

    m_contentLayout->addWidget(infoRow);

    // Screenshots Section
    QLabel* ssTitle = new QLabel("Game Previews");
    ssTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: white; margin: 10px 20px 0 20px;");
    m_contentLayout->addWidget(ssTitle);

    QScrollArea* ssScroll = new QScrollArea();
    ssScroll->setWidgetResizable(true);
    ssScroll->setFixedHeight(200);
    ssScroll->setStyleSheet("QScrollArea { border: none; background: transparent; } QScrollBar { height: 0px; }");
    
    QWidget* ssContainer = new QWidget();
    ssContainer->setStyleSheet("background: transparent; margin: 0 20px;");
    m_screenshotLayout = new QHBoxLayout(ssContainer);
    m_screenshotLayout->setContentsMargins(0, 0, 0, 0);
    m_screenshotLayout->setSpacing(15);
    m_screenshotLayout->setAlignment(Qt::AlignLeft);
    
    ssScroll->setWidget(ssContainer);
    m_contentLayout->addWidget(ssScroll);

    // Features and Security Row
    QWidget* detailsRow = new QWidget();
    detailsRow->setStyleSheet("margin: 20px 20px;");
    QHBoxLayout* detailsLayout = new QHBoxLayout(detailsRow);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    detailsLayout->setSpacing(40);
    detailsLayout->setAlignment(Qt::AlignTop);

    // Features Section
    QWidget* featuresWidget = new QWidget();
    m_featuresLayout = new QVBoxLayout(featuresWidget);
    m_featuresLayout->setAlignment(Qt::AlignTop);
    m_featuresLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* fTitle = new QLabel("Game Features");
    fTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");
    m_featuresLayout->addWidget(fTitle);
    
    detailsLayout->addWidget(featuresWidget, 1);

    // Security Section
    QWidget* securityWidget = new QWidget();
    m_securityLayout = new QVBoxLayout(securityWidget);
    m_securityLayout->setAlignment(Qt::AlignTop);
    m_securityLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* secTitle = new QLabel("# Security / DRM");
    secTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFB74D;");
    m_securityLayout->addWidget(secTitle);
    
    detailsLayout->addWidget(securityWidget, 1);

    m_contentLayout->addWidget(detailsRow);
    m_contentLayout->addStretch(1);

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
}

void GameDetailsPage::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit backClicked();
    }
    QWidget::keyPressEvent(event);
}

void GameDetailsPage::clear() {
    m_heroBanner->clear();
    m_heroBanner->setText("");
    m_descriptionLabel->setText("");
    
    // Clear screenshots
    QLayoutItem* child;
    while ((child = m_screenshotLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
    
    // Clear features (keep title)
    while (m_featuresLayout->count() > 1) {
        child = m_featuresLayout->takeAt(1);
        if (child->widget()) delete child->widget();
        delete child;
    }
    
    // Clear security (keep title)
    while (m_securityLayout->count() > 1) {
        child = m_securityLayout->takeAt(1);
        if (child->widget()) delete child->widget();
        delete child;
    }
}

void GameDetailsPage::showSkeleton() {
    clear();
    m_descriptionLabel->setText("Loading game details...");
    m_heroBanner->setStyleSheet("background: #2a2a2a; border-radius: 12px; margin: 20px 20px 0 20px;");
    
    // Add empty screenshot skeletons
    for (int i = 0; i < 3; ++i) {
        QLabel* emptySs = new QLabel();
        emptySs->setFixedSize(300, 169);
        emptySs->setStyleSheet("background: #2a2a2a; border-radius: 8px;");
        m_screenshotLayout->addWidget(emptySs);
    }
}

void GameDetailsPage::loadGame(const QString& appId, const QString& name, bool supported, bool hasFix) {
    m_appId = appId;
    m_gameName = name;
    m_supported = supported;
    m_hasFix = hasFix;

    // Reset UI to skeleton state
    showSkeleton();
    
    // Update Action Button
    m_installButton->setEnabled(supported);
    if (hasFix) {
        m_installButton->setDescription(QString("Download patch for %1").arg(name));
        m_installButton->setAccentColor(Colors::ACCENT_GREEN);
    } else {
        m_installButton->setDescription(QString("Generate patch for %1").arg(name));
        m_installButton->setAccentColor(Colors::PRIMARY);
    }

    // Set fallback hero image right away
    QString heroUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/library_hero.jpg").arg(appId);
    QNetworkRequest hReq{QUrl(heroUrl)};
    hReq.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    QNetworkReply* heroReply = m_networkManager->get(hReq);
    connect(heroReply, &QNetworkReply::finished, this, [this, heroReply]() {
        heroReply->deleteLater();
        if (heroReply->error() == QNetworkReply::NoError) {
            QPixmap rawPix;
            if (rawPix.loadFromData(heroReply->readAll())) {
                QPixmap rounded(rawPix.size());
                rounded.fill(Qt::transparent);
                QPainter p(&rounded);
                p.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(rounded.rect(), 12, 12);
                p.setClipPath(path);
                p.drawPixmap(0, 0, rawPix);
                m_heroBanner->setStyleSheet("margin: 20px 20px 0 20px;");
                m_heroBanner->setPixmap(rounded);
            }
        }
    });

    // Fetch details from Steam
    QUrl url(QString("https://store.steampowered.com/api/appdetails?appids=%1").arg(appId));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    QNetworkReply* reply = m_networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){ onDetailsReceived(reply); });
    
    // Give focus so Escape key works
    setFocus();
    
    // Reset scroll to top
    m_scrollArea->verticalScrollBar()->setValue(0);
}

void GameDetailsPage::onDetailsReceived(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        m_descriptionLabel->setText("Failed to load details from Steam.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject root = doc.object();
    
    if (root.contains(m_appId)) {
        QJsonObject appInfo = root[m_appId].toObject();
        if (appInfo["success"].toBool() && appInfo.contains("data")) {
            populate(appInfo["data"].toObject());
            return;
        }
    }
    m_descriptionLabel->setText("Game details not found on Steam.");
}

void GameDetailsPage::populate(const QJsonObject& data) {
    // Description
    QString desc = data["short_description"].toString();
    // Steam sometimes returns HTML in short_description (e.g., &quot;)
    // We can just set it as rich text, Qt handles basic HTML
    m_descriptionLabel->setText(desc);
    m_descriptionLabel->setTextFormat(Qt::RichText);

    // Screenshots
    QJsonArray screenshots = data["screenshots"].toArray();
    
    // Clear skeletons first
    QLayoutItem* child;
    while ((child = m_screenshotLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    int count = qMin(screenshots.size(), 6); // Max 6 screenshots
    for (int i = 0; i < count; ++i) {
        QJsonObject ss = screenshots[i].toObject();
        QString url = ss["path_thumbnail"].toString();
        
        QLabel* imgLbl = new QLabel();
        imgLbl->setFixedSize(300, 169);
        imgLbl->setStyleSheet("background: #2a2a2a; border-radius: 8px;");
        imgLbl->setScaledContents(true);
        m_screenshotLayout->addWidget(imgLbl);
        
        // Fetch Image
        QNetworkRequest req{QUrl(url)};
        req.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
        QNetworkReply* imgReply = m_networkManager->get(req);
        connect(imgReply, &QNetworkReply::finished, this, [imgReply, imgLbl]() {
            imgReply->deleteLater();
            if (imgReply->error() == QNetworkReply::NoError && imgLbl) {
                QPixmap rawPix;
                if (rawPix.loadFromData(imgReply->readAll())) {
                    QPixmap rounded(300, 169);
                    rounded.fill(Qt::transparent);
                    QPainter p(&rounded);
                    p.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(rounded.rect(), 8, 8);
                    p.setClipPath(path);
                    p.drawPixmap(rounded.rect(), rawPix.scaled(300, 169, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    if (imgLbl) imgLbl->setPixmap(rounded);
                }
            }
        });
    }

    // Features (Genres + Categories)
    QJsonArray genres = data["genres"].toArray();
    for (const QJsonValue& val : genres) {
        QString genre = val.toObject()["description"].toString();
        QLabel* l = new QLabel("• " + genre);
        l->setStyleSheet("color: #aaaaaa; font-size: 14px;");
        m_featuresLayout->addWidget(l);
    }
    
    QJsonArray cats = data["categories"].toArray();
    for (const QJsonValue& val : cats) {
        QString cat = val.toObject()["description"].toString();
        QLabel* l = new QLabel("• " + cat);
        l->setStyleSheet("color: #aaaaaa; font-size: 14px;");
        m_featuresLayout->addWidget(l);
    }

    // Security / DRM (Denuvo, etc.)
    bool hasDrm = false;
    QString drmNotice = data["drm_notice"].toString();
    QString legalNotice = data["legal_notice"].toString();
    
    if (!drmNotice.isEmpty()) {
        QLabel* l = new QLabel(drmNotice);
        l->setWordWrap(true);
        l->setStyleSheet("color: #FF5252; font-size: 14px;");
        m_securityLayout->addWidget(l);
        hasDrm = true;
    }
    if (legalNotice.contains("Denuvo", Qt::CaseInsensitive) || legalNotice.contains("Anti-cheat", Qt::CaseInsensitive)) {
        QLabel* l = new QLabel("Warning: Contains Third-party DRM/Anti-Cheat");
        l->setWordWrap(true);
        l->setStyleSheet("color: #FF5252; font-size: 14px; font-weight: bold;");
        m_securityLayout->addWidget(l);
        hasDrm = true;
    }
    
    if (!hasDrm) {
        QLabel* l = new QLabel("✓ No third-party DRM known");
        l->setStyleSheet("color: #4CAF50; font-size: 14px;");
        m_securityLayout->addWidget(l);
    }
}
