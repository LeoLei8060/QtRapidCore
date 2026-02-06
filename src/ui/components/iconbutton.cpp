#include "iconbutton.h"

IconButton::IconButton(QWidget *parent)
    : QPushButton(parent)
{}

void IconButton::setIconText(uint code)
{
    this->setText(QChar(code));
}
