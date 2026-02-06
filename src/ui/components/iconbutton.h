#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QObject>
#include <QPushButton>

class IconButton : public QPushButton
{
    Q_OBJECT
public:
    IconButton(QWidget *parent = nullptr);

    void setIconText(uint code);
};

#endif // ICONBUTTON_H
