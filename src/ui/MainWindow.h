#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include "core/Types.h"

class MapView;
class MapScene;
class BattleWidget;
class Character;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onMapExit(const QString &targetMapId);
    void onBattleTriggered(const QString &monsterId);
    void onNpcInteracted(const QString &npcId);
    void onBuildingEntered(const QString &buildingId);
    void onBattleFinished(bool won, const QString &enemyId,
                          int exp, int gold, QStringList loot,
                          int turnCount, const QString &killerId, bool skillUsed);

private:
    void loadGameData();
    void switchToMap(const QString &mapId, int startX = -1, int startY = -1);
    void switchToBattle(const QString &enemyTemplateId);
    void setupPlayerTeam();
    void addRecruitToTeam(const CharacterData &data);
    void doRecruit(const GeneralTemplate &enemyTmpl, int turnCount,
                   const QString &killerId, bool skillUsed);
    void processCaptive(int idx);

    QStackedWidget *m_stack;
    MapView *m_mapView = nullptr;
    MapScene *m_mapScene = nullptr;
    BattleWidget *m_battleWidget = nullptr;
    QVector<Character *> m_playerTeam;
    QVector<Character *> m_reserves;
    QString m_currentMapId;
    int m_gold = 100;
    QMap<QString, int> m_items;
    QStringList m_captives;
};
