#include "NpcSprite.h"
#include <QPainter>
#include <QPixmap>
#include <QFont>

NpcSprite::NpcSprite(const QString &npcId) : m_npcId(npcId)
{
    QPixmap pix(48, 48);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setBrush(QColor(46, 204, 113));
    p.setPen(QPen(QColor(39, 174, 96), 2));
    p.drawEllipse(8, 8, 32, 32);
    p.setPen(Qt::white);
    p.setFont(QFont("sans-serif", 10));
    p.drawText(pix.rect(), Qt::AlignCenter, QString::fromUtf8("\xe4\xba\xba"));
    p.end();
    setPixmap(pix);
    setOffset(-24, -24);
}
