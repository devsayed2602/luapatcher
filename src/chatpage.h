#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

class ChatPage : public QWidget {
    Q_OBJECT
public:
    explicit ChatPage(const QString& myUsername, const QString& friendUsername, QNetworkAccessManager* netMgr, QWidget* parent = nullptr);
    ~ChatPage();

signals:
    void backRequested();

private slots:
    void sendMessage();
    void fetchHistory();
    void onHistoryFetched();

private:
    void setupUI();
    void addMessageBubble(const QString& text, bool isMe, const QString& time);
    void clearLayout(QLayout* layout);

    QString m_myUsername;
    QString m_friendUsername;
    QNetworkAccessManager* m_netMgr;
    QTimer* m_pollTimer;

    QVBoxLayout* m_chatLayout;
    QScrollArea* m_scrollArea;
    QLineEdit* m_messageInput;
    QLabel* m_friendNameLabel;
    QLabel* m_statusLabel;
    
    QString m_lastMessageId;
};

#endif // CHATPAGE_H
