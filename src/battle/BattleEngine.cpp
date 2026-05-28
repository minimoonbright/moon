#include "BattleEngine.h"
#include "core/DataManager.h"
#include <QDebug>
#include <algorithm>

BattleEngine::BattleEngine(QObject *parent) : QObject(parent) {}

void BattleEngine::initBattle(const QVector<Character *> &playerTeam,
                               const QVector<Character *> &enemyTeam)
{
    m_playerUnits.clear();
    m_enemyUnits.clear();
    m_log.clear();
    m_turnOrder.clear();
    m_turnIndex = 0;
    m_battleOver = false;
    m_playerWon = false;
    m_turnCount = 0;
    m_killerTemplateId.clear();
    m_skillUsed = false;

    for (int i = 0; i < playerTeam.size(); ++i) {
        BattleUnit u;
        u.character = playerTeam[i];
        u.row = (i < 3) ? 0 : 1;
        u.isEnemy = false;
        u.battleIndex = i;
        m_playerUnits.append(u);
    }
    for (int i = 0; i < enemyTeam.size(); ++i) {
        BattleUnit u;
        u.character = enemyTeam[i];
        u.row = (i < 3) ? 0 : 1;
        u.isEnemy = true;
        u.battleIndex = i;
        m_enemyUnits.append(u);
    }

    buildTurnOrder();
}

void BattleEngine::buildTurnOrder()
{
    struct IndexedSpeed { int idx; bool isEnemy; int spd; };
    QVector<IndexedSpeed> all;
    for (int i = 0; i < m_playerUnits.size(); ++i)
        all.append({i, false, m_playerUnits[i].character->stats().spd});
    for (int i = 0; i < m_enemyUnits.size(); ++i)
        all.append({i, true, m_enemyUnits[i].character->stats().spd});

    std::sort(all.begin(), all.end(),
              [](const IndexedSpeed &a, const IndexedSpeed &b) {
                  return a.spd > b.spd;
              });

    m_turnOrder.clear();
    for (const auto &is : all)
        m_turnOrder.append(is.isEnemy ? (is.idx + 100) : is.idx);

    addLog(QString::fromUtf8("=== \xe6\x88\x98\xe6\x96\x97\xe5\xbc\x80\xe5\xa7\x8b ==="), QColor(255, 215, 0));
}

void BattleEngine::startBattle()
{
    m_turnIndex = -1;
    advanceTurn();
}

void BattleEngine::advanceTurn()
{
    if (m_battleOver) return;

    m_turnCount++;

    int attempts = 0;
    while (attempts < m_turnOrder.size() * 2) {
        m_turnIndex = (m_turnIndex + 1) % m_turnOrder.size();
        int idx = m_turnOrder[m_turnIndex];
        const BattleUnit *unit = nullptr;
        if (idx >= 100) {
            int eIdx = idx - 100;
            if (eIdx < m_enemyUnits.size()) unit = &m_enemyUnits[eIdx];
        } else {
            if (idx < m_playerUnits.size()) unit = &m_playerUnits[idx];
        }
        if (unit && unit->character->isAlive()) {
            if (unit->isEnemy) {
                executeAction(idx, BattleAction::Attack);
                return;
            } else {
                emit turnChanged(idx);
                return;
            }
        }
        attempts++;
    }
    checkDefeated(-1);
}

void BattleEngine::executeAction(int unitIndex, BattleAction action,
                                  const QString &skillId, int /*targetIndex*/)
{
    if (m_battleOver) return;

    BattleUnit *attacker = nullptr;
    if (unitIndex >= 100) {
        int eIdx = unitIndex - 100;
        if (eIdx < m_enemyUnits.size()) attacker = &m_enemyUnits[eIdx];
    } else {
        if (unitIndex < m_playerUnits.size()) attacker = &m_playerUnits[unitIndex];
    }
    if (!attacker || !attacker->character->isAlive()) return;

    switch (action) {
        case BattleAction::Attack: {
            if (!attacker->isEnemy)
                m_killerTemplateId = attacker->character->toData().templateId;
            auto &targets = attacker->isEnemy ? m_playerUnits : m_enemyUnits;
            BattleUnit *defender = nullptr;
            for (auto &t : targets) {
                if (t.row == 0 && t.character->isAlive()) { defender = &t; break; }
            }
            if (!defender) {
                for (auto &t : targets) {
                    if (t.character->isAlive()) { defender = &t; break; }
                }
            }
            if (!defender) return;

            int dmg = attacker->character->calcPhysicalDamage();
            int actual = defender->character->takeDamage(dmg);
            addLog(QString("%1\xe6\x94\xbb\xe5\x87\xbb%2\xef\xbc\x8c\xe9\x80\xa0\xe6\x88\x90 %3 \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3")
                       .arg(attacker->character->name())
                       .arg(defender->character->name())
                       .arg(actual),
                   attacker->isEnemy ? QColor(231, 76, 60) : QColor(46, 204, 113));
            checkDefeated(defender->isEnemy ? defender->battleIndex + 100 : defender->battleIndex);
            break;
        }
        case BattleAction::Skill: {
            m_skillUsed = true;
            if (!attacker->isEnemy)
                m_killerTemplateId = attacker->character->toData().templateId;
            if (skillId.isEmpty()) return;
            const auto *skillData = DataManager::instance().getSkill(skillId);
            if (!skillData) return;

            Character *c = attacker->character;
            if (c->mutableStats().mp < skillData->mpCost) return;
            c->mutableStats().mp -= skillData->mpCost;

            auto &targets = attacker->isEnemy ? m_playerUnits : m_enemyUnits;
            BattleUnit *defender = nullptr;
            bool canHitBack = (skillData->type == SkillType::Strategy ||
                               skillData->type == SkillType::Control ||
                               attacker->row == 1);
            for (auto &t : targets) {
                if (t.character->isAlive() && (canHitBack || t.row == 0)) {
                    defender = &t; break;
                }
            }
            if (!defender) return;

            if (skillData->type == SkillType::Heal) {
                int healAmt = skillData->power + c->stats().intel;
                auto &allies = attacker->isEnemy ? m_enemyUnits : m_playerUnits;
                BattleUnit *lowest = nullptr;
                for (auto &a : allies) {
                    if (a.character->isAlive()) {
                        if (!lowest || a.character->stats().hp < lowest->character->stats().hp)
                            lowest = &a;
                    }
                }
                if (lowest) {
                    lowest->character->heal(healAmt, 0);
                    addLog(QString("%1\xe4\xbd\xbf\xe7\x94\xa8\xe3\x80\x8c%2\xe3\x80\x8d\xef\xbc\x8c\xe5\x9b\x9e\xe5\xa4\x8d %3 HP")
                               .arg(c->name()).arg(skillData->name).arg(healAmt),
                           QColor(46, 204, 113));
                }
            } else {
                int dmg = calcSkillDamage(*attacker, *defender, *skillData);
                int actual = defender->character->takeDamage(dmg);
                addLog(QString("%1\xe4\xbd\xbf\xe7\x94\xa8\xe3\x80\x8c%2\xe3\x80\x8d\xef\xbc\x8c\xe5\xaf\xb9%3\xe9\x80\xa0\xe6\x88\x90 %4 \xe7\x82\xb9\xe4\xbc\xa4\xe5\xae\xb3")
                           .arg(c->name()).arg(skillData->name)
                           .arg(defender->character->name()).arg(actual),
                       QColor(52, 152, 219));
                checkDefeated(defender->isEnemy ? defender->battleIndex + 100 : defender->battleIndex);
            }
            break;
        }
        case BattleAction::Defend: {
            addLog(QString("%1\xe8\xbf\x9b\xe5\x85\xa5\xe9\x98\xb2\xe5\xbe\xa1\xe7\x8a\xb6\xe6\x80\x81").arg(attacker->character->name()));
            break;
        }
        case BattleAction::Item: break;
        case BattleAction::Flee: { executeFlee(); return; }
    }

    // 检查双方储备自动使用
    if (!m_battleOver) {
        for (auto &u : m_playerUnits)
            if (u.character->isAlive()) {
                QString s = u.character->checkReserves();
                if (!s.isEmpty())
                    addLog(u.character->name() + ": " + s, QColor(46, 204, 113, 200));
            }
        for (auto &u : m_enemyUnits)
            if (u.character->isAlive())
                u.character->checkReserves();
        advanceTurn();
    }
}

int BattleEngine::calcSkillDamage(const BattleUnit &attacker,
                                   const BattleUnit &/*defender*/,
                                   const SkillData &skill)
{
    int base = skill.power;
    if (skill.type == SkillType::Physical)
        base += attacker.character->calcPhysicalDamage();
    else
        base += attacker.character->calcStrategyDamage();
    return base;
}

void BattleEngine::checkDefeated(int /*unitIndex*/)
{
    bool allEnemyDead = true;
    for (const auto &u : m_enemyUnits)
        if (u.character->isAlive()) { allEnemyDead = false; break; }

    bool allPlayerDead = true;
    for (const auto &u : m_playerUnits)
        if (u.character->isAlive()) { allPlayerDead = false; break; }

    if (allEnemyDead) {
        m_battleOver = true;
        m_playerWon = true;
        m_expReward = m_enemyUnits.size() * 15;
        m_goldReward = m_enemyUnits.size() * 10;
        addLog(QString::fromUtf8("=== \xe6\x88\x98\xe6\x96\x97\xe8\x83\x9c\xe5\x88\xa9\xef\xbc\x81 ==="), QColor(255, 215, 0));
        emit battleEnded(true);
    } else if (allPlayerDead) {
        m_battleOver = true;
        m_playerWon = false;
        addLog(QString::fromUtf8("=== \xe5\x85\xa8\xe5\x86\x9b\xe8\xa6\x86\xe6\xb2\xa1... ==="), QColor(231, 76, 60));
        emit battleEnded(false);
    }
}

bool BattleEngine::isPlayersTurn() const
{
    if (m_turnIndex < 0 || m_turnIndex >= m_turnOrder.size()) return false;
    return m_turnOrder[m_turnIndex] < 100;
}

void BattleEngine::executeFlee()
{
    if (QRandomGenerator::global()->bounded(100) < 50) {
        m_battleOver = true;
        m_playerWon = false;
        addLog(QString::fromUtf8("\xe6\x88\x90\xe5\x8a\x9f\xe9\x80\x83\xe8\x84\xb1\xef\xbc\x81"), QColor(155, 89, 182));
        emit battleEnded(false);
    } else {
        addLog(QString::fromUtf8("\xe9\x80\x83\xe8\x84\xb1\xe5\xa4\xb1\xe8\xb4\xa5\xef\xbc\x81"), QColor(231, 76, 60));
        advanceTurn();
    }
}

void BattleEngine::addLog(const QString &text, QColor color)
{
    m_log.append({text, color});
    emit logAdded(m_log.last());
}
