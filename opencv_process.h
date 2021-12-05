#ifndef OPENCV_PROCESS_H
#define OPENCV_PROCESS_H

#include <QObject>
#include <QImage>
#include "opencv2/opencv.hpp"




class OpenCV_Process : public QObject
{
    Q_OBJECT
public:
    explicit OpenCV_Process(QObject *parent = nullptr);
    ~OpenCV_Process();

    int init();
    static void *opencv_capture_thread(void *ptr);

private:
    cv::VideoCapture capture;
    QImage MatToQImage(const cv::Mat& mat);

signals:
    void get_image(QImage image);
};

#endif // OPENCV_PROCESS_H
