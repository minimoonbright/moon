#include "SaveManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

bool SaveManager::saveToFile(const QString &path, const SaveData &data)
{
    QJsonObject root;
    root["currentMapId"] = data.currentMapId;
    root["gold"] = data.gold;

    QJsonArray teamArr;
    for (const auto &c : data.playerTeam)
        teamArr.append(c.toJson());
    root["playerTeam"] = teamArr;

    QJsonArray reserveArr;
    for (const auto &c : data.reserves)
        reserveArr.append(c.toJson());
    root["reserves"] = reserveArr;

    QJsonObject itemObj;
    for (auto it = data.items.begin(); it != data.items.end(); ++it)
        itemObj[it.key()] = it.value();
    root["items"] = itemObj;

    QJsonObject questObj;
    for (auto it = data.questProgress.begin(); it != data.questProgress.end(); ++it)
        questObj[it.key()] = it.value();
    root["questProgress"] = questObj;

    QFile file(path);
    if (!file.open(QFile::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson());
    return true;
}

SaveData SaveManager::loadFromFile(const QString &path)
{
    SaveData data;
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) return data;

    QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    data.currentMapId = root["currentMapId"].toString();
    data.gold = root["gold"].toInt();

    for (const auto &v : root["playerTeam"].toArray())
        data.playerTeam.append(CharacterData::fromJson(v.toObject()));
    for (const auto &v : root["reserves"].toArray())
        data.reserves.append(CharacterData::fromJson(v.toObject()));

    QJsonObject itemObj = root["items"].toObject();
    for (auto it = itemObj.begin(); it != itemObj.end(); ++it)
        data.items[it.key()] = it.value().toInt();

    return data;
}
