#ifndef NOTIFICATIONDIALOG_H
#define NOTIFICATIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>

class NotificationDialog : public QDialog {
    Q_OBJECT
public:
    explicit NotificationDialog(const QString& currentUsername, QNetworkAccessManager* netMgr, 
                                bool hasUpdate = false, const QString& updateVersion = "", 
                                const QString& updateMessage = "", const QString& updateUrl = "", 
                                QWidget* parent = nullptr);

signals:
    void requestHandled(); // Emitted when a request is accepted/rejected so MainWindow can refresh badge + friends list

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onClose();

private:
    void setupUI();
    void fetchPendingRequests();
    QWidget* createUpdateCard();
    QWidget* createRequestCard(const QString& username);
    void acceptRequest(const QString& username, QPointer<QWidget> card);
    void rejectRequest(const QString& username, QPointer<QWidget> card);
    void updateCountLabel();

    QString m_currentUsername;
    QNetworkAccessManager* m_netMgr;
    
    bool m_hasUpdate;
    QString m_updateVersion;
    QString m_updateMessage;
    QString m_updateUrl;
    
    QWidget* m_container;
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_requestsLayout;
    QLabel* m_countLabel;
    QLabel* m_emptyLabel;
    QPushButton* m_closeBtn;
    int m_pendingCount = 0;
};

#endif // NOTIFICATIONDIALOG_H
