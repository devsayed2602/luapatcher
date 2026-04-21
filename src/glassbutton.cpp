#include "glassbutton.h"
#include "utils/colors.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QPen>
#include <QFont>

// Map legacy emoji strings to Material icons
static MaterialIcons::Icon mapEmojiToIcon(const QString& emoji) {
    QString trimmed = emoji.trimmed();
    if (trimmed.contains("🔧") || trimmed.contains("wrench"))
        return MaterialIcons::Build;
    if (trimmed.contains("📚") || trimmed.contains("library") || trimmed.contains("book"))
        return MaterialIcons::Library;
    if (trimmed.contains("↻") || trimmed.contains("restart") || trimmed.contains("refresh"))
        return MaterialIcons::RestartAlt;
    if (trimmed.contains("🗑") || trimmed.contains("delete") || trimmed.contains("trash"))
        return MaterialIcons::Delete;
    if (trimmed.contains("⬇") || trimmed.contains("download"))
        return MaterialIcons::Download;
    if (trimmed.contains("⚡") || trimmed.contains("flash") || trimmed.contains("bolt"))
        return MaterialIcons::Flash;
    if (trimmed.contains("+") || trimmed.contains("add"))
        return MaterialIcons::Add;
    return MaterialIcons::Flash;
}

GlassButton::GlassButton(MaterialIcons::Icon icon, const QString& title,
                         const QString& description, const QString& accentColor,
                         QWidget* parent)
    : QPushButton(parent)
    , m_icon(icon)
    , m_titleText(title)
    , m_descText(description)
    , m_accentColor(accentColor)
    , m_isActive(false)
{
    setMinimumHeight(40);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setCursor(Qt::PointingHandCursor);
    
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(1.0);
}

GlassButton::GlassButton(const QString& iconChar, const QString& title,
                         const QString& description, const QString& accentColor,
                         QWidget* parent)
    : QPushButton(parent)
    , m_icon(mapEmojiToIcon(iconChar))
    , m_titleText(title)
    , m_descText(description)
    , m_accentColor(accentColor)
    , m_isActive(false)
{
    setMinimumHeight(40);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setCursor(Qt::PointingHandCursor);
    
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(1.0);
}

void GlassButton::setDescription(const QString& desc) {
    m_descText = desc;
    update();
}

void GlassButton::setEnabled(bool enabled) {
    QPushButton::setEnabled(enabled);
    m_opacityEffect->setOpacity(enabled ? 1.0 : 0.45);
    update();
}

void GlassButton::setAccentColor(const QString& color) {
    m_accentColor = color;
    update();
}

void GlassButton::setActive(bool active) {
    if (m_isActive == active) return;
    m_isActive = active;
    update();
}

void GlassButton::setMaterialIcon(MaterialIcons::Icon icon) {
    m_icon = icon;
    update();
}

void GlassButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    bool isHover = underMouse() && isEnabled();
    bool isPressed = isDown() && isEnabled();
    
    QRect r = rect();
    int h = r.height();
    bool isCompact = (h < 60);
    
    QRectF bgRect = QRectF(r).adjusted(1, 1, -1, -1);
    int radius = 10; // Reference: radius 10
    
    // Background — only on hover (reference: rgba(1,1,1,0.05))
    if (isHover || isPressed) {
        QColor bgColor = isPressed ? QColor(255, 255, 255, 20) : QColor(255, 255, 255, 13);
        QPainterPath bgPath;
        bgPath.addRoundedRect(bgRect, radius, radius);
        painter.fillPath(bgPath, bgColor);
    }
    
    // Active indicator is now drawn as a sidebar-level animated widget
    
    // Icon
    int iconSize = 22;
    int iconX = 24; // After pill area
    
    bool isNarrow = r.width() < 100;
    
    QRectF iconRect;
    if (isNarrow) {
        iconRect = QRectF((r.width() - iconSize) / 2.0, (h - iconSize) / 2.0, iconSize, iconSize);
    } else {
        iconRect = QRectF(iconX, (h - iconSize) / 2.0, iconSize, iconSize);
    }
    
    // All sidebar items use #EFECE3 per reference
    QColor iconColor = QColor("#EFECE3");
    MaterialIcons::draw(painter, iconRect, iconColor, m_icon);
    
    // Text (reference: 15px, active=white bold, inactive=#8FABD4 normal)
    if (!isNarrow) {
        int textX = iconX + iconSize + 15; // Reference: spacing 15
        int textW = r.width() - textX - 8;
        
        QColor textColor = QColor("#EFECE3");
        painter.setPen(textColor);
        
        QFont titleFont("Segoe UI", 13, m_isActive ? QFont::Bold : QFont::Normal);
        titleFont.setStyleStrategy(QFont::PreferAntialias);
        painter.setFont(titleFont);
        
        QRectF titleRect(textX, 0, textW, h);
        painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_titleText.trimmed());
    }
}
