#pragma once
#include <QObject>
#include "core/Types.h"

class Character : public QObject {
    Q_OBJECT
public:
    explicit Character(QObject *parent = nullptr);

    void initFromTemplate(const GeneralTemplate &tmpl, int level = 1);
    void initFromData(const CharacterData &data);
    CharacterData toData() const;

    const Stats &stats() const { return m_stats; }
    Stats &mutableStats() { return m_stats; }
    const QString &name() const { return m_data.name; }
    int level() const { return m_data.level; }

    int takeDamage(int rawDamage);
    int calcPhysicalDamage() const;
    int calcStrategyDamage() const;
    bool isAlive() const { return m_stats.hp > 0; }

    void addExp(int amount);
    bool checkLevelUp();

    bool equipItem(const EquipmentData &item);
    bool unequipItem(EquipmentSlot slot);
    const EquipmentData &equipmentAt(EquipmentSlot slot) const;

    bool learnSkill(const QString &skillId);
    QStringList &equippedSkills() { return m_data.equippedSkills; }

    void heal(int hpAmount, int mpAmount);
    void fullRestore();

    // 生命/魔法储备
    int hpReserve() const { return m_hpReserve; }
    int mpReserve() const { return m_mpReserve; }
    bool hpReserveActive() const { return m_hpReserveActive; }
    bool mpReserveActive() const { return m_mpReserveActive; }
    void addHpReserve(int amount) { m_hpReserve += amount; }
    void addMpReserve(int amount) { m_mpReserve += amount; }
    void toggleHpReserve() { m_hpReserveActive = !m_hpReserveActive; }
    void toggleMpReserve() { m_mpReserveActive = !m_mpReserveActive; }
    QString checkReserves(); // 返回使用了什么道具的描述

signals:
    void levelUp(int newLevel);

private:
    void recalcStats();
    CharacterData m_data;
    Stats m_stats;
    int m_hpReserve = 0;
    int m_mpReserve = 0;
    bool m_hpReserveActive = false;
    bool m_mpReserveActive = false;
};
