#include "mypushbutton.h"
#include <QGraphicsEffect>

MyPushButton::MyPushButton(QWidget *parent): QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
//    this->setAutoFillBackground(true);


    connect(this, &MyPushButton::pressed, this, &MyPushButton::buttonPressed);
    connect(this, &MyPushButton::released, this, &MyPushButton::buttonReleased);
}


void MyPushButton::buttonPressed()
{
//    QGraphicsDropShadowEffect *dropShadowEffect = new QGraphicsDropShadowEffect(this);
//    dropShadowEffect->setOffset(1, 1);
//    this->setGraphicsEffect(dropShadowEffect);  // 设置阴影效果

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
    opacityEffect->setOpacity(0.7);
    this->setGraphicsEffect(opacityEffect);     // 设置按钮透明度

}

void MyPushButton::buttonReleased()
{
//    QGraphicsDropShadowEffect *dropShadowEffect = new QGraphicsDropShadowEffect(this);
//    dropShadowEffect->setOffset(0,0);
//    this->setGraphicsEffect(dropShadowEffect);  // 设置阴影效果

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
    opacityEffect->setOpacity(1);
    this->setGraphicsEffect(opacityEffect);     // 设置按钮透明度
}
