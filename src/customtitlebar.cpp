#include "customtitlebar.h"
#include "utils/colors.h"
#include "utils/paths.h"
#include <QApplication>
#include <QPainter>
#include <QFile>

CustomTitleBar::CustomTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(40);
    setObjectName("titleBar");
    setStyleSheet(
        "QWidget#titleBar { background: transparent; }"
        "QPushButton { border: none; background: transparent; color: #8FABD4; font-size: 16px; font-family: 'Segoe UI Symbol'; }"
        "QPushButton:hover { background: rgba(255, 255, 255, 0.1); }"
        "QPushButton#closeBtn:hover { background: #E81123; color: white; }"
    );

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 0, 0, 0);
    layout->setSpacing(0);

    // App Icon & Title
    m_appIconLabel = new QLabel();
    m_appIconLabel->setFixedSize(20, 20);
    QString iconPath = Paths::getResourcePath("logo.ico");
    if (QFile::exists(iconPath)) {
        QPixmap logoPixmap(iconPath);
        m_appIconLabel->setPixmap(logoPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    layout->addWidget(m_appIconLabel);
    layout->addSpacing(10);

    m_titleLabel = new QLabel("Lua Patcher");
    m_titleLabel->setStyleSheet("color: #8FABD4; font-size: 13px; font-weight: 600; font-family: 'Segoe UI';");
    layout->addWidget(m_titleLabel);

    layout->addStretch();

    // Notification Bell Button
    m_notifBtn = new QPushButton("🔔");
    m_notifBtn->setObjectName("notifBtn");
    m_notifBtn->setFixedSize(46, 40);
    m_notifBtn->setToolTip("Notifications");
    m_notifBtn->setStyleSheet(
        "QPushButton#notifBtn { font-size: 15px; }"
    );
    connect(m_notifBtn, &QPushButton::clicked, this, &CustomTitleBar::notificationRequested);

    // Red badge overlay on the bell button
    m_notifBadge = new QLabel(m_notifBtn);
    m_notifBadge->setFixedSize(18, 18);
    m_notifBadge->setAlignment(Qt::AlignCenter);
    m_notifBadge->move(28, 4);
    m_notifBadge->setStyleSheet(
        "background: #E74C3C;"
        "color: white;"
        "font-size: 9px;"
        "font-weight: bold;"
        "font-family: 'Segoe UI';"
        "border-radius: 9px;"
        "border: none;"
    );
    m_notifBadge->hide();

    layout->addWidget(m_notifBtn);

    // System Buttons
    m_minBtn = new QPushButton("\u2014"); // Em Dash
    m_minBtn->setObjectName("minBtn");
    m_minBtn->setFixedSize(46, 40);
    m_minBtn->setToolTip("Minimize");
    connect(m_minBtn, &QPushButton::clicked, this, &CustomTitleBar::minimizeRequested);

    m_maxBtn = new QPushButton("\u25FB"); // White Medium Square
    m_maxBtn->setObjectName("maxBtn");
    m_maxBtn->setFixedSize(46, 40);
    m_maxBtn->setToolTip("Maximize");
    connect(m_maxBtn, &QPushButton::clicked, this, &CustomTitleBar::maximizeRequested);

    m_closeBtn = new QPushButton("\u2715"); // Multiplication X
    m_closeBtn->setObjectName("closeBtn");
    m_closeBtn->setFixedSize(46, 40);
    m_closeBtn->setToolTip("Close");
    connect(m_closeBtn, &QPushButton::clicked, this, &CustomTitleBar::closeRequested);

    layout->addWidget(m_minBtn);
    layout->addWidget(m_maxBtn);
    layout->addWidget(m_closeBtn);
}

void CustomTitleBar::setTitle(const QString& title) {
    m_titleLabel->setText(title);
}

void CustomTitleBar::updateBadge(int count) {
    if (count > 0) {
        m_notifBadge->setText(count > 9 ? "9+" : QString::number(count));
        m_notifBadge->show();
    } else {
        m_notifBadge->hide();
    }
}

void CustomTitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // We handle movement in MainWindow's WM_NCHITTEST
        event->ignore();
    }
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit maximizeRequested();
    }
}
