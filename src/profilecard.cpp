#include "profilecard.h"
#include "utils/colors.h"
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

ProfileCard::ProfileCard(const QString& username, const QJsonObject& userData, QWidget* parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Popup), 
      m_username(username), m_userData(userData)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(320, 450);
    
    setupUI();
}

void ProfileCard::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    m_container = new QWidget();
    m_container->setObjectName("profileContainer");
    m_container->setStyleSheet(
        "QWidget#profileContainer { "
        "  background: #0D1622; "
        "  border-radius: 30px; "
        "  border: 1px solid rgba(143, 171, 212, 0.15); "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(30, 40, 30, 30);
    layout->setSpacing(0);
    
    // Avatar Section
    int avSize = 140;
    QWidget* avatarWrapper = new QWidget();
    avatarWrapper->setFixedSize(avSize + 10, avSize + 10);
    avatarWrapper->setStyleSheet("background: transparent;");
    
    QLabel* avatarLabel = new QLabel(avatarWrapper);
    avatarLabel->setFixedSize(avSize, avSize);
    avatarLabel->move(5, 5);
    
    // Draw the avatar with the thick white border and star badge
    QPixmap avatarPix(avSize, avSize);
    avatarPix.fill(Qt::transparent);
    QPainter p(&avatarPix);
    p.setRenderHint(QPainter::Antialiasing);
    
    // Thick white ring
    p.setPen(QPen(QColor("#EFECE3"), 4));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(3, 3, avSize - 6, avSize - 6);
    
    // Avatar Image (Letter fallback)
    QString avData = m_userData["avatar_url"].toString();
    if (!avData.isEmpty()) {
        QPixmap original;
        original.loadFromData(QByteArray::fromBase64(avData.toUtf8()));
        if (!original.isNull()) {
            QPainterPath path;
            path.addEllipse(8, 8, avSize - 16, avSize - 16);
            p.setClipPath(path);
            p.drawPixmap(8, 8, avSize - 16, avSize - 16, original.scaled(avSize, avSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            p.setClipping(false);
        }
    } else {
        p.setBrush(QColor("#4A6FA5"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(8, 8, avSize - 16, avSize - 16);
        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 48, QFont::Bold));
        p.drawText(QRect(8, 8, avSize - 16, avSize - 16), Qt::AlignCenter, m_username.left(1).toUpper());
    }
    p.end();
    avatarLabel->setPixmap(avatarPix);
    
    // Star Badge (Black circle with white star)
    QLabel* badge = new QLabel(avatarWrapper);
    badge->setFixedSize(36, 36);
    badge->move(avSize - 30, avSize - 30);
    QPixmap badgePix(36, 36);
    badgePix.fill(Qt::transparent);
    QPainter bp(&badgePix);
    bp.setRenderHint(QPainter::Antialiasing);
    bp.setBrush(Qt::black);
    bp.setPen(QPen(QColor("#EFECE3"), 2));
    bp.drawEllipse(1, 1, 34, 34);
    bp.setBrush(QColor("#EFECE3"));
    bp.setPen(Qt::NoPen);
    QPolygonF star;
    star << QPointF(18, 8) << QPointF(21, 15) << QPointF(28, 15) 
         << QPointF(22, 19) << QPointF(24, 26) << QPointF(18, 22) 
         << QPointF(12, 26) << QPointF(14, 19) << QPointF(8, 15) << QPointF(15, 15);
    bp.drawPolygon(star);
    bp.end();
    badge->setPixmap(badgePix);
    
    layout->addWidget(avatarWrapper, 0, Qt::AlignCenter);
    layout->addSpacing(30);
    
    // Username
    QLabel* nameLabel = new QLabel(m_username);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("color: white; font-size: 28px; font-weight: 800; font-family: 'Segoe UI'; background: transparent;");
    layout->addWidget(nameLabel);
    
    // Badge Label
    QLabel* subLabel = new QLabel("PREMIUM MEMBER");
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet("color: #8FABD4; font-size: 13px; font-weight: 900; letter-spacing: 2px; margin-top: 5px; background: transparent;");
    layout->addWidget(subLabel);
    
    layout->addSpacing(40);
    
    // Separator line
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet("background: rgba(255, 255, 255, 0.05); border: none; max-height: 1px;");
    layout->addWidget(line);
    
    layout->addSpacing(30);
    
    // Stats Row
    QHBoxLayout* statsRow = new QHBoxLayout();
    
    auto createStat = [&](const QString& val, const QString& label) {
        QVBoxLayout* v = new QVBoxLayout();
        v->setSpacing(5);
        QLabel* vLbl = new QLabel(val);
        vLbl->setAlignment(Qt::AlignCenter);
        vLbl->setStyleSheet("color: white; font-size: 22px; font-weight: 800; background: transparent;");
        QLabel* lLbl = new QLabel(label);
        lLbl->setAlignment(Qt::AlignCenter);
        lLbl->setStyleSheet("color: #8FABD4; font-size: 10px; font-weight: bold; letter-spacing: 1px; background: transparent;");
        v->addWidget(vLbl);
        v->addWidget(lLbl);
        return v;
    };
    
    int gamesPatched = m_userData["games_patched"].toInt(0);
    statsRow->addLayout(createStat(QString::number(gamesPatched), "GAMES ADDED"));
    statsRow->addLayout(createStat("—", "FRIENDS"));
    
    layout->addLayout(statsRow);
    
    mainLayout->addWidget(m_container);
}

void ProfileCard::paintEvent(QPaintEvent*) {
    // Transparent background handled by WA_TranslucentBackground
}

void ProfileCard::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
}
