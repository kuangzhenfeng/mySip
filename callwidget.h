#ifndef CALLWIDGET_H
#define CALLWIDGET_H

#include <QWidget>
#include "global.h"

namespace Ui {
class CallWidget;
}

class CallWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CallWidget(QWidget *parent = nullptr);
    ~CallWidget();

    void show_receive_call_page();
    void show_ringing_page();
    void show_calling_page();


private:
    Ui::CallWidget *ui;

private slots:
    void set_image_slots(const QImage &image);

    void on_sip_answer_pushButton_clicked();
    void on_sip_refuse_invite_pushButton_clicked();
    void on_sip_cancel_pushButton_clicked();
    void on_sip_calling_hangUp_pushButton_clicked();

};

#endif // CALLWIDGET_H
