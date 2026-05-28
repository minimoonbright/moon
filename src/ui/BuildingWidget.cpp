#include "BuildingWidget.h"
#include <QVBoxLayout>

BuildingWidget::BuildingWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *label = new QLabel(QString::fromUtf8("\xe5\xbb\xba\xe7\xad\x91\xe6\x9c\xaa\xe5\xae\x9e\xe7\x8e\xb0"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}

void BuildingWidget::setBuildingType(const QString &) {}
