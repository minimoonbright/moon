#include "EquipWidget.h"
#include <QLabel>
#include <QVBoxLayout>

EquipWidget::EquipWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *label = new QLabel(QString::fromUtf8("\xe8\xa3\x85\xe5\xa4\x87\xe7\x95\x8c\xe9\x9d\xa2\xe6\x9c\xaa\xe5\xae\x9e\xe7\x8e\xb0"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}
