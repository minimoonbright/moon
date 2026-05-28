#include "DataManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

DataManager &DataManager::instance()
{
    static DataManager dm;
    return dm;
}

static QJsonObject readJsonFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Cannot open" << path;
        return {};
    }
    return QJsonDocument::fromJson(file.readAll()).object();
}

bool DataManager::loadAll(const QString &dataDir)
{
    auto loadArray = [&](const QString &filename, auto &map, auto fromJsonFn) {
        QString path = dataDir + "/" + filename;
        QJsonObject root = readJsonFile(path);
        QJsonArray arr = root["data"].toArray();
        for (const auto &v : arr) {
            QJsonObject obj = v.toObject();
            auto item = fromJsonFn(obj);
            map[item.id] = item;
        }
    };

    loadArray("generals.json", m_generals,
        [](const QJsonObject &o) { return GeneralTemplate::fromJson(o); });
    loadArray("skills.json", m_skills,
        [](const QJsonObject &o) { return SkillData::fromJson(o); });
    loadArray("equipment.json", m_equipments,
        [](const QJsonObject &o) { return EquipmentData::fromJson(o); });
    loadArray("items.json", m_items,
        [](const QJsonObject &o) { return ItemData::fromJson(o); });
    loadArray("maps.json", m_maps,
        [](const QJsonObject &o) { return MapData::fromJson(o); });

    qDebug() << "Loaded:" << m_generals.size() << "generals,"
             << m_skills.size() << "skills,"
             << m_equipments.size() << "equipments,"
             << m_items.size() << "items,"
             << m_maps.size() << "maps";
    return true;
}

const GeneralTemplate *DataManager::getGeneralTemplate(const QString &id) const
{
    auto it = m_generals.constFind(id);
    return it != m_generals.constEnd() ? &*it : nullptr;
}
const SkillData *DataManager::getSkill(const QString &id) const
{
    auto it = m_skills.constFind(id);
    return it != m_skills.constEnd() ? &*it : nullptr;
}
const EquipmentData *DataManager::getEquipment(const QString &id) const
{
    auto it = m_equipments.constFind(id);
    return it != m_equipments.constEnd() ? &*it : nullptr;
}
const ItemData *DataManager::getItem(const QString &id) const
{
    auto it = m_items.constFind(id);
    return it != m_items.constEnd() ? &*it : nullptr;
}
const MapData *DataManager::getMap(const QString &id) const
{
    auto it = m_maps.constFind(id);
    return it != m_maps.constEnd() ? &*it : nullptr;
}

QVector<const GeneralTemplate *> DataManager::allGeneralTemplates() const
{
    QVector<const GeneralTemplate *> result;
    for (auto it = m_generals.constBegin(); it != m_generals.constEnd(); ++it)
        result.append(&*it);
    return result;
}
QVector<const SkillData *> DataManager::allSkills() const
{
    QVector<const SkillData *> result;
    for (auto it = m_skills.constBegin(); it != m_skills.constEnd(); ++it)
        result.append(&*it);
    return result;
}
QVector<const EquipmentData *> DataManager::allEquipments() const
{
    QVector<const EquipmentData *> result;
    for (auto it = m_equipments.constBegin(); it != m_equipments.constEnd(); ++it)
        result.append(&*it);
    return result;
}
QVector<const MapData *> DataManager::allMaps() const
{
    QVector<const MapData *> result;
    for (auto it = m_maps.constBegin(); it != m_maps.constEnd(); ++it)
        result.append(&*it);
    return result;
}
