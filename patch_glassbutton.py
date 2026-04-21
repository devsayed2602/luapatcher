import sys

def patch_file():
    with open('src/glassbutton.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()

    out = []
    i = 0
    in_paintEvent = False
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('void GlassButton::paintEvent(QPaintEvent* event) {'):
            in_paintEvent = True
            
            # Write the new paintEvent based on new theme
            code = """void GlassButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    bool isHover = underMouse() && isEnabled();
    bool isPressed = isDown() && isEnabled();
    
    QRect r = rect();
    int h = r.height();
    bool isCompact = (h < 60);
    
    QRectF bgRect = QRectF(r).adjusted(1, 1, -1, -1);
    int radius = 8; // M3 standard corner radius
    
    // ── Background ──
    QColor bgColor;
    if (m_isActive) {
        // Active: very subtle translucent dark blue overlay
        bgColor = Colors::toQColor(Colors::SURFACE_BRIGHT);
        bgColor.setAlpha(80);
    } else if (isPressed) {
        bgColor = Colors::toQColor(Colors::SURFACE_VARIANT);
        bgColor.setAlpha(120);
    } else if (isHover) {
        bgColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGH);
        bgColor.setAlpha(60);
    } else {
        bgColor = QColor(0, 0, 0, 0); // Transparent when resting
    }
    
    QPainterPath bgPath;
    bgPath.addRoundedRect(bgRect, radius, radius);
    painter.fillPath(bgPath, bgColor);
    
    // ── Active indicator neon bar (left) ──
    if (m_isActive) {
        QLinearGradient neonGrad(0, 0, 8, 0);
        QColor primary = Colors::toQColor(Colors::PRIMARY); // Cream
        neonGrad.setColorAt(0, primary);
        
        QColor fadePrimary = primary;
        fadePrimary.setAlpha(0);
        neonGrad.setColorAt(1, fadePrimary);
        
        painter.fillRect(QRectF(0, h * 0.15, 4, h * 0.7), neonGrad);
        
        QPainterPath pill;
        pill.addRoundedRect(QRectF(0, h * 0.15, 3, h * 0.7), 1.5, 1.5);
        painter.fillPath(pill, primary);
    }
    
    // ── Border ── // Optional, usually navigation rails don't have borders, but we can add subtle outlines for hover
    QColor borderColor;
    if (isHover && !m_isActive) {
        borderColor = Colors::toQColor(Colors::OUTLINE_VARIANT);
        QPen pen(borderColor, 1);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(bgRect, radius, radius);
    }
    
    // ── Icon ──
    int iconSize = isCompact ? 20 : 24;
    int padding = isCompact ? 10 : 14;
    bool isNarrow = r.width() < 100;
    
    QRectF iconBgRect;
    if (isNarrow) {
        iconBgRect = QRectF((r.width() - (isCompact ? 30 : 36)) / 2.0,
                            (h - (isCompact ? 30 : 36)) / 2.0,
                            isCompact ? 30 : 36, isCompact ? 30 : 36);
    } else {
        iconBgRect = QRectF(padding, (h - (isCompact ? 30 : 36)) / 2.0,
                            isCompact ? 30 : 36, isCompact ? 30 : 36);
    }
    
    // Icon background is completely omitted for cleaner aesthetic, relying on pure icon colors
    
    QRectF iconDrawRect(
        iconBgRect.center().x() - iconSize / 2.0,
        iconBgRect.center().y() - iconSize / 2.0,
        iconSize, iconSize
    );
    
    QColor iconColor = m_isActive
        ? Colors::toQColor(Colors::PRIMARY) // Cream
        : Colors::toQColor(Colors::ON_SURFACE_VARIANT); // Light Blue / Greyed out
    MaterialIcons::draw(painter, iconDrawRect, iconColor, m_icon);
    
    // ── Text ──
    if (!isNarrow) {
        int textX = padding + iconBgRect.width() + (isCompact ? 10 : 14);
        int textW = r.width() - textX - 8;
        
        QColor textColor = m_isActive
            ? Colors::toQColor(Colors::PRIMARY)
            : Colors::toQColor(Colors::ON_SURFACE_VARIANT);
        painter.setPen(textColor);
        
        if (isCompact || m_descText.isEmpty()) {
            QRectF titleRect(textX, 0, textW, h);
            QFont titleFont("Segoe UI", isCompact ? 9 : 11, QFont::Bold);
            titleFont.setStyleStrategy(QFont::PreferAntialias);
            painter.setFont(titleFont);
            painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_titleText.trimmed());
        } else {
            int titleY = (h / 2) - 10;
            QRectF titleRect(textX, titleY - 2, textW, 20);
            QFont titleFont("Segoe UI", 11, QFont::Bold);
            titleFont.setStyleStrategy(QFont::PreferAntialias);
            painter.setFont(titleFont);
            painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_titleText);
            
            QRectF descRect(textX, titleY + 18, textW, 16);
            painter.setPen(Colors::toQColor(Colors::ON_SURFACE_VARIANT));
            QFont descFont("Segoe UI", 9);
            descFont.setStyleStrategy(QFont::PreferAntialias);
            painter.setFont(descFont);
            painter.drawText(descRect, Qt::AlignLeft | Qt::AlignVCenter, m_descText);
        }
    }
}
"""
            out.append(code)
            
            i += 1
            while i < len(lines) and not(lines[i].startswith('}')):
                i += 1
            # There might be multiple '}' but since it's the end of a method, let's fast forward carefully.
            # wait, it's safer to read line by line until I see another method or reach end.
            
            # let's just use Python's regex to replace from paintEvent to end of file, assuming it's the last method!
            # Let's check glassbutton.cpp
            continue
            
        if in_paintEvent:
            # We skip lines until we're out of paintEvent. GlassButton::paintEvent is the last method in glassbutton.cpp
            pass
        else:
            out.append(line)
        i += 1

    with open('src/glassbutton.cpp', 'w', encoding='utf-8') as f:
        f.writelines(out)

patch_file()
