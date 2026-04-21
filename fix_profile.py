"""Fix profile card to match image 2, revert game cards, shrink sidebars."""

def fix():
    with open('src/mainwindow.cpp', 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Decrease left sidebar: 260 -> 220
    content = content.replace(
        'm_sidebarWidget->setFixedWidth(260);',
        'm_sidebarWidget->setFixedWidth(220);'
    )
    
    # 2. Decrease right panel: 320 -> 280
    content = content.replace(
        'm_rightPanelWidget->setFixedWidth(320);',
        'm_rightPanelWidget->setFixedWidth(280);'
    )
    
    # 3. Right panel margins smaller
    content = content.replace(
        'rightLayout->setContentsMargins(25, 25, 25, 25);',
        'rightLayout->setContentsMargins(16, 20, 16, 20);'
    )
    
    # 4. Profile card height: 120 -> 200 (need room for avatar + name + stats)  
    content = content.replace(
        'm_topProfileWidget->setFixedHeight(120);',
        'm_topProfileWidget->setFixedHeight(210);'
    )
    
    # 5. Profile card background - match image 2: dark navy card (#1B222C-ish)
    content = content.replace(
        'm_topProfileWidget->setStyleSheet("QWidget { background: rgba(255, 255, 255, 13); border-radius: 20px; }");',
        'm_topProfileWidget->setStyleSheet("QWidget { background: rgba(30, 42, 58, 220); border-radius: 16px; }");'
    )
    
    # 6. Profile card layout - change from side-by-side to stacked (avatar centered on top)
    # Replace rpLayout margins for more breathing room
    content = content.replace(
        'rpLayout->setContentsMargins(16, 16, 16, 16);',
        'rpLayout->setContentsMargins(16, 20, 16, 12);\n    rpLayout->setSpacing(12);'
    )
    
    # 7. Change rpTopLayout from horizontal to center the avatar
    # Replace the entire rpTopLayout section
    old_top = '''    QHBoxLayout* rpTopLayout = new QHBoxLayout();
    rpTopLayout->setSpacing(12);
    QString displayName = m_username.isEmpty() ? "Xenon_Hunter" : m_username;
    QChar firstLetter = displayName.at(0).toUpper();
    QWidget* avatarContainer = new QWidget();
    avatarContainer->setFixedSize(54, 54);
    avatarContainer->setStyleSheet("background: transparent;");
    QPixmap avatarPix(54, 54);
    avatarPix.fill(Qt::transparent);
    QPainter avatarPainter(&avatarPix);
    avatarPainter.setRenderHint(QPainter::Antialiasing);
    avatarPainter.setBrush(QColor("#4A70A9"));
    avatarPainter.setPen(QPen(Colors::toQColor(Colors::ON_SURFACE), 2)); // White rim
    avatarPainter.drawEllipse(2, 2, 48, 48);
    avatarPainter.setPen(Colors::toQColor(Colors::ON_SURFACE));
    QFont avatarFont("Segoe UI", 20, QFont::Bold);
    avatarPainter.setFont(avatarFont);
    avatarPainter.drawText(QRect(2, 2, 48, 48), Qt::AlignCenter, QString(firstLetter));
    avatarPainter.setBrush(QColor("#2ECC71"));
    avatarPainter.setPen(QPen(Colors::toQColor(Colors::SURFACE_BRIGHT), 2)); // Rim matching background
    avatarPainter.drawEllipse(38, 38, 14, 14);
    avatarPainter.end();
    QLabel* avatarLabel = new QLabel(avatarContainer);
    avatarLabel->setPixmap(avatarPix);
    avatarLabel->setGeometry(0, 0, 54, 54);
    rpTopLayout->addWidget(avatarContainer);

    QVBoxLayout* nameLayout = new QVBoxLayout();
    nameLayout->setSpacing(4);
    nameLayout->setAlignment(Qt::AlignVCenter);
    m_topUsernameLabel = new QLabel(displayName);
    m_topUsernameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: white; background: transparent;");
    nameLayout->addWidget(m_topUsernameLabel);
    QHBoxLayout* lvlLayout = new QHBoxLayout();
    lvlLayout->setSpacing(8);
    QLabel* lvlPill = new QLabel("LVL 42");
    lvlPill->setStyleSheet("background: rgba(255,255,255,0.1); border-radius: 4px; padding: 2px 6px; font-size: 10px; font-weight: bold; color: white;");
    lvlLayout->addWidget(lvlPill);
    QWidget* progBar = new QWidget();
    progBar->setFixedSize(60, 4);
    progBar->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #EFECE3, stop:0.75 #EFECE3, stop:0.76 rgba(255,255,255,0.1), stop:1 rgba(255,255,255,0.1)); border-radius: 2px;");
    lvlLayout->addWidget(progBar);
    lvlLayout->addStretch();
    nameLayout->addLayout(lvlLayout);
    rpTopLayout->addLayout(nameLayout);
    rpTopLayout->addStretch();
    rpLayout->addLayout(rpTopLayout);'''

    new_top = '''    // -- Profile top section: avatar centered, name + lvl to the right --
    QHBoxLayout* rpTopLayout = new QHBoxLayout();
    rpTopLayout->setSpacing(14);
    rpTopLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QString displayName = m_username.isEmpty() ? "Xenon_Hunter" : m_username;
    QChar firstLetter = displayName.at(0).toUpper();

    // Avatar: 64x64 with white ring and green online dot (matching image 2)
    int avSz = 64;
    QWidget* avatarContainer = new QWidget();
    avatarContainer->setFixedSize(avSz + 4, avSz + 4);
    avatarContainer->setStyleSheet("background: transparent;");
    QPixmap avatarPix(avSz + 4, avSz + 4);
    avatarPix.fill(Qt::transparent);
    QPainter avatarPainter(&avatarPix);
    avatarPainter.setRenderHint(QPainter::Antialiasing);
    // White outer ring
    avatarPainter.setPen(QPen(QColor(180, 190, 200), 2));
    avatarPainter.setBrush(Qt::NoBrush);
    avatarPainter.drawEllipse(1, 1, avSz + 1, avSz + 1);
    // Inner filled circle
    avatarPainter.setBrush(QColor("#4A70A9"));
    avatarPainter.setPen(Qt::NoPen);
    avatarPainter.drawEllipse(3, 3, avSz - 2, avSz - 2);
    // Letter
    avatarPainter.setPen(QColor(255, 255, 255));
    QFont avatarFont("Segoe UI", 22, QFont::Bold);
    avatarPainter.setFont(avatarFont);
    avatarPainter.drawText(QRect(3, 3, avSz - 2, avSz - 2), Qt::AlignCenter, QString(firstLetter));
    // Green online dot (bottom-left, matching image 2)
    avatarPainter.setBrush(QColor("#2ECC71"));
    avatarPainter.setPen(QPen(QColor(30, 42, 58), 3));
    avatarPainter.drawEllipse(6, avSz - 14, 16, 16);
    avatarPainter.end();
    QLabel* avatarLabel = new QLabel(avatarContainer);
    avatarLabel->setPixmap(avatarPix);
    avatarLabel->setGeometry(0, 0, avSz + 4, avSz + 4);
    rpTopLayout->addWidget(avatarContainer);

    // Name + LVL pill to the right of avatar
    QVBoxLayout* nameLayout = new QVBoxLayout();
    nameLayout->setSpacing(6);
    nameLayout->setAlignment(Qt::AlignVCenter);
    m_topUsernameLabel = new QLabel(displayName);
    m_topUsernameLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: white; background: transparent;");
    nameLayout->addWidget(m_topUsernameLabel);

    // LVL pill + progress bar row
    QHBoxLayout* lvlLayout = new QHBoxLayout();
    lvlLayout->setSpacing(6);
    QLabel* lvlPill = new QLabel("LVL 42");
    lvlPill->setFixedHeight(18);
    lvlPill->setStyleSheet("background: rgba(74, 112, 169, 0.5); border-radius: 4px; padding: 1px 8px; font-size: 10px; font-weight: bold; color: white;");
    lvlLayout->addWidget(lvlPill);
    // Progress bar
    QProgressBar* xpBar = new QProgressBar();
    xpBar->setFixedSize(50, 4);
    xpBar->setRange(0, 100);
    xpBar->setValue(75);
    xpBar->setTextVisible(false);
    xpBar->setStyleSheet(
        "QProgressBar { background: rgba(255,255,255,0.08); border-radius: 2px; border: none; }"
        "QProgressBar::chunk { background: white; border-radius: 2px; }"
    );
    lvlLayout->addWidget(xpBar, 0, Qt::AlignVCenter);
    lvlLayout->addStretch();
    nameLayout->addLayout(lvlLayout);
    rpTopLayout->addLayout(nameLayout);
    rpTopLayout->addStretch();
    rpLayout->addLayout(rpTopLayout);'''

    content = content.replace(old_top, new_top)

    # 8. Sidebar inner margins - slightly tighter for 220px
    content = content.replace(
        'sidebarInnerLayout->setContentsMargins(30, 30, 30, 30);',
        'sidebarInnerLayout->setContentsMargins(20, 24, 20, 16);'
    )
    content = content.replace(
        'sidebarInnerLayout->setSpacing(20);',
        'sidebarInnerLayout->setSpacing(12);'
    )
    
    # 9. Spacer after header - tighter
    content = content.replace(
        'sidebarInnerLayout->addSpacing(40);',
        'sidebarInnerLayout->addSpacing(30);'
    )

    with open('src/mainwindow.cpp', 'w', encoding='utf-8') as f:
        f.write(content)
    print("done - mainwindow patched")

fix()
