#include "callwidget.h"
#include "ui_callwidget.h"

CallWidget::CallWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CallWidget)
{
    ui->setupUi(this);

    connect(&g_videoStreamReceiver, SIGNAL(get_image(QImage)), this, SLOT(set_image_slots(QImage)));

}

CallWidget::~CallWidget()
{
    delete ui;
}



void CallWidget::set_image_slots(const QImage &image)
{
    if (image.height()>0){
        QPixmap pix = QPixmap::fromImage(image.scaled(g_v4l2_video.get_width(),g_v4l2_video.get_height()));
        ui->video_label->setPixmap(pix);
        image.save("/mnt/image.jpg", "JPG");
    }
}


void CallWidget::on_sip_answer_pushButton_clicked()
{
    g_sipCall.send_answer_200_ok();
    ui->stackedWidget->setCurrentIndex(2);
}

void CallWidget::on_sip_refuse_invite_pushButton_clicked()
{
    g_sipCall.refuse_invite();
    this->hide();
}

void CallWidget::on_sip_cancel_pushButton_clicked()
{
    g_sipCall.cancel();
    this->hide();
}


void CallWidget::on_sip_calling_hangUp_pushButton_clicked()
{
    g_sipCall.hang_out();
    this->hide();
}


void CallWidget::show_receive_call_page()
{
    this->show();
    ui->stackedWidget->setCurrentIndex(0);
}

void CallWidget::show_ringing_page()
{
    this->show();
    ui->stackedWidget->setCurrentIndex(1);
}

void CallWidget::show_calling_page()
{
    this->show();
    ui->stackedWidget->setCurrentIndex(2);
}



