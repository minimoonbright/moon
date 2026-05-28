#pragma once
#include <QString>
#include <QJsonObject>
#include <QVector>
#include <QMap>
#include <QRandomGenerator>

// === 基础枚举 ===

enum class CharacterClass { Warrior, General, Strategist, Healer };
enum class Personality { Loyal, Greedy, Cowardly, Arrogant };
enum class ItemQuality { Normal, Fine, Rare, Epic };
enum class SkillType { Physical, Strategy, Heal, Control };
enum class EquipmentSlot { Weapon, Helmet, Armor, Boots, Accessory1, Accessory2, Mount };
enum class BattleAction { Attack, Skill, Defend, Item, Flee };

// === 属性 ===
struct Stats {
    int hp = 100;
    int maxHp = 100;
    int mp = 50;
    int maxMp = 50;
    int atk = 10;
    int def = 5;
    int spd = 10;
    int intel = 10;
    int critRate = 5;
    int dodgeRate = 5;
    int critDmg = 150;  // 暴击伤害百分比
    int benevolence = 50;
    int charisma = 50;

    static Stats fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

// === 技能 ===
struct SkillData {
    QString id;
    QString name;
    SkillType type = SkillType::Physical;
    int mpCost = 10;
    int power = 20;
    int level = 1;
    int usageCount = 0;

    static SkillData fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

// === 装备 ===
struct EquipmentData {
    QString id;
    QString name;
    EquipmentSlot slot = EquipmentSlot::Weapon;
    ItemQuality quality = ItemQuality::Normal;
    Stats bonus;
    QString specialEffect;
    int enhanceLevel = 0;
    int price = 0;

    static EquipmentData fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

// === 消耗品 ===
struct ItemData {
    QString id;
    QString name;
    QString description;
    int hpRestore = 0;
    int mpRestore = 0;
    int price = 0;

    static ItemData fromJson(const QJsonObject &obj);
};

// === 武将模板 ===
struct GeneralTemplate {
    QString id;
    QString name;
    CharacterClass charClass = CharacterClass::Warrior;
    Stats baseStats;
    QStringList skillIds;
    Personality personality = Personality::Loyal;
    int loyalty = 50;
    QString recruitHint;
    QString recruitCondition;

    static GeneralTemplate fromJson(const QJsonObject &obj);
};

// === 角色实例 ===
struct CharacterData {
    QString templateId;
    QString name;
    CharacterClass charClass = CharacterClass::Warrior;
    int level = 1;
    int exp = 0;
    Stats stats;
    QStringList equippedSkills;
    EquipmentData equipment[7];
    int gold = 0;
    bool isPlayer = false;

    static CharacterData fromTemplate(const GeneralTemplate &tmpl, int lvl = 1);
    QJsonObject toJson() const;
    static CharacterData fromJson(const QJsonObject &obj);
    int expToNextLevel() const { return level * level * 100; }
};

// === 地图定义 ===
struct MapData {
    QString id;
    QString name;
    int width = 10;
    int height = 8;
    QVector<QVector<int>> tiles;

    struct MapEntity {
        QString type;
        QString targetId;
        int x = 0, y = 0;
    };
    QVector<MapEntity> entities;

    struct Exit {
        int x = 0, y = 0;
        QString targetMapId;
    };
    QVector<Exit> exits;

    static MapData fromJson(const QJsonObject &obj);
};
