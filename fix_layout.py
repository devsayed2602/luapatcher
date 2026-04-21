import re

def main():
    # 1. Update mainwindow.cpp 7 to 5 columns + decrease banner height
    with open('src/mainwindow.cpp', 'r', encoding='utf-8') as f:
        mw = f.read()
    
    # Replace / 7 and % 7 with / 5 and % 5 in gridLayout->addWidget calls
    mw = re.sub(r'(m_gridLayout->addWidget\(card,\s*[^/]+)\s*/\s*7,\s*([^%]+)\s*%\s*7\);', 
                r'\1 / 5, \2 % 5);', mw)
                
    # Decrease hero banner height from 400 to 240
    mw = mw.replace('m_heroStack->setFixedHeight(400);', 'm_heroStack->setFixedHeight(240);')
    mw = mw.replace('slide->setFixedHeight(400);', 'slide->setFixedHeight(240);')

    with open('src/mainwindow.cpp', 'w', encoding='utf-8') as f:
        f.write(mw)
    
    # 2. Update gamecard.cpp sizes and skeleton coloring
    with open('src/gamecard.cpp', 'r', encoding='utf-8') as f:
        gc = f.read()

    # Change card size 190x285 -> 174x261
    gc = gc.replace('setFixedSize(190, 285);', 'setFixedSize(174, 261);')
    
    # Change skeleton loading colors to match dark background
    old_base_color = 'QColor baseColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGH);'
    new_base_color = 'QColor baseColor(255, 255, 255, 10); // Matches dark background better'
    gc = gc.replace(old_base_color, new_base_color)

    old_pulse_color = 'QColor pulseColor = Colors::toQColor(Colors::SURFACE_CONTAINER_HIGHEST);'
    new_pulse_color = 'QColor pulseColor(255, 255, 255, 20); // Subtle pulse'
    gc = gc.replace(old_pulse_color, new_pulse_color)

    # Change thumbnail placeholder color from surface variant to translucent black/white
    old_thumb_color = 'QColor thumbColor = Colors::toQColor(Colors::SURFACE_VARIANT);'
    new_thumb_color = 'QColor thumbColor(255, 255, 255, 12);'
    gc = gc.replace(old_thumb_color, new_thumb_color)
    
    with open('src/gamecard.cpp', 'w', encoding='utf-8') as f:
        f.write(gc)

    print("Success!")

if __name__ == "__main__":
    main()
