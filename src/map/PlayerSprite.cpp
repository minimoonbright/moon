#include "PlayerSprite.h"
#include <QPainter>
#include <QPixmap>
#include <QFont>

PlayerSprite::PlayerSprite()
{
    QPixmap pix(48, 48);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setBrush(QColor(52, 152, 219));
    p.setPen(QPen(QColor(41, 128, 185), 2));
    p.drawRoundedRect(4, 4, 40, 40, 8, 8);
    p.setPen(Qt::white);
    p.setFont(QFont("sans-serif", 16));
    p.drawText(pix.rect(), Qt::AlignCenter, QString::fromUtf8("\xe5\x88\x98"));
    p.end();
    setPixmap(pix);
    setOffset(-24, -24);
}
