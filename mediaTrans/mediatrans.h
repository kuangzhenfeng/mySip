#ifndef MEDIATRANS_H
#define MEDIATRANS_H

#include <QObject>
#include <pthread.h>

class MediaTrans : public QObject
{
    Q_OBJECT
public:
    explicit MediaTrans(QObject *parent = nullptr);

    static void *audio_send_thread(void *ptr);
    static void *audio_recv_thread(void *ptr);
    void audio_trans_start(int remote_audio_port);
    void audio_trans_stop();

    static void *video_send_thread(void *ptr);
    static void *video_recv_thread(void *ptr);
    void video_trans_start(int remote_video_port);
    void video_trans_stop();

private:
    volatile bool m_is_audio_trans_start;
    pthread_t m_aduio_send_thread_id;
    pthread_t m_aduio_recv_thread_id;
    volatile bool m_is_video_trans_start;
    pthread_t m_video_send_thread_id;
    pthread_t m_video_recv_thread_id;

signals:

public slots:
};

#endif // MEDIATRANS_H
