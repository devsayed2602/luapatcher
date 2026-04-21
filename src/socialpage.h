#ifndef SOCIALPAGE_H
#define SOCIALPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>

class FriendCard : public QWidget {
    Q_OBJECT
public:
    explicit FriendCard(const QJsonObject& data, bool isRequest, QWidget* parent = nullptr);
signals:
    void acceptClicked(const QString& username);
    void addClicked(const QString& username);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QJsonObject m_data;
    bool m_isRequest;
};

class SocialPage : public QWidget {
    Q_OBJECT
public:
    explicit SocialPage(const QString& myUsername, bool isGuest, QNetworkAccessManager* networkManager, QWidget* parent = nullptr);

    void setUserData(const QString& username, bool isGuest);
    void refresh();

private slots:
    void onSearchChanged(const QString& text);
    void doSearch();
    void onSearchFinished();
    void onFriendsFetched();
    void onPendingFetched();
    void onActionFinished();

    void sendFriendRequest(const QString& targetUser);
    void acceptFriendRequest(const QString& friendUser);

private:
    void setupUI();
    void clearLayout(QLayout* layout);

    QString m_myUsername;
    bool m_isGuest;
    QNetworkAccessManager* m_networkManager;
    
    QLineEdit* m_searchEdit;
    QTimer* m_searchTimer;
    
    QWidget* m_searchResultsWidget;
    QVBoxLayout* m_searchResultsLayout;
    
    QWidget* m_friendsWidget;
    QVBoxLayout* m_friendsLayout;
    
    QWidget* m_pendingWidget;
    QVBoxLayout* m_pendingLayout;

    QLabel* m_friendsTitle;
    QLabel* m_pendingTitle;
};

#endif // SOCIALPAGE_H
