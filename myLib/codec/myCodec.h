#ifndef MY_CODEC_H
#define MY_CODEC_H


#include <QWidget>
#include <QTimer>
#include <QMutex>
#include <QImage>
#include <QLabel>

#ifdef __cplusplus
extern "C"
{
#endif

//ffmpeg库
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>

#ifdef __cplusplus
}
#endif

#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif


class VideoStreamReceiver : public QObject
{
    Q_OBJECT

public:
    VideoStreamReceiver(QObject *parent = nullptr);
    ~VideoStreamReceiver();

    void setUrl(QString url);
    void startStream();
    void stopStream();
    int h264_decode(char* h264_buffer, int h264_buffer_len);

private:

    QMutex mutex;
    AVPicture  pAVPicture;
    AVFormatContext *pAVFormatContext;
    AVCodecContext *pAVCodecContext;
    AVFrame *pAVFrame;
    SwsContext *pSwsContext;
    AVPacket pAVPacket;
    AVCodecParserContext *m_pCodecParserCtx;

    int m_i_frameFinished;
    int videoStreamIndex;
    QString m_str_url;
    int videoWidth;
    int videoHeight;

    bool init();


signals:
    void get_image(QImage image);

private slots:


};



#endif
