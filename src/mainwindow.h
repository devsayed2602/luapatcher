#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QString>
#include <QMap>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScrollArea>
#include <QGridLayout>
#include <QSet>

class GlassButton;
class GameCard;
#include "utils/gameinfo.h"
#include "terminaldialog.h"

class GameDetailsPage;
class CustomTitleBar;

class LoadingSpinner;
class IndexDownloadWorker;

class LuaDownloadWorker;
class RestartWorker;
class GeneratorWorker;

class HeroBannerWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal imageScale READ imageScale WRITE setImageScale)
public:
    explicit HeroBannerWidget(QWidget* parent = nullptr);
    void setPixmap(const QPixmap& p);
    qreal imageScale() const { return m_imageScale; }
    void setImageScale(qreal s) { m_imageScale = s; update(); }
protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
private:
    QPixmap m_pixmap;
    qreal m_imageScale = 1.05;
    QPropertyAnimation* m_scaleAnim;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    enum class AppMode {
        LuaPatcher,
        Library,
        Settings,
        Discord
    };

    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onSyncDone(QList<GameInfo> games);
    void onSyncError(QString error);
    void onSearchChanged(const QString& text);
    void doSearch();
    void onSearchFinished(QNetworkReply* reply);
    void onGameNameFetched(QNetworkReply* reply);
    void onThumbnailDownloaded(QNetworkReply* reply);
    void onCardClicked(GameCard* card);
    void onSelectionChanged(bool selected, GameCard* card);
    void onRemoveSelectedClicked();
    void onClearLibraryClicked();
    void doAddGame();
    void runPatchLogic();
    void runGenerateLogic();
    void onPatchDone(QString path);
    void onPatchError(QString error);
    void doRestart();
    void doRemoveGame();
    void switchMode(AppMode mode);
    void updateModeUI();
    void processNextNameFetch();
    void loadVisibleThumbnails();

    // Game Details
    void onGameDetailsBack();
    void onInstallFromDetails(const QString& appId, const QString& name, bool hasFix);

    // Sidebar
    void expandSidebar();
    void collapseSidebarDelayed();

    // Window Management
    void onTitleBarMinimize();
    void onTitleBarMaximize();
    void onTitleBarClose();

private:
    void initUI();
    void startSync();
    void displayResults(const QJsonArray& items);
    void startBatchNameFetch();
    void cancelNameFetches();
    void clearGameCards();
    void displayRandomGames();
    void displayLibrary();
    void fetchTrendingGames();
    void onTrendingFetched(QNetworkReply* reply);
    void scrollCarousel();
    void loadNameCache();
    void saveNameCache();
    bool loadCachedIndex();
    void updateAmbientGlow();
    void enableAcrylicBlur();
    void updateHeroIndicators();
    void jumpToHeroSlide(int index);

    // UI Components
    QString m_username;
    QLabel* m_statusLabel;
    QLineEdit* m_searchInput;
    QStackedWidget* m_stack;
    LoadingSpinner* m_spinner;
    QProgressBar* m_progress;
    
    // Main Content
    CustomTitleBar* m_titleBar;
    QWidget* m_mainScrollContainer;
    QVBoxLayout* m_mainScrollLayout;
    QScrollArea* m_mainScrollArea;
    QLabel* m_leadingTitlesLabel;
    QStackedWidget* m_heroStack;
    
    QTimer* m_heroCarouselTimer;
    QList<GameInfo> m_heroGames;
    int m_currentHeroIndex = 0;
    QWidget* m_heroIndicators = nullptr;
    QHBoxLayout* m_heroIndicatorLayout = nullptr;
    
    // Trending (hidden row, cards go into hero banner now)
    QLabel* m_trendingTitle;
    QScrollArea* m_trendingScroll;
    QHBoxLayout* m_trendingLayout;
    QLabel* m_gridTitleLabel;
    QWidget* m_gridHeaderWidget;
    QPushButton* m_removeSelectedBtn;
    QPushButton* m_clearLibraryBtn;
    QWidget* m_gridContainer;
    QGridLayout* m_gridLayout;
    QList<GameCard*> m_gameCards;
    GameCard* m_selectedCard = nullptr;
    
    // Top Bar
    QWidget* m_topProfileWidget;
    QLabel* m_topUsernameLabel;
    
    // Header & Tabs
    GlassButton* m_tabLua;
    GlassButton* m_tabLibrary;
    GlassButton* m_tabSettings;
    GlassButton* m_tabDiscord;
    AppMode m_currentMode;

    GlassButton* m_btnRestart;
    TerminalDialog* m_terminalDialog;

    // Details Page
    GameDetailsPage* m_gameDetailsPage;

    // Sidebar
    QWidget* m_sidebarWidget;
    QWidget* m_rightPanelWidget;
    bool m_sidebarExpanded = false;
    QPropertyAnimation* m_sidebarAnimation;
    QTimer* m_sidebarCollapseTimer;
    
    // Animated sidebar indicator bar
    QWidget* m_sidebarIndicator = nullptr;
    QPropertyAnimation* m_indicatorAnimation = nullptr;
    
    QLabel* m_appTitleLabel;
    QLabel* m_navTitleLabel;
    QLabel* m_infoTitleLabel;

    // Data
    QList<GameInfo> m_supportedGames;
    QMap<QString, QString> m_selectedGame;
    
    // Network
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_activeReply;
    
    // Search debounce
    QTimer* m_debounceTimer;
    int m_currentSearchId;
    
    // Workers
    IndexDownloadWorker* m_syncWorker;
    LuaDownloadWorker* m_dlWorker;
    GeneratorWorker* m_genWorker;
    RestartWorker* m_restartWorker;
    
    // Batch name fetching
    QStringList m_pendingNameFetchIds;
    QList<QNetworkReply*> m_activeNameFetches;
    bool m_fetchingNames;
    int m_nameFetchSearchId;
    // Persistent name cache (appId -> gameName)
    QMap<QString, QString> m_nameCache;
    bool m_hasCachedData;  // true if we loaded from cache on startup
    // Thumbnail cache
    QMap<QString, QPixmap> m_thumbnailCache;
    QSet<QString> m_activeThumbnailDownloads;
    
    // Trending games (from SteamSpy)
    QStringList m_trendingAppIds;
    
    // Ambient Glow
    QTimer* m_glowTimer;
    QColor m_currentGlowColor;
    QColor m_targetGlowColor;
};

#endif // MAINWINDOW_H
