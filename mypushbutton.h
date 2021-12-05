#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>

class MyPushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MyPushButton(QWidget *parent = nullptr);

protected:

private slots:
    void buttonPressed();
    void buttonReleased();
};

#endif // MYPUSHBUTTON_H
