#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QPixmap>
#include "sipCall.h"
#include "global.h"
#include "callwidget.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();




private:
    Ui::MainWidget *ui;

    CallWidget callWidget;



private slots:
    void set_image_slots(const QImage &image);

    void on_sip_invite_pushButton_clicked();

    void show_call_widget_receive_call_page();
    void show_call_widget_ringing_page();
    void show_call_widget_calling_page();
    void hide_call_widget();





};

#endif // MAINWIDGET_H
