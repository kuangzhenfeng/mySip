#ifndef V4L2_VIDEO_H
#define V4L2_VIDEO_H

#include <QObject>
#include <QPixmap>
#include <queue>
#include "x264.h"


class V4L2_Video : public QObject
{
    Q_OBJECT
public:
    explicit V4L2_Video(QObject *parent = nullptr);

    int init();
    void start();
    void stop();
    int release();
    int video_read(char **buffer, int *buffer_size, enum nal_unit_type_e *type);

    int get_width();
    int get_height();
    int get_fps();


private:
    typedef struct VideoBuffer
    {
        uint8_t *start;
        unsigned int length;
        enum nal_unit_type_e type;
    }VideoBuffer;

    int fd;
    VideoBuffer *video_buffer;
    bool m_is_start;
    std::queue<VideoBuffer> m_encode_queue;     // 等待编码的帧队列
    std::queue<VideoBuffer> m_send_queue;       // 等待发送的帧队列
    pthread_mutex_t m_encode_queue_lock;
    pthread_mutex_t m_send_queue_lock;

    int frameHandle(VideoBuffer v_buffer);
    void yuyv422ToYuv420p(int inWidth, int inHeight, uint8_t *pSrc, uint8_t *pDest);
    static void *frame_process_thread(void *ptr);
    static void *yuyv422_to_H264_thread(void *ptr);



signals:


public slots:
};

#endif // V4L2_VIDEO_H
