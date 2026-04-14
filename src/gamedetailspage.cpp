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

namespace {
class HeroBannerWidget : public QLabel {
public:
    HeroBannerWidget(QWidget* parent = nullptr) : QLabel(parent) {
        setFixedHeight(350);
        setMinimumWidth(1);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    }
    void setHeroPixmap(const QPixmap& pix) {
        m_pix = pix;
        update();
    }
    void setLogoPixmap(const QPixmap& logo) {
        m_logo = logo;
        update();
    }
    void clearHero() {
        m_pix = QPixmap();
        m_logo = QPixmap();
        update();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addRoundedRect(rect(), 12, 12);
        p.setClipPath(path);
        
        if (m_pix.isNull()) {
            p.fillRect(rect(), QColor("#1e1e1e"));
            return;
        }
        
        // Scale to fill the exact current bounds dynamically, then center crop
        QPixmap scaled = m_pix.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int dx = (scaled.width() - width()) / 2;
        int dy = (scaled.height() - height()) / 2;
        p.drawPixmap(0, 0, scaled, dx, dy, width(), height());
        
        // Draw logo overlay (bottom-left, like Steam)
        if (!m_logo.isNull()) {
            // Subtle vignette gradient behind the logo area
            QLinearGradient vignette(QPointF(0, height() * 0.4), QPointF(0, height()));
            vignette.setColorAt(0, QColor(0, 0, 0, 0));
            vignette.setColorAt(0.6, QColor(0, 0, 0, 80));
            vignette.setColorAt(1, QColor(0, 0, 0, 160));
            p.fillRect(QRect(0, (int)(height() * 0.4), width(), height()), vignette);
            
            // Scale logo bigger (max 55% of banner width, max 65% of banner height)
            int maxLogoW = (int)(width() * 0.55);
            int maxLogoH = (int)(height() * 0.65);
            QPixmap scaledLogo = m_logo.scaled(maxLogoW, maxLogoH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            
            // Position: left, vertically centered
            int logoX = 30;
            int logoY = (height() - scaledLogo.height()) / 2;
            p.drawPixmap(logoX, logoY, scaledLogo);
        }
    }
private:
    QPixmap m_pix;
    QPixmap m_logo;
};
} // namespace

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
    topBar->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 0, 20, 0);

    QPushButton* realBackBtn = new QPushButton("← Back");
    realBackBtn->setStyleSheet("QPushButton { font-size: 16px; color: white; background: transparent; border: none; font-weight: bold; padding: 10px; }"
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
    m_contentLayout->setContentsMargins(20, 20, 20, 40); // Unified 20px padding
    m_contentLayout->setSpacing(30);

    // Hero Banner
    m_heroBanner = new HeroBannerWidget();
    m_contentLayout->addWidget(m_heroBanner);

    // Info Row (Description + Install Context)
    QWidget* infoRow = new QWidget();
    QHBoxLayout* infoLayout = new QHBoxLayout(infoRow);
    infoLayout->setContentsMargins(0, 0, 0, 0); // Rely on global 20px
    infoLayout->setSpacing(40);

    // Description Layout
    QVBoxLayout* descLayout = new QVBoxLayout();
    descLayout->setContentsMargins(0, 0, 0, 0);
    descLayout->setSpacing(8);
    descLayout->setAlignment(Qt::AlignTop);

    m_gameTitleLabel = new QLabel("");
    m_gameTitleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: white;");
    m_gameTitleLabel->setWordWrap(true);
    descLayout->addWidget(m_gameTitleLabel);

    m_descriptionLabel = new QLabel("Loading description...");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setMaximumWidth(800); // 1. Limit description width
    m_descriptionLabel->setStyleSheet("font-size: 14px; color: #dddddd; line-height: 1.5;");
    m_descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    descLayout->addWidget(m_descriptionLabel);
    
    infoLayout->addLayout(descLayout, 3); // Takes 3/4 space

    // Install Button Widget
    QWidget* actionWidget = new QWidget();
    QVBoxLayout* actionLayout = new QVBoxLayout(actionWidget);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setAlignment(Qt::AlignTop | Qt::AlignRight);
    
    m_installButton = new QPushButton("Install");
    m_installButton->setCursor(Qt::PointingHandCursor);
    m_installButton->setFixedSize(220, 45);
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: #2E7D32; color: white; font-weight: 800; font-size: 16px; border-radius: 4px; font-family: 'Segoe UI';"
        "}"
        "QPushButton:hover {"
        "  background: #388E3C;"
        "}"
        "QPushButton:disabled {"
        "  background: #424242; color: #757575;"
        "}"
    );
    
    connect(m_installButton, &QPushButton::clicked, this, [this]() {
        if (m_isDownloading) return;
        m_isDownloading = true;
        // Reset state before new attempt
        m_installButton->setText("Downloading...");
        m_installProgressBar->setValue(0);
        m_installProgressBar->show();
        
        emit addToLibraryClicked(m_appId, m_gameName, m_hasFix);
    });
    actionLayout->addWidget(m_installButton);
    
    m_installProgressBar = new QProgressBar();
    m_installProgressBar->setFixedSize(220, 4);
    m_installProgressBar->setTextVisible(false);
    m_installProgressBar->setStyleSheet(
        "QProgressBar { background: #1B5E20; border: none; border-radius: 2px; }"
        "QProgressBar::chunk { background: #00E676; border-radius: 2px; }"
    );
    m_installProgressBar->hide();
    actionLayout->addWidget(m_installProgressBar);
    
    infoLayout->addWidget(actionWidget, 1); // Takes 1/4 space

    m_contentLayout->addWidget(infoRow);

    // Screenshots Section
    QLabel* ssTitle = new QLabel("Game Previews");
    ssTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: white; margin: 10px 0px 0 0px;");
    m_contentLayout->addWidget(ssTitle);

    QScrollArea* ssScroll = new QScrollArea();
    ssScroll->setWidgetResizable(true);
    ssScroll->setFixedHeight(270);
    ssScroll->setStyleSheet("QScrollArea { border: none; background: transparent; } QScrollBar { height: 0px; }");
    
    QWidget* ssContainer = new QWidget();
    ssContainer->setStyleSheet("background: transparent;");
    m_screenshotLayout = new QHBoxLayout(ssContainer);
    m_screenshotLayout->setContentsMargins(0, 0, 0, 0); // Rely on global 20px
    m_screenshotLayout->setSpacing(8); // Gap reduced here
    m_screenshotLayout->setAlignment(Qt::AlignLeft);
    
    ssScroll->setWidget(ssContainer);
    
    // Create wrapper layout with navigation buttons
    QHBoxLayout* ssWrapperLayout = new QHBoxLayout();
    ssWrapperLayout->setContentsMargins(0, 0, 0, 0);
    ssWrapperLayout->setSpacing(10);
    
    QPushButton* btnPrev = new QPushButton("<");
    btnPrev->setFixedSize(40, 40);
    btnPrev->setCursor(Qt::PointingHandCursor);
    btnPrev->setStyleSheet("QPushButton { background: rgba(30,30,40,200); color: white; font-weight: bold; font-size: 20px; border-radius: 20px; font-family: 'Segoe UI', sans-serif; }"
                           "QPushButton:hover { background: rgba(255,255,255,50); }");
                           
    QPushButton* btnNext = new QPushButton(">");
    btnNext->setFixedSize(40, 40);
    btnNext->setCursor(Qt::PointingHandCursor);
    btnNext->setStyleSheet("QPushButton { background: rgba(30,30,40,200); color: white; font-weight: bold; font-size: 20px; border-radius: 20px; font-family: 'Segoe UI', sans-serif; }"
                           "QPushButton:hover { background: rgba(255,255,255,50); }");

    connect(btnPrev, &QPushButton::clicked, this, [ssScroll]() {
        QScrollBar* hb = ssScroll->horizontalScrollBar();
        hb->setValue(qMax(0, hb->value() - 428)); // Shift left by 1 item + spacing
    });
    
    connect(btnNext, &QPushButton::clicked, this, [ssScroll]() {
        QScrollBar* hb = ssScroll->horizontalScrollBar();
        hb->setValue(qMin(hb->maximum(), hb->value() + 428)); // Shift right by 1 item
    });

    ssWrapperLayout->addWidget(btnPrev);
    ssWrapperLayout->addWidget(ssScroll);
    ssWrapperLayout->addWidget(btnNext);
    
    m_contentLayout->addLayout(ssWrapperLayout);

    // Features and Security Row
    QWidget* detailsRow = new QWidget();
    QHBoxLayout* detailsLayout = new QHBoxLayout(detailsRow);
    detailsLayout->setContentsMargins(0, 0, 0, 0); // Rely on global 20px
    detailsLayout->setSpacing(40);
    detailsLayout->setAlignment(Qt::AlignTop);

    // Features Section
    QWidget* featuresWidget = new QWidget();
    m_featuresLayout = new QVBoxLayout(featuresWidget);
    m_featuresLayout->setAlignment(Qt::AlignTop);
    m_featuresLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* fTitle = new QLabel("Game Features");
    fTitle->setStyleSheet("font-size: 18px; font-weight: 800; color: white; font-family: 'Segoe UI'; border-bottom: 1px solid #333; padding-bottom: 4px;");
    m_featuresLayout->addWidget(fTitle);
    
    detailsLayout->addWidget(featuresWidget, 1);

    // Security Section
    QWidget* securityWidget = new QWidget();
    m_securityLayout = new QVBoxLayout(securityWidget);
    m_securityLayout->setAlignment(Qt::AlignTop);
    m_securityLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* secTitle = new QLabel("Security");
    secTitle->setStyleSheet("font-size: 18px; font-weight: 800; color: #FFB74D; font-family: 'Segoe UI'; border-bottom: 1px solid #333; padding-bottom: 4px;");
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
    static_cast<HeroBannerWidget*>(m_heroBanner)->clearHero();
    m_gameTitleLabel->setText("");
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
    
    // Invalidate any pending network requests modifying layout
    m_currentLoadId++;
}

void GameDetailsPage::showSkeleton() {
    clear();
    m_descriptionLabel->setText("Loading game details...");
    
    // Add empty screenshot skeletons
    for (int i = 0; i < 3; ++i) {
        QLabel* emptySs = new QLabel();
        emptySs->setFixedSize(420, 236);
        emptySs->setStyleSheet("background: #2a2a2a; border-radius: 8px;");
        m_screenshotLayout->addWidget(emptySs);
    }
}

void GameDetailsPage::loadGame(const QString& appId, const QString& name, bool supported, bool hasFix) {
    m_appId = appId;
    m_gameName = name;
    m_supported = supported;
    m_hasFix = hasFix;
    
    // Invalidate pending async callbacks
    m_currentLoadId++;
    int loadId = m_currentLoadId;

    // Reset UI to skeleton state
    m_isDownloading = false;
    showSkeleton();
    
    // Set game title immediately
    m_gameTitleLabel->setText(m_gameName);
    
    // Update Action Button
    m_installButton->setEnabled(supported);
    m_installButton->setText("Install");
    m_installProgressBar->hide();
    m_installProgressBar->setValue(0);
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: #2E7D32; color: white; font-weight: 800; font-size: 18px; border-radius: 8px; font-family: 'Segoe UI';"
        "}"
        "QPushButton:hover {"
        "  background: #388E3C;"
        "}"
        "QPushButton:disabled {"
        "  background: #424242; color: #757575;"
        "}"
    );

    // Fetch hero background image
    QString heroUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/library_hero.jpg").arg(appId);
    QNetworkRequest hReq{QUrl(heroUrl)};
    hReq.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    QNetworkReply* heroReply = m_networkManager->get(hReq);
    connect(heroReply, &QNetworkReply::finished, this, [this, heroReply, loadId]() {
        heroReply->deleteLater();
        if (m_currentLoadId != loadId) return; // Abort if aborted by back button
        if (heroReply->error() == QNetworkReply::NoError) {
            QPixmap rawPix;
            if (rawPix.loadFromData(heroReply->readAll())) {
                static_cast<HeroBannerWidget*>(m_heroBanner)->setHeroPixmap(rawPix);
            }
        }
    });
    
    // Fetch game logo overlay
    QString logoUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/logo.png").arg(appId);
    QNetworkRequest logoReq{QUrl(logoUrl)};
    logoReq.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    QNetworkReply* logoReply = m_networkManager->get(logoReq);
    connect(logoReply, &QNetworkReply::finished, this, [this, logoReply, loadId]() {
        logoReply->deleteLater();
        if (m_currentLoadId != loadId) return;
        if (logoReply->error() == QNetworkReply::NoError) {
            QPixmap logoPix;
            if (logoPix.loadFromData(logoReply->readAll())) {
                static_cast<HeroBannerWidget*>(m_heroBanner)->setLogoPixmap(logoPix);
            }
        }
    });

    // Fetch details from Steam
    QUrl url(QString("https://store.steampowered.com/api/appdetails?appids=%1").arg(appId));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    QNetworkReply* reply = m_networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, loadId](){ 
        if (m_currentLoadId == loadId) onDetailsReceived(reply); 
        else reply->deleteLater();
    });
    
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
        imgLbl->setFixedSize(420, 236);
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
                    QPixmap rounded(420, 236);
                    rounded.fill(Qt::transparent);
                    QPainter p(&rounded);
                    p.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(rounded.rect(), 8, 8);
                    p.setClipPath(path);
                    p.drawPixmap(rounded.rect(), rawPix.scaled(420, 236, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
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

void GameDetailsPage::updateInstallProgress(int pct) {
    if (!m_isDownloading) return;
    m_installProgressBar->show();
    m_installProgressBar->setValue(pct);
}

void GameDetailsPage::installFinished() {
    m_isDownloading = false;
    m_installProgressBar->hide();
    m_installButton->setText("Installed");
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: #2E7D32; color: white; font-weight: 800; font-size: 18px; border-radius: 8px; font-family: 'Segoe UI';"
        "}"
    );
}

void GameDetailsPage::installError(const QString& err) {
    m_isDownloading = false;
    m_installProgressBar->hide();
    m_installButton->setText("Error (Retry)");
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: #D32F2F; color: white; font-weight: 800; font-size: 18px; border-radius: 8px; font-family: 'Segoe UI';"
        "}"
        "QPushButton:hover {"
        "  background: #F44336;"
        "}"
    );
}
