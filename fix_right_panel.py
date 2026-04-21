import sys

def fix_file():
    with open('src/mainwindow.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()

    out = []
    i = 0
    in_replace = False
    while i < len(lines):
        line = lines[i]

        if 'QHBoxLayout* rpTopLayout = new QHBoxLayout();' in line and not in_replace:
            in_replace = True
            
            # Write the new profile widget layout
            out.append('    QHBoxLayout* rpTopLayout = new QHBoxLayout();\n')
            out.append('    rpTopLayout->setSpacing(12);\n')
            out.append('    QString displayName = m_username.isEmpty() ? "Xenon_Hunter" : m_username;\n')
            out.append('    QChar firstLetter = displayName.at(0).toUpper();\n')
            
            # Wrap avatar in a container so we can draw the green dot
            out.append('    QWidget* avatarContainer = new QWidget();\n')
            out.append('    avatarContainer->setFixedSize(54, 54);\n')
            out.append('    avatarContainer->setStyleSheet("background: transparent;");\n')
            
            # Pre-rendered avatar with green dot overlay
            out.append('    QPixmap avatarPix(54, 54);\n')
            out.append('    avatarPix.fill(Qt::transparent);\n')
            out.append('    QPainter avatarPainter(&avatarPix);\n')
            out.append('    avatarPainter.setRenderHint(QPainter::Antialiasing);\n')
            
            # Temporary static background avatar (just a placeholder color representing the stadium from image 2)
            out.append('    avatarPainter.setBrush(QColor("#4A70A9"));\n')
            out.append('    avatarPainter.setPen(QPen(Colors::toQColor(Colors::ON_SURFACE), 2)); // White rim\n')
            out.append('    avatarPainter.drawEllipse(2, 2, 48, 48);\n')
            out.append('    avatarPainter.setPen(Colors::toQColor(Colors::ON_SURFACE));\n')
            out.append('    QFont avatarFont("Segoe UI", 20, QFont::Bold);\n')
            out.append('    avatarPainter.setFont(avatarFont);\n')
            out.append('    avatarPainter.drawText(QRect(2, 2, 48, 48), Qt::AlignCenter, QString(firstLetter));\n')
            
            # Green dot online indicator
            out.append('    avatarPainter.setBrush(QColor("#2ECC71"));\n')
            out.append('    avatarPainter.setPen(QPen(Colors::toQColor(Colors::SURFACE_BRIGHT), 2)); // Rim matching background\n')
            out.append('    avatarPainter.drawEllipse(38, 38, 14, 14);\n')
            out.append('    avatarPainter.end();\n')
            
            out.append('    QLabel* avatarLabel = new QLabel(avatarContainer);\n')
            out.append('    avatarLabel->setPixmap(avatarPix);\n')
            out.append('    avatarLabel->setGeometry(0, 0, 54, 54);\n')
            out.append('    rpTopLayout->addWidget(avatarContainer);\n\n')
            
            out.append('    QVBoxLayout* nameLayout = new QVBoxLayout();\n')
            out.append('    nameLayout->setSpacing(4);\n')
            out.append('    nameLayout->setAlignment(Qt::AlignVCenter);\n')
            out.append('    m_topUsernameLabel = new QLabel(displayName);\n')
            out.append('    m_topUsernameLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: white; background: transparent; padding-top: 5px;");\n')
            out.append('    nameLayout->addWidget(m_topUsernameLabel);\n')
            
            # Level pill and progress bar
            out.append('    QHBoxLayout* lvlLayout = new QHBoxLayout();\n')
            out.append('    lvlLayout->setSpacing(8);\n')
            out.append('    QLabel* lvlPill = new QLabel("LVL 42");\n')
            out.append('    lvlPill->setStyleSheet("background: rgba(255,255,255,0.1); border-radius: 4px; padding: 2px 6px; font-size: 10px; font-weight: bold; color: white;");\n')
            out.append('    lvlLayout->addWidget(lvlPill);\n')
            
            # Progress bar
            out.append('    QWidget* progBar = new QWidget();\n')
            out.append('    progBar->setFixedSize(60, 4);\n')
            out.append('    progBar->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #EFECE3, stop:0.75 #EFECE3, stop:0.76 rgba(255,255,255,0.1), stop:1 rgba(255,255,255,0.1)); border-radius: 2px;");\n')
            out.append('    lvlLayout->addWidget(progBar);\n')
            out.append('    lvlLayout->addStretch();\n')
            out.append('    nameLayout->addLayout(lvlLayout);\n')
            out.append('    rpTopLayout->addLayout(nameLayout);\n')
            out.append('    rpTopLayout->addStretch();\n')
            out.append('    rpLayout->addLayout(rpTopLayout);\n\n')
            
            out.append('    QHBoxLayout* rpStatsLayout = new QHBoxLayout();\n')
            out.append('    rpStatsLayout->setSpacing(8);\n')
            
            out.append('    QWidget* achBox = new QWidget();\n')
            out.append('    achBox->setFixedHeight(65);\n')
            out.append('    achBox->setStyleSheet("background: rgba(143, 171, 212, 0.15); border-radius: 12px;");\n')
            out.append('    QVBoxLayout* achL = new QVBoxLayout(achBox);\n')
            out.append('    achL->setAlignment(Qt::AlignCenter);\n')
            out.append('    achL->setContentsMargins(0, 0, 0, 0);\n')
            out.append('    achL->setSpacing(4);\n')
            
            out.append('    QLabel* achNum = new QLabel("128");\n')
            out.append('    achNum->setStyleSheet("color: white; font-size: 14px; font-weight: 900; background: transparent;");\n')
            out.append('    achNum->setAlignment(Qt::AlignCenter);\n')
            out.append('    QLabel* achText = new QLabel("ACHIEVEMENTS");\n')
            out.append('    achText->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; background: transparent;").arg(Colors::ON_SURFACE_VARIANT));\n')
            out.append('    achText->setAlignment(Qt::AlignCenter);\n')
            out.append('    achL->addWidget(achNum);\n')
            out.append('    achL->addWidget(achText);\n')
            out.append('    rpStatsLayout->addWidget(achBox);\n\n')
            
            out.append('    QWidget* folBox = new QWidget();\n')
            out.append('    folBox->setFixedHeight(65);\n')
            out.append('    folBox->setStyleSheet("background: rgba(143, 171, 212, 0.15); border-radius: 12px;");\n')
            out.append('    QVBoxLayout* folL = new QVBoxLayout(folBox);\n')
            out.append('    folL->setAlignment(Qt::AlignCenter);\n')
            out.append('    folL->setContentsMargins(0, 0, 0, 0);\n')
            out.append('    folL->setSpacing(4);\n')
            
            out.append('    QLabel* folNum = new QLabel("1.2K");\n')
            out.append('    folNum->setStyleSheet("color: white; font-size: 14px; font-weight: 900; background: transparent;");\n')
            out.append('    folNum->setAlignment(Qt::AlignCenter);\n')
            out.append('    QLabel* folText = new QLabel("FOLLOWERS");\n')
            out.append('    folText->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; background: transparent;").arg(Colors::ON_SURFACE_VARIANT));\n')
            out.append('    folText->setAlignment(Qt::AlignCenter);\n')
            out.append('    folL->addWidget(folNum);\n')
            out.append('    folL->addWidget(folText);\n')
            out.append('    rpStatsLayout->addWidget(folBox);\n\n')
            
            out.append('    rpLayout->addLayout(rpStatsLayout);\n')
            out.append('    rightLayout->addWidget(m_topProfileWidget);\n')
            
            # Fast forward until rightLayout->addWidget(m_topProfileWidget);
            while i < len(lines) and 'rightLayout->addWidget(m_topProfileWidget);' not in lines[i]:
                i += 1
            i += 1
            continue

        out.append(line)
        i += 1

    with open('src/mainwindow.cpp', 'w', encoding='utf-8') as f:
        f.writelines(out)

fix_file()
