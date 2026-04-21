import sys

def patch_file():
    with open('src/gamecard.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()

    out = []
    i = 0
    in_paintEvent = False
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('void GameCard::paintEvent(QPaintEvent* event) {'):
            in_paintEvent = True
            out.append('void GameCard::paintEvent(QPaintEvent* event) {\n')
            out.append('    Q_UNUSED(event);\n')
            out.append('    QPainter painter(this);\n')
            out.append('    painter.setRenderHint(QPainter::Antialiasing);\n')
            out.append('    painter.setRenderHint(QPainter::SmoothPixmapTransform);\n\n')
            
            out.append('    QRectF cardRect = QRectF(rect()).adjusted(4, 4, -4, -4);\n')
            out.append('    int radius = 12;\n\n')
            
            out.append('    // Outer Glow on hover\n')
            out.append('    if (m_hovered || m_selected) {\n')
            out.append('        QColor glowColor = Colors::toQColor(Colors::SECONDARY);\n')
            out.append('        for (int i = 4; i >= 1; --i) {\n')
            out.append('            glowColor.setAlpha(20 / i);\n')
            out.append('            painter.setPen(QPen(glowColor, 2.0));\n')
            out.append('            painter.setBrush(Qt::NoBrush);\n')
            out.append('            painter.drawRoundedRect(cardRect.adjusted(-i, -i, i, i), radius + i, radius + i);\n')
            out.append('        }\n')
            out.append('    }\n\n')
            
            out.append('    // Base dark blue container (#4A70A9 base mapped to SURFACE_BRIGHT)\n')
            out.append('    QPainterPath cardPath;\n')
            out.append('    cardPath.addRoundedRect(cardRect, radius, radius);\n')
            out.append('    painter.fillPath(cardPath, Colors::toQColor(Colors::SURFACE_BRIGHT));\n\n')
            
            out.append('    // Thumbnail area (top 170px for example)\n')
            out.append('    int thumbHeight = cardRect.height() * 0.65;\n')
            out.append('    QRectF thumbRect(cardRect.left(), cardRect.top(), cardRect.width(), thumbHeight);\n\n')
            
            out.append('    painter.save();\n')
            out.append('    QPainterPath thumbPath;\n')
            out.append('    // Rounded top, square bottom\n')
            out.append('    thumbPath.addRoundedRect(thumbRect, radius, radius);\n')
            out.append('    QPainterPath bottomRect;\n')
            out.append('    bottomRect.addRect(thumbRect.left(), thumbRect.bottom() - radius, thumbRect.width(), radius);\n')
            out.append('    thumbPath = thumbPath.united(bottomRect);\n')
            out.append('    painter.setClipPath(thumbPath);\n\n')
            
            out.append('    if (m_hasThumbnail) {\n')
            out.append('        QSize thumbSize = thumbRect.size().toSize();\n')
            out.append('        QPixmap scaled = m_thumbnail.scaled(thumbSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);\n')
            out.append('        int sx = (scaled.width() - thumbSize.width()) / 2;\n')
            out.append('        int sy = (scaled.height() - thumbSize.height()) / 2;\n')
            out.append('        painter.drawPixmap(thumbRect.toRect(), scaled, QRect(sx, sy, thumbSize.width(), thumbSize.height()));\n')
            out.append('    } else {\n')
            out.append('        painter.fillRect(thumbRect, Colors::toQColor(Colors::SURFACE_CONTAINER));\n')
            out.append('    }\n')
            out.append('    painter.restore();\n\n')
            
            out.append('    // "NEW" Badge on Top Left\n')
            out.append('    QRectF badgeRect(cardRect.left() + 10, cardRect.top() + 10, 36, 18);\n')
            out.append('    painter.setPen(Qt::NoPen);\n')
            out.append('    painter.setBrush(QColor("#FFDE42")); // Gold accent from original idea if needed, or use PRIMARY (Cream)\n')
            out.append('    painter.drawRoundedRect(badgeRect, 4, 4);\n')
            out.append('    painter.setPen(QColor(0, 0, 0));\n')
            out.append('    QFont badgeFont("Segoe UI", 8, QFont::Bold);\n')
            out.append('    painter.setFont(badgeFont);\n')
            out.append('    painter.drawText(badgeRect, Qt::AlignCenter, "NEW");\n\n')
            
            out.append('    // Rating on Top Right\n')
            out.append('    QRectF ratingRect(cardRect.right() - 46, cardRect.top() + 10, 36, 18);\n')
            out.append('    painter.setBrush(QColor(0, 0, 0, 150));\n')
            out.append('    painter.drawRoundedRect(ratingRect, 4, 4);\n')
            out.append('    painter.setPen(Colors::toQColor(Colors::ON_SURFACE));\n')
            out.append('    painter.drawText(ratingRect, Qt::AlignCenter, "★ 4.8");\n\n')
            
            out.append('    // Bottom Info Area\n')
            out.append('    QRectF infoRect(cardRect.left(), cardRect.top() + thumbHeight, cardRect.width(), cardRect.height() - thumbHeight);\n')
            out.append('    \n')
            out.append('    // Name\n')
            out.append('    QString name = m_data.value("name", "Unknown");\n')
            out.append('    QFont nameFont("Segoe UI", 12, QFont::Bold);\n')
            out.append('    painter.setFont(nameFont);\n')
            out.append('    painter.setPen(Colors::toQColor(Colors::ON_SURFACE));\n')
            out.append('    QRectF nameRect(infoRect.left() + 12, infoRect.top() + 12, infoRect.width() - 24, 20);\n')
            out.append('    QFontMetrics fm(nameFont);\n')
            out.append('    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, fm.elidedText(name, Qt::ElideRight, nameRect.width()));\n\n')
            
            out.append('    // Genre / Type mock\n')
            out.append('    QFont subFont("Segoe UI", 9);\n')
            out.append('    painter.setFont(subFont);\n')
            out.append('    painter.setPen(Colors::toQColor(Colors::ON_SURFACE_VARIANT));\n')
            out.append('    QRectF subRect(infoRect.left() + 12, nameRect.bottom() + 4, infoRect.width() - 24, 14);\n')
            out.append('    painter.drawText(subRect, Qt::AlignLeft | Qt::AlignVCenter, "Action, Adventure");\n\n')
            
            out.append('    // Separator line\n')
            out.append('    painter.setPen(QPen(QColor(255, 255, 255, 20), 1));\n')
            out.append('    painter.drawLine(infoRect.left() + 12, infoRect.bottom() - 36, infoRect.right() - 12, infoRect.bottom() - 36);\n\n')
            
            out.append('    // Price Footer\n')
            out.append('    QRectF footerRect(infoRect.left() + 12, infoRect.bottom() - 32, infoRect.width() - 24, 26);\n')
            out.append('    painter.setFont(nameFont);\n')
            out.append('    painter.setPen(Colors::toQColor(Colors::ON_SURFACE));\n')
            out.append('    painter.drawText(footerRect, Qt::AlignLeft | Qt::AlignVCenter, "$49.99");\n\n')
            
            out.append('    // Play/Buy > button\n')
            out.append('    QRectF btnRect(footerRect.right() - 26, footerRect.top(), 26, 26);\n')
            out.append('    painter.setPen(Qt::NoPen);\n')
            out.append('    painter.setBrush(Colors::toQColor(Colors::PRIMARY));\n')
            out.append('    painter.drawRoundedRect(btnRect, 6, 6);\n')
            out.append('    painter.setPen(Colors::toQColor(Colors::ON_PRIMARY));\n')
            out.append('    painter.drawText(btnRect, Qt::AlignCenter, ">");\n\n')
            
            out.append('    // Border outline\n')
            out.append('    painter.setPen(QPen(Colors::toQColor(Colors::OUTLINE_VARIANT), 1));\n')
            out.append('    painter.setBrush(Qt::NoBrush);\n')
            out.append('    painter.drawPath(cardPath);\n')
            out.append('}\n')
            
            i += 1
            while i < len(lines) and not lines[i].startswith('void GameCard::mousePressEvent'):
                i += 1
            continue

        if in_paintEvent and line.startswith('void GameCard::mousePressEvent'):
            in_paintEvent = False
            
        if not in_paintEvent:
            out.append(line)
        i += 1

    with open('src/gamecard.cpp', 'w', encoding='utf-8') as f:
        f.writelines(out)

patch_file()
