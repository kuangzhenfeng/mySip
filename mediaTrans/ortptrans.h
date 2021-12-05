#ifndef ORTPTRANS_H
#define ORTPTRANS_H

#include <QObject>
#include "ortp/ortp.h"



class OrtpTrans : public QObject
{
    Q_OBJECT
public:
    explicit OrtpTrans(QObject *parent = nullptr);

    void start(int rtp_local_media_port, const char *rtp_remote_addr, int rtp_remote_port, int payload_type);
    void stop();
    int audio_send(const char *buffer, int len, uint32_t user_ts);
    int video_send(const char *buffer, int len, uint32_t user_ts, bool isSetMark);
    int audio_receive(char *buffer, int len, uint32_t ts, int *have_more);
    int video_receive(char *buffer, int len, uint32_t ts, int *have_more);

private:
    RtpSession *rtpSession;
    char *m_ssrc;

signals:

public slots:
};

#endif // ORTPTRANS_H
