#include "opencv_process.h"
#include <unistd.h>
#include "opencv2/imgproc/types_c.h"


using namespace std;
using namespace cv;
using namespace cv::dnn;

const float confidenceThreshold = 0.5;      // 置信度阈值

OpenCV_Process::OpenCV_Process(QObject *parent) : QObject(parent)
{

    capture.open(8);
//    capture.set(cv::CAP_PROP_FPS, 30);
//    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
//    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    pthread_t thread_id;
    pthread_create(&thread_id, 0, &opencv_capture_thread, this);
}

OpenCV_Process::~OpenCV_Process()
{
    capture.release();
}

int OpenCV_Process::init()
{

}


void *OpenCV_Process::opencv_capture_thread(void *ptr)
{
    OpenCV_Process *opencv_process = static_cast<OpenCV_Process *>(ptr);
    Mat frame;
    QImage image_output;
    Mat image_gray;      //定义两个Mat变量，用于存储每一帧的图像
    CascadeClassifier face_cascade;    //载入分类器，脸部识别
    //xml文档路径  opencv\sources\data\haarcascades
    if (!face_cascade.load("/root/face_fr/haarcascade_frontalface_alt.xml"))
    {
        printf("Load haarcascade_frontalface_alt failed!\n");
        return 0;
    }
    usleep(500 * 1000);


    while(1)
    {
        opencv_process->capture.read(frame);
        if (!frame.empty()) //判断当前帧是否捕捉成功
        {
            cvtColor(frame, image_gray, CV_BGR2GRAY);//转为灰度图
            equalizeHist(image_gray, image_gray);//直方图均衡化，增加对比度方便处理



            //vector 是个类模板 需要提供明确的模板实参 vector<Rect>则是个确定的类 模板的实例化
            vector<Rect> faceRect;//矩形框

            //检测关于脸部位置
            face_cascade.detectMultiScale(image_gray, faceRect, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));//检测     // TODO:此步耗时较高，待优化
            for (size_t i = 0; i < faceRect.size(); i++)
            {
                rectangle(frame, faceRect[i], Scalar(0, 0, 255));      //用矩形画出检测到的位置
            }

            image_output = opencv_process->MatToQImage(frame);
            emit opencv_process->get_image(image_output);

        }
        else
        {
            printf("frame is empty!\n");
        }
        usleep(30 * 1000); //延时30毫秒
    }
}




QImage OpenCV_Process::MatToQImage(const cv::Mat& mat)
{
    switch (mat.type())
    {
        // 8-bit, 4 channel
        case CV_8UC4:
            {
                QImage image(mat.data,
                             mat.cols, mat.rows,
                             static_cast<int>(mat.step),
                             QImage::Format_ARGB32);

                return image;
            }

         // 8-bit, 3 channel
         case CV_8UC3:
            {
                QImage image(mat.data,
                             mat.cols, mat.rows,
                             static_cast<int>(mat.step),
                             QImage::Format_RGB888);

                return image.rgbSwapped();
            }

         // 8-bit, 1 channel
        case CV_8UC1:
            {
                #if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
                QImage image(mat.data,
                             mat.cols, mat.rows,
                             static_cast<int>(mat.step),
                             QImage::Format_Grayscale8);
                #else
                static QVector<QRgb>  sColorTable;

                // only create our color table the first time
                if (sColorTable.isEmpty())
                {
                    sColorTable.resize( 256 );

                    for ( int i = 0; i < 256; ++i )
                    {
                        sColorTable[i] = qRgb( i, i, i );
                    }
                }

                QImage image(mat.data,
                             mat.cols, mat.rows,
                             static_cast<int>(mat.step),
                             QImage::Format_Indexed8 );

                image.setColorTable(sColorTable);
                #endif

                return image;
            }

        // wrong
        default:
            printf("ERROR: Mat could not be converted to QImage.");
            break;
    }
    return QImage();
}
