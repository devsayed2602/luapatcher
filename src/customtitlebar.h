#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>

class CustomTitleBar : public QWidget {
    Q_OBJECT
public:
    explicit CustomTitleBar(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void updateBadge(int count);

signals:
    void minimizeRequested();
    void maximizeRequested();
    void closeRequested();
    void notificationRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QLabel* m_appIconLabel;
    QLabel* m_titleLabel;
    QPushButton* m_notifBtn;
    QLabel* m_notifBadge;
    QPushButton* m_minBtn;
    QPushButton* m_maxBtn;
    QPushButton* m_closeBtn;
};

#endif // CUSTOMTITLEBAR_H
