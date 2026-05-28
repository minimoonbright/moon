#include "MainWindow.h"
#include "map/MapScene.h"
#include "map/MapView.h"
#include "map/PlayerSprite.h"
#include "battle/BattleWidget.h"
#include "character/Character.h"
#include "core/DataManager.h"
#include "recruit/RecruitSystem.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QStringList>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_stack = new QStackedWidget;
    setCentralWidget(m_stack);
    loadGameData();
    setupPlayerTeam();
    switchToMap("liu_bei_yard", 2, 2);
}

void MainWindow::loadGameData()
{
    DataManager::instance().loadAll("data");
}

void MainWindow::setupPlayerTeam()
{
    const auto *liubei = DataManager::instance().getGeneralTemplate("liu_bei");
    if (liubei) {
        auto *c = new Character(this);
        c->initFromTemplate(*liubei, 1);
        m_playerTeam.append(c);
    }
    const auto *guanyu = DataManager::instance().getGeneralTemplate("guan_yu");
    if (guanyu) {
        auto *c = new Character(this);
        c->initFromTemplate(*guanyu, 1);
        m_playerTeam.append(c);
    }
    const auto *zhangfei = DataManager::instance().getGeneralTemplate("zhang_fei");
    if (zhangfei) {
        auto *c = new Character(this);
        c->initFromTemplate(*zhangfei, 1);
        m_playerTeam.append(c);
    }
}

void MainWindow::switchToMap(const QString &mapId, int startX, int startY)
{
    const auto *mapData = DataManager::instance().getMap(mapId);
    if (!mapData) { qWarning() << "Map not found:" << mapId; return; }

    // 同地图战后恢复位置
    if (startX < 0 && mapId == m_currentMapId && m_savedPlayerX >= 0) {
        startX = m_savedPlayerX;
        startY = m_savedPlayerY;
    }
    m_currentMapId = mapId;
    m_savedPlayerX = m_savedPlayerY = -1;

    // 清理旧控件
    while (m_stack->count() > 0) {
        QWidget *w = m_stack->widget(0);
        m_stack->removeWidget(w);
        if (w != m_battleWidget) w->deleteLater();
    }

    m_mapScene = new MapScene(this);
    m_mapView = new MapView;
    m_mapView->setScene(m_mapScene);
    m_mapScene->loadMap(*mapData, startX, startY);

    connect(m_mapScene, &MapScene::mapExit, this, &MainWindow::onMapExit);
    connect(m_mapScene, &MapScene::battleTriggered, this, &MainWindow::onBattleTriggered);
    connect(m_mapScene, &MapScene::npcInteracted, this, &MainWindow::onNpcInteracted);
    connect(m_mapScene, &MapScene::buildingEntered, this, &MainWindow::onBuildingEntered);

    m_stack->addWidget(m_mapView);
    m_stack->setCurrentWidget(m_mapView);
    m_mapView->setFocus();
}

void MainWindow::switchToBattle(const QString &enemyTemplateId)
{
    const auto *enemyTmpl = DataManager::instance().getGeneralTemplate(enemyTemplateId);
    if (!enemyTmpl) return;

    QVector<Character *> enemies;
    auto *boss = new Character(this);
    boss->initFromTemplate(*enemyTmpl, enemyTmpl->baseStats.hp / 50 + 1);
    enemies.append(boss);

    const auto *soldier = DataManager::instance().getGeneralTemplate("huang_jin_soldier");
    if (soldier) {
        for (int i = 0; i < 2; ++i) {
            auto *s = new Character(this);
            s->initFromTemplate(*soldier, 1);
            enemies.append(s);
        }
    }

    for (auto *c : m_playerTeam) c->fullRestore();

    m_battleWidget = new BattleWidget;
    m_stack->addWidget(m_battleWidget);
    m_stack->setCurrentWidget(m_battleWidget);

    connect(m_battleWidget, &BattleWidget::battleFinished,
            this, &MainWindow::onBattleFinished);

    m_battleWidget->startBattle(m_playerTeam, enemies, enemyTemplateId);
}

void MainWindow::onBattleFinished(bool won, const QString &enemyId,
                                   int exp, int gold, QStringList,
                                   int turnCount, const QString &killerId, bool skillUsed)
{
    if (won) {
        m_gold += gold;
        for (auto *c : m_playerTeam) c->addExp(exp);

        const auto *enemyTmpl = DataManager::instance().getGeneralTemplate(enemyId);
        if (enemyTmpl && !enemyId.contains("soldier")) {
            doRecruit(*enemyTmpl, turnCount, killerId, skillUsed);
        }
    } else {
        for (auto *c : m_playerTeam) c->fullRestore();
    }
    switchToMap(m_currentMapId);
}

void MainWindow::doRecruit(const GeneralTemplate &enemyTmpl,
                            int turnCount, const QString &killerId, bool)
{
    RecruitSystem rs;
    CharacterData persuader;
    if (!m_playerTeam.isEmpty())
        persuader = m_playerTeam[0]->toData();

    QVector<CharacterData> teamData;
    for (auto *c : m_playerTeam) teamData.append(c->toData());

    if (rs.checkHiddenCondition(enemyTmpl, teamData, turnCount, killerId)) {
        auto result = rs.tryPersuade(enemyTmpl, persuader, "yi");
        result.success = true;
        result.message = QString::fromUtf8("%1\xe5\xbf\x83\xe6\x82\xa6\xe8\xaf\x9a\xe6\x9c\x8d\xe5\x9c\xb0\xe6\x8a\x95\xe5\x85\xa5\xe9\xba\xbe\xe4\xb8\x8b\xef\xbc\x81").arg(enemyTmpl.name);
        result.recruitedGeneral = CharacterData::fromTemplate(enemyTmpl, enemyTmpl.baseStats.hp / 50 + 1);
        QMessageBox::information(this, QString::fromUtf8("\xe6\x8b\x9b\xe9\x99\x8d\xe6\x88\x90\xe5\x8a\x9f"), result.message);
        addRecruitToTeam(result.recruitedGeneral);
        return;
    }

    QStringList recruitOptions;
    recruitOptions << QString::fromUtf8("\xf0\x9f\x92\xac \xe6\x88\x98\xe5\x90\x8e\xe5\x8a\x9d\xe9\x99\x8d");
    recruitOptions << QString::fromUtf8("\xe2\x9b\x93 \xe4\xbf\x98\xe8\x99\x8f");
    recruitOptions << QString::fromUtf8("\xf0\x9f\x9a\xab \xe6\x94\xbe\xe8\xb5\xb0");

    bool ok;
    QString action = QInputDialog::getItem(this,
        QString::fromUtf8("\xe5\xa4\x84\xe7\xbd\xae%1").arg(enemyTmpl.name),
        QString::fromUtf8("%1\xe5\xb7\xb2\xe8\xa2\xab\xe5\x87\xbb\xe8\xb4\xa5").arg(enemyTmpl.name),
        recruitOptions, 0, false, &ok);
    if (!ok) return;

    if (action.contains(QString::fromUtf8("\xe5\x8a\x9d\xe9\x99\x8d"))) {
        QStringList methods = rs.getPersuadeMethods();
        QString methodDesc = QInputDialog::getItem(this,
            QString::fromUtf8("\xe5\x8a\x9d\xe9\x99\x8d\xe6\x96\xb9\xe5\xbc\x8f"),
            QString::fromUtf8("\xe5\xaf\xb9%1\xe4\xbd\xbf\xe7\x94\xa8...").arg(enemyTmpl.name),
            methods, 0, false, &ok);
        if (!ok) return;

        QString methodKey;
        if (methodDesc.contains(QString::fromUtf8("\xe4\xb9\x89"))) methodKey = "yi";
        else if (methodDesc.contains(QString::fromUtf8("\xe5\x88\xa9"))) methodKey = "li";
        else if (methodDesc.contains(QString::fromUtf8("\xe6\xad\xa6"))) methodKey = "wu";
        else methodKey = "yi_da";

        auto result = rs.tryPersuade(enemyTmpl, persuader, methodKey);
        QMessageBox::information(this,
            result.success ? QString::fromUtf8("\xe6\x88\x90\xe5\x8a\x9f") : QString::fromUtf8("\xe5\xa4\xb1\xe8\xb4\xa5"),
            result.message);
        if (result.success) addRecruitToTeam(result.recruitedGeneral);

    } else if (action.contains(QString::fromUtf8("\xe4\xbf\x98\xe8\x99\x8f"))) {
        if (rs.tryCapture(enemyTmpl)) {
            QMessageBox::information(this,
                QString::fromUtf8("\xe4\xbf\x98\xe8\x99\x8f"),
                QString::fromUtf8("%1\xe5\xb7\xb2\xe8\xa2\xab\xe6\x8a\xbc\xe5\x85\xa5\xe5\x86\x9b\xe8\x90\xa5\xe3\x80\x82").arg(enemyTmpl.name));
            m_captives.append(enemyTmpl.id);
        } else {
            QMessageBox::information(this,
                QString::fromUtf8("\xe5\xa4\xb1\xe8\xb4\xa5"),
                QString::fromUtf8("%1\xe8\xb6\x81\xe4\xb9\xb1\xe9\x80\x83\xe8\xb5\xb0\xe4\xba\x86...").arg(enemyTmpl.name));
        }
    }
}

void MainWindow::addRecruitToTeam(const CharacterData &data)
{
    auto *newChar = new Character(this);
    newChar->initFromData(data);
    if (m_playerTeam.size() < 5)
        m_playerTeam.append(newChar);
    else
        m_reserves.append(newChar);
}

void MainWindow::onBuildingEntered(const QString &buildingId)
{
    if (buildingId == "inn") {
        for (auto *c : m_playerTeam) c->fullRestore();
        QMessageBox::information(this,
            QString::fromUtf8("\xe5\xae\xa2\xe6\xa0\x88"),
            QString::fromUtf8("HP/MP \xe5\xb7\xb2\xe5\x85\xa8\xe9\x83\xa8\xe6\x81\xa2\xe5\xa4\x8d\xef\xbc\x81"));
    } else if (buildingId == "residence") {
        if (m_reserves.isEmpty() && m_playerTeam.size() <= 3) {
            QMessageBox::information(this,
                QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf"),
                QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf\xe7\xa9\xba\xe9\x97\xb2\xe3\x80\x82\xe5\x8f\xaf\xe4\xbb\xa5\xe4\xbb\x8e\xe4\xba\xba\xe4\xba\x8b\xe5\xa4\x84\xe6\x8b\x9b\xe5\x8b\x9f\xe6\xad\xa6\xe5\xb0\x86\xe6\x9d\xa5\xe6\xad\xa4\xe4\xbc\x91\xe6\x81\xaf\xe3\x80\x82"));
            return;
        }

        QStringList options;
        options << QString::fromUtf8("\xe8\xb0\x83\xe5\x85\xa5\xe9\x98\x9f\xe4\xbc\x8d");
        options << QString::fromUtf8("\xe8\xb0\x83\xe5\x9b\x9e\xe5\x8e\xa2\xe6\x88\xbf");
        options << QString::fromUtf8("\xe6\x9f\xa5\xe7\x9c\x8b\xe9\x98\x9f\xe5\x91\x98");
        options << QString::fromUtf8("\xe7\xa6\xbb\xe5\xbc\x80");

        bool ok;
        QString act = QInputDialog::getItem(this,
            QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf - \xe6\xad\xa6\xe5\xb0\x86\xe7\xae\xa1\xe7\x90\x86"),
            QString::fromUtf8("\xe9\x98\x9f\xe4\xbc\x8d%1/5\xe4\xba\xba  \xe5\x8e\xa2\xe6\x88\xbf%2\xe4\xba\xba").arg(m_playerTeam.size()).arg(m_reserves.size()),
            options, 0, false, &ok);
        if (!ok || act.contains(QString::fromUtf8("\xe7\xa6\xbb\xe5\xbc\x80"))) return;

        if (act.contains(QString::fromUtf8("\xe8\xb0\x83\xe5\x85\xa5"))) {
            if (m_reserves.isEmpty()) {
                QMessageBox::information(this, QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf"), QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf\xe6\xb2\xa1\xe6\x9c\x89\xe6\xad\xa6\xe5\xb0\x86\xe3\x80\x82"));
                return;
            }
            QStringList names;
            for (auto *c : m_reserves)
                names << QString("%1 Lv.%2 HP:%3/%4")
                             .arg(c->name()).arg(c->level())
                             .arg(c->stats().hp).arg(c->stats().maxHp);

            QString choice = QInputDialog::getItem(this,
                QString::fromUtf8("\xe9\x80\x89\xe6\x8b\xa9\xe6\xad\xa6\xe5\xb0\x86"),
                QString::fromUtf8("\xe9\x80\x89\xe6\x8b\xa9\xe8\xa6\x81\xe5\x8a\xa0\xe5\x85\xa5\xe9\x98\x9f\xe4\xbc\x8d\xe7\x9a\x84\xe6\xad\xa6\xe5\xb0\x86\xef\xbc\x9a"),
                names, 0, false, &ok);
            if (!ok) return;
            int idx = names.indexOf(choice);
            if (idx < 0 || idx >= m_reserves.size()) return;

            Character *selected = m_reserves.takeAt(idx);

            if (m_playerTeam.size() >= 5) {
                QStringList teamNames;
                for (auto *c : m_playerTeam)
                    teamNames << QString("%1 Lv.%2").arg(c->name()).arg(c->level());

                QString swapChoice = QInputDialog::getItem(this,
                    QString::fromUtf8("\xe6\x9b\xbf\xe6\x8d\xa2\xe8\xb0\x81"),
                    QString::fromUtf8("\xe9\x98\x9f\xe4\xbc\x8d\xe5\xb7\xb2\xe6\xbb\xa1\xef\xbc\x8c\xe9\x80\x89\xe4\xb8\x80\xe4\xb8\xaa\xe6\x9b\xbf\xe6\x8d\xa2\xe5\x88\xb0\xe5\x8e\xa2\xe6\x88\xbf\xef\xbc\x9a"),
                    teamNames, 0, false, &ok);
                if (!ok) { m_reserves.append(selected); return; }
                int swapIdx = teamNames.indexOf(swapChoice);
                if (swapIdx < 0 || swapIdx >= m_playerTeam.size()) { m_reserves.append(selected); return; }

                Character *old = m_playerTeam.takeAt(swapIdx);
                m_reserves.append(old);
                m_playerTeam.insert(swapIdx, selected);
                QMessageBox::information(this, QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf"),
                    QString::fromUtf8("%1\xe5\x8a\xa0\xe5\x85\xa5\xe9\x98\x9f\xe4\xbc\x8d\xef\xbc\x8c%2\xe5\x9b\x9e\xe5\x8e\xa2\xe6\x88\xbf\xe4\xbc\x91\xe6\x81\xaf\xe3\x80\x82").arg(selected->name()).arg(old->name()));
            } else {
                m_playerTeam.append(selected);
                QMessageBox::information(this, QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf"),
                    QString::fromUtf8("%1\xe5\x8a\xa0\xe5\x85\xa5\xe9\x98\x9f\xe4\xbc\x8d\xef\xbc\x81").arg(selected->name()));
            }
        } else if (act.contains(QString::fromUtf8("\xe8\xb0\x83\xe5\x9b\x9e"))) {
            if (m_playerTeam.size() <= 1) {
                QMessageBox::information(this, QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf"), QString::fromUtf8("\xe8\x87\xb3\xe5\xb0\x91\xe4\xbf\x9d\xe7\x95\x991\xe4\xba\xba\xe5\x9c\xa8\xe9\x98\x9f\xe4\xbc\x8d\xe3\x80\x82"));
                return;
            }
            QStringList teamNames;
            for (auto *c : m_playerTeam)
                teamNames << QString("%1 Lv.%2").arg(c->name()).arg(c->level());

            QString choice = QInputDialog::getItem(this,
                QString::fromUtf8("\xe9\x80\x89\xe6\x8b\xa9\xe6\xad\xa6\xe5\xb0\x86"),
                QString::fromUtf8("\xe9\x80\x89\xe4\xb8\x80\xe4\xb8\xaa\xe5\x9b\x9e\xe5\x8e\xa2\xe6\x88\xbf\xe4\xbc\x91\xe6\x81\xaf\xef\xbc\x9a"),
                teamNames, 0, false, &ok);
            if (!ok) return;
            int idx = teamNames.indexOf(choice);
            if (idx < 0 || idx >= m_playerTeam.size()) return;

            Character *c = m_playerTeam.takeAt(idx);
            m_reserves.append(c);
            QMessageBox::information(this, QString::fromUtf8("\xe5\x8e\xa2\xe6\x88\xbf"),
                QString::fromUtf8("%1\xe5\xb7\xb2\xe5\x9b\x9e\xe5\x8e\xa2\xe6\x88\xbf\xe4\xbc\x91\xe6\x81\xaf\xe3\x80\x82").arg(c->name()));
        } else if (act.contains(QString::fromUtf8("\xe6\x9f\xa5\xe7\x9c\x8b"))) {
            QStringList info;
            for (int i = 0; i < m_playerTeam.size(); i++) {
                auto *c = m_playerTeam[i];
                const auto &s = c->stats();
                info << QString("%1. %2 Lv.%3 HP:%4/%5 ATK:%6 DEF:%7")
                            .arg(i + 1).arg(c->name()).arg(c->level())
                            .arg(s.hp).arg(s.maxHp).arg(s.atk).arg(s.def);
            }
            if (!m_reserves.isEmpty()) {
                info << "--- \xe5\x8e\xa2\xe6\x88\xbf ---";
                for (int i = 0; i < m_reserves.size(); i++) {
                    auto *c = m_reserves[i];
                    const auto &s = c->stats();
                    info << QString("%1. %2 Lv.%3 HP:%4/%5 ATK:%6 DEF:%7")
                                .arg(i + 1).arg(c->name()).arg(c->level())
                                .arg(s.hp).arg(s.maxHp).arg(s.atk).arg(s.def);
                }
            }
            QMessageBox::information(this, QString::fromUtf8("\xe9\x98\x9f\xe4\xbc\x8d\xe4\xbf\xa1\xe6\x81\xaf"), info.join("\n"));
        }
    } else if (buildingId == "ren_shi_chu") {
        QVector<const GeneralTemplate *> allGens = DataManager::instance().allGeneralTemplates();
        QVector<const GeneralTemplate *> candidates;
        for (auto *g : allGens) {
            if (g->id == "huang_jin_soldier") continue;
            if (g->id == "liu_bei" || g->id == "guan_yu" || g->id == "zhang_fei") continue;
            bool alreadyHave = false;
            for (auto *c : m_playerTeam) if (c->toData().templateId == g->id) { alreadyHave = true; break; }
            for (auto *c : m_reserves) if (c->toData().templateId == g->id) { alreadyHave = true; break; }
            for (const auto &cid : m_captives) if (cid == g->id) { alreadyHave = true; break; }
            if (!alreadyHave) candidates.append(g);
        }
        if (candidates.isEmpty()) {
            QMessageBox::information(this,
                QString::fromUtf8("\xe4\xba\xba\xe4\xba\x8b\xe5\xa4\x84"),
                QString::fromUtf8("\xe6\x9a\x82\xe6\x97\xb6\xe6\xb2\xa1\xe6\x9c\x89\xe4\xba\xba\xe6\x9d\xa5\xe5\xba\x94\xe8\x81\x98..."));
            return;
        }

        int nCandidates = qMin(candidates.size(), 3);
        QVector<int> picked;
        while (picked.size() < nCandidates) {
            int r = QRandomGenerator::global()->bounded(candidates.size());
            if (!picked.contains(r)) picked.append(r);
        }

        QStringList names;
        for (int idx : picked) names << candidates[idx]->name;

        bool ok;
        QString choice = QInputDialog::getItem(this,
            QString::fromUtf8("\xe4\xba\xba\xe4\xba\x8b\xe5\xa4\x84 - \xe6\x8b\x9b\xe8\x81\x98"),
            QString::fromUtf8("\xe4\xbb\xa5\xe4\xb8\x8b\xe6\xad\xa6\xe5\xb0\x86\xe5\x89\x8d\xe6\x9d\xa5\xe5\xba\x94\xe8\x81\x98\xef\xbc\x8c\xe9\x80\x89\xe6\x8b\xa9\xe4\xb8\x80\xe4\xb8\xaa\xe5\x8a\xa0\xe5\x85\xa5\xef\xbc\x9a"),
            names, 0, false, &ok);
        if (!ok) return;

        int chosenIdx = names.indexOf(choice);
        if (chosenIdx < 0 || chosenIdx >= picked.size()) return;

        const auto *tmpl = candidates[picked[chosenIdx]];
        auto *newChar = new Character(this);
        newChar->initFromTemplate(*tmpl, tmpl->baseStats.hp / 50 + 1);
        m_reserves.append(newChar);
        QMessageBox::information(this,
            QString::fromUtf8("\xe5\x85\xa5\xe8\x81\x8c"),
            QString::fromUtf8("%1\xe5\x8a\xa0\xe5\x85\xa5\xe4\xba\x86\xe4\xbd\xa0\xe7\x9a\x84\xe9\xba\xbe\xe4\xb8\x8b\xef\xbc\x8c\xe6\xad\xa3\xe5\x9c\xa8\xe5\x8e\xa2\xe6\x88\xbf\xe5\xbe\x85\xe5\x91\xbd\xe3\x80\x82").arg(tmpl->name));
    } else if (buildingId == "barracks") {
        if (m_captives.isEmpty()) {
            QMessageBox::information(this,
                QString::fromUtf8("\xe5\x86\x9b\xe8\x90\xa5"),
                QString::fromUtf8("\xe6\xb2\xa1\xe6\x9c\x89\xe4\xbf\x98\xe8\x99\x8f\xe3\x80\x82\xe5\x87\xbb\xe8\xb4\xa5\xe6\x95\x8c\xe5\xb0\x86\xe5\x90\x8e\xe5\x8f\xaf\xe9\x80\x89\xe4\xbf\x98\xe8\x99\x8f\xe3\x80\x82"));
            return;
        }
        QStringList captiveNames;
        for (const auto &cid : m_captives) {
            const auto *t = DataManager::instance().getGeneralTemplate(cid);
            captiveNames << (t ? t->name : cid);
        }
        bool ok;
        QString choice = QInputDialog::getItem(this,
            QString::fromUtf8("\xe5\x86\x9b\xe8\x90\xa5 - \xe4\xbf\x98\xe8\x99\x8f"),
            QString::fromUtf8("\xe9\x80\x89\xe6\x8b\xa9\xe4\xb8\x80\xe4\xb8\xaa\xe4\xbf\x98\xe8\x99\x8f\xe5\xa4\x84\xe7\x90\x86"),
            captiveNames, 0, false, &ok);
        if (!ok) return;

        int capIdx = captiveNames.indexOf(choice);
        if (capIdx < 0 || capIdx >= m_captives.size()) return;
        processCaptive(capIdx);
    }
}

void MainWindow::processCaptive(int idx)
{
    if (idx < 0 || idx >= m_captives.size()) return;
    const auto *tmpl = DataManager::instance().getGeneralTemplate(m_captives[idx]);
    if (!tmpl) { m_captives.removeAt(idx); return; }

    int humiliation = 0;
    while (true) {
        QStringList options;
        options << QString::fromUtf8("\xf0\x9f\x94\xa8 \xe6\x8b\xb7\xe6\x89\x93 (%1/100)").arg(humiliation);
        options << QString::fromUtf8("\xe2\x9c\x85 \xe9\x87\x8a\xe6\x94\xbe");
        options << QString::fromUtf8("\xe2\x86\xa9 \xe4\xb8\x8d\xe7\xae\xa1\xe4\xba\x86");

        bool ok;
        QString act = QInputDialog::getItem(this,
            QString::fromUtf8("\xe4\xbf\x98\xe8\x99\x8f:%1  \xe5\xbf\xa0\xe4\xb9\x89:%2").arg(tmpl->name).arg(tmpl->loyalty),
            QString::fromUtf8("\xe6\xaf\x8f\xe6\xac\xa1\xe6\x8b\xb7\xe6\x89\x93+10~30\xe5\xb1\x88\xe8\xbe\xb1\xe5\xba\xa6\xef\xbc\x8c\xe6\xbb\xa1100\xe5\xb1\x88\xe6\x9c\x8d\xe3\x80\x825%%\xe9\x80\x83\xe8\x84\xb1"),
            options, 0, false, &ok);

        if (!ok || act.contains(QString::fromUtf8("\xe4\xb8\x8d\xe7\xae\xa1"))) {
            break;
        } else if (act.contains(QString::fromUtf8("\xe9\x87\x8a\xe6\x94\xbe"))) {
            QMessageBox::information(this,
                QString::fromUtf8("\xe9\x87\x8a\xe6\x94\xbe"),
                QString::fromUtf8("\xe4\xbd\xa0\xe9\x87\x8a\xe6\x94\xbe\xe4\xba\x86%1\xe3\x80\x82").arg(tmpl->name));
            m_captives.removeAt(idx);
            break;
        } else if (act.contains(QString::fromUtf8("\xe6\x8b\xb7\xe6\x89\x93"))) {
            RecruitSystem rs;
            auto result = rs.tryTorture(*tmpl, humiliation);

            if (result.success) {
                QMessageBox::information(this,
                    QString::fromUtf8("\xe5\xb1\x88\xe6\x9c\x8d\xef\xbc\x81"),
                    result.message);
                addRecruitToTeam(result.recruitedGeneral);
                m_captives.removeAt(idx);
                break;
            } else if (result.message.contains(QString::fromUtf8("\xe9\x80\x83\xe8\x84\xb1"))) {
                QMessageBox::information(this,
                    QString::fromUtf8("\xe9\x80\x83\xe8\xb5\xb0"),
                    result.message);
                m_captives.removeAt(idx);
                break;
            } else {
                QMessageBox::information(this,
                    QString::fromUtf8("\xe6\x8b\xb7\xe6\x89\x93"),
                    result.message);
            }
        }
    }
}

void MainWindow::onMapExit(const QString &targetMapId) { switchToMap(targetMapId); }
void MainWindow::onBattleTriggered(const QString &monsterId)
{
    if (m_mapScene && m_mapScene->player()) {
        m_savedPlayerX = m_mapScene->player()->gridX();
        m_savedPlayerY = m_mapScene->player()->gridY();
    }
    switchToBattle(monsterId);
}
void MainWindow::onNpcInteracted(const QString &npcId) { qDebug() << "NPC:" << npcId; }
