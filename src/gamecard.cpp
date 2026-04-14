#include "gamecard.h"
#include "utils/colors.h"
#include "materialicons.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>

GameCard::GameCard(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(190, 285);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void GameCard::setGameData(const QMap<QString, QString>& data) {
    m_data = data;
    // Auto-start shimmer animation if no thumbnail yet
    if (!m_hasThumbnail && !m_isSkeleton && !data.isEmpty()) {
        if (!m_skeletonTimer) {
            m_skeletonTimer = new QTimer(this);
            connect(m_skeletonTimer, &QTimer::timeout, this, &GameCard::updateSkeletonPulse);
        }
        m_skeletonPulse = 0.0;
        m_pulseIncreasing = true;
        m_skeletonTimer->start(30);
    }
    update();
}

QMap<QString, QString> GameCard::gameData() const {
    return m_data;
}

void GameCard::setThumbnail(const QPixmap& pixmap) {
    m_thumbnail = pixmap;
    m_hasThumbnail = !pixmap.isNull();
    if (m_hasThumbnail) {
        extractDominantColor(pixmap);
        // Stop shimmer animation — real image arrived
        if (m_skeletonTimer) m_skeletonTimer->stop();
        m_skeletonPulse = 0.0;
    }
    update();
}

QColor GameCard::getDominantColor() const {
    return m_dominantColor;
}

void GameCard::extractDominantColor(const QPixmap& pixmap) {
    if (pixmap.isNull()) {
        m_dominantColor = QColor();
        return;
    }
    QImage img = pixmap.toImage().scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QColor mostVibrant = Colors::toQColor(Colors::PRIMARY);
    double maxScore = -1.0;
    
    for (int y = 0; y < img.height(); y += 2) {
        for (int x = 0; x < img.width(); x += 2) {
            QColor c = img.pixelColor(x, y);
            if (c.saturationF() < 0.1 || c.lightnessF() < 0.1 || c.lightnessF() > 0.9) continue;
            double score = c.saturationF() * c.lightnessF() * (c.lightnessF() > 0.5 ? 1.2 : 1.0);
            if (score > maxScore) {
                maxScore = score;
                mostVibrant = c;
            }
        }
    }
    if (maxScore > 0) m_dominantColor = mostVibrant;
}

bool GameCard::hasThumbnail() const {
    return m_hasThumbnail;
}

void GameCard::setSelected(bool selected) {
    if (m_selected == selected) return;
    m_selected = selected;
    update();
    emit selectionChanged(m_selected, this);
}

void GameCard::setSelectable(bool selectable) {
    if (m_isSelectable == selectable) return;
    m_isSelectable = selectable;
    if (!selectable && m_selected) {
        setSelected(false);
    }
}

bool GameCard::isSelected() const {
    return m_selected;
}

QString GameCard::appId() const {
    return m_data.value("appid");
}

void GameCard::setSkeleton(bool skeleton) {
    if (m_isSkeleton == skeleton) return;
    m_isSkeleton = skeleton;
    if (m_isSkeleton) {
        if (!m_skeletonTimer) {
            m_skeletonTimer = new QTimer(this);
            connect(m_skeletonTimer, &QTimer::timeout, this, &GameCard::updateSkeletonPulse);
        }
        m_skeletonPulse = 0.0;
        m_pulseIncreasing = true;
        m_skeletonTimer->start(30); // ~33 fps
    } else {
        if (m_skeletonTimer) m_skeletonTimer->stop();
        m_skeletonPulse = 0.0;
    }
    update();
}

bool GameCard::isSkeleton() const {
    return m_isSkeleton;
}

void GameCard::updateSkeletonPulse() {
    if (m_pulseIncreasing) {
        m_skeletonPulse += 0.05;
        if (m_skeletonPulse >= 1.0) {
            m_skeletonPulse = 1.0;
            m_pulseIncreasing = false;
        }
    } else {
        m_skeletonPulse -= 0.05;
        if (m_skeletonPulse <= 0.0) {
            m_skeletonPulse = 0.0;
            m_pulseIncreasing = true;
        }
    }
    update();
}

void GameCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF cardRect = QRectF(rect()).adjusted(4, 4, -4, -4);
    int radius = 8; // User requested sharp corners
    bool supported = (m_data.value("supported") == "true");

    // ── Outer Glow ──
    if (m_hovered || m_selected) {
        QColor glowColor = m_dominantColor.isValid() ? m_dominantColor : Colors::toQColor(Colors::PRIMARY);
        int maxGlow = m_selected ? 6 : 4;
        for (int i = maxGlow; i >= 1; --i) {
            QColor shadowColor = glowColor;
            shadowColor.setAlpha(m_selected ? 40 / i : 20 / i);
            QPen shadowPen(shadowColor, 2.0);
            painter.setPen(shadowPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(cardRect.adjusted(-i*2, -i*2, i*2, i*2), radius + i, radius + i);
        }
    } else if (m_hasThumbnail && m_dominantColor.isValid()) {
        // Subtle ambient glow even when resting
        for (int i = 2; i >= 1; --i) {
            QColor shadowColor = m_dominantColor;
            shadowColor.setAlpha(10 / i);
            QPen shadowPen(shadowColor, 2.0);
            painter.setPen(shadowPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(cardRect.adjusted(-i*2, -i*2, i*2, i*2), radius + i, radius + i);
        }
    }

    // Clip to rounded rect
    QPainterPath clipPath;
    clipPath.addRoundedRect(cardRect, radius, radius);
    painter.setClipPath(clipPath);

    if (m_isSkeleton) {
        // Base skeleton background
        QColor baseColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGH);
        // Blend dynamically with a slightly lighter color for the pulse
        QColor pulseColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGHEST);
        
        int r = baseColor.red() + (pulseColor.red() - baseColor.red()) * m_skeletonPulse;
        int g = baseColor.green() + (pulseColor.green() - baseColor.green()) * m_skeletonPulse;
        int b = baseColor.blue() + (pulseColor.blue() - baseColor.blue()) * m_skeletonPulse;
        QColor activeColor(r, g, b);

        painter.fillRect(cardRect.toRect(), activeColor);

        // Draw shimmer overlay
        QLinearGradient shimmer(cardRect.topLeft(), cardRect.bottomRight());
        shimmer.setColorAt(0, QColor(255, 255, 255, 0));
        shimmer.setColorAt(0.5, QColor(255, 255, 255, 10 + 15 * m_skeletonPulse));
        shimmer.setColorAt(1, QColor(255, 255, 255, 0));
        painter.fillRect(cardRect.toRect(), shimmer);

        // Draw skeleton placeholder for thumbnail
        QRectF thumbPlaceholder(cardRect.left(), cardRect.top(), cardRect.width(), cardRect.height() - 62);
        QColor thumbColor = Colors::toQColor(Colors::SURFACE_VARIANT);
        thumbColor.setAlphaF(0.4 + 0.3 * m_skeletonPulse);
        painter.fillRect(thumbPlaceholder.toRect(), thumbColor);

        // Draw skeleton placeholders for text in bottom area
        int infoHeight = 62;
        QRectF infoRect(cardRect.left(), cardRect.bottom() - infoHeight, cardRect.width(), infoHeight);
        QColor infoColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGHEST);
        infoColor.setAlphaF(0.5 + 0.3 * m_skeletonPulse);
        
        // Name placeholder
        QRectF namePlaceholder(infoRect.left() + 12, infoRect.top() + 14, infoRect.width() * 0.7, 14);
        QPainterPath namePath;
        namePath.addRoundedRect(namePlaceholder, 6, 6);
        painter.fillPath(namePath, infoColor);

        // ID placeholder
        QRectF idPlaceholder(infoRect.left() + 12, infoRect.top() + 36, infoRect.width() * 0.4, 10);
        QPainterPath idPath;
        idPath.addRoundedRect(idPlaceholder, 5, 5);
        painter.fillPath(idPath, infoColor);

        // Reset clip for border drawing
        painter.setClipRect(rect());

        // Resting outline variant
        QPen borderPen(Colors::toQColor(Colors::OUTLINE_VARIANT), 1);
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(cardRect, radius, radius);

        return; // Skip drawing real content
    }

    if (m_hasThumbnail) {
        // Scale proportionally to fill completely, then center-crop the excess
        QSize cardSize = cardRect.size().toSize();
        QPixmap scaled = m_thumbnail.scaled(cardSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int sx = (scaled.width() - cardSize.width()) / 2;
        int sy = (scaled.height() - cardSize.height()) / 2;
        painter.drawPixmap(cardRect.toRect(), scaled, QRect(sx, sy, cardSize.width(), cardSize.height()));
    } else if (!m_data.isEmpty()) {
        // No thumbnail yet — show skeleton shimmer instead of static gamepad icon
        QColor baseColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGH);
        QColor pulseColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGHEST);

        // Animate pulse if timer is running, otherwise use static base
        qreal pulse = m_skeletonPulse;
        int r = baseColor.red() + (pulseColor.red() - baseColor.red()) * pulse;
        int g = baseColor.green() + (pulseColor.green() - baseColor.green()) * pulse;
        int b = baseColor.blue() + (pulseColor.blue() - baseColor.blue()) * pulse;
        QColor activeColor(r, g, b);
        painter.fillRect(cardRect.toRect(), activeColor);

        // Animated shimmer sweep across the card
        qreal sweepPos = m_skeletonPulse;
        QLinearGradient shimmer(cardRect.left() + cardRect.width() * (sweepPos - 0.3), cardRect.top(),
                                cardRect.left() + cardRect.width() * (sweepPos + 0.3), cardRect.bottom());
        shimmer.setColorAt(0, QColor(255, 255, 255, 0));
        shimmer.setColorAt(0.5, QColor(255, 255, 255, 18));
        shimmer.setColorAt(1, QColor(255, 255, 255, 0));
        painter.fillRect(cardRect.toRect(), shimmer);
    } else {
        // Truly empty card — dark surface
        QColor surfaceColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGH);
        painter.fillRect(cardRect.toRect(), surfaceColor);
    }

    // ── Bottom info area (animated opacity) ──
    if (m_hoverOpacity > 0.01 || m_selected) {
        qreal opacity = m_selected ? 1.0 : m_hoverOpacity;
        int infoHeight = 70; 
        QRectF infoRect(cardRect.left(), cardRect.bottom() - infoHeight,
                        cardRect.width(), infoHeight);

        // Seamless gradient from transparent to dark (no separator line)
        QLinearGradient infoGrad(infoRect.topLeft(), infoRect.bottomLeft());
        infoGrad.setColorAt(0, QColor(0, 0, 0, 0));
        infoGrad.setColorAt(0.4, QColor(8, 8, 12, (int)(160 * opacity)));
        infoGrad.setColorAt(1, QColor(8, 8, 12, (int)(230 * opacity)));
        painter.fillRect(infoRect.toRect(), infoGrad);

        // Game name
        QString name = m_data.value("name", "Unknown");
        QFont nameFont("Roboto", 10, QFont::DemiBold);
        nameFont.setStyleStrategy(QFont::PreferAntialias);
        painter.setFont(nameFont);
        painter.setPen(QColor(255, 255, 255, (int)(240 * opacity)));

        QRectF nameRect(infoRect.left() + 12, infoRect.top() + 22,
                        infoRect.width() - 24, 22);
        QFontMetrics fm(nameFont);
        QString elidedName = fm.elidedText(name, Qt::ElideRight, (int)nameRect.width());
        painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

        // App ID
        QFont idFont("Roboto", 8);
        idFont.setStyleStrategy(QFont::PreferAntialias);
        painter.setFont(idFont);
        painter.setPen(QColor(180, 180, 200, (int)(200 * opacity)));
        QRectF idRect(infoRect.left() + 12, infoRect.top() + 44,
                      infoRect.width() - 24, 18);
        painter.drawText(idRect, Qt::AlignLeft | Qt::AlignVCenter,
                         QString("ID: %1").arg(m_data.value("appid", "?")));
    }

    // Reset clip for border drawing
    painter.setClipRect(rect());

    // ── Border & selection state ──
    if (m_selected) {
        // Glowing primary border
        QColor glowColor = m_dominantColor.isValid() ? m_dominantColor : Colors::toQColor(Colors::PRIMARY);
        QPen selPen(glowColor, 2.5);
        painter.setPen(selPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(cardRect, radius, radius);
    } else if (m_hovered) {
        // Subtle glass outline on hover
        QPen hovPen(Colors::toQColor(Colors::OUTLINE), 1.5);
        painter.setPen(hovPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(cardRect, radius, radius);

        // Top highlight shimmer
        painter.setClipPath(clipPath);
        QLinearGradient topShine(cardRect.topLeft(),
                                QPointF(cardRect.left(), cardRect.top() + 30));
        topShine.setColorAt(0, QColor(255, 255, 255, 15));
        topShine.setColorAt(1, QColor(255, 255, 255, 0));
        painter.fillRect(QRectF(cardRect.left(), cardRect.top(),
                                cardRect.width(), 30), topShine);
        painter.setClipRect(rect());
    } else {
        // Glass semitransparent outline
        QPen borderPen(Colors::toQColor(Colors::OUTLINE_VARIANT), 1);
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(cardRect, radius, radius);
    }

    // Reset clip for border drawing
    painter.setClipRect(rect());
}

void GameCard::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    if (m_isSkeleton) return;
    
    if (m_isSelectable) {
        setSelected(!m_selected);
    } else {
        emit clicked(this);
    }
}

void GameCard::enterEvent(QEnterEvent* event) {
    Q_UNUSED(event);
    if (m_isSkeleton) return;
    m_hovered = true;
    
    if (!m_hoverAnim) {
        m_hoverAnim = new QPropertyAnimation(this, "hoverOpacity", this);
    }
    m_hoverAnim->stop();
    m_hoverAnim->setDuration(200);
    m_hoverAnim->setStartValue(m_hoverOpacity);
    m_hoverAnim->setEndValue(1.0);
    m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_hoverAnim->start();
}

void GameCard::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    if (m_isSkeleton) return;
    m_hovered = false;
    
    if (!m_hoverAnim) {
        m_hoverAnim = new QPropertyAnimation(this, "hoverOpacity", this);
    }
    m_hoverAnim->stop();
    m_hoverAnim->setDuration(200);
    m_hoverAnim->setStartValue(m_hoverOpacity);
    m_hoverAnim->setEndValue(0.0);
    m_hoverAnim->setEasingCurve(QEasingCurve::InCubic);
    m_hoverAnim->start();
}
