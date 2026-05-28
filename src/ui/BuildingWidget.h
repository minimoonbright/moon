#pragma once
#include <QWidget>
#include <QLabel>

class BuildingWidget : public QWidget {
    Q_OBJECT
public:
    explicit BuildingWidget(QWidget *parent = nullptr);
    void setBuildingType(const QString &type);
};
