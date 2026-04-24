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

class NotificationDialog : public QDialog {
    Q_OBJECT
public:
    explicit NotificationDialog(const QString& currentUsername, QNetworkAccessManager* netMgr, QWidget* parent = nullptr);

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
    QWidget* createRequestCard(const QString& username);
    void acceptRequest(const QString& username, QWidget* card);
    void rejectRequest(const QString& username, QWidget* card);
    void updateCountLabel();

    QString m_currentUsername;
    QNetworkAccessManager* m_netMgr;
    
    QWidget* m_container;
    QScrollArea* m_scrollArea;
    QVBoxLayout* m_requestsLayout;
    QLabel* m_countLabel;
    QLabel* m_emptyLabel;
    QPushButton* m_closeBtn;
    int m_pendingCount = 0;
};

#endif // NOTIFICATIONDIALOG_H
