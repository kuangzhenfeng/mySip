#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDebug>
#include <QFontDatabase>



void forn_test(void);

extern int sip_main(void);


MainWidget::MainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    qDebug() << "SUCCESS!!!" <<endl;

    connect(&g_sipCall, &SipCall::show_call_widget_receive_call_page, this, &MainWidget::show_call_widget_receive_call_page);
    connect(&g_sipCall, &SipCall::show_call_widget_ringing_page, this, &MainWidget::show_call_widget_ringing_page);
    connect(&g_sipCall, &SipCall::show_call_widget_calling_page, this, &MainWidget::show_call_widget_calling_page);
    connect(&g_sipCall, &SipCall::hide_call_widget, this, &MainWidget::hide_call_widget);
    connect(&g_opencv_process, SIGNAL(get_image(QImage)), this, SLOT(set_image_slots(QImage)));






    forn_test();


}

MainWidget::~MainWidget()
{
    delete ui;
}



void forn_test(void)
{
    QFontDatabase database;

    foreach (const QString &family, database.families())
    {
       qDebug()<<family;
    }

    qDebug()<<"\r\n now is chinese font. \r\n";
    //下面为支持简体中文字体库
    foreach (const QString &family, database.families(QFontDatabase::SimplifiedChinese))
    {
       qDebug()<<family;
    }
}

void MainWidget::set_image_slots(const QImage &image)
{
    if (image.height()>0){
        QPixmap pix = QPixmap::fromImage(image.scaled(g_v4l2_video.get_width(),g_v4l2_video.get_height()));
        ui->video_label->setPixmap(pix);
//        image.save("/mnt/image.jpg", "JPG");
    }
}


void MainWidget::on_sip_invite_pushButton_clicked()
{
    g_sipCall.invite();
}



void MainWidget::show_call_widget_receive_call_page()
{
    callWidget.show_receive_call_page();
}

void MainWidget::show_call_widget_ringing_page()
{
    callWidget.show_ringing_page();
}

void MainWidget::show_call_widget_calling_page()
{
    callWidget.show_calling_page();
}

void MainWidget::hide_call_widget()
{
    callWidget.hide();
}



