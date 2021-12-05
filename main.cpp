#include "mainwidget.h"
#include <QApplication>
#include <QDebug>
#include "v4l2_video.h"
#include "pthread.h"
#include "opencv_process.h"


int main(int argc, char *argv[])
{
    setvbuf(stdout, nullptr, _IONBF, 0);  // 设置标准输出缓存为0

    QApplication a(argc, argv);
    MainWidget w;



//    g_sipCall.init();

//    g_v4l2_video.init();



    w.show();





    return a.exec();
}
