#ifndef GAMECARD_H
#define GAMECARD_H

#include <QWidget>
#include <QPixmap>
#include <QMap>
#include <QTimer>
#include <QPropertyAnimation>

class GameCard : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal hoverOpacity READ hoverOpacity WRITE setHoverOpacity)

public:
    explicit GameCard(QWidget* parent = nullptr);

    void setGameData(const QMap<QString, QString>& data);
    QMap<QString, QString> gameData() const;

    void setThumbnail(const QPixmap& pixmap);
    bool hasThumbnail() const;

    void setSelected(bool selected);
    bool isSelected() const;

    void setSelectable(bool selectable);
    bool isSelectable() const { return m_isSelectable; }

    QString appId() const;
    QColor getDominantColor() const;

    void setSkeleton(bool skeleton);
    bool isSkeleton() const;

    qreal hoverOpacity() const { return m_hoverOpacity; }
    void setHoverOpacity(qreal v) { m_hoverOpacity = v; update(); }

signals:
    void clicked(GameCard* card);
    void selectionChanged(bool selected, GameCard* card);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void updateSkeletonPulse();

private:
    QMap<QString, QString> m_data;
    QPixmap m_thumbnail;
    bool m_hasThumbnail = false;
    bool m_selected = false;
    bool m_isSelectable = false;
    bool m_hovered = false;
    bool m_isSkeleton = false;
    QTimer* m_skeletonTimer = nullptr;
    qreal m_skeletonPulse = 0.0;
    bool m_pulseIncreasing = true;
    QColor m_dominantColor;
    qreal m_hoverOpacity = 0.0;
    QPropertyAnimation* m_hoverAnim = nullptr;
    void extractDominantColor(const QPixmap& pixmap);
};

#endif // GAMECARD_H
