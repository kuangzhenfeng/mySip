#ifndef SIP_CALL_H
#define SIP_CALL_H

#include <QObject>
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
#include <pthread.h>
#include "myLib/timer/myTimer.h"


#define SIP_LISTEN_PORT 5060            // 本地监听端口
#define SIP_AUDIO_PORT 4000             // 音频传输端口（本地接收端口）
#define SIP_VIDEO_PORT 4001             // 视频传输端口（本地接收端口）

#define INVITE_TIMEOUT_TIME 60  // 对方发出邀请的超时时间，单位秒（s）


#define SIP_LOCAL_NAME "test_from"
#define SIP_REMOTE_NAME "test_to"
#define SIP_REMOTE_IP "192.168.8.226"
//#define SIP_REMOTE_IP "192.168.1.157"


class SipCall: public QObject
{
    Q_OBJECT

public:
    SipCall();

    int init();
    int invite();
    int send_answer_180_ringing();
    int send_answer_183_sessionProgress();
    int send_answer_200_ok();               // 本地接听对方呼叫
    int send_answer_486_busyHere();
    int send_ack();
    int refuse_invite();        // 本地拒接
    int hang_out();             // 本地通话时挂断
    int cancel();               // 本地呼出时取消


    static void *sip_event(void *ptr);
    static void invite_timeout_handle(int sig, siginfo_t *si, void *uc);

private:
    eXosip_t *ctx;
    int m_tid;
    int m_did;
    int m_cid;
    timer_t invite_timeout_timer_id;
    bool m_is_busy;
    pthread_mutex_t m_busy_lock;
    sdp_message_t *m_remote_sdp_msg;   //远程sdp
    sdp_media_t *m_remote_sdp_audio;   //远程sdp音频信息
    sdp_media_t *m_remote_sdp_video;   //远程sdp视频信息
   
    int sdp_complete_183_sessionProgress(int did, osip_message_t *answer);
    int sdp_complete_200_ok(int did, osip_message_t *answer);
    int terminate();

signals:
    void show_call_widget_receive_call_page();
    void show_call_widget_ringing_page();
    void show_call_widget_calling_page();
    void hide_call_widget();

    
};

#endif
