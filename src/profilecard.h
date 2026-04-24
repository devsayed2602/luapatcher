#ifndef PROFILECARD_H
#define PROFILECARD_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QJsonObject>

#include <QNetworkAccessManager>
#include <QPushButton>

class ProfileCard : public QDialog {
    Q_OBJECT
public:
    explicit ProfileCard(const QString& username, const QJsonObject& userData, QNetworkAccessManager* netMgr, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

signals:
    void avatarUpdated(const QString& newAvatarUrl);

private slots:
    void onChangeAvatar();

private:
    void setupUI();
    
    QString m_username;
    QJsonObject m_userData;
    QNetworkAccessManager* m_netMgr;
    QWidget* m_container;
    QLabel* m_avatarLabel;
};

#endif // PROFILECARD_H
