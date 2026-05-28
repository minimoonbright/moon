#include "MapView.h"
#include "MapScene.h"
#include "PlayerSprite.h"
#include <QKeyEvent>
#include <QMouseEvent>

MapView::MapView(QWidget *parent) : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);
}

void MapView::keyPressEvent(QKeyEvent *event)
{
    auto *scene = qobject_cast<MapScene *>(this->scene());
    if (!scene) return;

    switch (event->key()) {
        case Qt::Key_Up:    case Qt::Key_W: scene->movePlayer(0, -1); break;
        case Qt::Key_Down:  case Qt::Key_S: scene->movePlayer(0, 1);  break;
        case Qt::Key_Left:  case Qt::Key_A: scene->movePlayer(-1, 0); break;
        case Qt::Key_Right: case Qt::Key_D: scene->movePlayer(1, 0);  break;
        case Qt::Key_Return: case Qt::Key_Space:
            if (scene->player())
                scene->interactAt(scene->player()->gridX(), scene->player()->gridY());
            break;
    }
}

void MapView::mousePressEvent(QMouseEvent *event)
{
    auto *scene = qobject_cast<MapScene *>(this->scene());
    if (!scene || !scene->player()) return;

    int tileSize = 64;
    int tx = event->pos().x() / tileSize;
    int ty = event->pos().y() / tileSize;
    int px = scene->player()->gridX();
    int py = scene->player()->gridY();

    if (abs(tx - px) + abs(ty - py) == 1) {
        scene->movePlayer(tx - px, ty - py);
    }
}
