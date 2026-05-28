#pragma once
#include <QString>
#include "Types.h"

struct SaveData {
    QString currentMapId;
    QVector<CharacterData> playerTeam;
    QVector<CharacterData> reserves;
    int gold = 0;
    QMap<QString, int> items;
    QMap<QString, int> questProgress;
};

class SaveManager {
public:
    static bool saveToFile(const QString &path, const SaveData &data);
    static SaveData loadFromFile(const QString &path);
};
