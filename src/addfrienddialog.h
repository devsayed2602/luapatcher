#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QNetworkAccessManager>

#include <QListWidget>

class AddFriendDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddFriendDialog(const QString& currentUsername, QNetworkAccessManager* netMgr, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void sendInvitation();
    void onClose();
    void onSearchTextChanged(const QString& text);
    void onSearchFinished(QNetworkReply* reply);
    void onSearchResultClicked(QListWidgetItem* item);

private:
    void setupUI();
    
    QString m_currentUsername;
    QNetworkAccessManager* m_netMgr;
    QNetworkReply* m_searchReply = nullptr;
    
    QLineEdit* m_searchInput;
    QPushButton* m_sendBtn;
    QPushButton* m_closeBtn;
    QLabel* m_statusLabel;
    QListWidget* m_searchResults;
    
    QWidget* m_container;
};

#endif // ADDFRIENDDIALOG_H
