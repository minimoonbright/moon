#pragma once
#include <QGraphicsScene>
#include "core/Types.h"

class PlayerSprite;
class NpcSprite;
class MonsterSprite;

class MapScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit MapScene(QObject *parent = nullptr);

    void loadMap(const MapData &mapData, int startX = -1, int startY = -1);
    PlayerSprite *player() const { return m_player; }

    bool movePlayer(int dx, int dy);
    void interactAt(int x, int y);

signals:
    void mapExit(const QString &targetMapId);
    void battleTriggered(const QString &monsterId);
    void npcInteracted(const QString &npcId);
    void buildingEntered(const QString &buildingId);

private:
    void drawMapGrid();
    void placeEntities();
    bool isWalkable(int x, int y) const;

    const MapData *m_mapData = nullptr;
    PlayerSprite *m_player = nullptr;
    QVector<NpcSprite *> m_npcs;
    QVector<MonsterSprite *> m_monsters;
    int m_tileSize = 64;
};
