#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include "BattleEngine.h"

class BattleFieldWidget : public QWidget {
    Q_OBJECT
public:
    explicit BattleFieldWidget(QWidget *parent = nullptr);
    void setBattleEngine(BattleEngine *engine);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    BattleEngine *m_engine = nullptr;
    void drawUnit(QPainter &p, const BattleUnit &unit, bool isEnemy, const QRect &rect);
};

class BattleWidget : public QWidget {
    Q_OBJECT
public:
    explicit BattleWidget(QWidget *parent = nullptr);

    void startBattle(const QVector<Character *> &playerTeam,
                     const QVector<Character *> &enemyTeam,
                     const QString &enemyLeaderId = "");

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void battleFinished(bool won, const QString &enemyLeaderId,
                        int exp, int gold, QStringList loot,
                        int turnCount, const QString &killerId, bool skillUsed);

private slots:
    void onAttack();
    void onSkill();
    void onItem();
    void onView();
    void onDefend();
    void onFlee();
    void onTurnChanged(int unitIndex);
    void onBattleEnd(bool playerWon);

private:
    void setCommandsEnabled(bool enabled);
    void updateStatusArea();

    BattleEngine *m_engine;
    BattleFieldWidget *m_battleField;
    QTextEdit *m_logView;
    QPushButton *m_btnAttack, *m_btnSkill, *m_btnDefend, *m_btnItem, *m_btnFlee;
    QLabel *m_statusLabel;
    QLabel *m_turnLabel;
    QPushButton *m_btnView;
    QString m_enemyLeaderId;
    int m_selectedUnitIndex = -1;
};
