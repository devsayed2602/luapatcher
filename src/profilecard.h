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

class ProfileCard : public QDialog {
    Q_OBJECT
public:
    explicit ProfileCard(const QString& username, const QJsonObject& userData, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    
    QString m_username;
    QJsonObject m_userData;
    QWidget* m_container;
};

#endif // PROFILECARD_H
