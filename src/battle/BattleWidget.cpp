#include "BattleWidget.h"
#include "core/DataManager.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QPainter>
#include <QFont>
#include <QRandomGenerator>
#include <QInputDialog>
#include <QMessageBox>

// ============= BattleFieldWidget =============

BattleFieldWidget::BattleFieldWidget(QWidget *parent) : QWidget(parent) {}

void BattleFieldWidget::setBattleEngine(BattleEngine *engine)
{
    m_engine = engine;
}

void BattleFieldWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.fillRect(rect(), QColor(26, 26, 46));

    // 标题
    p.setPen(QColor(255, 215, 0));
    QFont titleFont("SimHei", 14, QFont::Bold);
    p.setFont(titleFont);
    p.drawText(QRect(0, 0, width(), 28), Qt::AlignCenter,
               QString::fromUtf8("\xe2\x9a\x94\xef\xb8\x8f \xe9\x81\x87\xe6\x95\x8c \xe2\x9a\x94\xef\xb8\x8f"));

    if (!m_engine) return;

    const auto &playerUnits = m_engine->playerUnits();
    const auto &enemyUnits = m_engine->enemyUnits();
    int nPlayer = playerUnits.size();
    int nEnemy = enemyUnits.size();

    int halfW = width() / 2;
    int topY = 44;
    int bottomY = height() - 8;
    int bodyH = bottomY - topY;
    int halfH = bodyH / 2;
    int unitW = halfW / 3;

    // === 我军标签 ===
    p.setPen(QColor(46, 204, 113));
    QFont labelFont("SimHei", 10, QFont::Bold);
    p.setFont(labelFont);
    p.drawText(QRect(0, topY - 20, halfW, 16), Qt::AlignCenter,
               QString::fromUtf8("\xe2\x80\x94\xe2\x80\x94 \xe6\x88\x91\xe5\x86\x9b \xe2\x80\x94\xe2\x80\x94"));

    // 我方 units
    for (int i = 0; i < nPlayer; i++) {
        int col = (i < 3) ? i : (i - 3);
        int row = (i < 3) ? 0 : 1;
        int ux = 4 + col * unitW;
        int uy = topY + row * halfH;
        QRect r(ux, uy, unitW - 8, halfH - 8);
        drawUnit(p, playerUnits[i], false, r);
    }

    // === 中线 VS ===
    p.setPen(QColor(255, 215, 0, 80));
    p.drawLine(halfW, topY, halfW, bottomY);
    QFont vsFont("SimHei", 18, QFont::Bold);
    p.setFont(vsFont);
    p.setPen(QColor(255, 215, 0, 60));
    p.drawText(QRect(halfW - 30, height() / 2 - 18, 60, 36), Qt::AlignCenter, "VS");

    // === 敌军标签 ===
    p.setPen(QColor(231, 76, 60));
    p.setFont(labelFont);
    p.drawText(QRect(halfW, topY - 20, halfW, 16), Qt::AlignCenter,
               QString::fromUtf8("\xe2\x80\x94\xe2\x80\x94 \xe6\x95\x8c\xe5\x86\x9b \xe2\x80\x94\xe2\x80\x94"));

    // 敌方 units
    for (int i = 0; i < nEnemy; i++) {
        int col = (i < 3) ? i : (i - 3);
        int row = (i < 3) ? 0 : 1;
        int ux = halfW + 4 + col * unitW;
        int uy = topY + row * halfH;
        QRect r(ux, uy, unitW - 8, halfH - 8);
        drawUnit(p, enemyUnits[i], true, r);
    }
}

void BattleFieldWidget::drawUnit(QPainter &p, const BattleUnit &unit, bool isEnemy, const QRect &rect)
{
    bool alive = unit.character->isAlive();
    QColor borderColor = isEnemy ? QColor(192, 57, 43) : QColor(39, 174, 96);
    QColor fillColor = isEnemy ? QColor(231, 76, 60, alive ? 35 : 15)
                               : QColor(46, 204, 113, alive ? 35 : 15);

    p.setPen(QPen(borderColor, 2));
    p.setBrush(fillColor);
    p.drawRoundedRect(rect, 6, 6);

    if (!alive) {
        p.setPen(QColor(180, 0, 0));
        QFont deadFont("SimHei", 14, QFont::Bold);
        p.setFont(deadFont);
        p.drawText(rect, Qt::AlignCenter, QString::fromUtf8("\xe9\x80\x80\xe5\x87\xba"));
        return;
    }

    const Stats &s = unit.character->stats();
    int cx = rect.x();
    int cw = rect.width();

    // 名字
    p.setPen(Qt::white);
    QFont nameFont("SimHei", 11, QFont::Bold);
    p.setFont(nameFont);
    p.drawText(QRect(cx, rect.y() + 2, cw, 18), Qt::AlignCenter, unit.character->name());

    // HP 条
    int barX = cx + 4;
    int barW = cw - 8;
    int barY = rect.y() + 22;
    int barH = 10;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(30, 30, 30));
    p.drawRoundedRect(barX, barY, barW, barH, 3, 3);

    double hpRatio = qMax(0.0, (double)s.hp / s.maxHp);
    QColor hpColor = hpRatio > 0.5 ? QColor(46, 204, 113) :
                      hpRatio > 0.25 ? QColor(243, 156, 18) : QColor(231, 76, 60);
    p.setBrush(hpColor);
    p.drawRoundedRect(barX, barY, (int)(barW * hpRatio), barH, 3, 3);

    QFont hpFont("SimHei", 8);
    p.setFont(hpFont);
    p.setPen(Qt::white);
    p.drawText(QRect(barX, barY, barW, barH), Qt::AlignCenter,
               QString("%1/%2").arg(s.hp).arg(s.maxHp));

    // MP 行
    QFont mpFont("SimHei", 8);
    p.setFont(mpFont);
    p.setPen(QColor(52, 152, 219));
    p.drawText(QRect(cx, barY + 11, cw, 12), Qt::AlignCenter,
               QString("MP:%1/%2").arg(s.mp).arg(s.maxMp));

    // === 属性（紧凑两列） ===
    int statY = barY + 24;
    int lineH = 14;
    QFont statFont("SimHei", 8);
    p.setFont(statFont);
    int halfW = cw / 2;

    struct StatEntry { QString label; int val; QColor color; bool isPct; };
    QVector<StatEntry> stats = {
        {QString::fromUtf8("\xe6\x94\xbb"), s.atk, QColor(231, 76, 60), false},
        {QString::fromUtf8("\xe9\x98\xb2"), s.def, QColor(52, 152, 219), false},
        {QString::fromUtf8("\xe9\x80\x9f"), s.spd, QColor(46, 204, 113), false},
        {QString::fromUtf8("\xe6\x99\xba"), s.intel, QColor(155, 89, 182), false},
        {QString::fromUtf8("\xe6\x9a\xb4"), s.critRate, QColor(243, 156, 18), true},
        {QString::fromUtf8("\xe9\x97\xaa"), s.dodgeRate, QColor(46, 204, 113), true},
        {QString::fromUtf8("\xe6\x9a\xb4\xe4\xbc\xa4"), s.critDmg, QColor(255, 100, 50), true},
    };

    for (int row = 0; row < 4; row++) {
        int sy = statY + row * lineH;
        for (int col = 0; col < 2; col++) {
            int idx = row * 2 + col;
            if (idx >= stats.size()) break;
            const auto &entry = stats[idx];
            int sx = cx + 6 + col * halfW;

            p.setPen(QColor(140, 140, 140));
            p.drawText(QRect(sx, sy, 20, lineH), Qt::AlignLeft, entry.label + ":");
            p.setPen(entry.color);
            QString valText = entry.isPct ? QString("%1%").arg(entry.val)
                                         : QString::number(entry.val);
            p.drawText(QRect(sx + 20, sy, halfW - 26, lineH), Qt::AlignLeft, valText);
        }
    }

    // 储备信息
    Character *ch = unit.character;
    int resY = rect.bottom() - 24;
    if (ch->hpReserve() > 0 || ch->mpReserve() > 0) {
        QFont resFont("SimHei", 7);
        p.setFont(resFont);
        if (ch->hpReserve() > 0) {
            p.setPen(ch->hpReserveActive() ? QColor(46, 204, 113) : QColor(100, 100, 100));
            p.drawText(QRect(cx + 4, resY, cw - 8, 12), Qt::AlignLeft,
                       QString::fromUtf8("\xe5\x82\xa8HP:%1").arg(ch->hpReserve()));
        }
        if (ch->mpReserve() > 0) {
            p.setPen(ch->mpReserveActive() ? QColor(52, 152, 219) : QColor(100, 100, 100));
            p.drawText(QRect(cx + cw / 2, resY, cw / 2 - 4, 12), Qt::AlignRight,
                       QString::fromUtf8("\xe5\x82\xa8MP:%1").arg(ch->mpReserve()));
        }
    }

    // 前/后排标签
    p.setPen(isEnemy ? QColor(231, 76, 60, 150) : QColor(46, 204, 113, 150));
    QFont rowFont("SimHei", 8);
    p.setFont(rowFont);
    p.drawText(QRect(cx, rect.bottom() - 12, cw, 12), Qt::AlignCenter,
               unit.row == 0 ? QString::fromUtf8("\xe5\x89\x8d\xe6\x8e\x92") : QString::fromUtf8("\xe5\x90\x8e\xe6\x8e\x92"));
}

// ============= BattleWidget =============

BattleWidget::BattleWidget(QWidget *parent)
    : QWidget(parent), m_engine(new BattleEngine(this))
{
    auto *mainLayout = new QVBoxLayout(this);

    // 状态区域
    m_statusLabel = new QLabel;
    m_statusLabel->setMinimumHeight(36);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel { background: #0d1b2a; border: 1px solid #1b3a4b; "
        "color: #2ecc71; border-radius: 6px; font-size: 13px; padding: 6px; }");
    mainLayout->addWidget(m_statusLabel);

    // 当前行动
    m_turnLabel = new QLabel;
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setMinimumHeight(28);
    m_turnLabel->setStyleSheet(
        "QLabel { background: rgba(231,76,60,0.15); border: 1px dashed #e74c3c; "
        "color: #e74c3c; border-radius: 6px; font-size: 14px; font-weight: bold; padding: 4px; }");
    mainLayout->addWidget(m_turnLabel);

    // 战场
    m_battleField = new BattleFieldWidget;
    m_battleField->setMinimumHeight(320);
    mainLayout->addWidget(m_battleField, 1);

    // 日志
    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setMaximumHeight(110);
    m_logView->setStyleSheet("QTextEdit { background: #0a0a1a; color: #bbb; font-size: 12px; border: 1px solid #333; }");
    mainLayout->addWidget(m_logView);

    // 指令面板
    auto *cmdLayout = new QHBoxLayout;
    QString btnBase(
        "QPushButton { color: white; border: none; border-radius: 6px; "
        "font-size: 14px; padding: 10px 20px; min-width: 80px; }"
        "QPushButton:hover { opacity: 0.85; }"
        "QPushButton:disabled { background: #444; color: #777; }");

    m_btnAttack = new QPushButton(QString::fromUtf8("\xe2\x9a\x94\xef\xb8\x8f \xe6\x94\xbb\xe5\x87\xbb"));
    m_btnAttack->setStyleSheet("QPushButton { background: #c0392b; } " + btnBase);
    m_btnSkill  = new QPushButton(QString::fromUtf8("\xe2\x9c\xa8 \xe6\x8a\x80\xe8\x83\xbd"));
    m_btnSkill->setStyleSheet("QPushButton { background: #2471a3; } " + btnBase);
    m_btnDefend = new QPushButton(QString::fromUtf8("\xf0\x9f\x9b\xa1\xef\xb8\x8f \xe9\x98\xb2\xe5\xbe\xa1"));
    m_btnDefend->setStyleSheet("QPushButton { background: #1e8449; } " + btnBase);
    m_btnItem   = new QPushButton(QString::fromUtf8("\xf0\x9f\x8e\x92 \xe9\x81\x93\xe5\x85\xb7"));
    m_btnItem->setStyleSheet("QPushButton { background: #b7950b; color: #333; } " + btnBase);
    m_btnFlee   = new QPushButton(QString::fromUtf8("\xf0\x9f\x8f\x83 \xe6\x92\xa4\xe9\x80\x80"));
    m_btnFlee->setStyleSheet("QPushButton { background: #7d3c98; } " + btnBase);
    m_btnView   = new QPushButton(QString::fromUtf8("\xf0\x9f\x94\x8d \xe6\x9f\xa5\xe7\x9c\x8b"));
    m_btnView->setStyleSheet("QPushButton { background: #555; } " + btnBase);

    for (auto *btn : {m_btnAttack, m_btnSkill, m_btnDefend, m_btnItem, m_btnFlee, m_btnView})
        cmdLayout->addWidget(btn);
    mainLayout->addLayout(cmdLayout);

    connect(m_btnAttack, &QPushButton::clicked, this, &BattleWidget::onAttack);
    connect(m_btnSkill,  &QPushButton::clicked, this, &BattleWidget::onSkill);
    connect(m_btnDefend, &QPushButton::clicked, this, &BattleWidget::onDefend);
    connect(m_btnItem,   &QPushButton::clicked, this, &BattleWidget::onItem);
    connect(m_btnFlee,   &QPushButton::clicked, this, &BattleWidget::onFlee);
    connect(m_btnView,   &QPushButton::clicked, this, &BattleWidget::onView);

    connect(m_engine, &BattleEngine::turnChanged, this, &BattleWidget::onTurnChanged);
    connect(m_engine, &BattleEngine::battleEnded, this, &BattleWidget::onBattleEnd);
    connect(m_engine, &BattleEngine::logAdded, this, [this](const BattleLogEntry &e) {
        m_logView->append(QString("<span style='color:%1'>%2</span>")
                              .arg(e.color.name()).arg(e.text));
        m_logView->verticalScrollBar()->setValue(
            m_logView->verticalScrollBar()->maximum());
        m_battleField->update();
        updateStatusArea();
    });
}

void BattleWidget::startBattle(const QVector<Character *> &playerTeam,
                                const QVector<Character *> &enemyTeam,
                                const QString &enemyLeaderId)
{
    m_enemyLeaderId = enemyLeaderId;
    m_logView->clear();
    m_engine->initBattle(playerTeam, enemyTeam);
    m_battleField->setBattleEngine(m_engine);
    m_battleField->update();
    setCommandsEnabled(false);
    updateStatusArea();
    m_engine->startBattle();
}

void BattleWidget::updateStatusArea()
{
    QStringList playerInfo, enemyInfo;
    for (const auto &u : m_engine->playerUnits()) {
        const auto &s = u.character->stats();
        playerInfo << QString("%1 HP:%2/%3 ATK:%4")
                          .arg(u.character->name()).arg(s.hp).arg(s.maxHp).arg(s.atk);
    }
    for (const auto &u : m_engine->enemyUnits()) {
        const auto &s = u.character->stats();
        enemyInfo << QString("%1 HP:%2/%3 ATK:%4")
                          .arg(u.character->name()).arg(s.hp).arg(s.maxHp).arg(s.atk);
    }
    m_statusLabel->setText(
        QString::fromUtf8("\xe6\x88\x91: %1  |  \xe6\x95\x8c: %2")
            .arg(playerInfo.join(", ")).arg(enemyInfo.join(", ")));
}

void BattleWidget::onTurnChanged(int unitIndex)
{
    m_selectedUnitIndex = unitIndex;
    setCommandsEnabled(true);
    if (unitIndex >= 0 && unitIndex < m_engine->playerUnits().size()) {
        const auto &u = m_engine->playerUnits()[unitIndex];
        m_turnLabel->setText(
            QString::fromUtf8("\xe2\x96\xb6 %1 \xe7\x9a\x84\xe8\xa1\x8c\xe5\x8a\xa8\xe5\x9b\x9e\xe5\x90\x88 \xe2\x97\x80")
                .arg(u.character->name()));
    }
}

void BattleWidget::setCommandsEnabled(bool enabled)
{
    m_btnAttack->setEnabled(enabled);
    m_btnSkill->setEnabled(enabled);
    m_btnDefend->setEnabled(enabled);
    m_btnItem->setEnabled(enabled);
    m_btnFlee->setEnabled(enabled);
    m_btnView->setEnabled(true);
}

void BattleWidget::onAttack()
{
    setCommandsEnabled(false);
    m_engine->executeAction(m_selectedUnitIndex, BattleAction::Attack);
}

void BattleWidget::onSkill()
{
    if (m_selectedUnitIndex < 0 || m_selectedUnitIndex >= m_engine->playerUnits().size())
        return;

    Character *c = m_engine->playerUnits()[m_selectedUnitIndex].character;
    const auto &skillIds = c->equippedSkills();
    if (skillIds.isEmpty()) {
        setCommandsEnabled(false);
        m_engine->executeAction(m_selectedUnitIndex, BattleAction::Attack);
        return;
    }

    QStringList skillDescs;
    for (int i = 0; i < skillIds.size(); ++i) {
        const auto *sd = DataManager::instance().getSkill(skillIds[i]);
        if (sd) {
            QString desc = QString("%1  MP:%2 \xe5\xa8\x81\xe5\x8a\x9b:%3")
                               .arg(sd->name).arg(sd->mpCost).arg(sd->power);
            if (c->stats().mp < sd->mpCost)
                desc += QString::fromUtf8(" [MP\xe4\xb8\x8d\xe8\xb6\xb3]");
            skillDescs << desc;
        }
    }

    bool ok;
    QString choice = QInputDialog::getItem(this,
        QString::fromUtf8("\xe9\x80\x89\xe6\x8b\xa9\xe6\x8a\x80\xe8\x83\xbd"),
        c->name() + QString("  MP:%1/%2").arg(c->stats().mp).arg(c->stats().maxMp),
        skillDescs, 0, false, &ok);

    if (!ok) return;
    int idx = skillDescs.indexOf(choice);
    if (idx >= 0 && idx < skillIds.size()) {
        const auto *sd = DataManager::instance().getSkill(skillIds[idx]);
        if (sd && c->stats().mp >= sd->mpCost) {
            setCommandsEnabled(false);
            m_engine->executeAction(m_selectedUnitIndex, BattleAction::Skill, skillIds[idx]);
        }
    }
}

void BattleWidget::onItem()
{
    if (m_selectedUnitIndex < 0 || m_selectedUnitIndex >= m_engine->playerUnits().size())
        return;

    Character *c = m_engine->playerUnits()[m_selectedUnitIndex].character;

    QStringList itemDescs;
    QStringList actions; // "use" or "deposit"

    // 金疮药
    itemDescs << QString::fromUtf8("\xe9\x87\x91\xe7\x96\xae\xe8\x8d\xaf \xe4\xbd\xbf\xe7\x94\xa8(HP+50)");
    actions << "heal_hp";
    itemDescs << QString::fromUtf8("\xe9\x87\x91\xe7\x96\xae\xe8\x8d\xaf \xe5\xad\x98\xe5\x85\xa5HP\xe5\x82\xa8\xe5\xa4\x87(+50)");
    actions << "deposit_hp";

    // 回神丹
    itemDescs << QString::fromUtf8("\xe5\x9b\x9e\xe7\xa5\x9e\xe4\xb8\xb9 \xe4\xbd\xbf\xe7\x94\xa8(MP+30)");
    actions << "heal_mp";
    itemDescs << QString::fromUtf8("\xe5\x9b\x9e\xe7\xa5\x9e\xe4\xb8\xb9 \xe5\xad\x98\xe5\x85\xa5MP\xe5\x82\xa8\xe5\xa4\x87(+30)");
    actions << "deposit_mp";

    // 力量符
    itemDescs << QString::fromUtf8("\xe5\x8a\x9b\xe9\x87\x8f\xe7\xac\xa6 (ATK+10)");
    actions << "atk_buff";
    // 护身符
    itemDescs << QString::fromUtf8("\xe6\x8a\xa4\xe8\xba\xab\xe7\xac\xa6 (DEF+8)");
    actions << "def_buff";

    // 储备开关
    QString hpTag = c->hpReserveActive() ? QString::fromUtf8("[ON]") : QString::fromUtf8("[OFF]");
    QString mpTag = c->mpReserveActive() ? QString::fromUtf8("[ON]") : QString::fromUtf8("[OFF]");
    itemDescs << QString::fromUtf8("HP\xe5\x82\xa8\xe5\xa4\x87\xe5\xbc\x80\xe5\x85\xb3 %1 (\xe4\xbd\x99\xe9\x87\x8f:%2)").arg(hpTag).arg(c->hpReserve());
    actions << "toggle_hp";
    itemDescs << QString::fromUtf8("MP\xe5\x82\xa8\xe5\xa4\x87\xe5\xbc\x80\xe5\x85\xb3 %1 (\xe4\xbd\x99\xe9\x87\x8f:%2)").arg(mpTag).arg(c->mpReserve());
    actions << "toggle_mp";

    bool ok;
    QString choice = QInputDialog::getItem(this,
        QString::fromUtf8("\xe9\x81\x93\xe5\x85\xb7\xe4\xb8\x8e\xe5\x82\xa8\xe5\xa4\x87"),
        QString::fromUtf8("%1 HP:%2/%3 MP:%4/%5 \xe5\x82\xa8HP:%6 \xe5\x82\xa8MP:%7")
            .arg(c->name()).arg(c->stats().hp).arg(c->stats().maxHp)
            .arg(c->stats().mp).arg(c->stats().maxMp)
            .arg(c->hpReserve()).arg(c->mpReserve()),
        itemDescs, 0, false, &ok);

    if (!ok) return;
    int idx = itemDescs.indexOf(choice);
    if (idx < 0 || idx >= actions.size()) return;

    setCommandsEnabled(false);
    const QString &act = actions[idx];

    if (act == "heal_hp") {
        c->heal(50, 0);
        m_logView->append(QString("<span style='color:#2ecc71'>%1 \xe4\xbd\xbf\xe7\x94\xa8\xe9\x87\x91\xe7\x96\xae\xe8\x8d\xaf HP+50</span>").arg(c->name()));
    } else if (act == "heal_mp") {
        c->heal(0, 30);
        m_logView->append(QString("<span style='color:#3498db'>%1 \xe4\xbd\xbf\xe7\x94\xa8\xe5\x9b\x9e\xe7\xa5\x9e\xe4\xb8\xb9 MP+30</span>").arg(c->name()));
    } else if (act == "deposit_hp") {
        c->addHpReserve(50);
        m_logView->append(QString("<span style='color:#2ecc71'>%1 \xe5\xad\x98\xe5\x85\xa5HP\xe5\x82\xa8\xe5\xa4\x87 +50 (\xe4\xbd\x99:%2)</span>").arg(c->name()).arg(c->hpReserve()));
    } else if (act == "deposit_mp") {
        c->addMpReserve(30);
        m_logView->append(QString("<span style='color:#3498db'>%1 \xe5\xad\x98\xe5\x85\xa5MP\xe5\x82\xa8\xe5\xa4\x87 +30 (\xe4\xbd\x99:%2)</span>").arg(c->name()).arg(c->mpReserve()));
    } else if (act == "toggle_hp") {
        c->toggleHpReserve();
        m_logView->append(QString("<span style='color:#888'>%1 HP\xe5\x82\xa8\xe5\xa4\x87: %2</span>")
            .arg(c->name()).arg(c->hpReserveActive() ? QString::fromUtf8("\xe5\xbc\x80") : QString::fromUtf8("\xe5\x85\xb3")));
        setCommandsEnabled(true); // 切换开关不消耗回合
        m_battleField->update();
        return;
    } else if (act == "toggle_mp") {
        c->toggleMpReserve();
        m_logView->append(QString("<span style='color:#888'>%1 MP\xe5\x82\xa8\xe5\xa4\x87: %2</span>")
            .arg(c->name()).arg(c->mpReserveActive() ? QString::fromUtf8("\xe5\xbc\x80") : QString::fromUtf8("\xe5\x85\xb3")));
        setCommandsEnabled(true); // 切换开关不消耗回合
        m_battleField->update();
        return;
    } else if (act == "atk_buff") {
        c->mutableStats().atk += 10;
        m_logView->append(QString("<span style='color:#e74c3c'>%1 \xe4\xbd\xbf\xe7\x94\xa8\xe5\x8a\x9b\xe9\x87\x8f\xe7\xac\xa6, ATK+10</span>").arg(c->name()));
    } else if (act == "def_buff") {
        c->mutableStats().def += 8;
        m_logView->append(QString("<span style='color:#3498db'>%1 \xe4\xbd\xbf\xe7\x94\xa8\xe6\x8a\xa4\xe8\xba\xab\xe7\xac\xa6, DEF+8</span>").arg(c->name()));
    }

    m_battleField->update();
    updateStatusArea();
    m_engine->executeAction(m_selectedUnitIndex, BattleAction::Item);
}

void BattleWidget::onView()
{
    QStringList lines;
    lines << QString::fromUtf8("\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81 \xe6\x88\x91\xe6\x96\xb9\xe8\xa3\x85\xe5\xa4\x87 \xe2\x94\x81\xe2\x94\x81\xe2\x94\x81");

    for (const auto &u : m_engine->playerUnits()) {
        const auto &s = u.character->stats();
        lines << QString("%1  Lv.%2  HP:%3/%4  MP:%5/%6")
                     .arg(u.character->name()).arg(u.character->level())
                     .arg(s.hp).arg(s.maxHp).arg(s.mp).arg(s.maxMp);
        lines << QString("  ATK:%1  DEF:%2  SPD:%3  INT:%4  CRIT:%5%  DODGE:%6%  CRITDMG:%7%")
                     .arg(s.atk).arg(s.def).arg(s.spd).arg(s.intel)
                     .arg(s.critRate).arg(s.dodgeRate).arg(s.critDmg);

        // 装备槽
        QStringList equipInfo;
        const char *slotNames[] = {"\xe6\xad\xa6", "\xe5\xa4\xb4", "\xe7\x94\xb2", "\xe9\x9d\xb4", "\xe9\xa5\xb01", "\xe9\xa5\xb02", "\xe9\xaa\x91"};
        for (int i = 0; i < 7; ++i) {
            const auto &eq = u.character->equipmentAt(static_cast<EquipmentSlot>(i));
            if (!eq.id.isEmpty())
                equipInfo << QString("%1:%2").arg(slotNames[i]).arg(eq.name);
        }
        if (!equipInfo.isEmpty())
            lines << QString("  \xe8\xa3\x85\xe5\xa4\x87: %1").arg(equipInfo.join(", "));
        else
            lines << QString::fromUtf8("  \xe8\xa3\x85\xe5\xa4\x87: \xe6\x97\xa0");

        // 技能
        QStringList skillNames;
        for (const auto &sid : u.character->equippedSkills()) {
            const auto *sd = DataManager::instance().getSkill(sid);
            if (sd) skillNames << sd->name;
        }
        lines << QString("  \xe6\x8a\x80\xe8\x83\xbd: %1").arg(skillNames.isEmpty() ? QString::fromUtf8("\xe6\x97\xa0") : skillNames.join(", "));
        lines << "";
    }

    // 背包道具
    lines << QString::fromUtf8("\xe2\x94\x81\xe2\x94\x81\xe2\x94\x81 \xe8\x83\x8c\xe5\x8c\x85\xe9\x81\x93\xe5\x85\xb7 \xe2\x94\x81\xe2\x94\x81\xe2\x94\x81");
    lines << QString::fromUtf8("\xe9\x87\x91\xe7\x96\xae\xe8\x8d\xaf(HP+50) | \xe5\x9b\x9e\xe7\xa5\x9e\xe4\xb8\xb9(MP+30)");
    lines << QString::fromUtf8("\xe5\x8a\x9b\xe9\x87\x8f\xe7\xac\xa6(ATK+10) | \xe6\x8a\xa4\xe8\xba\xab\xe7\xac\xa6(DEF+8)");

    QMessageBox::information(this,
        QString::fromUtf8("\xe8\xa3\x85\xe5\xa4\x87\xe4\xb8\x8e\xe8\x83\x8c\xe5\x8c\x85"),
        lines.join("\n"));
}

void BattleWidget::onDefend()
{
    setCommandsEnabled(false);
    m_engine->executeAction(m_selectedUnitIndex, BattleAction::Defend);
}

void BattleWidget::onFlee()
{
    m_engine->executeFlee();
}

void BattleWidget::onBattleEnd(bool playerWon)
{
    setCommandsEnabled(false);
    m_battleField->update();
    if (playerWon) {
        m_logView->append(
            QString("<span style='color:#ffd700'>\xe7\xbb\x8f\xe9\xaa\x8c +%1  |  \xe9\x87\x91\xe5\xb8\x81 +%2</span>")
                .arg(m_engine->expReward()).arg(m_engine->goldReward()));
    }
    emit battleFinished(playerWon, m_enemyLeaderId,
                        m_engine->expReward(), m_engine->goldReward(),
                        m_engine->lootItems(),
                        m_engine->turnCount(), m_engine->killerTemplateId(),
                        m_engine->skillWasUsed());
}
