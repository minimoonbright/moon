#include "MonsterSprite.h"
#include <QPainter>
#include <QPixmap>
#include <QFont>

MonsterSprite::MonsterSprite(const QString &monsterId) : m_monsterId(monsterId)
{
    QPixmap pix(48, 48);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setBrush(QColor(231, 76, 60));
    p.setPen(QPen(QColor(192, 57, 43), 2));
    p.drawRoundedRect(4, 4, 40, 40, 6, 6);
    p.setPen(Qt::white);
    p.setFont(QFont("sans-serif", 10));
    p.drawText(pix.rect(), Qt::AlignCenter, QString::fromUtf8("\xe6\x80\xaa"));
    p.end();
    setPixmap(pix);
    setOffset(-24, -24);
}
