//#ifndef VIDEOSTREAMRECEIVER_H
//#define VIDEOSTREAMRECEIVER_H

///**
//*   RTSP流媒体播放
//*   RTSP测试地址：rtsp://www.mym9.com/101065?from=2019-06-28/01:12:13
//*/

//#include <QWidget>
//#include <QTimer>
//#include <QMutex>
//#include <QImage>
//#include <QLabel>

//#ifdef __cplusplus
//extern "C"
//{
//#endif

////ffmpeg库
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
//#include <libavdevice/avdevice.h>
//#include <libavformat/version.h>
//#include <libavutil/time.h>
//#include <libavutil/mathematics.h>

//#ifdef __cplusplus
//}
//#endif

//#ifndef INT64_C
//#define INT64_C
//#define UINT64_C
//#endif

//namespace Ui {
//class VideoStreamReceiver;
//}

//class VideoStreamReceiver : public QWidget
//{
//    Q_OBJECT

//public:
//    VideoStreamReceiver(QWidget *parent = nullptr);
//    ~VideoStreamReceiver();

//    void setUrl(QString url);
//    void startStream();
//    void stopStream();

//private:
//    Ui::VideoStreamReceiver *ui;

//    QMutex mutex;
//    AVPicture  pAVPicture;
//    AVFormatContext *pAVFormatContext;
//    AVCodecContext *pAVCodecContext;
//    AVFrame *pAVFrame;
//    SwsContext *pSwsContext;
//    AVPacket pAVPacket;

//    QTimer *m_timerPlay;
//    int m_i_frameFinished;
//    int videoStreamIndex;
//    int m_i_w;
//    int m_i_h;
//    QLabel *m_label;
//    QString m_str_url;
//    int videoWidth;
//    int videoHeight;

//    bool Init();


//signals:
//    void GetImage(QImage image);

//private slots:
//    void SetImageSlots(const QImage &image);
//    void playSlots();
//    void on_start_pushButton_clicked();
//    void on_stop_pushButton_clicked();

//};

//#endif // VIDEOSTREAMRECEIVER_H
