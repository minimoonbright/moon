#include "Character.h"
#include "core/DataManager.h"
#include <QRandomGenerator>

Character::Character(QObject *parent) : QObject(parent) {}

void Character::initFromTemplate(const GeneralTemplate &tmpl, int level)
{
    m_data = CharacterData::fromTemplate(tmpl, level);
    recalcStats();
    m_stats.hp = m_stats.maxHp;
    m_stats.mp = m_stats.maxMp;
}

void Character::initFromData(const CharacterData &data)
{
    m_data = data;
    recalcStats();
    m_stats.hp = m_stats.maxHp;
    m_stats.mp = m_stats.maxMp;
}

CharacterData Character::toData() const { return m_data; }

void Character::recalcStats()
{
    int oldHp = m_stats.hp;
    int oldMp = m_stats.mp;
    m_stats = m_data.stats;
    for (const auto &eq : m_data.equipment) {
        m_stats.atk += eq.bonus.atk;
        m_stats.def += eq.bonus.def;
        m_stats.spd += eq.bonus.spd;
        m_stats.intel += eq.bonus.intel;
        m_stats.maxHp += eq.bonus.maxHp;
        m_stats.maxMp += eq.bonus.maxMp;
        if (eq.enhanceLevel > 0) {
            m_stats.atk += eq.bonus.atk * eq.enhanceLevel / 10;
            m_stats.def += eq.bonus.def * eq.enhanceLevel / 10;
        }
    }
    m_stats.hp = qMin(oldHp, m_stats.maxHp);
    m_stats.mp = qMin(oldMp, m_stats.maxMp);
}

int Character::takeDamage(int rawDamage)
{
    int reduced = qMax(1, rawDamage - m_stats.def / 2);
    if (QRandomGenerator::global()->bounded(100) < m_stats.dodgeRate)
        return 0;
    m_stats.hp = qMax(0, m_stats.hp - reduced);
    return reduced;
}

int Character::calcPhysicalDamage() const
{
    int base = m_stats.atk + QRandomGenerator::global()->bounded(m_stats.atk / 2 + 1);
    if (QRandomGenerator::global()->bounded(100) < m_stats.critRate)
        base = base * m_stats.critDmg / 100;
    return base;
}

int Character::calcStrategyDamage() const
{
    return m_stats.intel + QRandomGenerator::global()->bounded(m_stats.intel / 2 + 1);
}

void Character::addExp(int amount)
{
    m_data.exp += amount;
    while (checkLevelUp()) {}
}

bool Character::checkLevelUp()
{
    int needed = m_data.expToNextLevel();
    if (m_data.exp < needed) return false;
    m_data.exp -= needed;
    m_data.level++;
    switch (m_data.charClass) {
        case CharacterClass::Warrior:
            m_data.stats.atk += 4; m_data.stats.def += 3;
            m_data.stats.spd += 1; m_data.stats.intel += 1;
            break;
        case CharacterClass::General:
            m_data.stats.atk += 3; m_data.stats.def += 2;
            m_data.stats.spd += 3; m_data.stats.intel += 2;
            break;
        case CharacterClass::Strategist:
            m_data.stats.atk += 1; m_data.stats.def += 1;
            m_data.stats.spd += 2; m_data.stats.intel += 4;
            break;
        case CharacterClass::Healer:
            m_data.stats.atk += 1; m_data.stats.def += 2;
            m_data.stats.spd += 2; m_data.stats.intel += 3;
            break;
    }
    m_data.stats.maxHp += 20;
    m_data.stats.hp = m_data.stats.maxHp;
    m_data.stats.maxMp += 10;
    m_data.stats.mp = m_data.stats.maxMp;
    recalcStats();
    emit levelUp(m_data.level);
    return true;
}

bool Character::equipItem(const EquipmentData &item)
{
    int idx = int(item.slot);
    if (idx < 0 || idx >= 7) return false;
    m_data.equipment[idx] = item;
    recalcStats();
    return true;
}

bool Character::unequipItem(EquipmentSlot slot)
{
    int idx = int(slot);
    m_data.equipment[idx] = EquipmentData{};
    recalcStats();
    return true;
}

const EquipmentData &Character::equipmentAt(EquipmentSlot slot) const
{
    return m_data.equipment[int(slot)];
}

bool Character::learnSkill(const QString &skillId)
{
    if (m_data.equippedSkills.size() >= 5) return false;
    if (m_data.equippedSkills.contains(skillId)) return false;
    m_data.equippedSkills.append(skillId);
    return true;
}

void Character::heal(int hpAmount, int mpAmount)
{
    m_stats.hp = qMin(m_stats.maxHp, m_stats.hp + hpAmount);
    m_stats.mp = qMin(m_stats.maxMp, m_stats.mp + mpAmount);
}

void Character::fullRestore()
{
    m_stats.hp = m_stats.maxHp;
    m_stats.mp = m_stats.maxMp;
}

QString Character::checkReserves()
{
    QString result;
    int hpThreshold = m_stats.maxHp / 2;
    int mpThreshold = m_stats.maxMp / 3;

    // HP储备自动使用
    if (m_hpReserveActive && m_hpReserve > 0 && m_stats.hp < hpThreshold && m_stats.hp > 0) {
        int need = m_stats.maxHp - m_stats.hp;
        int use = qMin(need, m_hpReserve);
        m_stats.hp += use;
        m_hpReserve -= use;
        result = QString::fromUtf8("\xe5\x82\xa8\xe5\xa4\x87:HP+%1").arg(use);
        if (m_hpReserve <= 0) m_hpReserveActive = false;
    }

    // MP储备自动使用
    if (m_mpReserveActive && m_mpReserve > 0 && m_stats.mp < mpThreshold && m_stats.mp >= 0) {
        int need = m_stats.maxMp - m_stats.mp;
        int use = qMin(need, m_mpReserve);
        m_stats.mp += use;
        m_mpReserve -= use;
        if (!result.isEmpty()) result += "  ";
        result += QString::fromUtf8("\xe5\x82\xa8\xe5\xa4\x87:MP+%1").arg(use);
        if (m_mpReserve <= 0) m_mpReserveActive = false;
    }

    return result;
}
