#include "mainwindow.h"
#include "glassbutton.h"
#include "gamecard.h"
#include "loadingspinner.h"
#include "materialicons.h"
#include "workers/indexdownloadworker.h"
#include "workers/luadownloadworker.h"
#include "workers/generatorworker.h"
#include "workers/restartworker.h"
#include "utils/colors.h"
#include "utils/paths.h"
#include "config.h"
#include <QDesktopServices>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>
// windows.h defines ERROR as a macro which conflicts with Colors::ERROR
#undef ERROR
#endif

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPointer>
#include <QFileDialog>
#include <QStackedLayout>
#include <QPropertyAnimation>
#include <QPainter>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QFile>
#include <QDir>
#include <QPixmap>
#include <QPainterPath>
#include <QFileDialog>
#include <QScrollBar>
#include <QRandomGenerator>
#include <algorithm>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QSettings>

// ── Inline helper: a QWidget that paints a single Material icon ──
class MaterialIconWidget : public QWidget {
public:
    MaterialIconWidget(MaterialIcons::Icon icon, const QColor& color, int size = 24, QWidget* parent = nullptr)
        : QWidget(parent), m_icon(icon), m_color(color) {
        setFixedSize(size, size);
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QRectF r(4, 4, width() - 8, height() - 8);
        MaterialIcons::draw(p, r, m_color, m_icon);
    }
private:
    MaterialIcons::Icon m_icon;
    QColor m_color;
};

// ── Inline helper: a QPushButton that paints a Material icon ──
class MaterialIconButton : public QPushButton {
public:
    MaterialIconButton(MaterialIcons::Icon icon, const QColor& color, int size = 40, QWidget* parent = nullptr)
        : QPushButton(parent), m_icon(icon), m_color(color) {
        setFixedSize(size, size);
        setCursor(Qt::PointingHandCursor);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        // Background
        QColor bg = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGH);
        if (underMouse()) bg = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGHEST);
        QPainterPath path;
        path.addRoundedRect(QRectF(rect()), width() / 2.0, height() / 2.0);
        p.fillPath(path, bg);
        // Icon
        int pad = 10;
        QRectF iconRect(pad, pad, width() - 2 * pad, height() - 2 * pad);
        MaterialIcons::draw(p, iconRect, m_color, m_icon);
    }
private:
    MaterialIcons::Icon m_icon;
    QColor m_color;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_currentMode(AppMode::LuaPatcher)
    , m_networkManager(nullptr)
    , m_activeReply(nullptr)
    , m_currentSearchId(0)
    , m_syncWorker(nullptr)
    , m_dlWorker(nullptr)
    , m_genWorker(nullptr)
    , m_restartWorker(nullptr)
    , m_fetchingNames(false)
    , m_nameFetchSearchId(0)
    , m_hasCachedData(false)
{
    setWindowTitle("Steam Lua Patcher");
    setMinimumSize(900, 600);
    resize(1350, 820);
    setAcceptDrops(true);
    
    // ── Enable Transparency for Desktop Blur ──
    setAttribute(Qt::WA_TranslucentBackground);
    
    QString iconPath = Paths::getResourcePath("logo.ico");
    if (QFile::exists(iconPath)) {
        setWindowIcon(QIcon(iconPath));
    }
    
    initUI();
    
    // Apply Desktop Acrylic/Mica Blur
    enableAcrylicBlur();
    
    // Retrieve username from settings (already checked in main.cpp)
    QSettings settings("LuaPatcher", "SteamLuaPatcher");
    m_username = settings.value("username", "User").toString();
    
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    connect(m_debounceTimer, &QTimer::timeout, this, &MainWindow::doSearch);
    
    m_currentGlowColor = Colors::toQColor(Colors::PRIMARY);
    m_targetGlowColor = m_currentGlowColor;
    m_glowTimer = new QTimer(this);
    connect(m_glowTimer, &QTimer::timeout, this, &MainWindow::updateAmbientGlow);
    
    QTimer::singleShot(10, this, [this]() {
        m_networkManager = new QNetworkAccessManager(this);
        connect(m_networkManager, &QNetworkAccessManager::finished,
                this, &MainWindow::onSearchFinished);
        startSync();
        fetchTrendingGames();
    });
}

MainWindow::~MainWindow() {
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
    }
}

// ── Win32 Acrylic/Mica Blur ──
void MainWindow::enableAcrylicBlur() {
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    
    // Step 1: Extend DWM frame into entire client area (required for blur visibility)
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    
    // Step 2: Try Windows 11 system backdrop (DWMWA_SYSTEMBACKDROP_TYPE = 38)
    // Value 3 = DWMSBT_TRANSIENTWINDOW (Acrylic), Value 2 = DWMSBT_MAINWINDOW (Mica)
    int backdropType = 3;
    HRESULT hr = DwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
    
    // Step 3: If Win11 backdrop failed, use SetWindowCompositionAttribute (Win10 1803+)
    if (FAILED(hr)) {
        struct ACCENT_POLICY {
            int AccentState;
            int AccentFlags;
            int GradientColor;
            int AnimationId;
        };
        struct WINDOWCOMPOSITIONATTRIBDATA {
            int Attrib;
            PVOID pvData;
            SIZE_T cbData;
        };
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
        
        HMODULE hUser = GetModuleHandleA("user32.dll");
        if (hUser) {
            auto SetWCA = (pSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
            if (SetWCA) {
                ACCENT_POLICY policy = {};
                policy.AccentState = 4; // ACCENT_ENABLE_ACRYLICBLURBEHIND
                policy.AccentFlags = 2; // ACCENT_FLAG_DRAW_ALL
                policy.GradientColor = 0x99191B21; // AABBGGRR — dark tint
                
                WINDOWCOMPOSITIONATTRIBDATA data = {};
                data.Attrib = 19; // WCA_ACCENT_POLICY
                data.pvData = &policy;
                data.cbData = sizeof(policy);
                
                SetWCA(hwnd, &data);
            }
        }
    }
#endif
}

// ── Native event passthrough ──
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
    return QMainWindow::nativeEvent(eventType, message, result);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        if (obj->property("isHeroSlide").toBool()) {
            QString appId = obj->property("gameAppId").toString();
            for (const auto& g : m_supportedGames) {
                if (g.id == appId) {
                    QMap<QString, QString> cd;
                    cd["name"] = m_nameCache.value(g.id, g.id);
                    cd["appid"] = g.id;
                    cd["supported"] = "true";
                    cd["hasFix"] = g.hasFix ? "true" : "false";
                    GameCard tempCard(this);
                    tempCard.setGameData(cd);
                    onCardClicked(&tempCard);
                    return true;
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::scrollCarousel() {
    if (m_heroStack->count() == 0 || !m_heroStack->isVisible()) return;
    m_currentHeroIndex++;
    if (m_currentHeroIndex >= m_heroStack->count()) {
        m_currentHeroIndex = 0;
    }
    
    // Crossfade to the next banner
    m_heroStack->setCurrentIndex(m_currentHeroIndex);
}

void MainWindow::updateAmbientGlow() {
    int dr = m_targetGlowColor.red() - m_currentGlowColor.red();
    int dg = m_targetGlowColor.green() - m_currentGlowColor.green();
    int db = m_targetGlowColor.blue() - m_currentGlowColor.blue();
    
    if (qAbs(dr) < 2 && qAbs(dg) < 2 && qAbs(db) < 2) {
        m_currentGlowColor = m_targetGlowColor;
        m_glowTimer->stop();
    } else {
        m_currentGlowColor.setRed(m_currentGlowColor.red() + dr * 0.1);
        m_currentGlowColor.setGreen(m_currentGlowColor.green() + dg * 0.1);
        m_currentGlowColor.setBlue(m_currentGlowColor.blue() + db * 0.1);
    }
    update();
}

void MainWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // ── Semi-transparent dark background allowing desktop blur to show through ──
    QLinearGradient bgGrad(0, 0, rect().width(), rect().height());
    QColor color1(25, 27, 33); color1.setAlpha(170); // ~65% opacity
    QColor color2(18, 19, 23); color2.setAlpha(170);
    bgGrad.setColorAt(0.0, color1);
    bgGrad.setColorAt(1.0, color2);
    painter.fillRect(rect(), bgGrad);
    
    // ── Large warm orange/red ambient glow (top right, behind the banner) ──
    QRadialGradient glow1(rect().width() * 0.75, rect().height() * 0.2, rect().width() * 0.6);
    QColor warmRed(220, 60, 40); // vibrant orange-red
    warmRed.setAlpha(35);
    glow1.setColorAt(0, warmRed);
    glow1.setColorAt(0.5, QColor(warmRed.red(), warmRed.green(), warmRed.blue(), 15));
    glow1.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), glow1);
    
    // ── Subtle secondary glow (bottom left) ──
    QRadialGradient glow2(rect().width() * 0.2, rect().height() * 0.8, rect().width() * 0.5);
    QColor warmAmber(180, 100, 40);
    warmAmber.setAlpha(20);
    glow2.setColorAt(0, warmAmber);
    glow2.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), glow2);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls) {
            if (url.isLocalFile() && url.toLocalFile().endsWith(".lua", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;

    QString pluginDir = Config::getSteamPluginDir();
    QDir dir(pluginDir);
    if (!dir.exists()) dir.mkpath(".");

    int count = 0;
    QString lastFile;

    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            QString srcPath = url.toLocalFile();
            if (srcPath.endsWith(".lua", Qt::CaseInsensitive)) {
                QString fileName = QFileInfo(srcPath).fileName();
                QString destPath = dir.filePath(fileName);
                
                QFile::remove(destPath); // Overwrite
                if (QFile::copy(srcPath, destPath)) {
                    count++;
                    lastFile = fileName;
                }
            }
        }
    }

    if (count > 0) {
        m_statusLabel->setText(QString("Installed %1 patch%2").arg(count).arg(count > 1 ? "es" : ""));
        if (m_currentMode == AppMode::Library) {
            displayLibrary();
        } else {
            // Switch to library to show the new patch
            m_tabLibrary->animateClick();
        }
        event->acceptProposedAction();
    }
}

void MainWindow::initUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    
    QHBoxLayout* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    
    // ──── Material Navigation Rail (Sidebar) ────
    QWidget* sidebarWidget = new QWidget();
    sidebarWidget->setFixedWidth(230);
    sidebarWidget->setAttribute(Qt::WA_StyledBackground);
    sidebarWidget->setAutoFillBackground(false);
    sidebarWidget->setStyleSheet(QString(
        "background-color: transparent; border-right: 1px solid rgba(255, 255, 255, 10);"
    ));
    
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(16, 24, 16, 16);
    sidebarLayout->setSpacing(8);
    
    // ── App header with actual logo ──
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);
    
    QLabel* appIconLabel = new QLabel();
    appIconLabel->setFixedSize(36, 36);
    QString iconPath = Paths::getResourcePath("logo.ico");
    if (QFile::exists(iconPath)) {
        QPixmap logoPixmap(iconPath);
        appIconLabel->setPixmap(logoPixmap.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Fallback: draw a Material Flash icon
        appIconLabel->setStyleSheet(QString(
            "background: %1; border-radius: 10px; border: none;"
        ).arg(Colors::PRIMARY_CONTAINER));
    }
    appIconLabel->setStyleSheet(appIconLabel->styleSheet() + " border: none; background: transparent;");
    headerLayout->addWidget(appIconLabel);
    
    QLabel* title = new QLabel("Lua Patcher");
    title->setStyleSheet(QString(
        "font-size: 17px; font-weight: 700; color: %1; background: transparent; border: none; font-family: 'Roboto', 'Segoe UI';"
    ).arg(Colors::ON_SURFACE));
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    sidebarLayout->addLayout(headerLayout);
    sidebarLayout->addSpacing(20);
    
    // ── Section label ──
    QLabel* navLabel = new QLabel("NAVIGATION");
    navLabel->setStyleSheet(QString(
        "font-size: 10px; font-weight: 600; color: %1; letter-spacing: 1px;"
        " background: transparent; border: none; padding-left: 4px; font-family: 'Roboto', 'Segoe UI';"
    ).arg(Colors::OUTLINE));
    sidebarLayout->addWidget(navLabel);
    sidebarLayout->addSpacing(4);
    
    // Navigation tabs
    m_tabLua = new GlassButton(MaterialIcons::Download, " App Store", "", Colors::PRIMARY);
    m_tabLua->setFixedHeight(44);
    connect(m_tabLua, &QPushButton::clicked, this, [this](){ switchMode(AppMode::LuaPatcher); });
    sidebarLayout->addWidget(m_tabLua);

    m_tabLibrary = new GlassButton(MaterialIcons::Library, " Library", "", Colors::ACCENT_GREEN);
    m_tabLibrary->setFixedHeight(44);
    connect(m_tabLibrary, &QPushButton::clicked, this, [this](){ switchMode(AppMode::Library); });
    sidebarLayout->addWidget(m_tabLibrary);
    
    sidebarLayout->addSpacing(8);
    
    m_tabSettings = new GlassButton(MaterialIcons::Settings, " Settings", "", Colors::OUTLINE);
    m_tabSettings->setFixedHeight(44);
    connect(m_tabSettings, &QPushButton::clicked, this, [this](){ switchMode(AppMode::Settings); });
    sidebarLayout->addWidget(m_tabSettings);
    
    m_tabDiscord = new GlassButton(MaterialIcons::Discord, " Discord", "", Colors::ACCENT_BLUE);
    m_tabDiscord->setFixedHeight(44);
    connect(m_tabDiscord, &QPushButton::clicked, this, [this](){
        QDesktopServices::openUrl(QUrl("https://discord.gg/your-server"));
    });
    sidebarLayout->addWidget(m_tabDiscord);
    
    sidebarLayout->addSpacing(8);
    
    // Material divider
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1; border: none;").arg(Colors::OUTLINE_VARIANT));
    sidebarLayout->addWidget(line);
    sidebarLayout->addSpacing(4);
    
    m_statusLabel = new QLabel("Initializing...");
    m_statusLabel->setStyleSheet(QString(
        "color: %1; font-size: 11px; font-family: 'Roboto', 'Segoe UI'; background: transparent; border: none;"
    ).arg(Colors::ON_SURFACE_VARIANT));
    m_statusLabel->setWordWrap(true);
    // Removed status label from sidebar as requested by user
    // sidebarLayout->addWidget(m_statusLabel);
    
    sidebarLayout->addStretch();
    
    // ── Section label for actions ──
    QLabel* actionsLabel = new QLabel("ACTIONS");
    actionsLabel->setStyleSheet(QString(
        "font-size: 10px; font-weight: 600; color: %1; letter-spacing: 1px;"
        " background: transparent; border: none; padding-left: 4px; font-family: 'Roboto', 'Segoe UI';"
    ).arg(Colors::OUTLINE));
    sidebarLayout->addWidget(actionsLabel);
    sidebarLayout->addSpacing(4);
    
    // Action buttons
    m_btnAddToLibrary = new GlassButton(MaterialIcons::Add, "Add to Library", "Install / Generate Patch", Colors::ACCENT_GREEN);
    m_btnAddToLibrary->setFixedHeight(52);
    m_btnAddToLibrary->setEnabled(false);
    connect(m_btnAddToLibrary, &QPushButton::clicked, this, &MainWindow::doAddGame);
    sidebarLayout->addWidget(m_btnAddToLibrary);

    m_btnRemove = new GlassButton(MaterialIcons::Delete, "Remove", "Remove from Library", Colors::ACCENT_RED);
    m_btnRemove->setFixedHeight(52);
    m_btnRemove->setEnabled(false);
    m_btnRemove->hide();
    connect(m_btnRemove, &QPushButton::clicked, this, &MainWindow::doRemoveGame);
    sidebarLayout->addWidget(m_btnRemove);
    
    sidebarLayout->addSpacing(6);
    m_btnRestart = new GlassButton(MaterialIcons::RestartAlt, "Restart Steam", "Apply Changes", Colors::PRIMARY);
    m_btnRestart->setFixedHeight(52);
    connect(m_btnRestart, &QPushButton::clicked, this, &MainWindow::doRestart);
    sidebarLayout->addWidget(m_btnRestart);
    sidebarLayout->addSpacing(12);
    
    // Divider before version info
    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setFixedHeight(1);
    line2->setStyleSheet(QString("background: %1; border: none;").arg(Colors::OUTLINE_VARIANT));
    sidebarLayout->addWidget(line2);
    sidebarLayout->addSpacing(8);
    
    QLabel* infoLabel = new QLabel(QString("v%1<br>by <a href=\"https://github.com/sayedalimollah2602-prog\" style=\"color: %2; text-decoration: none;\">leVI</a> & <a href=\"https://github.com/raxnmint\" style=\"color: %2; text-decoration: none;\">raxnmint</a>").arg(Config::APP_VERSION).arg(Colors::ON_SURFACE_VARIANT));
    infoLabel->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold; font-family: 'Roboto', 'Segoe UI'; background: transparent; border: none;").arg(Colors::ON_SURFACE_VARIANT));
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    infoLabel->setOpenExternalLinks(true);
    sidebarLayout->addWidget(infoLabel);
    rootLayout->addWidget(sidebarWidget);

    // ──── Content Area ────
    QWidget* contentWidget = new QWidget();
    contentWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    // ── Top Bar (Search + Profile) ──
    QWidget* topBarWidget = new QWidget();
    QHBoxLayout* topBarLayout = new QHBoxLayout(topBarWidget);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(16);
    
    // Search Container
    QWidget* searchContainer = new QWidget();
    searchContainer->setStyleSheet(QString(
        "background: rgba(255, 255, 255, 12); border-radius: 16px; border: 1px solid rgba(255, 255, 255, 25);"
    ));
    searchContainer->setFixedHeight(56);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(12, 4, 12, 4);
    
    MaterialIconWidget* searchIconWidget = new MaterialIconWidget(
        MaterialIcons::Search, Colors::toQColor(Colors::ON_SURFACE_VARIANT), 32);
    searchLayout->addWidget(searchIconWidget);
    
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Search thousands of games...");
    m_searchInput->setStyleSheet(QString(
        "QLineEdit { background: transparent; border: none; font-size: 15px; color: %1; padding: 0 8px; }"
        "QLineEdit:focus { border: none; background: transparent; }"
    ).arg(Colors::ON_SURFACE));
    connect(m_searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    searchLayout->addWidget(m_searchInput);
    
    MaterialIconButton* refreshBtn = new MaterialIconButton(
        MaterialIcons::Refresh, Colors::toQColor(Colors::ON_SURFACE_VARIANT), 36);
    connect(refreshBtn, &QPushButton::clicked, this, [this]() {
        if (m_searchInput->text().trimmed().isEmpty()) startSync(); else doSearch();
    });
    searchLayout->addWidget(refreshBtn);
    topBarLayout->addWidget(searchContainer, 1);
    
    // Profile Widget
    m_topProfileWidget = new QWidget();
    m_topProfileWidget->setFixedHeight(56);
    m_topProfileWidget->setMinimumWidth(160);
    m_topProfileWidget->setStyleSheet(
        "background: transparent; border: none; border-radius: 16px;"
    );
    QHBoxLayout* profileLayout = new QHBoxLayout(m_topProfileWidget);
    profileLayout->setContentsMargins(12, 0, 16, 0);
    profileLayout->setSpacing(10);
    
    // Circular avatar with first letter
    QString displayName = m_username.isEmpty() ? "Guest" : m_username;
    QChar firstLetter = displayName.at(0).toUpper();
    
    // Generate a deterministic vibrant color from username
    static const QColor avatarColors[] = {
        QColor(229, 115, 115), QColor(186, 104, 200),
        QColor(100, 181, 246), QColor(77, 208, 225),
        QColor(129, 199, 132), QColor(255, 183, 77),
        QColor(240, 98, 146),  QColor(149, 117, 205),
    };
    int colorIdx = 0;
    for (QChar c : displayName) colorIdx += c.unicode();
    QColor avatarColor = avatarColors[colorIdx % 8];
    
    int avatarSize = 36;
    QPixmap avatarPix(avatarSize, avatarSize);
    avatarPix.fill(Qt::transparent);
    QPainter avatarPainter(&avatarPix);
    avatarPainter.setRenderHint(QPainter::Antialiasing);
    avatarPainter.setBrush(avatarColor);
    avatarPainter.setPen(Qt::NoPen);
    avatarPainter.drawEllipse(0, 0, avatarSize, avatarSize);
    avatarPainter.setPen(Qt::white);
    QFont avatarFont("Segoe UI", 15, QFont::Bold);
    avatarPainter.setFont(avatarFont);
    avatarPainter.drawText(QRect(0, 0, avatarSize, avatarSize), Qt::AlignCenter, QString(firstLetter));
    avatarPainter.end();
    
    QLabel* avatarLabel = new QLabel();
    avatarLabel->setPixmap(avatarPix);
    avatarLabel->setFixedSize(avatarSize, avatarSize);
    avatarLabel->setStyleSheet("background: transparent; border: none;");
    profileLayout->addWidget(avatarLabel);
    
    m_topUsernameLabel = new QLabel(displayName);
    m_topUsernameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white; background: transparent; border: none;");
    profileLayout->addWidget(m_topUsernameLabel, 1);
    topBarLayout->addWidget(m_topProfileWidget);
    
    mainLayout->addWidget(topBarWidget);
    
    // Stacked widget: page 0 = loading, page 1 = main content
    m_stack = new QStackedWidget();
    
    QWidget* pageLoading = new QWidget();
    QVBoxLayout* layLoading = new QVBoxLayout(pageLoading);
    layLoading->setAlignment(Qt::AlignCenter);
    m_spinner = new LoadingSpinner();
    layLoading->addWidget(m_spinner);
    m_stack->addWidget(pageLoading); // index 0
    
    // ── Main Content Page (Scrollable) ──
    m_mainScrollArea = new QScrollArea();
    m_mainScrollArea->setWidgetResizable(true);
    m_mainScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mainScrollArea->setFrameShape(QFrame::NoFrame);
    m_mainScrollArea->setStyleSheet(QString(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: %1; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: %2; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: %3; }"
    ).arg("rgba(0,0,0,0)").arg(Colors::OUTLINE_VARIANT).arg(Colors::OUTLINE));
    
    m_mainScrollContainer = new QWidget();
    m_mainScrollContainer->setStyleSheet("background: transparent;");
    m_mainScrollLayout = new QVBoxLayout(m_mainScrollContainer);
    // Added horizontal padding (24px) so the banner and grid fit gracefully within the window bounds
    m_mainScrollLayout->setContentsMargins(10, 0, 10, 20);
    m_mainScrollLayout->setSpacing(16);
    
    // 1. Hero Stack — holds up to 4 trending games, shows exactly one at a time
    m_leadingTitlesLabel = new QLabel("Leading Titles");
    m_leadingTitlesLabel->setMaximumWidth(1200);
    m_leadingTitlesLabel->setStyleSheet("font-size: 20px; font-weight: bold; padding-left: 4px; color: white;");
    
    m_heroStack = new QStackedWidget();
    m_heroStack->setFixedHeight(240);
    m_heroStack->setMaximumWidth(1200); // Increased width to fit window more completely
    m_heroStack->setStyleSheet("background: transparent; border: none; border-radius: 12px;");
    
    m_mainScrollLayout->addWidget(m_leadingTitlesLabel, 0, Qt::AlignLeft);
    m_mainScrollLayout->addWidget(m_heroStack, 0, Qt::AlignHCenter);
    
    // We can fade crossfade manually, but QStackedWidget doesn't officially animate between indices out-of-the-box.
    // However, it solves the scrolling 'halfway' visual bug and stays exactly fixed.
    m_heroCarouselTimer = new QTimer(this);
    connect(m_heroCarouselTimer, &QTimer::timeout, this, &MainWindow::scrollCarousel);
    
    
    // 2. Hidden trending container (still needed for trending card data)
    m_trendingTitle = new QLabel("Trending Games");
    m_trendingTitle->hide();
    m_mainScrollLayout->addWidget(m_trendingTitle);
    
    m_trendingScroll = new QScrollArea();
    m_trendingScroll->setWidgetResizable(true);
    m_trendingScroll->setFixedHeight(0);
    m_trendingScroll->hide();
    m_trendingScroll->setStyleSheet("background: transparent; border: none;");
    QWidget* trendContainer = new QWidget();
    m_trendingLayout = new QHBoxLayout(trendContainer);
    m_trendingLayout->setContentsMargins(0, 0, 0, 0);
    m_trendingLayout->setAlignment(Qt::AlignLeft);
    m_trendingScroll->setWidget(trendContainer);
    m_mainScrollLayout->addWidget(m_trendingScroll);
    
    // 3. All Games Grid
    m_gridTitleLabel = new QLabel("All Available Games");
    m_gridTitleLabel->setStyleSheet("font-size: 20px; font-weight: bold; padding-left: 4px; color: white;");
    m_mainScrollLayout->addWidget(m_gridTitleLabel);
    
    m_gridContainer = new QWidget();
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setContentsMargins(4, 4, 4, 4);
    m_gridLayout->setSpacing(14);
    m_gridLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_mainScrollLayout->addWidget(m_gridContainer);
    
    m_mainScrollArea->setWidget(m_mainScrollContainer);
    connect(m_mainScrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MainWindow::loadVisibleThumbnails);
    
    m_stack->addWidget(m_mainScrollArea); // index 1
    
    // 4. Settings Page
    QWidget* settingsWidget = new QWidget();
    settingsWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsWidget);
    settingsLayout->setAlignment(Qt::AlignTop);
    
    QLabel* settingsTitle = new QLabel("Settings");
    settingsTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");
    settingsLayout->addWidget(settingsTitle);
    settingsLayout->addSpacing(20);
    
    QLabel* pathLabel = new QLabel("Steam Plugin Folder Path:");
    pathLabel->setStyleSheet("color: rgba(255,255,255,160); font-size: 14px;");
    settingsLayout->addWidget(pathLabel);
    
    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLineEdit* pathInput = new QLineEdit(Config::getSteamPluginDir());
    pathInput->setStyleSheet("background: rgba(8, 10, 18, 150); border: 1px solid rgba(255,255,255,25); color: white; padding: 10px; border-radius: 6px; font-size: 14px;");
    pathInput->setReadOnly(true);
    pathLayout->addWidget(pathInput);
    
    GlassButton* browseBtn = new GlassButton(MaterialIcons::Search, " Browse...", "", Colors::PRIMARY);
    browseBtn->setFixedHeight(44);
    connect(browseBtn, &QPushButton::clicked, this, [this, pathInput]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Steam Plugin Directory", pathInput->text());
        if (!dir.isEmpty()) {
            // Unify separators visually
            pathInput->setText(dir);
            QSettings s("LuaPatcher", "SteamLuaPatcher");
            s.setValue("PluginDir", dir);
        }
    });
    pathLayout->addWidget(browseBtn);
    settingsLayout->addLayout(pathLayout);
    settingsLayout->addStretch();
    m_stack->addWidget(settingsWidget); // index 2
    
    mainLayout->addWidget(m_stack);
    
    // Progress bar - Material linear progress
    m_progress = new QProgressBar();
    m_progress->setFixedHeight(4);
    m_progress->setTextVisible(false);
    m_progress->setStyleSheet(QString(
        "QProgressBar { background: %1; border-radius: 2px; }"
        "QProgressBar::chunk { background: %2; border-radius: 2px; }"
    ).arg(Colors::SURFACE_VARIANT).arg(Colors::PRIMARY));
    m_progress->hide();
    mainLayout->addWidget(m_progress);
    
    rootLayout->addWidget(contentWidget);
    m_terminalDialog = new TerminalDialog(this);
    updateModeUI();
}

void MainWindow::clearGameCards() {
    m_selectedCard = nullptr;
    
    // Clear trending layout
    while (QLayoutItem* item = m_trendingLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->hide();
            widget->deleteLater();
        }
        delete item;
    }
    
    // Clear grid layout
    while (QLayoutItem* item = m_gridLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->hide();
            widget->deleteLater();
        }
        delete item;
    }
    
    m_gameCards.clear();
}

void MainWindow::displayRandomGames() {
    clearGameCards();
    m_selectedGame.clear();
    m_btnAddToLibrary->setEnabled(false);
    cancelNameFetches();
    m_pendingNameFetchIds.clear();

    if (m_supportedGames.isEmpty()) return;

    // Show Home-specific components
    if (m_leadingTitlesLabel) m_leadingTitlesLabel->show();
    if (m_heroStack) m_heroStack->show();
    m_mainScrollArea->show();
    
    // Update grid title
    m_gridTitleLabel->setText("All Available Games");

    // Build a set of supported IDs for fast lookup
    QSet<QString> supportedIds;
    for (const auto& g : m_supportedGames) supportedIds.insert(g.id);

    // Featured trending games for hero banner carousel
    QList<GameInfo> carouselGames;
    if (!m_trendingAppIds.isEmpty()) {
        for (const QString& tid : m_trendingAppIds) {
            if (!supportedIds.contains(tid)) continue;
            for (const auto& g : m_supportedGames) {
                if (g.id == tid) { carouselGames.append(g); break; }
            }
            if (carouselGames.size() >= 4) break;
        }
    }
    
    // Fallback: pick random supported games
    if (carouselGames.size() < 4 && !m_supportedGames.isEmpty()) {
        QList<GameInfo> randGames = m_supportedGames;
        auto *rng = QRandomGenerator::global();
        for (int i = randGames.size() - 1; i > 0; --i) {
            randGames.swapItemsAt(i, rng->bounded(i + 1));
        }
        for (int i = 0; i < qMin(4 - carouselGames.size(), (int)randGames.size()); ++i) {
            carouselGames.append(randGames[i]);
        }
    }
    
    // Clear old carousel slides
    while (m_heroStack->count() > 0) {
        QWidget* w = m_heroStack->widget(0);
        m_heroStack->removeWidget(w);
        w->deleteLater();
    }
    m_currentHeroIndex = 0;
    
    // Build new slides
    for (const GameInfo& game : carouselGames) {
        QString featuredId = game.id;
        QString featuredName = game.name;
        if (featuredName.isEmpty() || featuredName == game.id) {
            featuredName = m_nameCache.contains(game.id) ? m_nameCache[game.id] : game.id;
        }
        
        QWidget* slide = new QWidget();
        slide->setFixedHeight(240);
        slide->setCursor(Qt::PointingHandCursor);
        slide->setProperty("isHeroSlide", true);
        slide->setProperty("gameAppId", featuredId);
        slide->installEventFilter(this);
        
        QStackedLayout* stack = new QStackedLayout(slide);
        stack->setStackingMode(QStackedLayout::StackAll);
        stack->setContentsMargins(0,0,0,0);
        
        QLabel* imgLabel = new QLabel();
        imgLabel->setStyleSheet("border-radius: 12px; border: none; background: transparent;");
        imgLabel->setScaledContents(true);
        stack->addWidget(imgLabel);
        
        QWidget* overlay = new QWidget();
        overlay->setFixedHeight(240);
        overlay->setStyleSheet(
            "background: qlineargradient(x1:0, y1:1, x2:0, y2:0, stop:0 rgba(0,0,0,220), stop:0.4 rgba(0,0,0,0));"
            "border-radius: 12px;"
        );
        QVBoxLayout* overlayLayout = new QVBoxLayout(overlay);
        overlayLayout->addStretch(1);
        
        QLabel* nameLbl = new QLabel(featuredName);
        nameLbl->setStyleSheet("font-size: 26px; font-weight: bold; color: white; background: transparent; border: none;");
        nameLbl->setAlignment(Qt::AlignHCenter);
        overlayLayout->addWidget(nameLbl);
        
        QLabel* idLbl = new QLabel(QString("App ID: %1").arg(featuredId));
        idLbl->setStyleSheet("font-size: 14px; color: #aaaaaa; background: transparent; border: none;");
        idLbl->setAlignment(Qt::AlignHCenter);
        overlayLayout->addWidget(idLbl);
        overlayLayout->addSpacing(15);
        
        stack->addWidget(overlay);
        m_heroStack->addWidget(slide);
        
        // Fetch high-quality hero image, with fallback to low-res header
        QString heroUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/library_hero.jpg").arg(featuredId);
        QNetworkRequest req{QUrl(heroUrl)};
        req.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
        QNetworkReply* heroReply = m_networkManager->get(req);
        
        QPointer<QLabel> safeImgLabel(imgLabel);
        auto* nm = m_networkManager;
        connect(heroReply, &QNetworkReply::finished, this, [heroReply, safeImgLabel, nm, featuredId]() {
            heroReply->deleteLater();
            bool success = false;
            if (heroReply->error() == QNetworkReply::NoError && safeImgLabel) {
                QPixmap rawPix;
                if (rawPix.loadFromData(heroReply->readAll())) {
                    // Create rounded version to ensure corners aren't sharp
                    QPixmap rounded(rawPix.size());
                    rounded.fill(Qt::transparent);
                    QPainter painter(&rounded);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(rounded.rect(), 35, 35); // Visual radius for the raw buffer
                    painter.setClipPath(path);
                    painter.drawPixmap(0, 0, rawPix);
                    
                    if (safeImgLabel) safeImgLabel->setPixmap(rounded);
                    success = true;
                }
            }
            // Fallback strategy if HD library hero is missing (fetch low-res header)
            if (!success && safeImgLabel && nm) {
                QString fallbackUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/header.jpg").arg(featuredId);
                QNetworkRequest fallbackReq{QUrl(fallbackUrl)};
                fallbackReq.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
                QNetworkReply* fallbackReply = nm->get(fallbackReq);
                connect(fallbackReply, &QNetworkReply::finished, [fallbackReply, safeImgLabel]() {
                    fallbackReply->deleteLater();
                    if (fallbackReply->error() == QNetworkReply::NoError && safeImgLabel) {
                        QPixmap rawPix;
                        if (rawPix.loadFromData(fallbackReply->readAll())) {
                            QPixmap rounded(rawPix.size());
                            rounded.fill(Qt::transparent);
                            QPainter painter(&rounded);
                            painter.setRenderHint(QPainter::Antialiasing);
                            QPainterPath path;
                            path.addRoundedRect(rounded.rect(), 25, 25);
                            painter.setClipPath(path);
                            painter.drawPixmap(0, 0, rawPix);
                            if (safeImgLabel) safeImgLabel->setPixmap(rounded);
                        }
                    }
                });
            }
        });
    }
    
    // Start slider timer to rotate every 5 seconds
    m_heroCarouselTimer->start(5000);

    // All Games Grid — shuffled
    QList<GameInfo> shuffled = m_supportedGames;
    auto *rng = QRandomGenerator::global();
    for (int i = shuffled.size() - 1; i > 0; --i) {
        int j = rng->bounded(i + 1);
        shuffled.swapItemsAt(i, j);
    }
    
    int gridIdx = 0;
    for (const GameInfo& game : shuffled) {
        if (gridIdx >= 36) break;
        
        QMap<QString, QString> cd;
        cd["name"] = game.name;
        cd["appid"] = game.id;
        cd["supported"] = "true";
        cd["hasFix"] = game.hasFix ? "true" : "false";

        if (cd["name"].isEmpty() || cd["name"] == game.id) {
            cd["name"] = m_nameCache.contains(game.id) ? m_nameCache[game.id] : "Loading...";
        }
        if (cd["name"] == "Loading...") m_pendingNameFetchIds.append(game.id);

        GameCard* card = new GameCard(m_gridLayout->parentWidget());
        card->setGameData(cd);
        connect(card, &GameCard::clicked, this, &MainWindow::onCardClicked);
        m_gridLayout->addWidget(card, gridIdx / 6, gridIdx % 6);
        m_gameCards.append(card);

        if (m_thumbnailCache.contains(game.id)) card->setThumbnail(m_thumbnailCache[game.id]);
        gridIdx++;
    }

    QTimer::singleShot(50, this, &MainWindow::loadVisibleThumbnails);
    if (!m_pendingNameFetchIds.isEmpty()) startBatchNameFetch();

    m_statusLabel->setText(QString("Home: %1 games discovered").arg(m_gameCards.count()));
    m_stack->setCurrentIndex(1);
    m_spinner->stop();
}

// ---- Fetch trending games from SteamSpy ----
void MainWindow::fetchTrendingGames() {
    if (!m_networkManager) return;
    
    QUrl url("https://steamspy.com/api.php?request=top100in2weeks");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    
    QNetworkReply* reply = m_networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onTrendingFetched(reply);
    });
}

void MainWindow::onTrendingFetched(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) return;
    
    QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    m_trendingAppIds.clear();
    
    // SteamSpy returns {appid: {data...}, ...} — keys are the app IDs
    // They come sorted by popularity already
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_trendingAppIds.append(it.key());
    }
    
    // Refresh the display if we're on the home screen
    if (m_currentMode == AppMode::LuaPatcher && m_searchInput->text().trimmed().isEmpty()) {
        displayRandomGames();
    }
}

// ---- Display installed patches (Library) ----
void MainWindow::displayLibrary() {
    clearGameCards();
    m_selectedGame.clear();
    m_btnAddToLibrary->setEnabled(false);
    m_btnRemove->setEnabled(false);
    
    // Hide Home-specific components
    if (m_leadingTitlesLabel) m_leadingTitlesLabel->hide();
    if (m_heroStack) m_heroStack->hide();
    if (m_trendingTitle) m_trendingTitle->hide();
    if (m_trendingScroll) m_trendingScroll->hide();
    if (m_gridTitleLabel) m_gridTitleLabel->setText("Installed Patches");

    QStringList pluginDirs = Config::getAllSteamPluginDirs();
    QSet<QString> installedAppIds;
    
    for (const QString& dirPath : pluginDirs) {
        QDir dir(dirPath);
        QStringList luaFiles = dir.entryList({"*.lua"}, QDir::Files);
        for (const QString& file : luaFiles) {
            QString appId = QFileInfo(file).baseName();
            if (!appId.isEmpty()) installedAppIds.insert(appId);
        }
    }

    if (installedAppIds.isEmpty()) {
        m_statusLabel->setText("No patches installed found.");
        m_stack->setCurrentIndex(1);
        return;
    }

    int count = 0;
    for (const QString& appId : installedAppIds) {
        if (count >= 100) break;

        QString name = m_nameCache.contains(appId) ? m_nameCache[appId] : "Loading...";
        bool hasFix = false;
        for (const auto& g : m_supportedGames) {
            if (g.id == appId) { hasFix = g.hasFix; break; }
        }

        if (name == "Loading...") m_pendingNameFetchIds.append(appId);

        GameCard* card = new GameCard(m_gridLayout->parentWidget());
        card->setGameData({{"name", name}, {"appid", appId}, {"supported", "local"}, {"hasFix", hasFix ? "true" : "false"}});
        connect(card, &GameCard::clicked, this, &MainWindow::onCardClicked);
        m_gridLayout->addWidget(card, count / 6, count % 6);
        m_gameCards.append(card);

        if (m_thumbnailCache.contains(appId)) card->setThumbnail(m_thumbnailCache[appId]);
        count++;
    }

    QTimer::singleShot(50, this, &MainWindow::loadVisibleThumbnails);
    if (!m_pendingNameFetchIds.isEmpty()) startBatchNameFetch();
    
    m_statusLabel->setText(QString("Library: %1 patches found").arg(m_gameCards.count()));
    m_stack->setCurrentIndex(1);
    m_spinner->stop();
}

// ---- Sync ----
void MainWindow::startSync() {
    // Load persistent name cache from disk
    loadNameCache();

    // Try to load cached index for instant display
    if (loadCachedIndex()) {
        m_hasCachedData = true;
        m_statusLabel->setText("Syncing in background...");
    } else {
        // No cache available - show skeleton placeholders in grid
        m_hasCachedData = false;
        clearGameCards();
        for (int i = 0; i < 12; ++i) {
            GameCard* card = new GameCard(m_gridLayout->parentWidget());
            card->setSkeleton(true);
            m_gridLayout->addWidget(card, i / 6, i % 6);
            m_gameCards.append(card);
        }
        m_stack->setCurrentIndex(1);
        m_spinner->stop();
    }

    // Always start background sync to get fresh data
    m_syncWorker = new IndexDownloadWorker(this);
    connect(m_syncWorker, &IndexDownloadWorker::finished, this, &MainWindow::onSyncDone);
    connect(m_syncWorker, &IndexDownloadWorker::progress, [this](QString msg) {
        // Only show sync progress if we don't already have cached data displayed
        if (!m_hasCachedData) m_statusLabel->setText(msg);
    });
    connect(m_syncWorker, &IndexDownloadWorker::error, this, &MainWindow::onSyncError);
    m_syncWorker->start();
}

bool MainWindow::loadCachedIndex() {
    QString indexPath = Paths::getLocalIndexPath();
    if (!QFile::exists(indexPath)) return false;

    QFile file(indexPath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject indexData = doc.object();
    QJsonArray arr = indexData["games"].toArray();
    if (arr.isEmpty()) return false;

    // Parse games and apply name cache
    QList<GameInfo> games;
    games.reserve(arr.size());
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        GameInfo game;
        game.id = obj["id"].toString();
        game.name = obj["name"].toString();
        // Apply cached name if the index has an unknown name
        if (game.name.startsWith("Unknown Game") && m_nameCache.contains(game.id)) {
            game.name = m_nameCache[game.id];
        }
        game.hasFix = obj["has_fix"].toBool(false);
        games.append(game);
    }

    m_supportedGames = games;
    m_searchInput->setFocus();

    // Display immediately
    if (m_currentMode == AppMode::LuaPatcher) {
        displayRandomGames();
    } else if (m_currentMode == AppMode::Library) {
        displayLibrary();
    }
    return true;
}

void MainWindow::onSyncDone(QList<GameInfo> games) {
    // Apply name cache to freshly downloaded data
    for (GameInfo& game : games) {
        if (game.name.startsWith("Unknown Game") && m_nameCache.contains(game.id)) {
            game.name = m_nameCache[game.id];
        }
    }
    m_supportedGames = games;
    m_spinner->stop();
    m_stack->setCurrentIndex(1);

    if (m_hasCachedData) {
        // Background refresh done - only re-display if user isn't searching
        m_hasCachedData = false;
        m_statusLabel->setText(QString("Ready • %1 games").arg(m_supportedGames.size()));
        if (m_searchInput->text().trimmed().isEmpty()) {
            // Don't disrupt the user, just update data silently
        }
    } else {
        // First load (no cache was available)
        m_statusLabel->setText("Ready");
        m_searchInput->setFocus();
        if (!m_searchInput->text().isEmpty()) {
            doSearch();
        } else if (m_currentMode == AppMode::LuaPatcher) {
            displayRandomGames();
        } else if (m_currentMode == AppMode::Library) {
            displayLibrary();
        }
    }
}

void MainWindow::onSyncError(QString error) {
    m_spinner->stop();
    m_stack->setCurrentIndex(1);
    m_statusLabel->setText("Offline Mode");
    QMessageBox::warning(this, "Connection Error",
                         QString("Could not sync library:\n%1").arg(error));
}

// ---- Search ----
void MainWindow::onSearchChanged(const QString& text) {
    QString trimmed = text.trimmed();
    
    // UI feedback for home vs search
    if (trimmed.isEmpty()) {
        if (m_leadingTitlesLabel) m_leadingTitlesLabel->show();
        if (m_heroStack) m_heroStack->show();
        if (m_trendingTitle) m_trendingTitle->show();
        if (m_trendingScroll) m_trendingScroll->show();
        
        clearGameCards();
        if (m_currentMode == AppMode::LuaPatcher) {
            displayRandomGames();
        } else if (m_currentMode == AppMode::Library) {
            displayLibrary();
        }
    } else {
        if (m_leadingTitlesLabel) m_leadingTitlesLabel->hide();
        if (m_heroStack) m_heroStack->hide();
        if (m_trendingTitle) m_trendingTitle->hide();
        if (m_trendingScroll) m_trendingScroll->hide();
        m_debounceTimer->stop();
        m_debounceTimer->start(400);
    }
}

void MainWindow::doSearch() {
    QString query = m_searchInput->text().trimmed();
    if (query.isEmpty()) return;
    if (!m_networkManager) return;
    
    cancelNameFetches();
    m_currentSearchId++;
    m_statusLabel->setText("Searching...");
    
    QJsonArray localResults;
    int count = 0;
    for (const auto& game : m_supportedGames) {
        if (count >= 100) break;
        if (m_currentMode == AppMode::Library) {
        }
        
        if (game.name.contains(query, Qt::CaseInsensitive) || game.id == query) {
            QJsonObject item;
            item["id"] = game.id;
            item["name"] = game.name;
            item["supported_local"] = true;
            localResults.append(item);
            count++;
        }
    }
    displayResults(localResults);
    
    m_spinner->start();
    if (m_gameCards.isEmpty()) m_stack->setCurrentIndex(0);
    
    bool isNumeric;
    query.toInt(&isNumeric);
    
    if (isNumeric) {
        QUrl urlStore(QString("https://store.steampowered.com/api/appdetails?appids=%1").arg(query));
        QNetworkRequest reqStore(urlStore);
        QNetworkReply* repStore = m_networkManager->get(reqStore);
        repStore->setProperty("sid", m_currentSearchId);
        repStore->setProperty("type", "steam_details");
        repStore->setProperty("query_id", query);
    } else {
        if (m_activeReply) m_activeReply->abort();
        QUrl url("https://store.steampowered.com/api/storesearch");
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("term", query);
        urlQuery.addQueryItem("l", "english");
        urlQuery.addQueryItem("cc", "US");
        url.setQuery(urlQuery);
        QNetworkRequest request(url);
        m_activeReply = m_networkManager->get(request);
        m_activeReply->setProperty("sid", m_currentSearchId);
        m_activeReply->setProperty("type", "store_search");
    }
}

void MainWindow::onSearchFinished(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply == m_activeReply) m_activeReply = nullptr;
    if (reply->error() == QNetworkReply::OperationCanceledError) return;
    
    QString type = reply->property("type").toString();
    if (type.isEmpty()) return;
    
    int sid = reply->property("sid").toInt();
    if (sid != m_currentSearchId) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        if (m_gameCards.isEmpty() && type == "store_search")
            m_statusLabel->setText("Search failed");
        return;
    }
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    QList<QJsonObject> newItems;
    
    if (type == "store_search") {
        QJsonArray remoteItems = obj["items"].toArray();
        for (const QJsonValue& val : remoteItems)
            newItems.append(val.toObject());
    }
    else if (type == "steam_details") {
        QString qId = reply->property("query_id").toString();
        bool ok = false;
        if (obj.contains(qId)) {
            QJsonObject root = obj[qId].toObject();
            if (root["success"].toBool() && root.contains("data")) {
                QJsonObject d = root["data"].toObject();
                QJsonObject item;
                item["id"] = d["steam_appid"].toInt();
                item["name"] = d["name"].toString();
                newItems.append(item);
                ok = true;
            }
        }
        if (!ok) {
            QUrl urlSpy(QString("https://steamspy.com/api.php?request=appdetails&appid=%1").arg(qId));
            QNetworkReply* repSpy = m_networkManager->get(QNetworkRequest(urlSpy));
            repSpy->setProperty("sid", sid);
            repSpy->setProperty("type", "steamspy_details");
            return;
        }
    }
    else if (type == "steamspy_details") {
        if (obj.contains("name") && !obj["name"].toString().isEmpty()) {
            QJsonObject item;
            item["id"] = obj["appid"].isDouble() ? obj["appid"].toInt() : obj["appid"].toString().toInt();
            item["name"] = obj["name"].toString();
            newItems.append(item);
        }
    }
    
    m_spinner->stop();
    m_stack->setCurrentIndex(1);
    
    QMap<QString, GameCard*> cardMap;
    for (GameCard* c : m_gameCards) cardMap.insert(c->appId(), c);
    
    bool changed = false;
    
    for (const auto& item : newItems) {
        QString id = QString::number(item["id"].toInt());
        QString name = item["name"].toString("Unknown");
        
        bool supported = false;
        bool hasFix = false;
        for (const auto& g : m_supportedGames) {
            if (g.id == id) { supported = true; hasFix = g.hasFix; break; }
        }
        
        if (cardMap.contains(id)) {
            GameCard* existing = cardMap[id];
            QMap<QString, QString> ed = existing->gameData();
            if (ed["name"].contains("Unknown", Qt::CaseInsensitive) || ed["name"] == id) {
                ed["name"] = name;
                ed["supported"] = supported ? "true" : "false";
                ed["hasFix"] = hasFix ? "true" : "false";
                existing->setGameData(ed);
                changed = true;
            }
        } else {
            QMap<QString, QString> cd;
            cd["name"] = name;
            cd["appid"] = id;
            cd["supported"] = supported ? "true" : "false";
            cd["hasFix"] = hasFix ? "true" : "false";
            
            GameCard* card = new GameCard(m_gridLayout->parentWidget());
            card->setGameData(cd);
            connect(card, &GameCard::clicked, this, &MainWindow::onCardClicked);
            
            int idx = m_gameCards.count();
            m_gridLayout->addWidget(card, idx / 6, idx % 6);
            m_gameCards.append(card);
            cardMap.insert(id, card);
            changed = true;
            
            if (m_thumbnailCache.contains(id)) {
                card->setThumbnail(m_thumbnailCache[id]);
            } else if (!m_activeThumbnailDownloads.contains(id)) {
                m_activeThumbnailDownloads.insert(id);
                QString thumbUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/library_600x900_2x.jpg").arg(id);
                QNetworkReply* tr = m_networkManager->get(QNetworkRequest{QUrl(thumbUrl)});
                tr->setProperty("appid", id);
                connect(tr, &QNetworkReply::finished, this, [this, tr]() {
                    onThumbnailDownloaded(tr);
                });
            }
        }
    }
    
    m_statusLabel->setText(m_gameCards.isEmpty()
        ? "No results found"
        : QString("Found %1 results").arg(m_gameCards.count()));
}

// ---- Display results as grid cards ----
void MainWindow::displayResults(const QJsonArray& items) {
    clearGameCards();
    m_selectedGame.clear();
    m_btnAddToLibrary->setEnabled(false);
    cancelNameFetches();
    m_pendingNameFetchIds.clear();

    if (items.isEmpty()) {
        m_statusLabel->setText("No results found.");
        return;
    }

    // Hide Home components
    if (m_leadingTitlesLabel) m_leadingTitlesLabel->hide();
    if (m_heroStack) m_heroStack->hide();
    if (m_trendingTitle) m_trendingTitle->hide();
    if (m_trendingScroll) m_trendingScroll->hide();
    if (m_gridTitleLabel) m_gridTitleLabel->setText(QString("Results (%1)").arg(items.size()));

    int idx = 0;
    for (const QJsonValue& val : items) {
        if (idx >= 120) break;
        QJsonObject item = val.toObject();
        QString name = item["name"].toString("Unknown");
        QString appid = item.contains("id")
            ? (item["id"].isString() ? item["id"].toString() : QString::number(item["id"].toInt()))
            : "0";
        
        bool supported = false;
        bool hasFix = false;
        for (const auto& g : m_supportedGames) {
            if (g.id == appid) { supported = true; hasFix = g.hasFix; break; }
        }
        
        GameCard* card = new GameCard(m_gridLayout->parentWidget());
        card->setGameData({{"name", name}, {"appid", appid}, {"supported", supported ? "true" : "false"}, {"hasFix", hasFix ? "true" : "false"}});
        connect(card, &GameCard::clicked, this, &MainWindow::onCardClicked);
        m_gridLayout->addWidget(card, idx / 6, idx % 6);
        m_gameCards.append(card);
        
        if (m_thumbnailCache.contains(appid)) card->setThumbnail(m_thumbnailCache[appid]);
        if (name.startsWith("Unknown Game") || name == "Unknown") m_pendingNameFetchIds.append(appid);
        idx++;
    }
    
    m_statusLabel->setText(QString("Search: Found %1 matches").arg(items.size()));
    QTimer::singleShot(50, this, &MainWindow::loadVisibleThumbnails);
    if (!m_pendingNameFetchIds.isEmpty()) startBatchNameFetch();
}

// ---- Card clicked ----
void MainWindow::onCardClicked(GameCard* card) {
    if (m_selectedCard) m_selectedCard->setSelected(false);
    
    if (!card) {
        m_selectedCard = nullptr;
        m_selectedGame.clear();
        m_btnAddToLibrary->setEnabled(false);
        m_statusLabel->setText("Ready");
        m_targetGlowColor = Colors::toQColor(Colors::PRIMARY);
        m_glowTimer->start(16);
        return;
    }
    
    m_selectedCard = card;
    card->setSelected(true);
    
    QMap<QString, QString> data = card->gameData();
    m_selectedGame = data;
    bool isSupported = (data["supported"] == "true");
    
    if (m_currentMode == AppMode::LuaPatcher) {
        m_btnAddToLibrary->setEnabled(true);
        if (isSupported) {
            m_btnAddToLibrary->setDescription(QString("Install patch for %1").arg(data["name"]));
            m_btnAddToLibrary->setAccentColor(Colors::ACCENT_GREEN);
        } else {
            m_btnAddToLibrary->setDescription(QString("Generate patch for %1").arg(data["name"]));
            m_btnAddToLibrary->setAccentColor(Colors::PRIMARY);
        }
    } else if (m_currentMode == AppMode::Library) {
        m_btnRemove->setEnabled(true);
        m_btnRemove->setDescription(QString("Remove %1 from Library").arg(data["name"]));
    }
    m_statusLabel->setText(QString("Selected: %1").arg(data["name"]));
    
    m_targetGlowColor = card->getDominantColor().isValid() ? card->getDominantColor() : Colors::toQColor(Colors::PRIMARY);
    m_glowTimer->start(16);
}

// ---- Patch / Generate / Restart / Fix / Remove ----
void MainWindow::doAddGame() {
    if (m_selectedGame.isEmpty()) return;
    bool isSupported = (m_selectedGame["supported"] == "true");
    if (isSupported) runPatchLogic(); else runGenerateLogic();
}

void MainWindow::doRemoveGame() {
    if (m_selectedGame.isEmpty()) return;
    QString appId = m_selectedGame["appid"];
    QString name = m_selectedGame["name"];
    
    if (QMessageBox::question(this, "Remove Patch", 
        QString("Are you sure you want to remove the patch for %1?\nThis will delete the lua file from your Steam plugin folder.").arg(name),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
        
    QStringList pluginDirs = Config::getAllSteamPluginDirs();
    bool deleted = false;
    
    for (const QString& dirPath : pluginDirs) {
        QDir dir(dirPath);
        QString filePath = dir.filePath(appId + ".lua");
        if (QFile::exists(filePath)) {
            if (QFile::remove(filePath)) deleted = true;
        }
    }
    
    if (deleted) {
        m_statusLabel->setText(QString("Removed patch for %1").arg(name));
        displayLibrary();
    } else {
        QMessageBox::warning(this, "Error", "Failed to remove patch file. It may not exist or is in use.");
    }
}

void MainWindow::runPatchLogic() {
    if (m_selectedGame.isEmpty()) return;
    m_btnAddToLibrary->setEnabled(false);
    m_progress->setValue(0);
    m_terminalDialog->clear();
    m_terminalDialog->appendLog(QString("Initializing patch for: %1").arg(m_selectedGame["name"]), "INFO");
    m_terminalDialog->show();
    
    m_dlWorker = new LuaDownloadWorker(m_selectedGame["appid"], this);
    connect(m_dlWorker, &LuaDownloadWorker::finished, this, &MainWindow::onPatchDone);
    connect(m_dlWorker, &LuaDownloadWorker::progress, [this](qint64 dl, qint64 total) {
        if (total > 0) m_progress->setValue(static_cast<int>(dl * 100 / total));
    });
    connect(m_dlWorker, &LuaDownloadWorker::status, [this](QString msg) { m_statusLabel->setText(msg); });
    connect(m_dlWorker, &LuaDownloadWorker::log, m_terminalDialog, &TerminalDialog::appendLog);
    connect(m_dlWorker, &LuaDownloadWorker::error, this, &MainWindow::onPatchError);
    m_dlWorker->start();
}

void MainWindow::onPatchDone(QString path) {
    try {
        m_terminalDialog->appendLog("Patch file downloaded. Installing...", "INFO");
        QStringList targetDirs = Config::getAllSteamPluginDirs();
        if (targetDirs.isEmpty()) {
            targetDirs.append(Config::getSteamPluginDir());
            m_terminalDialog->appendLog("No cached plugin paths found, using default.", "WARN");
        }
        bool ok = false;
        QString lastErr;
        for (const QString& pluginDir : targetDirs) {
            m_terminalDialog->appendLog(QString("checking for stplug folder: %1").arg(pluginDir), "INFO");
            QDir dir(pluginDir);
            if (dir.exists()) {
                m_terminalDialog->appendLog(QString("found stplug in %1").arg(pluginDir), "INFO");
            } else {
                m_terminalDialog->appendLog(QString("creating stplug folder in %1").arg(pluginDir), "INFO");
                if (!dir.mkpath(pluginDir)) {
                    m_terminalDialog->appendLog(QString("Failed to create directory: %1").arg(pluginDir), "ERROR");
                    continue;
                }
            }
            QString dest = dir.filePath(m_selectedGame["appid"] + ".lua");
            if (QFile::exists(dest)) { m_terminalDialog->appendLog("Removing existing patch file...", "INFO"); QFile::remove(dest); }
            m_terminalDialog->appendLog(QString("Copying patch to %1").arg(dest), "INFO");
            if (QFile::copy(path, dest)) { m_terminalDialog->appendLog("Copy successful", "SUCCESS"); ok = true; }
            else { lastErr = "Failed to copy patch file to " + pluginDir; m_terminalDialog->appendLog(lastErr, "ERROR"); }
        }
        if (!ok) throw std::runtime_error(lastErr.toStdString());
        QFile::remove(path);
        m_progress->hide();
        m_btnAddToLibrary->setEnabled(true);
        m_statusLabel->setText("Patch Installed!");
        m_terminalDialog->appendLog("All operations completed successfully.", "SUCCESS");
        m_terminalDialog->setFinished(true);
    } catch (const std::exception& e) {
        onPatchError(QString::fromStdString(e.what()));
    }
}

void MainWindow::onPatchError(QString error) {
    m_progress->hide();
    m_btnAddToLibrary->setEnabled(true);
    m_statusLabel->setText("Error");
    m_terminalDialog->appendLog(QString("Process failed: %1").arg(error), "ERROR");
    m_terminalDialog->setFinished(false);
}

void MainWindow::runGenerateLogic() {
    if (m_selectedGame.isEmpty()) return;
    m_btnAddToLibrary->setEnabled(false);
    m_progress->setValue(0);
    m_terminalDialog->clear();
    m_terminalDialog->appendLog(QString("Initializing generation for: %1 (%2)").arg(m_selectedGame["name"]).arg(m_selectedGame["appid"]), "INFO");
    m_terminalDialog->show();
    
    m_genWorker = new GeneratorWorker(m_selectedGame["appid"], this);
    connect(m_genWorker, &GeneratorWorker::finished, this, [this](QString) {
        m_progress->hide();
        m_btnAddToLibrary->setEnabled(true);
        m_statusLabel->setText("Patch Generated & Installed!");
        m_terminalDialog->setFinished(true);
        QString appId = m_selectedGame["appid"];
        for (GameCard* card : m_gameCards) {
            if (card->appId() == appId) {
                QMap<QString, QString> d = card->gameData();
                d["supported"] = "true";
                card->setGameData(d);
                break;
            }
        }
        m_btnAddToLibrary->setDescription(QString("Re-patch %1").arg(m_selectedGame["name"]));
        m_btnAddToLibrary->setAccentColor(Colors::ACCENT_GREEN);
    });
    connect(m_genWorker, &GeneratorWorker::progress, [this](qint64 dl, qint64 total) {
        if (total > 0) m_progress->setValue(static_cast<int>(dl * 100 / total));
    });
    connect(m_genWorker, &GeneratorWorker::status, [this](QString msg) { m_statusLabel->setText(msg); });
    connect(m_genWorker, &GeneratorWorker::log, m_terminalDialog, &TerminalDialog::appendLog);
    connect(m_genWorker, &GeneratorWorker::error, this, &MainWindow::onPatchError);
    m_genWorker->start();
}

void MainWindow::doRestart() {
    if (QMessageBox::question(this, "Restart Steam?", "Close Steam and all games?",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    m_restartWorker = new RestartWorker(this);
    connect(m_restartWorker, &RestartWorker::finished, m_statusLabel, &QLabel::setText);
    m_restartWorker->start();
}

// ---- Mode switching ----
void MainWindow::cancelNameFetches() {
    m_fetchingNames = false;
    for (QNetworkReply* r : m_activeNameFetches) { if (r) { r->abort(); r->deleteLater(); } }
    m_activeNameFetches.clear();
    m_pendingNameFetchIds.clear();
}

void MainWindow::switchMode(AppMode mode) {
    if (m_currentMode == mode) return;
    m_currentMode = mode;
    updateModeUI();
    
    onCardClicked(nullptr);
    clearGameCards();
    
    if (m_currentMode == AppMode::LuaPatcher) {
        if (m_searchInput->text().trimmed().isEmpty()) {
            displayRandomGames();
        } else {
            doSearch();
        }
    } else if (m_currentMode == AppMode::Library) {
        displayLibrary();
    }
}

void MainWindow::updateModeUI() {
    m_tabLua->setAccentColor(m_currentMode == AppMode::LuaPatcher ? Colors::PRIMARY : "transparent");
    m_tabLibrary->setAccentColor(m_currentMode == AppMode::Library ? Colors::ACCENT_GREEN : "transparent");
    m_tabSettings->setAccentColor(m_currentMode == AppMode::Settings ? Colors::PRIMARY : "transparent");
    m_tabDiscord->setAccentColor("transparent");

    m_btnAddToLibrary->hide();
    m_btnRemove->hide();
    
    if (m_currentMode == AppMode::LuaPatcher) {
        m_btnAddToLibrary->show();
    } else if (m_currentMode == AppMode::Library) {
        m_btnRemove->show();
    }
    
    if (m_currentMode == AppMode::Settings) {
        m_stack->setCurrentIndex(2);
    } else {
        m_stack->setCurrentIndex(1);
    }
}

// ---- Batch name fetch ----
void MainWindow::startBatchNameFetch() {
    if (m_pendingNameFetchIds.isEmpty()) { m_fetchingNames = false; m_spinner->stop(); return; }
    m_fetchingNames = true;
    m_nameFetchSearchId = m_currentSearchId;
    m_spinner->start();
    m_statusLabel->setText(QString("Found %1 results %2 Fetching game names...").arg(m_gameCards.count()).arg(QChar(0x2022)));
    // Fetch all displayed cards concurrently for faster loading
    for (int i = 0; i < 12 && !m_pendingNameFetchIds.isEmpty(); ++i) processNextNameFetch();
}

void MainWindow::processNextNameFetch() {
    if (m_pendingNameFetchIds.isEmpty() || !m_fetchingNames) {
        if (m_activeNameFetches.isEmpty() && m_fetchingNames) {
            m_fetchingNames = false; m_spinner->stop();
            m_statusLabel->setText(QString("Found %1 results").arg(m_gameCards.count()));
        }
        return;
    }
    QString appId = m_pendingNameFetchIds.takeFirst();
    QUrl url(QString("https://store.steampowered.com/api/appdetails?appids=%1").arg(appId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
    QNetworkReply* reply = m_networkManager->get(request);
    reply->setProperty("fetch_appid", appId);
    reply->setProperty("fetch_type", "steam_store");
    reply->setProperty("fetch_sid", m_nameFetchSearchId);
    m_activeNameFetches.append(reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onGameNameFetched(reply); });
}

void MainWindow::onGameNameFetched(QNetworkReply* reply) {
    reply->deleteLater();
    m_activeNameFetches.removeOne(reply);
    int fetchSid = reply->property("fetch_sid").toInt();
    if (fetchSid != m_nameFetchSearchId || !m_fetchingNames) { processNextNameFetch(); return; }
    
    QString appId = reply->property("fetch_appid").toString();
    QString fetchType = reply->property("fetch_type").toString();
    QString gameName;
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        if (fetchType == "steam_store") {
            if (obj.contains(appId)) {
                QJsonObject root = obj[appId].toObject();
                if (root["success"].toBool() && root.contains("data"))
                    gameName = root["data"].toObject()["name"].toString();
            }
        } else if (fetchType == "steamspy") {
            if (obj.contains("name") && !obj["name"].toString().isEmpty())
                gameName = obj["name"].toString();
        }
    }
    
    if (gameName.isEmpty() && fetchType == "steam_store") {
        QUrl spyUrl(QString("https://steamspy.com/api.php?request=appdetails&appid=%1").arg(appId));
        QNetworkRequest req(spyUrl);
        req.setHeader(QNetworkRequest::UserAgentHeader, "SteamLuaPatcher/2.0");
        QNetworkReply* spyReply = m_networkManager->get(req);
        spyReply->setProperty("fetch_appid", appId);
        spyReply->setProperty("fetch_type", "steamspy");
        spyReply->setProperty("fetch_sid", m_nameFetchSearchId);
        m_activeNameFetches.append(spyReply);
        connect(spyReply, &QNetworkReply::finished, this, [this, spyReply]() { onGameNameFetched(spyReply); });
        return;
    }
    
    if (!gameName.isEmpty()) {
        // Persist to name cache so we never fetch this again
        m_nameCache[appId] = gameName;
        saveNameCache();

        for (GameCard* card : m_gameCards) {
            if (card->appId() == appId) {
                QMap<QString, QString> d = card->gameData();
                d["name"] = gameName;
                card->setGameData(d);
                break;
            }
        }
    }
    processNextNameFetch();
}

// ---- Thumbnail lazy loading ----
void MainWindow::loadVisibleThumbnails() {
    if (!m_mainScrollArea || !m_networkManager) return;
    QRect visibleRect = m_mainScrollArea->viewport()->rect();
    
    for (GameCard* card : m_gameCards) {
        // Map card position correctly to scroll viewport
        QPoint pos = card->mapTo(m_mainScrollArea->viewport(), QPoint(0,0));
        QRect cardInView(pos, card->size());
        
        // Add some buffer for smoother loading
        if (!visibleRect.adjusted(-200, -200, 200, 200).intersects(cardInView)) continue;
        
        QString appId = card->appId();
        if (appId.isEmpty() || card->hasThumbnail()) continue;
        if (m_thumbnailCache.contains(appId)) { card->setThumbnail(m_thumbnailCache[appId]); continue; }
        if (m_activeThumbnailDownloads.contains(appId)) continue;
        
        m_activeThumbnailDownloads.insert(appId);
        QString thumbUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/library_600x900_2x.jpg").arg(appId);
        QNetworkRequest req{QUrl(thumbUrl)};
        req.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1), appId);
        
        QNetworkReply* tr = m_networkManager->get(req);
        tr->setProperty("appid", appId);
        connect(tr, &QNetworkReply::finished, this, [this, tr]() { onThumbnailDownloaded(tr); });
    }
}

void MainWindow::onThumbnailDownloaded(QNetworkReply* reply) {
    reply->deleteLater();
    QString appId = reply->property("appid").toString();
    m_activeThumbnailDownloads.remove(appId);
    
    if (appId.isEmpty()) return;
    
    QPixmap pixmap;
    bool success = (reply->error() == QNetworkReply::NoError) && pixmap.loadFromData(reply->readAll());
    
    // Fallback logic for older games that don't have the new vertical Steam library asset
    if (!success) {
        QString originalUrl = reply->url().toString();
        if (originalUrl.endsWith("library_600x900_2x.jpg")) {
            m_activeThumbnailDownloads.insert(appId);
            QString fallbackUrl = QString("https://cdn.akamai.steamstatic.com/steam/apps/%1/header.jpg").arg(appId);
            QNetworkRequest req{QUrl(fallbackUrl)};
            QNetworkReply* tr = m_networkManager->get(req);
            tr->setProperty("appid", appId);
            connect(tr, &QNetworkReply::finished, this, [this, tr]() { onThumbnailDownloaded(tr); });
            return;
        }
        
        // If even fallback fails, aggressively cache failure as a null pixmap
        // to prevent spamming the steam servers on every frame scroll
        m_thumbnailCache[appId] = QPixmap();
        return;
    }
    
    // Save successful fetch to cache and aggressively update matching UI cards
    m_thumbnailCache[appId] = pixmap;
    for (GameCard* card : m_gameCards) {
        if (card->appId() == appId) {
            card->setThumbnail(pixmap);
            // Don't break, multiple modes might show the same appID (e.g. search + library)
        }
    }
}

// ---- Persistent name cache ----
void MainWindow::loadNameCache() {
    QString path = QDir(Paths::getLocalCacheDir()).filePath("name_cache.json");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_nameCache[it.key()] = it.value().toString();
    }
}

void MainWindow::saveNameCache() {
    QString path = QDir(Paths::getLocalCacheDir()).filePath("name_cache.json");
    QJsonObject obj;
    for (auto it = m_nameCache.begin(); it != m_nameCache.end(); ++it) {
        obj[it.key()] = it.value();
    }
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        file.close();
    }
}
