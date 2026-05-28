#pragma once
#include "Types.h"
#include <QMap>

class DataManager {
public:
    static DataManager &instance();

    bool loadAll(const QString &dataDir);

    const GeneralTemplate *getGeneralTemplate(const QString &id) const;
    const SkillData      *getSkill(const QString &id) const;
    const EquipmentData  *getEquipment(const QString &id) const;
    const ItemData       *getItem(const QString &id) const;
    const MapData        *getMap(const QString &id) const;

    QVector<const GeneralTemplate *> allGeneralTemplates() const;
    QVector<const SkillData *>       allSkills() const;
    QVector<const EquipmentData *>   allEquipments() const;
    QVector<const MapData *>         allMaps() const;

private:
    DataManager() = default;
    QMap<QString, GeneralTemplate> m_generals;
    QMap<QString, SkillData>       m_skills;
    QMap<QString, EquipmentData>   m_equipments;
    QMap<QString, ItemData>        m_items;
    QMap<QString, MapData>         m_maps;
};
