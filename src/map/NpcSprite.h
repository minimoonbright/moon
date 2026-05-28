#pragma once
#include <QGraphicsPixmapItem>
#include <QString>

class NpcSprite : public QGraphicsPixmapItem {
public:
    explicit NpcSprite(const QString &npcId);
    QString npcId() const { return m_npcId; }

private:
    QString m_npcId;
};
