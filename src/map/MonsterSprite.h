#pragma once
#include <QGraphicsPixmapItem>
#include <QString>

class MonsterSprite : public QGraphicsPixmapItem {
public:
    explicit MonsterSprite(const QString &monsterId);
    QString monsterId() const { return m_monsterId; }
    int gridX() const { return m_gridX; }
    int gridY() const { return m_gridY; }
    void setGridPos(int x, int y) { m_gridX = x; m_gridY = y; }

private:
    QString m_monsterId;
    int m_gridX = 0, m_gridY = 0;
};
