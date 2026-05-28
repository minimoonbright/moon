#pragma once
#include <QGraphicsPixmapItem>

class PlayerSprite : public QGraphicsPixmapItem {
public:
    explicit PlayerSprite();

    int gridX() const { return m_gridX; }
    int gridY() const { return m_gridY; }
    void setGridPos(int x, int y) { m_gridX = x; m_gridY = y; }

private:
    int m_gridX = 0, m_gridY = 0;
};
