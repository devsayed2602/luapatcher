#ifndef MATERIALICONS_H
#define MATERIALICONS_H

#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QColor>
#ifdef HAS_QT_SVG
#include <QtSvg/QSvgRenderer>
#endif
#include <QByteArray>

class MaterialIcons {
public:
    enum Icon {
        Home,
        Download,
        Build,
        Library,
        Refresh,
        Delete,
        Add,
        RestartAlt,
        Search,
        Gamepad,
        CheckCircle,
        Flash,
        Settings,
        Discord,
        Group,
        PersonAdd,
        Logout,
        Steam,
        Notifications
    };

    static void draw(QPainter& p, const QRectF& rect, const QColor& color, Icon icon) {
#ifdef HAS_QT_SVG
        // Premium SVG rendering path
        static QSvgRenderer* renderers[256] = { nullptr };
        
        if (!renderers[icon]) {
            QString svgString = getSvgString(icon);
            if (svgString.isEmpty()) return;
            renderers[icon] = new QSvgRenderer(svgString.toUtf8());
        }
        
        renderers[icon]->render(&p, rect);
#else
        // Fallback QPainterPath rendering (no Qt6Svg needed)
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.translate(rect.topLeft());
        qreal sx = rect.width() / 24.0;
        qreal sy = rect.height() / 24.0;
        p.scale(sx, sy);
        p.setPen(Qt::NoPen);
        p.setBrush(color);

        switch (icon) {
        case Home: {
            QPainterPath path;
            path.moveTo(3, 12); path.lineTo(12, 3); path.lineTo(21, 12);
            path.lineTo(19, 12); path.lineTo(19, 20); path.lineTo(15, 20);
            path.lineTo(15, 14); path.lineTo(9, 14); path.lineTo(9, 20);
            path.lineTo(5, 20); path.lineTo(5, 12); path.closeSubpath();
            p.drawPath(path);
            break;
        }
        case Library: {
            p.drawRect(QRectF(3, 4, 4, 16));
            p.drawRect(QRectF(9, 4, 4, 16));
            p.drawRect(QRectF(15, 4, 4, 16));
            break;
        }
        case Search: {
            QColor c = p.brush().color();
            p.setBrush(Qt::NoBrush);
            QPen pen(c, 2.2); pen.setCapStyle(Qt::RoundCap); p.setPen(pen);
            p.drawEllipse(QPointF(11, 11), 6, 6);
            p.drawLine(QPointF(16, 16), QPointF(20, 20));
            break;
        }
        case Refresh: {
            QColor c = p.brush().color();
            p.setBrush(Qt::NoBrush);
            QPen pen(c, 2.2); pen.setCapStyle(Qt::RoundCap); p.setPen(pen);
            p.drawArc(QRectF(4, 4, 16, 16), 90 * 16, -270 * 16);
            p.setPen(Qt::NoPen); p.setBrush(c);
            QPainterPath arrow; arrow.moveTo(20, 8); arrow.lineTo(20, 3); arrow.lineTo(15, 8); arrow.closeSubpath();
            p.drawPath(arrow);
            break;
        }
        case Settings: {
            // Gear cog icon
            QColor c = color;
            p.setBrush(Qt::NoBrush);
            QPen pen(c, 2); pen.setCapStyle(Qt::RoundCap); p.setPen(pen);
            // Outer gear teeth (8 notches via lines)
            for (int i = 0; i < 8; i++) {
                double angle = i * 45.0 * 3.14159 / 180.0;
                double ix = 12 + 7 * qCos(angle);
                double iy = 12 + 7 * qSin(angle);
                double ox = 12 + 9.5 * qCos(angle);
                double oy = 12 + 9.5 * qSin(angle);
                p.drawLine(QPointF(ix, iy), QPointF(ox, oy));
            }
            // Outer circle
            p.drawEllipse(QPointF(12, 12), 7, 7);
            // Inner filled circle
            p.setPen(Qt::NoPen); p.setBrush(c);
            p.drawEllipse(QPointF(12, 12), 3, 3);
            break;
        }
        case Steam: {
            // Circular restart arrow with power symbol
            QColor c = color;
            p.setBrush(Qt::NoBrush);
            QPen pen(c, 2.2); pen.setCapStyle(Qt::RoundCap); p.setPen(pen);
            // Circular arc (270 degrees)
            p.drawArc(QRectF(4, 4, 16, 16), 90 * 16, -270 * 16);
            // Arrow head
            p.setPen(Qt::NoPen); p.setBrush(c);
            QPainterPath arrow;
            arrow.moveTo(20, 8); arrow.lineTo(20, 3); arrow.lineTo(15, 8); arrow.closeSubpath();
            p.drawPath(arrow);
            // Power symbol: vertical line
            p.setBrush(Qt::NoBrush);
            QPen pwrPen(c, 1.8); pwrPen.setCapStyle(Qt::RoundCap); p.setPen(pwrPen);
            p.drawLine(QPointF(12, 9), QPointF(12, 13));
            // Power symbol: arc
            p.drawArc(QRectF(8, 10, 8, 8), 45 * 16, 90 * 16);
            break;
        }
        case Discord: {
            // Simplified Discord controller shape
            QPainterPath path;
            path.addRoundedRect(QRectF(4, 6, 16, 12), 4, 4);
            p.drawPath(path);
            p.setBrush(QColor(0,0,0));
            p.drawEllipse(QPointF(9, 12), 1.5, 1.5);
            p.drawEllipse(QPointF(15, 12), 1.5, 1.5);
            break;
        }
        case Gamepad: {
            QPainterPath path;
            path.addRoundedRect(QRectF(3, 7, 18, 10), 5, 5);
            p.drawPath(path);
            // D-pad
            p.setBrush(QColor(0,0,0));
            p.drawRect(QRectF(7, 10.5, 4, 1.5));
            p.drawRect(QRectF(8.25, 9, 1.5, 4.5));
            // Buttons
            p.drawEllipse(QPointF(16, 11), 1, 1);
            p.drawEllipse(QPointF(18, 13), 1, 1);
            break;
        }
        case CheckCircle: {
            QColor c = color;
            p.setBrush(Qt::NoBrush);
            QPen pen(c, 2); p.setPen(pen);
            p.drawEllipse(QPointF(12, 12), 9, 9);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            p.setPen(pen);
            p.drawLine(QPointF(8, 12), QPointF(11, 15));
            p.drawLine(QPointF(11, 15), QPointF(16, 9));
            break;
        }
        case Flash: {
            QPainterPath bolt;
            bolt.moveTo(13, 2); bolt.lineTo(6, 13); bolt.lineTo(11, 13);
            bolt.lineTo(11, 22); bolt.lineTo(18, 11); bolt.lineTo(13, 11);
            bolt.closeSubpath();
            p.drawPath(bolt);
            break;
        }
        case Notifications: {
            QPainterPath bell;
            bell.moveTo(12, 22);
            bell.arcTo(10, 20, 4, 4, 0, -180);
            p.drawPath(bell);
            p.drawPath(QPainterPath());
            QPainterPath body;
            body.moveTo(18, 16); body.lineTo(18, 11);
            body.quadTo(18, 7.9, 13.5, 4.6); body.lineTo(13.5, 4);
            body.quadTo(13.5, 2.5, 12, 2.5); body.quadTo(10.5, 2.5, 10.5, 4); body.lineTo(10.5, 4.6);
            body.quadTo(6, 7.9, 6, 11); body.lineTo(6, 16);
            body.lineTo(4, 18); body.lineTo(4, 19); body.lineTo(20, 19); body.lineTo(20, 18); body.closeSubpath();
            p.drawPath(body);
            break;
        }
        case Build: {
            // Wrench
            QPainterPath wrench;
            wrench.moveTo(14.7, 6.3); wrench.lineTo(19, 2); wrench.lineTo(22, 5);
            wrench.lineTo(17.7, 9.3);
            wrench.quadTo(18, 10.5, 18, 12);
            wrench.quadTo(18, 16, 14, 18);
            wrench.quadTo(10, 20, 6, 16);
            wrench.quadTo(2, 12, 4, 8);
            wrench.quadTo(6, 4, 10, 4);
            wrench.quadTo(12, 4, 14.7, 6.3);
            p.drawPath(wrench);
            break;
        }
        case RestartAlt:
        case Add:
        case Delete:
        case Group:
        default: {
            p.drawEllipse(QPointF(12, 12), 6, 6);
            break;
        }
        }

        p.restore();
#endif
    }

private:
#ifdef HAS_QT_SVG
    static QString getSvgString(Icon icon) {
        switch (icon) {
            case Home: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="gradHome" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" stop-color="#A8DB8F" />
                            <stop offset="100%" stop-color="#4CB8C4" />
                        </linearGradient>
                    </defs>
                    <path d="M3 10l9-7 9 7v11a2 2 0 01-2 2H5a2 2 0 01-2-2V10z" fill="url(#gradHome)" opacity="0.9"/>
                    <path d="M9 22V12h6v10" fill="#1A1C23" />
                </svg>
            )SVG";
            case Library: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="gradLib" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" stop-color="#8FABD4" />
                            <stop offset="100%" stop-color="#4A658A" />
                        </linearGradient>
                        <linearGradient id="gradLibHighlight" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" stop-color="#EFECE3" />
                            <stop offset="100%" stop-color="#A8DB8F" />
                        </linearGradient>
                    </defs>
                    <rect x="3" y="4" width="4" height="16" rx="1" fill="url(#gradLib)"/>
                    <rect x="9" y="4" width="4" height="16" rx="1" fill="url(#gradLib)"/>
                    <path d="M16.5 4.5l3.5 15-3.8.9-3.5-15 3.8-.9z" fill="url(#gradLibHighlight)"/>
                </svg>
            )SVG";
            case Settings: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="gradSet" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" stop-color="#95A5A6" />
                            <stop offset="100%" stop-color="#34495E" />
                        </linearGradient>
                    </defs>
                    <path fill-rule="evenodd" clip-rule="evenodd" d="M12 15.5C13.933 15.5 15.5 13.933 15.5 12C15.5 10.067 13.933 8.5 12 8.5C10.067 8.5 8.5 10.067 8.5 12C8.5 13.933 10.067 15.5 12 15.5ZM12 13.5C12.8284 13.5 13.5 12.8284 13.5 12C13.5 11.1716 12.8284 10.5 12 10.5C11.1716 10.5 10.5 11.1716 10.5 12C10.5 12.8284 11.1716 13.5 12 13.5Z" fill="#A8DB8F"/>
                    <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z" fill="url(#gradSet)" opacity="0.8"/>
                </svg>
            )SVG";
            case Steam: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="gradRestart" x1="4" y1="2" x2="20" y2="22" gradientUnits="userSpaceOnUse">
                            <stop offset="0%" stop-color="#7EC8E3" />
                            <stop offset="100%" stop-color="#3B82F6" />
                        </linearGradient>
                    </defs>
                    <path d="M17.65 6.35A7.96 7.96 0 0 0 12 4c-4.42 0-7.99 3.58-7.99 8s3.57 8 7.99 8c3.73 0 6.84-2.55 7.73-6" stroke="url(#gradRestart)" stroke-width="2.4" stroke-linecap="round" fill="none"/>
                    <path d="M20.5 2.5v5.5H15" stroke="url(#gradRestart)" stroke-width="2.4" stroke-linecap="round" stroke-linejoin="round" fill="none"/>
                    <line x1="12" y1="9" x2="12" y2="12.5" stroke="#EFECE3" stroke-width="1.8" stroke-linecap="round"/>
                    <path d="M9.17 10.17a4 4 0 1 0 5.66 0" stroke="#EFECE3" stroke-width="1.6" stroke-linecap="round" fill="none"/>
                </svg>
            )SVG";
            case PersonAdd: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <defs>
                        <linearGradient id="gradUser" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" stop-color="#8FABD4" />
                            <stop offset="100%" stop-color="#4A658A" />
                        </linearGradient>
                    </defs>
                    <circle cx="10" cy="7" r="4" fill="url(#gradUser)"/>
                    <path d="M18 19v-2c0-2.2-1.8-4-4-4H6c-2.2 0-4 1.8-4 4v2h16z" fill="url(#gradUser)"/>
                    <circle cx="18" cy="18" r="5" fill="#A8DB8F"/>
                    <path d="M18 16v4M16 18h4" stroke="#1A1C23" stroke-width="2" stroke-linecap="round"/>
                </svg>
            )SVG";
            case Logout: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M16 17l5-5-5-5M21 12H9" stroke="#E74C3C" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                    <path d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4" stroke="#E74C3C" stroke-width="2" stroke-linecap="round" opacity="0.6"/>
                </svg>
            )SVG";
            case Discord: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M19.3 5.4c-1.3-.6-2.8-1-4.3-1.3-.2.3-.4.7-.5 1-1.6-.2-3.1-.2-4.6 0-.1-.3-.3-.7-.5-1-1.5.3-3 .7-4.3 1.3C2 10.3 1.2 15 1.5 19.6c1.7 1.3 3.4 2.1 5 2.6.4-.5.8-1.1 1.1-1.7-1.7-.5-3.3-1.4-4.6-2.5 2 1.4 4.3 2.3 6.7 2.7 1.3.2 2.6.2 3.9 0 2.4-.4 4.7-1.3 6.7-2.7-1.3 1.1-2.9 2-4.6 2.5.3.6.7 1.2 1.1 1.7 1.6-.5 3.3-1.3 5-2.6.3-4.6-.5-9.3-3.8-14.2z" fill="#5865F2"/>
                    <circle cx="8.5" cy="12" r="1.5" fill="#FFFFFF"/>
                    <circle cx="15.5" cy="12" r="1.5" fill="#FFFFFF"/>
                </svg>
            )SVG";
            case Download: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M21 15v4a2 2 0 01-2 2H5a2 2 0 01-2-2v-4" stroke="#A8DB8F" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                    <path d="M7 10l5 5 5-5M12 15V3" stroke="#A8DB8F" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                </svg>
            )SVG";
            case Search: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <circle cx="11" cy="11" r="7" stroke="#8FABD4" stroke-width="2" stroke-linecap="round"/>
                    <path d="M20 20l-4-4" stroke="#8FABD4" stroke-width="2" stroke-linecap="round"/>
                </svg>
            )SVG";
            case Refresh: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M21 12a9 9 0 11-1.35-4.73l-2.45 2.45" stroke="#8FABD4" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                    <path d="M21 4v5.5h-5.5" stroke="#8FABD4" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                </svg>
            )SVG";
            case Notifications: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M12 22C13.1 22 14 21.1 14 20H10C10 21.1 10.9 22 12 22ZM18 16V11C18 7.9 16.3 5.3 13.5 4.6V4C13.5 3.2 12.8 2.5 12 2.5C11.2 2.5 10.5 3.2 10.5 4V4.6C7.7 5.3 6 7.9 6 11V16L4 18V19H20V18L18 16Z" fill="#8FABD4"/>
                </svg>
            )SVG";
            default: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <rect x="4" y="4" width="16" height="16" rx="4" fill="#EFECE3" opacity="0.3"/>
                </svg>
            )SVG";
        }
    }
#endif
};

#endif // MATERIALICONS_H
