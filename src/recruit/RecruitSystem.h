#pragma once
#include <QObject>
#include "core/Types.h"

struct RecruitResult {
    bool success = false;
    QString message;
    CharacterData recruitedGeneral;
};

class RecruitSystem : public QObject {
    Q_OBJECT
public:
    explicit RecruitSystem(QObject *parent = nullptr);

    RecruitResult tryPersuade(const GeneralTemplate &target,
                               const CharacterData &persuader,
                               const QString &method);

    bool tryCapture(const GeneralTemplate &target);
    RecruitResult tryTorture(const GeneralTemplate &target, int &humiliation);

    bool checkHiddenCondition(const GeneralTemplate &target,
                              const QVector<CharacterData> &playerTeam,
                              int turnCount, const QString &killerId);

    QStringList getPersuadeMethods() const;

signals:
    void recruitSuccess(const QString &generalName);
    void recruitFailed(const QString &reason);
};
