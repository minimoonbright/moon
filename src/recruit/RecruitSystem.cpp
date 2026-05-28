#include "RecruitSystem.h"
#include <QRandomGenerator>

RecruitSystem::RecruitSystem(QObject *parent) : QObject(parent) {}

QStringList RecruitSystem::getPersuadeMethods() const
{
    return {QString::fromUtf8("\xe4\xbb\xa5\xe4\xb9\x89\xe7\x9b\xb8\xe5\x8a\x9d"),
            QString::fromUtf8("\xe4\xbb\xa5\xe5\x88\xa9\xe7\x9b\xb8\xe8\xaf\xb1"),
            QString::fromUtf8("\xe4\xbb\xa5\xe6\xad\xa6\xe7\x9b\xb8\xe9\x80\xbc"),
            QString::fromUtf8("\xe5\xa4\xa7\xe4\xb9\x89\xe5\x87\x9b\xe7\x84\xb6")};
}

RecruitResult RecruitSystem::tryPersuade(const GeneralTemplate &target,
                                          const CharacterData &persuader,
                                          const QString &method)
{
    RecruitResult result;
    int baseChance = 30;

    if (method == "yi" && target.personality == Personality::Loyal)
        baseChance += 40;
    else if (method == "li" && target.personality == Personality::Greedy)
        baseChance += 40;
    else if (method == "wu" && target.personality == Personality::Cowardly)
        baseChance += 40;
    else if (method == "yi_da" && target.personality == Personality::Arrogant)
        baseChance += 40;

    baseChance += (persuader.stats.benevolence - 30) / 5;
    baseChance += (persuader.stats.charisma - 30) / 5;

    int roll = QRandomGenerator::global()->bounded(100);
    if (roll < baseChance) {
        result.success = true;
        result.message = QString("%1 \xe8\xa2\xab\xe4\xbd\xa0\xe7\x9a\x84\xe8\xaf\x9a\xe6\x84\x8f\xe6\x89\x93\xe5\x8a\xa8\xef\xbc\x8c\xe6\x84\xbf\xe6\x84\x8f\xe5\x8a\xa0\xe5\x85\xa5\xe9\xba\xbe\xe4\xb8\x8b\xef\xbc\x81").arg(target.name);
        result.recruitedGeneral = CharacterData::fromTemplate(target, target.baseStats.hp / 50 + 1);
    } else {
        result.success = false;
        result.message = QString("%1 \xe6\x8b\x92\xe7\xbb\x9d\xe4\xba\x86\xe4\xbd\xa0\xe7\x9a\x84\xe6\x8b\x9b\xe6\x8f\xbd").arg(target.name);
    }
    return result;
}

bool RecruitSystem::tryCapture(const GeneralTemplate &target)
{
    int baseRate = 90;
    return QRandomGenerator::global()->bounded(100) < baseRate;
}

RecruitResult RecruitSystem::tryTorture(const GeneralTemplate &target, int &humiliation)
{
    RecruitResult result;
    int gain = 10 + QRandomGenerator::global()->bounded(20);
    humiliation += gain;

    if (humiliation >= 100) {
        result.success = true;
        result.message = QString("%1 \xe7\xbb\x88\xe4\xba\x8e\xe5\xb1\x88\xe6\x9c\x8d\xef\xbc\x8c\xe6\x84\xbf\xe6\x84\x8f\xe5\xbd\x92\xe9\x99\x8d\xef\xbc\x81").arg(target.name);
        result.recruitedGeneral = CharacterData::fromTemplate(target, target.baseStats.hp / 50 + 1);
    } else {
        int riskRoll = QRandomGenerator::global()->bounded(100);
        if (riskRoll < 5) {
            result.success = false;
            result.message = QString("%1 \xe5\xae\x81\xe6\xad\xbb\xe4\xb8\x8d\xe5\xb1\x88\xef\xbc\x8c\xe8\xb6\x81\xe5\xa4\x9c\xe8\x89\xb2\xe9\x80\x83\xe8\x84\xb1\xe4\xba\x86...").arg(target.name);
        } else {
            result.success = false;
            result.message = QString("\xe6\x8b\xb7\xe6\x89\x93 %1... \xe5\xb1\x88\xe8\xbe\xb1\xe5\xba\xa6 +%2 (\xe5\xbd\x93\xe5\x89\x8d %3/100)")
                                 .arg(target.name).arg(gain).arg(humiliation);
        }
    }
    return result;
}

bool RecruitSystem::checkHiddenCondition(const GeneralTemplate &target,
                                          const QVector<CharacterData> &playerTeam,
                                          int turnCount, const QString &killerId)
{
    QString cond = target.recruitCondition;
    if (cond.isEmpty()) return false;

    if (cond == "guanyu_vs_zhoucang") return killerId == "guan_yu";
    if (cond == "no_skill_liaohua") return true;
    if (cond == "turn_limit_guanhai") return turnCount <= 10;
    if (cond == "three_yellow_turban") {
        int count = 0;
        for (const auto &c : playerTeam) {
            if (c.templateId == "zhang_yan" || c.templateId == "liu_pi" ||
                c.templateId == "gong_du") count++;
        }
        return count >= 3;
    }
    return false;
}
