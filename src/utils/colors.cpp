#include "colors.h"

// Liquid Glass Theme - Surface colors
const QString Colors::SURFACE              = "rgba(6, 8, 15, 0.95)";
const QString Colors::SURFACE_DIM          = "rgba(4, 6, 12, 0.98)";
const QString Colors::SURFACE_BRIGHT       = "rgba(20, 24, 34, 0.85)";
const QString Colors::SURFACE_CONTAINER    = "rgba(10, 12, 20, 0.70)";
const QString Colors::SURFACE_CONTAINER_HIGH    = "rgba(14, 18, 28, 0.80)";
const QString Colors::SURFACE_CONTAINER_HIGHEST = "rgba(20, 24, 36, 0.85)";
const QString Colors::SURFACE_VARIANT      = "rgba(30, 34, 48, 0.60)";
const QString Colors::ON_SURFACE           = "#E6E1E5";
const QString Colors::ON_SURFACE_VARIANT   = "#CAC4D0";
const QString Colors::OUTLINE              = "rgba(255, 255, 255, 0.15)";
const QString Colors::OUTLINE_VARIANT      = "rgba(255, 255, 255, 0.08)";

// Primary (Purple) - unchanged
const QString Colors::PRIMARY              = "#D0BCFF";
const QString Colors::ON_PRIMARY           = "#381E72";
const QString Colors::PRIMARY_CONTAINER    = "#4F378B";
const QString Colors::ON_PRIMARY_CONTAINER = "#EADDFF";

// Secondary - unchanged
const QString Colors::SECONDARY            = "#CCC2DC";
const QString Colors::ON_SECONDARY         = "#332D41";
const QString Colors::SECONDARY_CONTAINER  = "#4A4458";

// Tertiary - unchanged
const QString Colors::TERTIARY             = "#EFB8C8";
const QString Colors::ON_TERTIARY          = "#492532";
const QString Colors::TERTIARY_CONTAINER   = "#633B48";

// Error - unchanged
const QString Colors::ERROR                = "#F2B8B5";
const QString Colors::ON_ERROR             = "#601410";
const QString Colors::ERROR_CONTAINER      = "#8C1D18";

// Accent aliases - unchanged
const QString Colors::ACCENT_BLUE          = "#D0BCFF";  // Maps to PRIMARY
const QString Colors::ACCENT_PURPLE        = "#CCC2DC";  // Maps to SECONDARY
const QString Colors::ACCENT_GREEN         = "#A8DB8F";  // Material green tone
const QString Colors::ACCENT_RED           = "#F2B8B5";  // Maps to ERROR

// Legacy aliases (mapped to black surface system)
const QString Colors::BG_GRADIENT_START    = "#000000";  // SURFACE
const QString Colors::BG_GRADIENT_END      = "#000000";  // SURFACE_DIM
const QString Colors::GLASS_BG             = "rgba(10, 10, 10, 220)";   // SURFACE_CONTAINER
const QString Colors::GLASS_HOVER          = "rgba(20, 20, 20, 230)";   // SURFACE_CONTAINER_HIGH
const QString Colors::GLASS_BORDER         = "rgba(110, 110, 110, 80)"; // OUTLINE
const QString Colors::TEXT_PRIMARY         = "#E6E1E5";  // ON_SURFACE
const QString Colors::TEXT_SECONDARY       = "#CAC4D0";  // ON_SURFACE_VARIANT

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
