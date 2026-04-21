"""
Apply reference UI theme to the Lua Patcher app.
Matches: reference ui/Main.qml, SidebarItem.qml, GameCard.qml
"""

def patch_glassbutton():
    """Rewrite GlassButton paintEvent to match SidebarItem.qml:
    - Active: 4px cream pill on left + white bold text
    - Inactive: no pill + #8FABD4 muted blue text
    - Hover: rgba(255,255,255,0.05) bg
    - Height 45px, radius 10, text 15px
    """
    with open('src/glassbutton.cpp', 'r', encoding='utf-8') as f:
        content = f.read()

    # Find paintEvent and replace it entirely
    start = content.find('void GlassButton::paintEvent(QPaintEvent* event) {')
    if start == -1:
        print("ERROR: paintEvent not found in glassbutton.cpp")
        return

    # The paintEvent is the last function in glassbutton.cpp, ends at closing }
    # Find matching closing brace
    new_paint = r'''void GlassButton::paintEvent(QPaintEvent* event) {
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
    
    // Active indicator — 4px cream pill on left (reference: SidebarItem.qml)
    if (m_isActive) {
        QColor pillColor = QColor("#EFECE3"); // colorAccent
        QPainterPath pill;
        pill.addRoundedRect(QRectF(4, (h - 24) / 2.0, 4, 24), 2, 2);
        painter.fillPath(pill, pillColor);
    }
    
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
    
    // Reference: active = white, inactive = #8FABD4
    QColor iconColor = m_isActive ? QColor(255, 255, 255) : QColor("#8FABD4");
    MaterialIcons::draw(painter, iconRect, iconColor, m_icon);
    
    // Text (reference: 15px, active=white bold, inactive=#8FABD4 normal)
    if (!isNarrow) {
        int textX = iconX + iconSize + 15; // Reference: spacing 15
        int textW = r.width() - textX - 8;
        
        QColor textColor = m_isActive ? QColor(255, 255, 255) : QColor("#8FABD4");
        painter.setPen(textColor);
        
        QFont titleFont("Segoe UI", 11, m_isActive ? QFont::Bold : QFont::Normal);
        titleFont.setStyleStrategy(QFont::PreferAntialias);
        painter.setFont(titleFont);
        
        QRectF titleRect(textX, 0, textW, h);
        painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_titleText.trimmed());
    }
}
'''
    content = content[:start] + new_paint
    with open('src/glassbutton.cpp', 'w', encoding='utf-8') as f:
        f.write(content)
    print("✓ glassbutton.cpp patched")


def patch_gamecard():
    """Rewrite GameCard paintEvent to match GameCard.qml:
    - 280x340, radius 20, surface #4A70A9
    - Border: rgba(255,255,255,0.05)
    - Inner thumbnail: top area, radius 12, black bg
    - Title: white 18px bold
    - Genre: #8FABD4 12px
    - Drop shadow: offset 5/5, radius 15
    """
    with open('src/gamecard.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()

    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        
        if line.strip().startswith('void GameCard::paintEvent(QPaintEvent* event) {'):
            # Write new paintEvent
            out.append('void GameCard::paintEvent(QPaintEvent* event) {\n')
            out.append('    Q_UNUSED(event);\n')
            out.append('    QPainter painter(this);\n')
            out.append('    painter.setRenderHint(QPainter::Antialiasing);\n')
            out.append('    painter.setRenderHint(QPainter::SmoothPixmapTransform);\n\n')
            
            out.append('    QRectF cardRect = QRectF(rect()).adjusted(4, 4, -4, -4);\n')
            out.append('    int radius = 20; // Reference: radius 20\n\n')
            
            # Drop shadow (reference: offset 5/5, radius 15)
            out.append('    // Drop shadow\n')
            out.append('    if (!m_isSkeleton) {\n')
            out.append('        for (int s = 6; s >= 1; --s) {\n')
            out.append('            QColor shadowColor(0, 0, 0, 30 / s);\n')
            out.append('            painter.setPen(Qt::NoPen);\n')
            out.append('            painter.setBrush(shadowColor);\n')
            out.append('            painter.drawRoundedRect(cardRect.adjusted(-s, -s, s+3, s+3), radius + s, radius + s);\n')
            out.append('        }\n')
            out.append('    }\n\n')
            
            # Card surface
            out.append('    // Card surface — #4A70A9 (reference: colorCard)\n')
            out.append('    QPainterPath cardPath;\n')
            out.append('    cardPath.addRoundedRect(cardRect, radius, radius);\n')
            out.append('    painter.setClipPath(cardPath);\n')
            out.append('    painter.fillPath(cardPath, QColor("#4A70A9"));\n\n')
            
            # Skeleton state
            out.append('    if (m_isSkeleton) {\n')
            out.append('        QColor baseColor(60, 90, 140);\n')
            out.append('        QColor pulseColor(80, 120, 180);\n')
            out.append('        int r2 = baseColor.red() + (pulseColor.red() - baseColor.red()) * m_skeletonPulse;\n')
            out.append('        int g2 = baseColor.green() + (pulseColor.green() - baseColor.green()) * m_skeletonPulse;\n')
            out.append('        int b2 = baseColor.blue() + (pulseColor.blue() - baseColor.blue()) * m_skeletonPulse;\n')
            out.append('        painter.fillPath(cardPath, QColor(r2, g2, b2));\n')
            out.append('        // Thumbnail placeholder\n')
            out.append('        QRectF thumbPlaceholder(cardRect.left() + 15, cardRect.top() + 15, cardRect.width() - 30, 180);\n')
            out.append('        QPainterPath thumbPath; thumbPath.addRoundedRect(thumbPlaceholder, 12, 12);\n')
            out.append('        QColor thumbColor(50, 75, 120);\n')
            out.append('        thumbColor.setAlphaF(0.4 + 0.3 * m_skeletonPulse);\n')
            out.append('        painter.fillPath(thumbPath, thumbColor);\n')
            out.append('        // Name placeholder\n')
            out.append('        QRectF namePh(cardRect.left() + 15, thumbPlaceholder.bottom() + 16, cardRect.width() * 0.6, 14);\n')
            out.append('        QPainterPath np; np.addRoundedRect(namePh, 6, 6);\n')
            out.append('        painter.fillPath(np, thumbColor);\n')
            out.append('        painter.setClipRect(rect());\n')
            out.append('        // Border\n')
            out.append('        painter.setPen(QPen(QColor(255, 255, 255, 13), 1));\n')
            out.append('        painter.setBrush(Qt::NoBrush);\n')
            out.append('        painter.drawRoundedRect(cardRect, radius, radius);\n')
            out.append('        return;\n')
            out.append('    }\n\n')
            
            # Thumbnail area (reference: 180px, radius 12, top area with margins 15)
            out.append('    // Thumbnail area — reference: 180px, radius 12, margins 15\n')
            out.append('    QRectF thumbRect(cardRect.left() + 15, cardRect.top() + 15, cardRect.width() - 30, 180);\n')
            out.append('    QPainterPath thumbClip;\n')
            out.append('    thumbClip.addRoundedRect(thumbRect, 12, 12);\n\n')
            
            out.append('    if (m_hasThumbnail) {\n')
            out.append('        painter.save();\n')
            out.append('        painter.setClipPath(thumbClip);\n')
            out.append('        QSize thumbSize = thumbRect.size().toSize();\n')
            out.append('        QPixmap scaled = m_thumbnail.scaled(thumbSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);\n')
            out.append('        int sx = (scaled.width() - thumbSize.width()) / 2;\n')
            out.append('        int sy = (scaled.height() - thumbSize.height()) / 2;\n')
            out.append('        painter.drawPixmap(thumbRect.toRect(), scaled, QRect(sx, sy, thumbSize.width(), thumbSize.height()));\n')
            out.append('        painter.restore();\n')
            out.append('    } else if (!m_data.isEmpty()) {\n')
            out.append('        // Loading shimmer\n')
            out.append('        painter.save();\n')
            out.append('        painter.setClipPath(thumbClip);\n')
            out.append('        QColor baseColor(30, 50, 80);\n')
            out.append('        painter.fillRect(thumbRect, baseColor);\n')
            out.append('        QLinearGradient shimmer(thumbRect.left(), thumbRect.top(), thumbRect.right(), thumbRect.bottom());\n')
            out.append('        shimmer.setColorAt(0, QColor(255, 255, 255, 0));\n')
            out.append('        shimmer.setColorAt(0.5, QColor(255, 255, 255, 15));\n')
            out.append('        shimmer.setColorAt(1, QColor(255, 255, 255, 0));\n')
            out.append('        painter.fillRect(thumbRect, shimmer);\n')
            out.append('        painter.restore();\n')
            out.append('    } else {\n')
            out.append('        painter.fillPath(thumbClip, QColor(0, 0, 0)); // Reference: color "#000"\n')
            out.append('    }\n\n')
            
            # Text area
            out.append('    // Title — reference: white, 18px, bold\n')
            out.append('    QString name = m_data.value("name", "Unknown");\n')
            out.append('    QFont nameFont("Segoe UI", 13, QFont::Bold);\n')
            out.append('    nameFont.setStyleStrategy(QFont::PreferAntialias);\n')
            out.append('    painter.setFont(nameFont);\n')
            out.append('    painter.setPen(QColor(255, 255, 255));\n')
            out.append('    QRectF nameRect(cardRect.left() + 15, thumbRect.bottom() + 14, cardRect.width() - 30, 22);\n')
            out.append('    QFontMetrics fm(nameFont);\n')
            out.append('    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, fm.elidedText(name, Qt::ElideRight, nameRect.width()));\n\n')
            
            # Genre — reference: #8FABD4, 12px
            out.append('    // Genre — reference: #8FABD4, 12px\n')
            out.append('    QFont genreFont("Segoe UI", 9);\n')
            out.append('    painter.setFont(genreFont);\n')
            out.append('    painter.setPen(QColor("#8FABD4"));\n')
            out.append('    QRectF genreRect(cardRect.left() + 15, nameRect.bottom() + 4, cardRect.width() - 30, 16);\n')
            out.append('    painter.drawText(genreRect, Qt::AlignLeft | Qt::AlignVCenter, "Action / Adventure");\n\n')
            
            # Hover glow
            out.append('    // Hover border glow\n')
            out.append('    painter.setClipRect(rect());\n')
            out.append('    if (m_hovered || m_selected) {\n')
            out.append('        QColor glowColor = m_dominantColor.isValid() ? m_dominantColor : QColor("#EFECE3");\n')
            out.append('        for (int gi = 3; gi >= 1; --gi) {\n')
            out.append('            glowColor.setAlpha(m_selected ? 50 / gi : 25 / gi);\n')
            out.append('            painter.setPen(QPen(glowColor, 2.0));\n')
            out.append('            painter.setBrush(Qt::NoBrush);\n')
            out.append('            painter.drawRoundedRect(cardRect.adjusted(-gi, -gi, gi, gi), radius + gi, radius + gi);\n')
            out.append('        }\n')
            out.append('    }\n\n')
            
            # Border — reference: border.color: Qt.rgba(1,1,1,0.05)
            out.append('    // Border — reference: rgba(255,255,255,0.05)\n')
            out.append('    painter.setPen(QPen(QColor(255, 255, 255, 13), 1));\n')
            out.append('    painter.setBrush(Qt::NoBrush);\n')
            out.append('    painter.drawRoundedRect(cardRect, radius, radius);\n')
            out.append('}\n')
            
            # Skip old paintEvent body
            i += 1
            brace_count = 1
            while i < len(lines) and brace_count > 0:
                for ch in lines[i]:
                    if ch == '{': brace_count += 1
                    elif ch == '}': brace_count -= 1
                if brace_count > 0:
                    i += 1
            i += 1  # skip the closing }
            continue
        
        out.append(line)
        i += 1

    with open('src/gamecard.cpp', 'w', encoding='utf-8') as f:
        f.writelines(out)
    print("✓ gamecard.cpp patched")


def patch_mainwindow():
    """Apply reference UI layout changes to mainwindow.cpp:
    - Sidebar: 260px
    - Right panel: 320px, profile card 120px, radius 20, rgba(1,1,1,0.05)
    - Hero banner: 400px height, radius 25
    - Remove NAVIGATION label
    """
    with open('src/mainwindow.cpp', 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Sidebar width: 200 -> 260
    content = content.replace(
        "m_sidebarWidget->setFixedWidth(200);",
        "m_sidebarWidget->setFixedWidth(260);"
    )
    
    # 2. Sidebar inner margins: match reference margins 30
    content = content.replace(
        'sidebarInnerLayout->setContentsMargins(10, 24, 16, 16);',
        'sidebarInnerLayout->setContentsMargins(30, 30, 30, 30);'
    )
    content = content.replace(
        'sidebarInnerLayout->setSpacing(8);',
        'sidebarInnerLayout->setSpacing(20);'
    )
    
    # 3. Sidebar border — match reference: rgba(1,1,1,0.05)
    content = content.replace(
        'background-color: transparent; border-right: 1px solid rgba(255, 255, 255, 10);',
        'background-color: #000000; border-right: 1px solid rgba(255, 255, 255, 13);'
    )
    
    # 4. App title — reference: "IGNITE" but keep "Lua Patcher", make it 22px bold
    content = content.replace(
        "font-size: 17px; font-weight: 700;",
        "font-size: 20px; font-weight: 700; letter-spacing: -1px;"
    )
    
    # 5. Nav button height — reference: implicitHeight 45
    content = content.replace('m_tabLua->setFixedHeight(44);', 'm_tabLua->setFixedHeight(45);')
    content = content.replace('m_tabLibrary->setFixedHeight(44);', 'm_tabLibrary->setFixedHeight(45);')
    content = content.replace('m_tabSettings->setFixedHeight(44);', 'm_tabSettings->setFixedHeight(45);')
    
    # 6. Right panel width: 280 -> 320
    content = content.replace(
        "m_rightPanelWidget->setFixedWidth(280);",
        "m_rightPanelWidget->setFixedWidth(320);"
    )
    
    # 7. Right panel background — reference: rgba(1,1,1,0.02), border rgba(1,1,1,0.05)
    content = content.replace(
        'm_rightPanelWidget->setStyleSheet("background: transparent;");',
        'm_rightPanelWidget->setStyleSheet("background: rgba(255, 255, 255, 5); border-left: 1px solid rgba(255, 255, 255, 13);");'
    )
    
    # 8. Right panel margins — reference: margins 25, spacing 30
    content = content.replace(
        'rightLayout->setContentsMargins(0, 20, 20, 20);',
        'rightLayout->setContentsMargins(25, 25, 25, 25);'
    )
    content = content.replace(
        'rightLayout->setSpacing(24);',
        'rightLayout->setSpacing(30);'
    )
    
    # 9. Profile card — reference: height 120, radius 20, rgba(1,1,1,0.05)
    content = content.replace(
        'm_topProfileWidget->setFixedHeight(160);',
        'm_topProfileWidget->setFixedHeight(120);'
    )
    content = content.replace(
        'QString("QWidget { background: %1; border-radius: 16px; }").arg(Colors::SURFACE_BRIGHT)',
        '"QWidget { background: rgba(255, 255, 255, 13); border-radius: 20px; }"'
    )
    
    # 10. Profile name — reference: white 18px bold
    content = content.replace(
        '"font-size: 15px; font-weight: bold; color: white; background: transparent; padding-top: 5px;"',
        '"font-size: 16px; font-weight: bold; color: white; background: transparent;"'
    )
    
    # 11. Hero banner height — reference: 400px
    content = content.replace(
        "m_heroStack->setFixedHeight(240);",
        "m_heroStack->setFixedHeight(400);"
    )
    
    # 12. Hero slide height
    content = content.replace(
        "slide->setFixedHeight(240);",
        "slide->setFixedHeight(400);"
    )
    
    # 13. Hero banner radius — reference: radius 25
    content = content.replace(
        'm_heroStack->setStyleSheet("background: transparent; border: none; border-radius: 12px;");',
        'm_heroStack->setStyleSheet("background: transparent; border: none; border-radius: 25px;");'
    )
    
    # 14. Content area margins — reference: 40px top margin, columns width - 60
    content = content.replace(
        'mainLayout->setContentsMargins(20, 20, 20, 20);',
        'mainLayout->setContentsMargins(30, 40, 30, 20);'
    )
    
    # 15. Remove "NAVIGATION" label text but keep widget — just hide it
    content = content.replace(
        'sidebarInnerLayout->addWidget(m_navTitleLabel);',
        'm_navTitleLabel->hide();\n    sidebarInnerLayout->addWidget(m_navTitleLabel);'
    )
    
    # 16. Spacer between header and tabs — reference: Item { Layout.preferredHeight: 40 }
    content = content.replace(
        "sidebarInnerLayout->addSpacing(20);",
        "sidebarInnerLayout->addSpacing(40);"
    )
    
    # 17. Title label text colors — use proper white/colorMuted
    content = content.replace(
        """<span style='color: #ffffff;'>Leading</span> <span style='color: #bb86fc;'>Titles</span>""",
        """<span style='color: #ffffff;'>TRENDING</span>"""
    )
    content = content.replace(
        """<span style='color: #ffffff;'>All Available</span> <span style='color: #bb86fc;'>Games</span>""",
        """<span style='color: #ffffff;'>GAME STORE</span>"""
    )
    
    with open('src/mainwindow.cpp', 'w', encoding='utf-8') as f:
        f.write(content)
    print("✓ mainwindow.cpp patched")


if __name__ == '__main__':
    patch_glassbutton()
    patch_gamecard()
    patch_mainwindow()
    print("\n✅ All reference UI patches applied successfully!")
