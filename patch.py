import sys

def patch_file():
    with open('src/mainwindow.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()

    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if '// Navigation tabs' in line:
            out.append('    // Navigation tabs\n')
            out.append('    m_tabLua = new GlassButton(MaterialIcons::Home, " Home", "", Colors::PRIMARY);\n')
            out.append('    m_tabLua->setFixedHeight(44);\n')
            out.append('    connect(m_tabLua, &QPushButton::clicked, this, [this](){ switchMode(AppMode::LuaPatcher); });\n')
            out.append('    sidebarInnerLayout->addWidget(m_tabLua);\n\n')
            
            out.append('    m_tabLibrary = new GlassButton(MaterialIcons::VideogameAsset, " Games", "", Colors::PRIMARY);\n')
            out.append('    m_tabLibrary->setFixedHeight(44);\n')
            out.append('    connect(m_tabLibrary, &QPushButton::clicked, this, [this](){ switchMode(AppMode::Library); });\n')
            out.append('    sidebarInnerLayout->addWidget(m_tabLibrary);\n\n')
            
            out.append('    GlassButton* m_tabStats = new GlassButton(MaterialIcons::Leaderboard, " Stats", "", Colors::PRIMARY);\n')
            out.append('    m_tabStats->setFixedHeight(44);\n')
            out.append('    sidebarInnerLayout->addWidget(m_tabStats);\n\n')
            
            out.append('    GlassButton* m_tabStore = new GlassButton(MaterialIcons::Storefront, " Store", "", Colors::PRIMARY);\n')
            out.append('    m_tabStore->setFixedHeight(44);\n')
            out.append('    sidebarInnerLayout->addWidget(m_tabStore);\n\n')
            
            out.append('    sidebarInnerLayout->addStretch();\n\n')
            
            out.append('    m_tabSettings = new GlassButton(MaterialIcons::Settings, " Settings", "", Colors::OUTLINE);\n')
            out.append('    m_tabSettings->setFixedHeight(44);\n')
            out.append('    connect(m_tabSettings, &QPushButton::clicked, this, [this](){ switchMode(AppMode::Settings); });\n')
            out.append('    sidebarInnerLayout->addWidget(m_tabSettings);\n')
            out.append('    sidebarInnerLayout->addSpacing(8);\n\n')
            
            out.append('    // Pro Pass Promo Card\n')
            out.append('    QWidget* proPassCard = new QWidget();\n')
            out.append('    proPassCard->setStyleSheet(QString(\n')
            out.append('        "QWidget { background: %1; border-radius: 12px; border: 1px solid %2; }"\n')
            out.append('    ).arg("rgba(74, 112, 169, 0.25)").arg(Colors::OUTLINE_VARIANT));\n')
            out.append('    QVBoxLayout* proPassLayout = new QVBoxLayout(proPassCard);\n')
            out.append('    proPassLayout->setContentsMargins(12, 12, 12, 12);\n\n')
            
            out.append('    QLabel* proPassTitle = new QLabel("PRO PASS");\n')
            out.append('    proPassTitle->setStyleSheet("font-weight: bold; font-size: 11px; color: " + Colors::ON_SURFACE + "; border: none; background: transparent;");\n')
            out.append('    QLabel* proPassDesc = new QLabel("Unlock premium character skins and double XP.");\n')
            out.append('    proPassDesc->setWordWrap(true);\n')
            out.append('    proPassDesc->setStyleSheet("font-size: 9px; color: " + Colors::ON_SURFACE_VARIANT + "; border: none; background: transparent;");\n\n')
            
            out.append('    QPushButton* upgradeBtn = new QPushButton("UPGRADE NOW");\n')
            out.append('    upgradeBtn->setFixedHeight(28);\n')
            out.append('    upgradeBtn->setStyleSheet(QString(\n')
            out.append('        "QPushButton { background: %1; color: %2; font-weight: bold; font-size: 10px; border-radius: 6px; border: none; padding: 0px; }"\n')
            out.append('    ).arg(Colors::PRIMARY).arg(Colors::ON_PRIMARY));\n')
            out.append('    upgradeBtn->setCursor(Qt::PointingHandCursor);\n\n')
            
            out.append('    proPassLayout->addWidget(proPassTitle);\n')
            out.append('    proPassLayout->addWidget(proPassDesc);\n')
            out.append('    proPassLayout->addWidget(upgradeBtn);\n')
            out.append('    sidebarInnerLayout->addWidget(proPassCard);\n\n')
            out.append('    m_statusLabel = new QLabel("Initializing...");\n')
            out.append('    m_statusLabel->hide();\n')
            
            # fast forward i to skip up to sidebarInnerLayout->addWidget(m_tabDiscord) approx 40 lines
            while i < len(lines) and 'sidebarInnerLayout->addWidget(m_tabDiscord);' not in lines[i]:
                i += 1
            i += 1 # skip that line too
            continue

        if 'MaterialIconButton* notifBtn = new MaterialIconButton(' in line:
            # We are at the right place to inject the right panel inside the root layout later
            out.append('    // Top Bar Right Side (Notifications + Mini Profile)\n')
            out.append('    QWidget* topActions = new QWidget();\n')
            out.append('    QHBoxLayout* topActionsLayout = new QHBoxLayout(topActions);\n')
            out.append('    topActionsLayout->setContentsMargins(0, 0, 0, 0);\n\n')
            out.append('    MaterialIconButton* notifBtn = new MaterialIconButton(\n')
            out.append('        MaterialIcons::Notifications, Colors::toQColor(Colors::ON_SURFACE_VARIANT), 40);\n')
            out.append('    topActionsLayout->addWidget(notifBtn);\n\n')
            out.append('    MaterialIconButton* profileSmallBtn = new MaterialIconButton(\n')
            out.append('        MaterialIcons::AccountCircle, Colors::toQColor(Colors::ON_SURFACE_VARIANT), 40);\n')
            out.append('    topActionsLayout->addWidget(profileSmallBtn);\n')
            out.append('    topBarLayout->addWidget(topActions);\n\n')
            out.append('    mainLayout->addWidget(topBarWidget);\n\n')
            
            out.append('    // ── Right Panel ──\n')
            out.append('    m_rightPanelWidget = new QWidget();\n')
            out.append('    m_rightPanelWidget->setFixedWidth(280);\n')
            out.append('    m_rightPanelWidget->setStyleSheet("background: transparent;");\n')
            out.append('    QVBoxLayout* rightLayout = new QVBoxLayout(m_rightPanelWidget);\n')
            out.append('    rightLayout->setContentsMargins(0, 20, 20, 20);\n')
            out.append('    rightLayout->setSpacing(24);\n\n')
            
            out.append('    m_topProfileWidget = new QWidget();\n')
            out.append('    m_topProfileWidget->setFixedHeight(160);\n')
            out.append('    m_topProfileWidget->setStyleSheet(QString("QWidget { background: %1; border-radius: 16px; }").arg(Colors::SURFACE_BRIGHT));\n')
            out.append('    QVBoxLayout* rpLayout = new QVBoxLayout(m_topProfileWidget);\n')
            out.append('    rpLayout->setContentsMargins(16, 16, 16, 16);\n\n')
            
            out.append('    QHBoxLayout* rpTopLayout = new QHBoxLayout();\n')
            out.append('    QString displayName = m_username.isEmpty() ? "Xenon_Hunter" : m_username;\n')
            out.append('    QChar firstLetter = displayName.at(0).toUpper();\n')
            out.append('    QColor avatarColor = Colors::toQColor(Colors::SECONDARY);\n')
            out.append('    int avatarSize = 48;\n')
            out.append('    QPixmap avatarPix(avatarSize, avatarSize);\n')
            out.append('    avatarPix.fill(Qt::transparent);\n')
            out.append('    QPainter avatarPainter(&avatarPix);\n')
            out.append('    avatarPainter.setRenderHint(QPainter::Antialiasing);\n')
            out.append('    avatarPainter.setBrush(avatarColor);\n')
            out.append('    avatarPainter.setPen(Qt::NoPen);\n')
            out.append('    avatarPainter.drawEllipse(0, 0, avatarSize, avatarSize);\n')
            out.append('    avatarPainter.setPen(Colors::toQColor(Colors::ON_SURFACE));\n')
            out.append('    QFont avatarFont("Segoe UI", 20, QFont::Bold);\n')
            out.append('    avatarPainter.setFont(avatarFont);\n')
            out.append('    avatarPainter.drawText(QRect(0, 0, avatarSize, avatarSize), Qt::AlignCenter, QString(firstLetter));\n')
            out.append('    avatarPainter.end();\n\n')
            
            out.append('    QLabel* avatarLabel = new QLabel();\n')
            out.append('    avatarLabel->setPixmap(avatarPix);\n')
            out.append('    avatarLabel->setFixedSize(avatarSize, avatarSize);\n')
            out.append('    rpTopLayout->addWidget(avatarLabel);\n\n')
            
            out.append('    QVBoxLayout* nameLayout = new QVBoxLayout();\n')
            out.append('    m_topUsernameLabel = new QLabel(displayName);\n')
            out.append('    m_topUsernameLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: " + Colors::ON_SURFACE + ";");\n')
            out.append('    nameLayout->addWidget(m_topUsernameLabel);\n')
            out.append('    QLabel* lvlLabel = new QLabel("LVL 42  --");\n')
            out.append('    lvlLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: " + Colors::ON_SURFACE_VARIANT + ";");\n')
            out.append('    nameLayout->addWidget(lvlLabel);\n')
            out.append('    rpTopLayout->addLayout(nameLayout);\n')
            out.append('    rpTopLayout->addStretch();\n')
            out.append('    rpLayout->addLayout(rpTopLayout);\n\n')
            
            out.append('    QHBoxLayout* rpStatsLayout = new QHBoxLayout();\n')
            out.append('    QLabel* achLabel = new QLabel("<b>128</b><br><span style=\'font-size:9px;\'>ACHIEVEMENTS</span>");\n')
            out.append('    achLabel->setAlignment(Qt::AlignCenter);\n')
            out.append('    achLabel->setStyleSheet("color: " + Colors::ON_SURFACE + ";");\n')
            out.append('    rpStatsLayout->addWidget(achLabel);\n')
            out.append('    QLabel* folLabel = new QLabel("<b>1.2K</b><br><span style=\'font-size:9px;\'>FOLLOWERS</span>");\n')
            out.append('    folLabel->setAlignment(Qt::AlignCenter);\n')
            out.append('    folLabel->setStyleSheet("color: " + Colors::ON_SURFACE + ";");\n')
            out.append('    rpStatsLayout->addWidget(folLabel);\n')
            out.append('    rpLayout->addLayout(rpStatsLayout);\n')
            out.append('    rightLayout->addWidget(m_topProfileWidget);\n\n')
            
            out.append('    QLabel* actHeader = new QLabel("LATEST ACTIVITY");\n')
            out.append('    actHeader->setStyleSheet("font-size: 11px; font-weight: bold; letter-spacing: 1px; color: " + Colors::ON_SURFACE + ";");\n')
            out.append('    rightLayout->addWidget(actHeader);\n\n')
            
            out.append('    for(int j=0; j<3; j++) {\n')
            out.append('        QWidget* actBox = new QWidget();\n')
            out.append('        actBox->setFixedHeight(40);\n')
            out.append('        QHBoxLayout* actL = new QHBoxLayout(actBox);\n')
            out.append('        actL->setContentsMargins(0,0,0,0);\n')
            out.append('        QWidget* iconBox = new QWidget();\n')
            out.append('        iconBox->setFixedSize(24,24);\n')
            out.append('        iconBox->setStyleSheet(QString("background: %1; border-radius: 6px;").arg(Colors::SURFACE_VARIANT));\n')
            out.append('        QLabel* actText = new QLabel("<b>User</b> achieved something<br><span style=\'font-size:10px; color:" + Colors::ON_SURFACE_VARIANT + ";\'>2m ago</span>");\n')
            out.append('        actText->setStyleSheet("color: " + Colors::ON_SURFACE + "; font-size: 11px;");\n')
            out.append('        actL->addWidget(iconBox);\n')
            out.append('        actL->addWidget(actText);\n')
            out.append('        rightLayout->addWidget(actBox);\n')
            out.append('    }\n')
            out.append('    rightLayout->addStretch();\n\n')
            
            out.append('    rootLayout->addWidget(contentWidget);\n')
            out.append('    rootLayout->addWidget(m_rightPanelWidget);\n')
            
            # fast forward to m_stack
            while i < len(lines) and 'm_stack = new QStackedWidget();' not in lines[i]:
                i += 1
            continue

        out.append(line)
        i += 1

    with open('src/mainwindow.cpp', 'w', encoding='utf-8') as f:
        f.writelines(out)

patch_file()
