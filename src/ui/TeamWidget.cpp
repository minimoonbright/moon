#include "TeamWidget.h"
#include <QLabel>
#include <QVBoxLayout>

TeamWidget::TeamWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *label = new QLabel(QString::fromUtf8("\xe9\x98\x9f\xe4\xbc\x8d\xe7\x95\x8c\xe9\x9d\xa2\xe6\x9c\xaa\xe5\xae\x9e\xe7\x8e\xb0"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}
