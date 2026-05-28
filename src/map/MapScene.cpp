#include "MapScene.h"
#include "PlayerSprite.h"
#include "NpcSprite.h"
#include "MonsterSprite.h"
#include <QPen>
#include <QBrush>
#include <QFontMetrics>
#include <QPixmap>

MapScene::MapScene(QObject *parent) : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 960, 540);
}

void MapScene::loadMap(const MapData &mapData, int startX, int startY)
{
    m_mapData = &mapData;
    clear();
    m_npcs.clear();
    m_monsters.clear();

    drawMapGrid();
    placeEntities();

    int px, py;
    if (startX >= 0 && startY >= 0) {
        px = startX; py = startY;
    } else {
        px = mapData.width / 2;
        py = mapData.height / 2;
        if (!mapData.exits.isEmpty()) {
            px = mapData.exits[0].x;
            py = mapData.exits[0].y;
        }
    }
    m_player = new PlayerSprite();
    m_player->setPos(px * m_tileSize + m_tileSize / 2,
                     py * m_tileSize + m_tileSize / 2);
    m_player->setGridPos(px, py);
    addItem(m_player);
}

void MapScene::drawMapGrid()
{
    for (int y = 0; y < m_mapData->height; ++y) {
        for (int x = 0; x < m_mapData->width; ++x) {
            QColor color;
            int tile = (y < m_mapData->tiles.size() && x < m_mapData->tiles[y].size())
                           ? m_mapData->tiles[y][x] : 0;
            switch (tile) {
                case 0: color = QColor(90, 150, 70); break;
                case 1: color = QColor(160, 130, 80); break;
                case 2: color = QColor(60, 100, 50); break;
                case 3: color = QColor(100, 140, 180); break;
                default: color = QColor(90, 150, 70); break;
            }
            addRect(x * m_tileSize, y * m_tileSize,
                    m_tileSize, m_tileSize,
                    QPen(QColor(60, 100, 40), 1), QBrush(color));
        }
    }
}

void MapScene::placeEntities()
{
    for (const auto &ent : m_mapData->entities) {
        if (ent.type == "npc") {
            auto *npc = new NpcSprite(ent.targetId);
            npc->setPos(ent.x * m_tileSize + m_tileSize / 2,
                        ent.y * m_tileSize + m_tileSize / 2);
            addItem(npc);
            m_npcs.append(npc);
        } else if (ent.type == "monster") {
            auto *monster = new MonsterSprite(ent.targetId);
            monster->setGridPos(ent.x, ent.y);
            monster->setPos(ent.x * m_tileSize + m_tileSize / 2,
                            ent.y * m_tileSize + m_tileSize / 2);
            addItem(monster);
            m_monsters.append(monster);
        } else if (ent.type == "building") {
            bool hasImage = false;
            // 加载建筑图片
            QString imgPath;
            if (ent.targetId == "barracks")   imgPath = "D:/MyProject/game/image/jun_ying.png";
            else if (ent.targetId == "inn")        imgPath = "D:/MyProject/game/image/ke_zhan.png";
            else if (ent.targetId == "blacksmith") imgPath = "D:/MyProject/game/image/tie_jiang_pu.png";

            if (!imgPath.isEmpty()) {
                QPixmap img(imgPath);
                if (!img.isNull()) {
                    img = img.scaled(m_tileSize, m_tileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    auto *pixItem = addPixmap(img);
                    pixItem->setPos(ent.x * m_tileSize + (m_tileSize - img.width()) / 2,
                                    ent.y * m_tileSize + (m_tileSize - img.height()) / 2);
                    pixItem->setData(0, ent.targetId);
                    hasImage = true;
                }
            }

            if (!hasImage) {
                auto *rect = addRect(ent.x * m_tileSize, ent.y * m_tileSize,
                                     m_tileSize, m_tileSize,
                                     QPen(QColor(139, 90, 43), 2),
                                     QBrush(QColor(139, 90, 43)));
                rect->setData(0, ent.targetId);
            }

            // 建筑名字（没有图片的才显示文字）
            QString label;
            if (ent.targetId == "residence") label = QString::fromUtf8("厢房");
            else if (ent.targetId == "ren_shi_chu") label = QString::fromUtf8("人事处");

            if (!label.isEmpty()) {
                QFont f("SimHei", 11, QFont::Bold);
                QFontMetrics fm(f);
                int textW = fm.horizontalAdvance(label);
                int textH = fm.height();
                auto *text = addText(label);
                text->setDefaultTextColor(Qt::white);
                text->setFont(f);
                text->setPos(ent.x * m_tileSize + (m_tileSize - textW) / 2,
                             ent.y * m_tileSize + m_tileSize - textH - 4);
            }
        }
    }
    for (const auto &exit : m_mapData->exits) {
        auto *rect = addRect(exit.x * m_tileSize + 8, exit.y * m_tileSize + 8,
                             m_tileSize - 16, m_tileSize - 16,
                             QPen(QColor(255, 215, 0, 100), 2),
                             QBrush(QColor(255, 215, 0, 30)));
        rect->setData(0, QString("exit:%1").arg(exit.targetMapId));
    }
}

bool MapScene::isWalkable(int x, int y) const
{
    if (x < 0 || x >= m_mapData->width || y < 0 || y >= m_mapData->height)
        return false;
    int tile = (y < m_mapData->tiles.size() && x < m_mapData->tiles[y].size())
                   ? m_mapData->tiles[y][x] : 0;
    return tile != 3;
}

bool MapScene::movePlayer(int dx, int dy)
{
    if (!m_player) return false;
    int nx = m_player->gridX() + dx;
    int ny = m_player->gridY() + dy;
    if (!isWalkable(nx, ny)) return false;

    for (auto *monster : m_monsters) {
        if (monster->gridX() == nx && monster->gridY() == ny) {
            emit battleTriggered(monster->monsterId());
            return false;
        }
    }

    for (auto *npc : m_npcs) {
        if (npc->pos().x() / m_tileSize == nx && npc->pos().y() / m_tileSize == ny) {
            emit npcInteracted(npc->npcId());
            return false;
        }
    }

    for (const auto &exit : m_mapData->exits) {
        if (exit.x == nx && exit.y == ny) {
            emit mapExit(exit.targetMapId);
            return false;
        }
    }

    m_player->setGridPos(nx, ny);
    m_player->setPos(nx * m_tileSize + m_tileSize / 2,
                     ny * m_tileSize + m_tileSize / 2);
    return true;
}

void MapScene::interactAt(int x, int y)
{
    for (const auto &ent : m_mapData->entities) {
        if (ent.x == x && ent.y == y && ent.type == "building") {
            emit buildingEntered(ent.targetId);
            return;
        }
    }
}
