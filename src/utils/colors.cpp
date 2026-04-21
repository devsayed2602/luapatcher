#include "colors.h"

// Ignite Gaming Dashboard Theme - Surface colors
const QString Colors::SURFACE              = "#000000"; // Absolute black background
const QString Colors::SURFACE_DIM          = "rgba(0, 0, 0, 0.9)";
const QString Colors::SURFACE_BRIGHT       = "#8FABD4"; // Dark blue base
const QString Colors::SURFACE_CONTAINER    = "rgba(74, 112, 169, 0.15)"; // Very subtle dark blue for cards overlay
const QString Colors::SURFACE_CONTAINER_HIGH    = "rgba(74, 112, 169, 0.25)";
const QString Colors::SURFACE_CONTAINER_HIGHEST = "rgba(74, 112, 169, 0.35)";
const QString Colors::SURFACE_VARIANT      = "rgba(74, 112, 169, 0.50)";
const QString Colors::ON_SURFACE           = "#EFECE3"; // Cream text
const QString Colors::ON_SURFACE_VARIANT   = "#8FABD4"; // Light blue secondary text
const QString Colors::OUTLINE              = "rgba(143, 171, 212, 0.25)"; // Light blue outline
const QString Colors::OUTLINE_VARIANT      = "rgba(143, 171, 212, 0.10)";

// Primary (Cream Accent)
const QString Colors::PRIMARY              = "#EFECE3"; // Action buttons
const QString Colors::ON_PRIMARY           = "#000000";
const QString Colors::PRIMARY_CONTAINER    = "rgba(239, 236, 227, 0.2)";
const QString Colors::ON_PRIMARY_CONTAINER = "#EFECE3";

// Secondary (Light Blue Accent)
const QString Colors::SECONDARY            = "#8FABD4";
const QString Colors::ON_SECONDARY         = "#000000";
const QString Colors::SECONDARY_CONTAINER  = "rgba(143, 171, 212, 0.2)";

// Tertiary (Dark Blue)
const QString Colors::TERTIARY             = "#8FABD4";
const QString Colors::ON_TERTIARY          = "#EFECE3";
const QString Colors::TERTIARY_CONTAINER   = "rgba(74, 112, 169, 0.6)";

// Error
const QString Colors::ERROR                = "#F2B8B5";
const QString Colors::ON_ERROR             = "#601410";
const QString Colors::ERROR_CONTAINER      = "#8C1D18";

// Accent aliases
const QString Colors::ACCENT_BLUE          = "#8FABD4";
const QString Colors::ACCENT_PURPLE        = "#8FABD4";
const QString Colors::ACCENT_GREEN         = "#EFECE3";
const QString Colors::ACCENT_RED           = "#F2B8B5";

// Legacy aliases (mapped to new system)
const QString Colors::BG_GRADIENT_START    = "#000000";
const QString Colors::BG_GRADIENT_END      = "#000000";
const QString Colors::GLASS_BG             = "rgba(74, 112, 169, 0.15)";
const QString Colors::GLASS_HOVER          = "rgba(74, 112, 169, 0.25)";
const QString Colors::GLASS_BORDER         = "rgba(143, 171, 212, 0.25)";
const QString Colors::TEXT_PRIMARY         = "#EFECE3";
const QString Colors::TEXT_SECONDARY       = "#8FABD4";

QColor Colors::toQColor(const QString& colorStr) {
    // QColor cannot parse CSS rgba() strings, so we handle it manually
    if (colorStr.startsWith("rgba(") && colorStr.endsWith(")")) {
        QString inner = colorStr.mid(5, colorStr.length() - 6);
        QStringList parts = inner.split(",");
        if (parts.size() == 4) {
            int r = parts[0].trimmed().toInt();
            int g = parts[1].trimmed().toInt();
            int b = parts[2].trimmed().toInt();
            double aVal = parts[3].trimmed().toDouble();
            int a = (aVal <= 1.0) ? qRound(aVal * 255.0) : qRound(aVal);
            return QColor(r, g, b, a);
        }
    }
    return QColor(colorStr);
}

DynamicTheme Colors::currentTheme;

void Colors::extractFromPixmap(const QPixmap& pixmap) {
    if (pixmap.isNull()) return;

    QImage img = pixmap.toImage().scaled(50, 50, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    
    QColor mostVibrant = QColor("#D0BCFF");
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
    
    if (maxScore > 0) {
        currentTheme.primary = mostVibrant;
        
        int h, s, l, a;
        mostVibrant.getHsl(&h, &s, &l, &a);
        
        QColor secondary;
        secondary.setHsl((h + 30) % 360, qMax(0, s - 20), l, a);
        currentTheme.secondary = secondary;
        
        currentTheme.glow = mostVibrant;
        currentTheme.glow.setAlpha(80);
        
        QColor accent;
        accent.setHsl((h + 180) % 360, s, qMin(255, l + 20), a); // Complementary hue for accent
        currentTheme.accent = accent;
    }
}
