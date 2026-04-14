#ifndef GAMEDETAILSPAGE_H
#define GAMEDETAILSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QPushButton>
#include <QProgressBar>
#include <QMap>

// Forward declaration
class QPushButton;

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
    QWidget* createScreenshotCard(const QPixmap& pixmap);

    QNetworkAccessManager* m_networkManager;

    // Current game
    QString m_appId;
    QString m_gameName;
    bool m_supported = false;
    bool m_hasFix = false;

    // UI Elements
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;

    // Hero
    QLabel* m_heroBanner;

    // Info row
    QLabel* m_gameTitleLabel;
    QLabel* m_descriptionLabel;
    QPushButton* m_installButton;
    QProgressBar* m_installProgressBar;
    bool m_isDownloading = false;

    // Screenshots
    QHBoxLayout* m_screenshotLayout;

    // Features & Security
    QVBoxLayout* m_featuresLayout;
    QVBoxLayout* m_securityLayout;
};

#endif // GAMEDETAILSPAGE_H
