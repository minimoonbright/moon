#include "Types.h"
#include <QJsonArray>

// === Stats ===
Stats Stats::fromJson(const QJsonObject &o)
{
    Stats s;
    s.hp = o["hp"].toInt(100);
    s.maxHp = o["maxHp"].toInt(s.hp);
    s.mp = o["mp"].toInt(50);
    s.maxMp = o["maxMp"].toInt(s.mp);
    s.atk = o["atk"].toInt(10);
    s.def = o["def"].toInt(5);
    s.spd = o["spd"].toInt(10);
    s.intel = o["intel"].toInt(10);
    s.critRate = o["critRate"].toInt(5);
    s.dodgeRate = o["dodgeRate"].toInt(5);
    s.critDmg = o["critDmg"].toInt(150);
    s.benevolence = o["benevolence"].toInt(50);
    s.charisma = o["charisma"].toInt(50);
    return s;
}

QJsonObject Stats::toJson() const
{
    return {
        {"hp", hp}, {"maxHp", maxHp},
        {"mp", mp}, {"maxMp", maxMp},
        {"atk", atk}, {"def", def},
        {"spd", spd}, {"intel", intel},
        {"critRate", critRate}, {"dodgeRate", dodgeRate}, {"critDmg", critDmg},
        {"benevolence", benevolence}, {"charisma", charisma}
    };
}

// === SkillData ===
SkillData SkillData::fromJson(const QJsonObject &o)
{
    SkillData s;
    s.id = o["id"].toString();
    s.name = o["name"].toString();
    QString typeStr = o["type"].toString();
    if (typeStr == "physical") s.type = SkillType::Physical;
    else if (typeStr == "strategy") s.type = SkillType::Strategy;
    else if (typeStr == "heal") s.type = SkillType::Heal;
    else s.type = SkillType::Control;
    s.mpCost = o["mpCost"].toInt(10);
    s.power = o["power"].toInt(20);
    s.level = o["level"].toInt(1);
    return s;
}

QJsonObject SkillData::toJson() const
{
    QString typeStr;
    switch (type) {
        case SkillType::Physical: typeStr = "physical"; break;
        case SkillType::Strategy: typeStr = "strategy"; break;
        case SkillType::Heal:     typeStr = "heal"; break;
        case SkillType::Control:  typeStr = "control"; break;
    }
    return {
        {"id", id}, {"name", name}, {"type", typeStr},
        {"mpCost", mpCost}, {"power", power}, {"level", level},
        {"usageCount", usageCount}
    };
}

// === EquipmentData ===
EquipmentData EquipmentData::fromJson(const QJsonObject &o)
{
    EquipmentData e;
    e.id = o["id"].toString();
    e.name = o["name"].toString();
    QString slotStr = o["slot"].toString();
    if (slotStr == "weapon") e.slot = EquipmentSlot::Weapon;
    else if (slotStr == "helmet") e.slot = EquipmentSlot::Helmet;
    else if (slotStr == "armor") e.slot = EquipmentSlot::Armor;
    else if (slotStr == "boots") e.slot = EquipmentSlot::Boots;
    else if (slotStr == "accessory1") e.slot = EquipmentSlot::Accessory1;
    else if (slotStr == "accessory2") e.slot = EquipmentSlot::Accessory2;
    else e.slot = EquipmentSlot::Mount;
    int qual = o["quality"].toInt(0);
    e.quality = static_cast<ItemQuality>(qual);
    e.bonus = Stats::fromJson(o["bonus"].toObject());
    e.specialEffect = o["specialEffect"].toString();
    e.price = o["price"].toInt(0);
    return e;
}

QJsonObject EquipmentData::toJson() const
{
    QString slotStr;
    switch (slot) {
        case EquipmentSlot::Weapon: slotStr = "weapon"; break;
        case EquipmentSlot::Helmet: slotStr = "helmet"; break;
        case EquipmentSlot::Armor: slotStr = "armor"; break;
        case EquipmentSlot::Boots: slotStr = "boots"; break;
        case EquipmentSlot::Accessory1: slotStr = "accessory1"; break;
        case EquipmentSlot::Accessory2: slotStr = "accessory2"; break;
        case EquipmentSlot::Mount: slotStr = "mount"; break;
    }
    return {
        {"id", id}, {"name", name}, {"slot", slotStr},
        {"quality", int(quality)}, {"bonus", bonus.toJson()},
        {"specialEffect", specialEffect},
        {"enhanceLevel", enhanceLevel}, {"price", price}
    };
}

// === ItemData ===
ItemData ItemData::fromJson(const QJsonObject &o)
{
    ItemData item;
    item.id = o["id"].toString();
    item.name = o["name"].toString();
    item.description = o["description"].toString();
    item.hpRestore = o["hpRestore"].toInt(0);
    item.mpRestore = o["mpRestore"].toInt(0);
    item.price = o["price"].toInt(0);
    return item;
}

// === GeneralTemplate ===
GeneralTemplate GeneralTemplate::fromJson(const QJsonObject &o)
{
    GeneralTemplate g;
    g.id = o["id"].toString();
    g.name = o["name"].toString();
    QString clsStr = o["class"].toString();
    if (clsStr == "warrior") g.charClass = CharacterClass::Warrior;
    else if (clsStr == "general") g.charClass = CharacterClass::General;
    else if (clsStr == "strategist") g.charClass = CharacterClass::Strategist;
    else g.charClass = CharacterClass::Healer;
    g.baseStats = Stats::fromJson(o["stats"].toObject());
    for (const auto &v : o["skills"].toArray())
        g.skillIds.append(v.toString());
    QString pers = o["personality"].toString();
    if (pers == "greedy") g.personality = Personality::Greedy;
    else if (pers == "cowardly") g.personality = Personality::Cowardly;
    else if (pers == "arrogant") g.personality = Personality::Arrogant;
    else g.personality = Personality::Loyal;
    g.loyalty = o["loyalty"].toInt(50);
    g.recruitHint = o["recruitHint"].toString();
    g.recruitCondition = o["recruitCondition"].toString();
    return g;
}

// === CharacterData ===
CharacterData CharacterData::fromTemplate(const GeneralTemplate &tmpl, int lvl)
{
    CharacterData c;
    c.templateId = tmpl.id;
    c.name = tmpl.name;
    c.charClass = tmpl.charClass;
    c.level = lvl;
    c.stats = tmpl.baseStats;
    c.stats.maxHp += (lvl - 1) * 20;
    c.stats.hp = c.stats.maxHp;
    c.stats.maxMp += (lvl - 1) * 10;
    c.stats.mp = c.stats.maxMp;
    c.stats.atk += (lvl - 1) * 3;
    c.stats.def += (lvl - 1) * 2;
    c.stats.spd += (lvl - 1) * 2;
    c.equippedSkills = tmpl.skillIds.mid(0, 5);
    return c;
}

QJsonObject CharacterData::toJson() const
{
    QJsonArray equipArr;
    for (const auto &e : equipment) equipArr.append(e.toJson());
    return {
        {"templateId", templateId}, {"name", name},
        {"level", level}, {"exp", exp},
        {"stats", stats.toJson()},
        {"skills", QJsonArray::fromStringList(equippedSkills)},
        {"equipment", equipArr},
        {"gold", gold}, {"isPlayer", isPlayer}
    };
}

CharacterData CharacterData::fromJson(const QJsonObject &o)
{
    CharacterData c;
    c.templateId = o["templateId"].toString();
    c.name = o["name"].toString();
    c.level = o["level"].toInt(1);
    c.exp = o["exp"].toInt(0);
    c.stats = Stats::fromJson(o["stats"].toObject());
    for (const auto &v : o["skills"].toArray())
        c.equippedSkills.append(v.toString());
    QJsonArray equipArr = o["equipment"].toArray();
    for (int i = 0; i < equipArr.size() && i < 7; ++i)
        c.equipment[i] = EquipmentData::fromJson(equipArr[i].toObject());
    c.gold = o["gold"].toInt(0);
    c.isPlayer = o["isPlayer"].toBool(false);
    return c;
}

// === MapData ===
MapData MapData::fromJson(const QJsonObject &o)
{
    MapData m;
    m.id = o["id"].toString();
    m.name = o["name"].toString();
    m.width = o["width"].toInt(10);
    m.height = o["height"].toInt(8);
    QJsonArray rows = o["tiles"].toArray();
    for (const auto &rowVal : rows) {
        QJsonArray row = rowVal.toArray();
        QVector<int> tileRow;
        for (const auto &t : row)
            tileRow.append(t.toInt());
        m.tiles.append(tileRow);
    }
    QJsonArray ents = o["entities"].toArray();
    for (const auto &ev : ents) {
        QJsonObject eo = ev.toObject();
        MapData::MapEntity e;
        e.type = eo["type"].toString();
        e.targetId = eo["targetId"].toString();
        e.x = eo["x"].toInt();
        e.y = eo["y"].toInt();
        m.entities.append(e);
    }
    QJsonArray exitsArr = o["exits"].toArray();
    for (const auto &ex : exitsArr) {
        QJsonObject eo = ex.toObject();
        MapData::Exit e;
        e.x = eo["x"].toInt();
        e.y = eo["y"].toInt();
        e.targetMapId = eo["targetMapId"].toString();
        m.exits.append(e);
    }
    return m;
}
