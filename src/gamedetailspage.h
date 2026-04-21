#ifndef GAMEDETAILSPAGE_H
#define GAMEDETAILSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QPushButton>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QMap>

class GameDetailsPage : public QWidget {
    Q_OBJECT

public:
    explicit GameDetailsPage(QNetworkAccessManager* networkManager, QWidget* parent = nullptr);

    void loadGame(const QString& appId, const QString& name, bool supported, bool hasFix);
    void clear();

signals:
    void backClicked();
    void addToLibraryClicked(const QString& appId, const QString& name, bool hasFix);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onDetailsReceived(QNetworkReply* reply);

public slots:
    void updateInstallProgress(int pct);
    void installFinished();
    void installError(const QString& err);

private:
    void buildUI();
    void showSkeleton();
    void populate(const QJsonObject& data);
    void playEntranceAnimations();

    QNetworkAccessManager* m_networkManager;

    // Current game
    QString m_appId;
    QString m_gameName;
    bool m_supported = false;
    bool m_hasFix = false;

    // Load guard
    int m_currentLoadId = 0;

    // Main scroll
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_contentWidget = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;

    // Hero banner (custom painted widget, defined in .cpp)
    QWidget* m_heroBanner = nullptr;

    // Overlay widgets (children of hero banner)
    QPushButton* m_backButton = nullptr;
    QWidget* m_overlayContainer = nullptr;   // Bottom overlay with title/tags/buttons
    QWidget* m_tagContainer = nullptr;
    QHBoxLayout* m_tagLayout = nullptr;
    QLabel* m_heroTitleLabel = nullptr;
    QLabel* m_heroSubtitleLabel = nullptr;
    QPushButton* m_installButton = nullptr;
    QPushButton* m_wishlistButton = nullptr;
    QProgressBar* m_installProgressBar = nullptr;
    bool m_isDownloading = false;

    // Below-banner content
    QWidget* m_infoRow = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QWidget* m_specsCard = nullptr;
    QVBoxLayout* m_specsLayout = nullptr;

    // Screenshots
    QWidget* m_screenshotSection = nullptr;
    QHBoxLayout* m_screenshotLayout = nullptr;

    // Features & Security
    QWidget* m_detailsRow = nullptr;
    QVBoxLayout* m_featuresLayout = nullptr;
    QGridLayout* m_featuresGrid = nullptr;
    QVBoxLayout* m_securityLayout = nullptr;
};

#endif // GAMEDETAILSPAGE_H
