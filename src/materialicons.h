#ifndef MATERIALICONS_H
#define MATERIALICONS_H

#include <QPainter>
#include <QRect>
#include <QColor>
#include <QtSvg/QSvgRenderer>
#include <QByteArray>
#include <QMap>

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
        Steam
    };

    static void draw(QPainter& p, const QRectF& rect, const QColor& color, Icon icon) {
        QString svgString = getSvgString(icon);
        if (svgString.isEmpty()) return;
        
        // Dynamically replace the keyword "currentColor" with the passed color if needed
        // but for premium UI, we use the embedded gradients!
        
        QSvgRenderer renderer(svgString.toUtf8());
        renderer.render(&p, rect);
    }

private:
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
                        <linearGradient id="gradSteamAccent" x1="0%" y1="0%" x2="100%" y2="100%">
                            <stop offset="0%" stop-color="#66C0F4" />
                            <stop offset="100%" stop-color="#19548E" />
                        </linearGradient>
                    </defs>
                    <path d="M12 0C5.373 0 0 5.373 0 12c0 5.485 3.666 10.126 8.653 11.534l3.19-4.577c-.125-.133-.2-.314-.2-.516 0-.348.232-.647.545-.765l.775-2.235c-.035-.11-.055-.226-.055-.347 0-1.066.864-1.93 1.93-1.93 1.066 0 1.93.864 1.93 1.93 0 1.066-.864 1.93-1.93 1.93-.844 0-1.554-.54-1.81-1.286l-2.298.796c.114.305.176.634.176.976 0 1.58-1.28 2.86-2.86 2.86-.33 0-.648-.056-.94-.158l-3.21 4.606C7.382 23.633 9.615 24 12 24c6.627 0 12-5.373 12-12S18.627 0 12 0zm4.84 8.78c-1.396 0-2.527 1.13-2.527 2.526 0 1.396 1.13 2.527 2.527 2.527 1.396 0 2.527-1.13 2.527-2.527 0-1.396-1.13-2.527-2.527-2.527zm0 1.1c.787 0 1.427.64 1.427 1.426 0 .788-.64 1.428-1.427 1.428-.787 0-1.426-.64-1.426-1.428 0-.787.64-1.426 1.426-1.426zm-7.666 7.424c-1.136 0-2.056-.92-2.056-2.056 0-1.136.92-2.057 2.056-2.057 1.137 0 2.057.92 2.057 2.057 0 1.136-.92 2.056-2.057 2.056zm0-.5c.857 0 1.556-.7 1.556-1.557 0-.856-.7-1.556-1.556-1.556-.856 0-1.556.7-1.556 1.556 0 .857.7 1.557 1.556 1.557z" fill="url(#gradSteamAccent)"/>
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
            // Fallback for others - simple geometric shapes or generic code
            default: return R"SVG(
                <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <rect x="4" y="4" width="16" height="16" rx="4" fill="#EFECE3" opacity="0.3"/>
                </svg>
            )SVG";
        }
    }
};

#endif // MATERIALICONS_H
