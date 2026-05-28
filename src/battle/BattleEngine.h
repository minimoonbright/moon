#pragma once
#include <QObject>
#include <QVector>
#include <QColor>
#include "character/Character.h"

struct BattleUnit {
    Character *character = nullptr;
    int row = 0;
    bool isEnemy = false;
    int battleIndex = 0;
};

struct BattleLogEntry {
    QString text;
    QColor color;
};

class BattleEngine : public QObject {
    Q_OBJECT
public:
    explicit BattleEngine(QObject *parent = nullptr);

    void initBattle(const QVector<Character *> &playerTeam,
                    const QVector<Character *> &enemyTeam);
    void startBattle();
    void executeAction(int unitIndex, BattleAction action,
                       const QString &skillId = "", int targetIndex = -1);
    void executeFlee();

    bool isBattleOver() const { return m_battleOver; }
    bool playerWon() const { return m_playerWon; }
    int expReward() const { return m_expReward; }
    int goldReward() const { return m_goldReward; }
    QStringList lootItems() const { return m_lootItems; }
    int turnCount() const { return m_turnCount; }
    QString killerTemplateId() const { return m_killerTemplateId; }
    bool skillWasUsed() const { return m_skillUsed; }

    const QVector<BattleUnit> &playerUnits() const { return m_playerUnits; }
    const QVector<BattleUnit> &enemyUnits() const { return m_enemyUnits; }
    const QVector<BattleLogEntry> &battleLog() const { return m_log; }
    bool isPlayersTurn() const;

signals:
    void logAdded(const BattleLogEntry &entry);
    void battleEnded(bool playerWon);
    void turnChanged(int unitIndex);

private:
    void buildTurnOrder();
    void advanceTurn();
    void checkDefeated(int unitIndex);
    void addLog(const QString &text, QColor color = Qt::white);
    int calcSkillDamage(const BattleUnit &attacker, const BattleUnit &defender,
                        const SkillData &skill);

    QVector<BattleUnit> m_playerUnits;
    QVector<BattleUnit> m_enemyUnits;
    QVector<int> m_turnOrder;
    QVector<BattleLogEntry> m_log;
    int m_turnIndex = 0;
    bool m_battleOver = false;
    bool m_playerWon = false;
    int m_expReward = 0;
    int m_goldReward = 0;
    QStringList m_lootItems;
    int m_turnCount = 0;
    QString m_killerTemplateId;
    bool m_skillUsed = false;
};
