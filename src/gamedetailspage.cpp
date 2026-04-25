#include "gamedetailspage.h"
#include "utils/colors.h"

#include <QTimer>

#include <QPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QRegularExpression>
#include <QFrame>

// ============================================================
// CinematicHeroBanner — full-bleed hero with gradient scrim
// ============================================================
namespace {

class CinematicHeroBanner : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal imageScale READ imageScale WRITE setImageScale)
public:
    explicit CinematicHeroBanner(QWidget* parent = nullptr)
        : QWidget(parent), m_imageScale(1.05)
    {
        setFixedHeight(450);
        setMinimumWidth(1);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMouseTracking(true);

        m_scaleAnim = new QPropertyAnimation(this, "imageScale", this);
        m_scaleAnim->setDuration(1000);
        m_scaleAnim->setEasingCurve(QEasingCurve::OutSine);
    }

    void setHeroPixmap(const QPixmap& pix) { m_pixmap = pix; update(); }
    void clearHero() { m_pixmap = QPixmap(); update(); }

    qreal imageScale() const { return m_imageScale; }
    void setImageScale(qreal s) { m_imageScale = s; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        // Dark fallback
        if (m_pixmap.isNull()) {
            QLinearGradient fallback(0, 0, 0, height());
            fallback.setColorAt(0, QColor(20, 20, 30));
            fallback.setColorAt(1, QColor(5, 5, 10));
            p.fillRect(rect(), fallback);
        } else {
            // Scale + center crop with cinematic zoom
            p.save();
            p.setClipRect(rect());
            QPointF center = QPointF(width() / 2.0, height() / 2.0);
            p.translate(center);
            p.scale(m_imageScale, m_imageScale);
            p.translate(-center);

            QSize targetSize = size();
            QPixmap scaled = m_pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            int sx = qMax(0, (scaled.width() - targetSize.width()) / 2);
            int sy = qMax(0, (scaled.height() - targetSize.height()) / 2);
            int sw = qMin(targetSize.width(), scaled.width() - sx);
            int sh = qMin(targetSize.height(), scaled.height() - sy);
            p.drawPixmap(rect(), scaled, QRect(sx, sy, sw, sh));
            p.restore();
        }

        // Gradient scrim overlay — dark at bottom, transparent at top
        QLinearGradient scrim(0, 0, 0, height());
        scrim.setColorAt(0.0, QColor(0, 0, 0, 0));
        scrim.setColorAt(0.3, QColor(0, 0, 0, 20));
        scrim.setColorAt(0.6, QColor(0, 0, 0, 120));
        scrim.setColorAt(0.85, QColor(0, 0, 0, 200));
        scrim.setColorAt(1.0, QColor(0, 0, 0, 255)); // Fully opaque at bottom to transition seamlessly
        
        // Disable antialiasing for the fill to prevent subpixel bounding artifacts
        p.setRenderHint(QPainter::Antialiasing, false);
        p.fillRect(QRect(-1, -1, width() + 2, height() + 2), scrim);
        p.setRenderHint(QPainter::Antialiasing, true);

        // Subtle top vignette for the back button area
        QLinearGradient topScrim(0, 0, 0, 80);
        topScrim.setColorAt(0.0, QColor(0, 0, 0, 100));
        topScrim.setColorAt(1.0, QColor(0, 0, 0, 0));
        p.fillRect(QRect(0, 0, width(), 80), topScrim);
    }

    void enterEvent(QEnterEvent* event) override {
        Q_UNUSED(event);
        m_scaleAnim->stop();
        m_scaleAnim->setStartValue(m_imageScale);
        m_scaleAnim->setEndValue(1.0);
        m_scaleAnim->start();
    }

    void leaveEvent(QEvent* event) override {
        Q_UNUSED(event);
        m_scaleAnim->stop();
        m_scaleAnim->setStartValue(m_imageScale);
        m_scaleAnim->setEndValue(1.05);
        m_scaleAnim->start();
    }

private:
    QPixmap m_pixmap;
    qreal m_imageScale;
    QPropertyAnimation* m_scaleAnim;
};

// ============================================================
// ScreenshotCard — rounded image with hover zoom
// ============================================================
class ScreenshotCard : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal imageScale READ imageScale WRITE setImageScale)
public:
    explicit ScreenshotCard(QWidget* parent = nullptr)
        : QWidget(parent), m_imageScale(1.0)
    {
        setFixedSize(480, 270);
        setCursor(Qt::PointingHandCursor);

        m_scaleAnim = new QPropertyAnimation(this, "imageScale", this);
        m_scaleAnim->setDuration(300);
        m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    }

    void setScreenshot(const QPixmap& pix) { m_pixmap = pix; update(); }

    qreal imageScale() const { return m_imageScale; }
    void setImageScale(qreal s) { m_imageScale = s; update(); }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);

        QRectF r = QRectF(rect()).adjusted(2, 2, -2, -2);
        int radius = 8;

        QPainterPath clip;
        clip.addRoundedRect(r, radius, radius);
        p.setClipPath(clip);

        if (m_pixmap.isNull()) {
            p.fillRect(r.toRect(), QColor(30, 30, 40));
        } else {
            p.save();
            QPointF center = r.center();
            p.translate(center);
            p.scale(m_imageScale, m_imageScale);
            p.translate(-center);

            QSize cardSize = r.size().toSize();
            QPixmap scaled = m_pixmap.scaled(cardSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            int sx = qMax(0, (scaled.width() - cardSize.width()) / 2);
            int sy = qMax(0, (scaled.height() - cardSize.height()) / 2);
            p.drawPixmap(r.toRect(), scaled, QRect(sx, sy, cardSize.width(), cardSize.height()));
            p.restore();
        }

        // Subtle border
        p.setClipRect(rect());
        p.setPen(QPen(QColor(143, 171, 212, 30), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r, radius, radius);
    }

    void enterEvent(QEnterEvent*) override {
        m_scaleAnim->stop();
        m_scaleAnim->setStartValue(m_imageScale);
        m_scaleAnim->setEndValue(1.03);
        m_scaleAnim->start();
    }

    void leaveEvent(QEvent*) override {
        m_scaleAnim->stop();
        m_scaleAnim->setStartValue(m_imageScale);
        m_scaleAnim->setEndValue(1.0);
        m_scaleAnim->start();
    }

private:
    QPixmap m_pixmap;
    qreal m_imageScale;
    QPropertyAnimation* m_scaleAnim;
};

} // namespace

// Need MOC for anonymous namespace Q_OBJECT classes
#include "gamedetailspage.moc"

// ============================================================
// GameDetailsPage Implementation
// ============================================================

GameDetailsPage::GameDetailsPage(QNetworkAccessManager* networkManager, QWidget* parent)
    : QWidget(parent), m_networkManager(networkManager)
{
    setFocusPolicy(Qt::StrongFocus);
    buildUI();
}

void GameDetailsPage::buildUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Scroll Area ──
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 6px; }"
        "QScrollBar::handle:vertical { background: rgba(143,171,212,0.3); border-radius: 3px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: rgba(143,171,212,0.5); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
    );

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background: transparent;");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 40);
    m_contentLayout->setSpacing(0);

    // ══════════════════════════════════════════
    // 1. CINEMATIC HERO BANNER
    // ══════════════════════════════════════════
    m_heroBanner = new CinematicHeroBanner();
    m_contentLayout->addWidget(m_heroBanner);

    // Back button — floating top-left overlay on banner
    m_backButton = new QPushButton("← Back", m_heroBanner);
    m_backButton->setGeometry(20, 16, 100, 36);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setStyleSheet(
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
    connect(m_backButton, &QPushButton::clicked, this, &GameDetailsPage::backClicked);

    // Overlay container — anchored to bottom of banner
    m_overlayContainer = new QWidget(m_heroBanner);
    m_overlayContainer->setStyleSheet("background: transparent;");
    // Will be positioned in resizeEvent-like fashion via layout trick:
    // We use a layout on the hero banner with a spacer pushing content to bottom
    QVBoxLayout* bannerLayout = new QVBoxLayout(m_heroBanner);
    bannerLayout->setContentsMargins(40, 60, 40, 30);
    bannerLayout->setSpacing(0);
    bannerLayout->addStretch(1); // Push everything to bottom
    bannerLayout->addWidget(m_overlayContainer);

    // Inside the overlay container
    QVBoxLayout* overlayLayout = new QVBoxLayout(m_overlayContainer);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    overlayLayout->setSpacing(10);

    // Game title (only title + buttons in banner, no tags or description)
    m_heroTitleLabel = new QLabel("");
    m_heroTitleLabel->setStyleSheet(
        "font-size: 42px; font-weight: 800; color: #FFFFFF;"
        "font-family: 'Segoe UI'; letter-spacing: -1px;"
        "background: transparent;"
    );
    m_heroTitleLabel->setWordWrap(true);
    overlayLayout->addWidget(m_heroTitleLabel);

    overlayLayout->addSpacing(14);

    // Hidden containers kept for API compatibility, given a parent to prevent memory leak
    m_tagContainer = new QWidget(m_overlayContainer);
    m_tagContainer->hide();
    m_tagLayout = new QHBoxLayout(m_tagContainer);
    m_tagLayout->setContentsMargins(0, 0, 0, 0);
    m_tagLayout->setSpacing(8);
    m_heroSubtitleLabel = new QLabel("", m_overlayContainer);
    m_heroSubtitleLabel->hide();

    // Action buttons row
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->setSpacing(12);
    btnRow->setAlignment(Qt::AlignLeft);

    m_installButton = new QPushButton("INSTALL");
    m_installButton->setCursor(Qt::PointingHandCursor);
    m_installButton->setFixedSize(180, 50);
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: #00E676; color: #000000; font-weight: 800; font-size: 16px;"
        "  border-radius: 14px; font-family: 'Segoe UI'; border: none;"
        "  letter-spacing: 1px;"
        "}"
        "QPushButton:hover {"
        "  background: #00C853;"
        "}"
        "QPushButton:disabled {"
        "  background: rgba(255,255,255,0.1); color: rgba(255,255,255,0.3);"
        "}"
    );
    connect(m_installButton, &QPushButton::clicked, this, [this]() {
        if (m_isDownloading) return;
        m_isDownloading = true;
        m_installButton->setText("DOWNLOADING...");
        m_installProgressBar->setValue(0);
        m_installProgressBar->show();
        emit addToLibraryClicked(m_appId, m_gameName, m_hasFix);
    });
    btnRow->addWidget(m_installButton);

    m_wishlistButton = new QPushButton("♡  WISHLIST");
    m_wishlistButton->setCursor(Qt::PointingHandCursor);
    m_wishlistButton->setFixedSize(160, 50);
    m_wishlistButton->setStyleSheet(
        "QPushButton {"
        "  background: transparent; color: #EFECE3; font-weight: 600; font-size: 14px;"
        "  border: 1px solid rgba(255,255,255,0.25); border-radius: 14px;"
        "  font-family: 'Segoe UI';"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255,255,255,0.08); border-color: rgba(255,255,255,0.4);"
        "}"
    );
    btnRow->addWidget(m_wishlistButton);

    btnRow->addStretch(1);
    overlayLayout->addLayout(btnRow);

    // Progress bar (shown during download)
    m_installProgressBar = new QProgressBar();
    m_installProgressBar->setFixedHeight(4);
    m_installProgressBar->setMaximumWidth(340);
    m_installProgressBar->setTextVisible(false);
    m_installProgressBar->setStyleSheet(
        "QProgressBar { background: rgba(255,255,255,0.08); border: none; border-radius: 2px; }"
        "QProgressBar::chunk { background: #00E676; border-radius: 2px; }"
    );
    m_installProgressBar->hide();
    overlayLayout->addWidget(m_installProgressBar);

    // ══════════════════════════════════════════
    // 2. BELOW-BANNER: Description + Specs
    // ══════════════════════════════════════════
    m_infoRow = new QWidget();
    m_infoRow->setStyleSheet("background: transparent;");
    QHBoxLayout* infoLayout = new QHBoxLayout(m_infoRow);
    infoLayout->setContentsMargins(40, 30, 40, 0);
    infoLayout->setSpacing(30);
    infoLayout->setAlignment(Qt::AlignTop);

    // Left column — description
    QVBoxLayout* descCol = new QVBoxLayout();
    descCol->setContentsMargins(0, 0, 0, 0);
    descCol->setSpacing(12);
    descCol->setAlignment(Qt::AlignTop);

    QLabel* aboutTitle = new QLabel("ABOUT THIS GAME");
    aboutTitle->setStyleSheet(
        "font-size: 13px; font-weight: 700; color: #8FABD4;"
        "font-family: 'Segoe UI'; letter-spacing: 2px;"
        "background: transparent;"
    );
    descCol->addWidget(aboutTitle);

    m_descriptionLabel = new QLabel("Loading description...");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet(
        "font-size: 14px; color: #EFECE3; line-height: 1.6;"
        "font-family: 'Segoe UI'; background: transparent;"
    );
    m_descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    descCol->addWidget(m_descriptionLabel);
    descCol->addStretch(1);

    infoLayout->addLayout(descCol, 1);

    // Right column — Specs Neo-Card
    m_specsCard = new QWidget();
    m_specsCard->setFixedWidth(320);
    m_specsCard->setStyleSheet(
        "background: rgba(20,20,30,0.85);"
        "border: 1px solid rgba(143,171,212,0.15);"
        "border-radius: 16px;"
    );
    m_specsLayout = new QVBoxLayout(m_specsCard);
    m_specsLayout->setContentsMargins(24, 24, 24, 24);
    m_specsLayout->setSpacing(16);

    QLabel* specsTitle = new QLabel("SPECIFICATIONS");
    specsTitle->setStyleSheet(
        "font-size: 12px; font-weight: 700; color: #8FABD4;"
        "font-family: 'Segoe UI'; letter-spacing: 2px;"
        "border: none; background: transparent;"
    );
    m_specsLayout->addWidget(specsTitle);

    // Specs will be populated dynamically
    m_specsLayout->addStretch(1);

    infoLayout->addWidget(m_specsCard, 0, Qt::AlignTop);

    m_contentLayout->addWidget(m_infoRow);

    // ══════════════════════════════════════════
    // 3. SCREENSHOTS SECTION
    // ══════════════════════════════════════════
    m_screenshotSection = new QWidget();
    m_screenshotSection->setStyleSheet("background: transparent;");
    QVBoxLayout* ssMainLayout = new QVBoxLayout(m_screenshotSection);
    ssMainLayout->setContentsMargins(40, 30, 40, 0);
    ssMainLayout->setSpacing(16);

    QLabel* ssTitle = new QLabel("GAME PREVIEWS");
    ssTitle->setStyleSheet(
        "font-size: 13px; font-weight: 700; color: #8FABD4;"
        "font-family: 'Segoe UI'; letter-spacing: 2px;"
        "background: transparent;"
    );
    ssMainLayout->addWidget(ssTitle);

    // Horizontal scroll with nav arrows
    QHBoxLayout* ssNavLayout = new QHBoxLayout();
    ssNavLayout->setContentsMargins(0, 0, 0, 0);
    ssNavLayout->setSpacing(10);

    QPushButton* btnPrev = new QPushButton("‹");
    btnPrev->setFixedSize(36, 36);
    btnPrev->setCursor(Qt::PointingHandCursor);
    btnPrev->setStyleSheet(
        "QPushButton { background: rgba(20,20,30,0.8); color: #EFECE3;"
        "  font-size: 22px; font-weight: bold; border-radius: 18px;"
        "  border: 1px solid rgba(143,171,212,0.15); font-family: 'Segoe UI'; }"
        "QPushButton:hover { background: rgba(143,171,212,0.2); }"
    );

    QPushButton* btnNext = new QPushButton("›");
    btnNext->setFixedSize(36, 36);
    btnNext->setCursor(Qt::PointingHandCursor);
    btnNext->setStyleSheet(btnPrev->styleSheet());

    QScrollArea* ssScroll = new QScrollArea();
    ssScroll->setWidgetResizable(true);
    ssScroll->setFixedHeight(290);
    ssScroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:horizontal { height: 0px; }"
    );

    QWidget* ssContainer = new QWidget();
    ssContainer->setStyleSheet("background: transparent;");
    m_screenshotLayout = new QHBoxLayout(ssContainer);
    m_screenshotLayout->setContentsMargins(0, 0, 0, 0);
    m_screenshotLayout->setSpacing(12);
    m_screenshotLayout->setAlignment(Qt::AlignLeft);
    ssScroll->setWidget(ssContainer);

    connect(btnPrev, &QPushButton::clicked, this, [ssScroll]() {
        QScrollBar* hb = ssScroll->horizontalScrollBar();
        hb->setValue(qMax(0, hb->value() - 400));
    });
    connect(btnNext, &QPushButton::clicked, this, [ssScroll]() {
        QScrollBar* hb = ssScroll->horizontalScrollBar();
        hb->setValue(qMin(hb->maximum(), hb->value() + 400));
    });

    ssNavLayout->addWidget(btnPrev);
    ssNavLayout->addWidget(ssScroll, 1);
    ssNavLayout->addWidget(btnNext);
    ssMainLayout->addLayout(ssNavLayout);

    m_contentLayout->addWidget(m_screenshotSection);

    // ══════════════════════════════════════════
    // 4. FEATURES & SECURITY
    // ══════════════════════════════════════════
    m_detailsRow = new QWidget();
    m_detailsRow->setStyleSheet("background: transparent;");
    QHBoxLayout* detailsLayout = new QHBoxLayout(m_detailsRow);
    detailsLayout->setContentsMargins(40, 30, 40, 0);
    detailsLayout->setSpacing(30);
    detailsLayout->setAlignment(Qt::AlignTop);

    // Features Neo-Card
    QFrame* featuresWidget = new QFrame();
    featuresWidget->setObjectName("featureCard");
    featuresWidget->setStyleSheet(
        "#featureCard {"
        "  background: rgba(20,20,30,0.7);"
        "  border: 1px solid rgba(255,255,255,0.05);"
        "  border-radius: 16px;"
        "}"
        "#featureCard:hover {"
        "  border: 1px solid rgba(255,255,255,0.15);"
        "}"
    );
    m_featuresLayout = new QVBoxLayout(featuresWidget);
    m_featuresLayout->setContentsMargins(24, 24, 24, 24);
    m_featuresLayout->setSpacing(16);
    m_featuresLayout->setAlignment(Qt::AlignTop);

    QLabel* fTitle = new QLabel("GAME FEATURES");
    fTitle->setStyleSheet(
        "font-size: 14px; font-weight: 900; color: #EFECE3;"
        "font-family: 'Segoe UI'; letter-spacing: 2px;"
        "border: none; background: transparent;"
    );
    m_featuresLayout->addWidget(fTitle);

    m_featuresGrid = new QGridLayout();
    m_featuresGrid->setSpacing(15);
    m_featuresGrid->setAlignment(Qt::AlignTop);
    m_featuresLayout->addLayout(m_featuresGrid);
    
    detailsLayout->addWidget(featuresWidget, 1);

    // Security Neo-Card
    QFrame* securityWidget = new QFrame();
    securityWidget->setObjectName("securityCard");
    securityWidget->setStyleSheet(
        "#securityCard {"
        "  background: rgba(20,30,20,0.7);"
        "  border: 1px solid rgba(0,255,0,0.05);"
        "  border-radius: 16px;"
        "}"
        "#securityCard:hover {"
        "  border: 1px solid rgba(0,255,0,0.2);"
        "}"
    );
    m_securityLayout = new QVBoxLayout(securityWidget);
    m_securityLayout->setContentsMargins(24, 24, 24, 24);
    m_securityLayout->setSpacing(16);
    m_securityLayout->setAlignment(Qt::AlignTop);

    QLabel* secTitle = new QLabel("SECURITY");
    secTitle->setStyleSheet(
        "font-size: 14px; font-weight: 900; color: #EFECE3;"
        "font-family: 'Segoe UI'; letter-spacing: 2px;"
        "border: none; background: transparent;"
    );
    m_securityLayout->addWidget(secTitle);
    
    // Verified Status Box
    QFrame* verifiedBox = new QFrame();
    verifiedBox->setStyleSheet(
        "QFrame {"
        "  background: rgba(0,255,0,0.05);"
        "  border: 1px solid rgba(0,255,0,0.1);"
        "  border-radius: 12px;"
        "}"
    );
    QHBoxLayout* verifiedLayout = new QHBoxLayout(verifiedBox);
    verifiedLayout->setContentsMargins(16, 16, 16, 16);
    verifiedLayout->setSpacing(16);
    
    QLabel* checkIcon = new QLabel("✓");
    checkIcon->setFixedSize(36, 36);
    checkIcon->setAlignment(Qt::AlignCenter);
    checkIcon->setStyleSheet(
        "background: rgba(0,255,0,0.15);"
        "border-radius: 18px;"
        "color: #00E676;"
        "font-size: 18px;"
        "font-weight: 900;"
        "border: none;"
    );
    verifiedLayout->addWidget(checkIcon);
    
    QLabel* statusTextLbl = new QLabel("Verified for Secure Play: passed all safety and cloud sync checks.");
    statusTextLbl->setWordWrap(true);
    statusTextLbl->setStyleSheet(
        "color: #00E676; font-size: 13px; font-weight: 600;"
        "background: transparent; border: none;"
    );
    verifiedLayout->addWidget(statusTextLbl, 1);
    
    m_securityLayout->addWidget(verifiedBox);
    
    detailsLayout->addWidget(securityWidget, 1);

    m_contentLayout->addWidget(m_detailsRow);
    m_contentLayout->addStretch(1);

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
}

// ── Entrance Animations ──
void GameDetailsPage::playEntranceAnimations() {
    // Animate title in the banner
    if (m_heroTitleLabel) {
        auto* effect = new QGraphicsOpacityEffect(m_heroTitleLabel);
        effect->setOpacity(0.0);
        m_heroTitleLabel->setGraphicsEffect(effect);

        auto* fadeAnim = new QPropertyAnimation(effect, "opacity", this);
        fadeAnim->setDuration(600);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPointer<QWidget> safeTitle(m_heroTitleLabel);
        connect(fadeAnim, &QPropertyAnimation::finished, this, [safeTitle]() {
            // CRITICAL FIX: Schedule effect removal for the next event loop tick.
            // If we remove it immediately, Qt6 crashes trying to access the deleted effect target inside the animation loop.
            if (safeTitle) {
                QTimer::singleShot(0, safeTitle, [safeTitle]() {
                    if (safeTitle) safeTitle->setGraphicsEffect(nullptr);
                });
            }
        });

        QPointer<QPropertyAnimation> safeAnim(fadeAnim);
        QPointer<QGraphicsEffect> safeEffect(effect);
        QTimer::singleShot(100, this, [safeAnim, safeEffect]() {
            if (safeAnim && safeEffect) safeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            else if (safeAnim) safeAnim->deleteLater();
        });
    }

    // Animate below-banner sections with CLEANUP to prevent scroll artifacts
    QList<QWidget*> belowBanner = {m_infoRow, m_screenshotSection, m_detailsRow};
    int delay = 300;
    for (QWidget* w : belowBanner) {
        if (!w) continue;
        auto* effect = new QGraphicsOpacityEffect(w);
        effect->setOpacity(0.0);
        w->setGraphicsEffect(effect);

        auto* fadeAnim = new QPropertyAnimation(effect, "opacity", this);
        fadeAnim->setDuration(500);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

        QPointer<QWidget> safeW(w);
        connect(fadeAnim, &QPropertyAnimation::finished, this, [safeW]() {
            // CRITICAL FIX: Schedule effect removal later
            if (safeW) {
                QTimer::singleShot(0, safeW, [safeW]() {
                    if (safeW) safeW->setGraphicsEffect(nullptr);
                });
            }
        });

        QPointer<QPropertyAnimation> safeAnim(fadeAnim);
        QPointer<QGraphicsEffect> safeEffect(effect);
        QTimer::singleShot(delay, this, [safeAnim, safeEffect]() {
            if (safeAnim && safeEffect) safeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            else if (safeAnim) safeAnim->deleteLater();
        });
        delay += 100;
    }
}

void GameDetailsPage::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit backClicked();
    }
    QWidget::keyPressEvent(event);
}

void GameDetailsPage::clear() {
    static_cast<CinematicHeroBanner*>(m_heroBanner)->clearHero();
    m_heroTitleLabel->setText("");
    m_heroSubtitleLabel->setText("");
    m_descriptionLabel->setText("");

    // Clear tags
    QLayoutItem* child;
    while ((child = m_tagLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    // Clear screenshots
    while ((child = m_screenshotLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    // Clear specs (keep title)
    while (m_specsLayout->count() > 2) { // title + stretch
        child = m_specsLayout->takeAt(1);
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    // Clear features grid
    // Clear features grid
    while ((child = m_featuresGrid->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }



    // Clear security (keep title)
    while (m_securityLayout->count() > 1) {
        child = m_securityLayout->takeAt(1);
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    // Stop any running opacity animations before deleting their graphics effect
    // Rapidly clicking games can trigger a crash if the old animation engine is still executing
    for (QPropertyAnimation* anim : this->findChildren<QPropertyAnimation*>()) {
        if (anim->propertyName() == "opacity") {
            anim->stop();
        }
    }

    // CRITICAL FIX: Schedule effect removal for the next event loop tick.
    // If we remove them immediately while an animation is finishing, Qt6 crashes.
    auto safeRemoveEffect = [](QWidget* w) {
        if (!w) return;
        QPointer<QWidget> sw(w);
        QTimer::singleShot(0, w, [sw]() {
            if (sw) sw->setGraphicsEffect(nullptr);
        });
    };

    safeRemoveEffect(m_tagContainer);
    safeRemoveEffect(m_heroTitleLabel);
    safeRemoveEffect(m_heroSubtitleLabel);
    safeRemoveEffect(m_infoRow);
    safeRemoveEffect(m_screenshotSection);
    safeRemoveEffect(m_detailsRow);

    m_currentLoadId++;
}

void GameDetailsPage::showSkeleton() {
    clear();
    m_descriptionLabel->setText("Loading game details...");

    // Skeleton screenshot placeholders
    for (int i = 0; i < 4; ++i) {
        ScreenshotCard* card = new ScreenshotCard();
        m_screenshotLayout->addWidget(card);
    }
}

void GameDetailsPage::loadGame(const QString& appId, const QString& name, bool supported, bool hasFix) {
    m_appId = appId;
    m_gameName = name;
    m_supported = supported;
    m_hasFix = hasFix;

    m_isDownloading = false;
    showSkeleton();

    int loadId = m_currentLoadId;

    // Set title immediately
    m_heroTitleLabel->setText(m_gameName);

    // Update install button — reset BOTH text and style for the new game
    m_isDownloading = false;
    m_installButton->setEnabled(supported);
    m_installButton->setText("INSTALL");
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: #00E676; color: #000000; font-weight: 800; font-size: 16px;"
        "  border-radius: 14px; font-family: 'Segoe UI'; border: none;"
        "  letter-spacing: 1px;"
        "}"
        "QPushButton:hover {"
        "  background: #00C853;"
        "}"
        "QPushButton:disabled {"
        "  background: rgba(255,255,255,0.1); color: rgba(255,255,255,0.3);"
        "}"
    );
    m_installProgressBar->hide();
    m_installProgressBar->setValue(0);

    // Play entrance animations
    QTimer::singleShot(50, this, &GameDetailsPage::playEntranceAnimations);

    // Fetch hero background image
    QString heroUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/library_hero.jpg").arg(appId);
    QNetworkRequest hReq{QUrl(heroUrl)};
    hReq.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    hReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    QNetworkReply* heroReply = m_networkManager->get(hReq);
    connect(heroReply, &QNetworkReply::finished, this, [this, heroReply, loadId]() {
        heroReply->deleteLater();
        if (m_currentLoadId != loadId) return;
        if (heroReply->error() == QNetworkReply::NoError) {
            QPixmap rawPix;
            if (rawPix.loadFromData(heroReply->readAll())) {
                static_cast<CinematicHeroBanner*>(m_heroBanner)->setHeroPixmap(rawPix);
            }
        }
    });

    // Fetch details from Steam API
    QUrl url(QString("https://store.steampowered.com/api/appdetails?appids=%1").arg(appId));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    QNetworkReply* reply = m_networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, loadId](){
        if (m_currentLoadId == loadId) onDetailsReceived(reply);
        else reply->deleteLater();
    });

    setFocus();
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
    // ── Short description (hero subtitle) ──
    QString shortDesc = data["short_description"].toString();
    m_heroSubtitleLabel->setText(shortDesc);
    m_heroSubtitleLabel->setTextFormat(Qt::RichText);

    // ── Full description (below banner) ──
    QString fullDesc = data["detailed_description"].toString();
    if (fullDesc.isEmpty()) fullDesc = shortDesc;
    // Strip HTML tags for cleaner reading
    QString cleanDesc = fullDesc;
    cleanDesc.remove(QRegularExpression("<[^>]*>"));
    // Limit length
    if (cleanDesc.length() > 800) cleanDesc = cleanDesc.left(800) + "...";
    m_descriptionLabel->setText(cleanDesc);

    // ── Category Tags ──
    QJsonArray genres = data["genres"].toArray();
    // Clear existing tags
    QLayoutItem* tagChild;
    while ((tagChild = m_tagLayout->takeAt(0)) != nullptr) {
        if (tagChild->widget()) tagChild->widget()->deleteLater();
        delete tagChild;
    }
    for (const QJsonValue& val : genres) {
        QString genre = val.toObject()["description"].toString();
        QLabel* pill = new QLabel(genre);
        pill->setStyleSheet(
            "background: rgba(255,255,255,0.12);"
            "color: #EFECE3; font-size: 11px; font-weight: 600;"
            "font-family: 'Segoe UI';"
            "padding: 4px 14px; border-radius: 12px;"
        );
        pill->setFixedHeight(24);
        m_tagLayout->addWidget(pill);
    }

    // ── Specs Neo-Card ──
    // Remove old spec entries (keep title at 0 and stretch at end)
    while (m_specsLayout->count() > 2) {
        QLayoutItem* item = m_specsLayout->takeAt(1);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto addSpecRow = [this](const QString& key, const QString& value) {
        QWidget* row = new QWidget();
        row->setStyleSheet("background: transparent; border: none;");
        QVBoxLayout* rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(2);

        QLabel* keyLabel = new QLabel(key);
        keyLabel->setStyleSheet(
            "font-size: 11px; color: #8FABD4; font-weight: 600;"
            "font-family: 'Segoe UI'; letter-spacing: 1px;"
            "background: transparent; border: none;"
        );
        rowLayout->addWidget(keyLabel);

        QLabel* valLabel = new QLabel(value);
        valLabel->setStyleSheet(
            "font-size: 14px; color: #EFECE3; font-weight: 700;"
            "font-family: 'Segoe UI';"
            "background: transparent; border: none;"
        );
        valLabel->setWordWrap(true);
        rowLayout->addWidget(valLabel);

        // Insert before the stretch
        m_specsLayout->insertWidget(m_specsLayout->count() - 1, row);
    };

    // Release date
    if (data.contains("release_date")) {
        QJsonObject rd = data["release_date"].toObject();
        addSpecRow("RELEASE DATE", rd["date"].toString("Unknown"));
    }

    // Publishers
    QJsonArray publishers = data["publishers"].toArray();
    if (!publishers.isEmpty()) {
        QStringList pubNames;
        for (const QJsonValue& v : publishers) pubNames << v.toString();
        addSpecRow("PUBLISHER", pubNames.join(", "));
    }

    // Developers
    QJsonArray developers = data["developers"].toArray();
    if (!developers.isEmpty()) {
        QStringList devNames;
        for (const QJsonValue& v : developers) devNames << v.toString();
        addSpecRow("DEVELOPER", devNames.join(", "));
    }


    // Platforms
    if (data.contains("platforms")) {
        QJsonObject plat = data["platforms"].toObject();
        QStringList platList;
        if (plat["windows"].toBool()) platList << "Windows";
        if (plat["mac"].toBool()) platList << "macOS";
        if (plat["linux"].toBool()) platList << "Linux";
        addSpecRow("PLATFORMS", platList.join(", "));
    }

    // App ID
    addSpecRow("APP ID", m_appId);

    // Add separator line in specs card
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background: rgba(143,171,212,0.15); border: none; max-height: 1px;");
    separator->setFixedHeight(1);
    m_specsLayout->insertWidget(m_specsLayout->count() - 1, separator);

    // Extract Game Size
    QString gameSize = "Unknown";
    auto extractSize = [](const QString& reqStr) -> QString {
        // Look for words like Storage/Hard Drive followed by a size
        QRegularExpression re("(?:Storage|Hard Drive|Space).*?(\\d+(?:\\.\\d+)?\\s*[MG]B)", 
                              QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = re.match(reqStr);
        if (match.hasMatch()) {
            return match.captured(1).toUpper(); // e.g. "50 GB"
        }
        // Fallback: Just look for any MB/GB value followed by "available" or "space"
        QRegularExpression re2("(\\d+(?:\\.\\d+)?\\s*[MG]B)\\s*(?:available|free|space)", 
                               QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match2 = re2.match(reqStr);
        if (match2.hasMatch()) {
            return match2.captured(1).toUpper();
        }
        return "";
    };

    auto searchReqs = [&](const QString& osKey) -> bool {
        if (data.contains(osKey) && data[osKey].isObject()) {
            QJsonObject reqs = data[osKey].toObject();
            if (reqs.contains("minimum")) {
                QString sz = extractSize(reqs["minimum"].toString());
                if (!sz.isEmpty()) { gameSize = sz; return true; }
            }
            if (reqs.contains("recommended")) {
                QString sz = extractSize(reqs["recommended"].toString());
                if (!sz.isEmpty()) { gameSize = sz; return true; }
            }
        }
        return false;
    };

    // Try PC, then Mac, then Linux
    if (!searchReqs("pc_requirements")) {
        if (!searchReqs("mac_requirements")) {
            searchReqs("linux_requirements");
        }
    }

    addSpecRow("SIZE", gameSize);


    // ── Screenshots ──
    QJsonArray screenshots = data["screenshots"].toArray();

    // Clear skeleton placeholders
    QLayoutItem* ssChild;
    while ((ssChild = m_screenshotLayout->takeAt(0)) != nullptr) {
        if (ssChild->widget()) ssChild->widget()->deleteLater();
        delete ssChild;
    }

    int count = qMin(screenshots.size(), 8);
    int loadId = m_currentLoadId;
    for (int i = 0; i < count; ++i) {
        QJsonObject ss = screenshots[i].toObject();
        QString url = ss["path_thumbnail"].toString();

        ScreenshotCard* card = new ScreenshotCard();
        m_screenshotLayout->addWidget(card);

        QNetworkRequest imgReq{QUrl(url)};
        imgReq.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
        imgReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply* imgReply = m_networkManager->get(imgReq);
        QPointer<ScreenshotCard> safeCard(card);
        connect(imgReply, &QNetworkReply::finished, this, [imgReply, safeCard, loadId, this]() {
            imgReply->deleteLater();
            if (m_currentLoadId != loadId) return;
            if (imgReply->error() == QNetworkReply::NoError && safeCard) {
                QPixmap rawPix;
                if (rawPix.loadFromData(imgReply->readAll())) {
                    safeCard->setScreenshot(rawPix);
                }
            }
        });
    }

    // ── Features ──
    auto createFeaturePod = [](const QString& text) {
        QFrame* pod = new QFrame();
        pod->setStyleSheet(
            "QFrame { background: rgba(255,255,255,0.03); border-radius: 12px; border: none; }"
        );
        QHBoxLayout* hl = new QHBoxLayout(pod);
        hl->setContentsMargins(12, 10, 12, 10);
        hl->setSpacing(10);
        
        QString icon = QString::fromUtf8("\xE2\x9A\x99"); // ⚙️ Default gear
        if (text.contains("Controller", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x8E\xAE"); // 🎮
        else if (text.contains("Achievement", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x8F\x86"); // 🏆
        else if (text.contains("Single", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x91\xA4"); // 👤
        else if (text.contains("Multi", Qt::CaseInsensitive) || text.contains("Co-op", Qt::CaseInsensitive) || text.contains("MMO", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x91\xA5"); // 👥
        else if (text.contains("Cloud", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xE2\x98\x81"); // ☁️
        else if (text.contains("Card", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x83\x8F"); // 🃏
        else if (text.contains("In-App", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x92\xB8"); // 💸
        else if (text.contains("Action", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xE2\x9A\x94"); // ⚔️
        else if (text.contains("Adventure", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x97\xBA"); // 🗺️
        else if (text.contains("RPG", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x93\x9C"); // 📜
        else if (text.contains("Strategy", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xE2\x99\x9F"); // ♟️
        else if (text.contains("Racing", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xF0\x9F\x8F\x8E"); // 🏎️
        else if (text.contains("Sports", Qt::CaseInsensitive)) icon = QString::fromUtf8("\xE2\x9A\xBD"); // ⚽

        QLabel* iconLbl = new QLabel(icon);
        iconLbl->setStyleSheet("background: transparent; color: #8FABD4; font-size: 16px; border: none; font-family: 'Segoe UI Emoji', 'Apple Color Emoji', 'Noto Color Emoji';");
        hl->addWidget(iconLbl);

        QLabel* textLbl = new QLabel(text);
        textLbl->setStyleSheet("background: transparent; color: #EFECE3; font-size: 12px; font-weight: 600; border: none; font-family: 'Segoe UI';");
        hl->addWidget(textLbl, 1);
        return pod;
    };

    QStringList allFeatures;
    for (const QJsonValue& val : genres) {
        allFeatures.append(val.toObject()["description"].toString());
    }
    QJsonArray cats = data["categories"].toArray();
    for (const QJsonValue& val : cats) {
        allFeatures.append(val.toObject()["description"].toString());
    }
    allFeatures.removeDuplicates();
    
    int row = 0;
    int col = 0;
    int itemsAdded = 0;
    for (const QString& feat : allFeatures) {
        if (itemsAdded >= 10) break; // Limit to 10 to keep it clean (5 rows of 2)
        m_featuresGrid->addWidget(createFeaturePod(feat), row, col);
        col++;
        if (col > 1) {
            col = 0;
            row++;
        }
        itemsAdded++;
    }

    // ── Security / DRM ──
    bool hasDrm = false;
    QString drmNotice = data["drm_notice"].toString();
    QString legalNotice = data["legal_notice"].toString();

    if (!drmNotice.isEmpty()) {
        QLabel* l = new QLabel(drmNotice);
        l->setWordWrap(true);
        l->setStyleSheet("color: #FF5252; font-size: 13px; background: transparent; border: none;");
        m_securityLayout->addWidget(l);
        hasDrm = true;
    }
    if (legalNotice.contains("Denuvo", Qt::CaseInsensitive) || legalNotice.contains("Anti-cheat", Qt::CaseInsensitive)) {
        QLabel* l = new QLabel("⚠ Contains Third-party DRM / Anti-Cheat");
        l->setWordWrap(true);
        l->setStyleSheet("color: #FF5252; font-size: 13px; font-weight: bold; background: transparent; border: none;");
        m_securityLayout->addWidget(l);
        hasDrm = true;
    }

    if (!hasDrm) {
        QLabel* l = new QLabel("✓ No third-party DRM detected");
        l->setStyleSheet("color: #00E676; font-size: 13px; background: transparent; border: none;");
        m_securityLayout->addWidget(l);
    }
}

// ── Install State Management ──

void GameDetailsPage::updateInstallProgress(int pct) {
    if (!m_isDownloading) return;
    m_installProgressBar->show();
    m_installProgressBar->setValue(pct);
}

void GameDetailsPage::installFinished() {
    m_isDownloading = false;
    m_installProgressBar->hide();
    m_installButton->setText("INSTALLED ✓");
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: rgba(0,230,118,0.2); color: #00E676; font-weight: 800; font-size: 16px;"
        "  border-radius: 14px; font-family: 'Segoe UI'; border: 1px solid rgba(0,230,118,0.4);"
        "  letter-spacing: 1px;"
        "}"
    );
}

void GameDetailsPage::installError(const QString& err) {
    Q_UNUSED(err);
    m_isDownloading = false;
    m_installProgressBar->hide();
    m_installButton->setText("ERROR — RETRY");
    m_installButton->setStyleSheet(
        "QPushButton {"
        "  background: #D32F2F; color: white; font-weight: 800; font-size: 16px;"
        "  border-radius: 14px; font-family: 'Segoe UI'; border: none;"
        "  letter-spacing: 1px;"
        "}"
        "QPushButton:hover {"
        "  background: #F44336;"
        "}"
    );
}
