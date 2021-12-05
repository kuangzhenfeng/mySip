#include "sipCall.h"
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <QDebug>
#include "global.h"
#include "network.h"





SipCall::SipCall()
{
    m_is_busy = false;
    pthread_mutex_init(&m_busy_lock, nullptr);
    TRACE_INITIALIZE(6, NULL);
}

int SipCall::sdp_complete_183_sessionProgress(int did, osip_message_t *answer)
{
    (void)did;
    char local_name[64] = SIP_LOCAL_NAME;
    char local_ip[64] = {0};
    char SDP_buf[4096];
    // eXosip_guess_localip(ctx, AF_INET, local_ip, sizeof(local_ip)); // 获取本地IP
    get_eth0_ip(local_ip);

    // 构造SDP信息体
    snprintf(SDP_buf, 4096,
             "v=0\r\n"                  // SDP版本
             "o=%s 0 0 IN IP4 %s\r\n" // 用户名、ID、版本、网络类型、地址类型、IP地址
             "s=conversation\r\n"       // 会话名称
             "c=IN IP4 %s\r\n"
             "t=0 0\r\n"                      // 开始时间、结束时间。此处不需要设置
             "m=audio %d RTP/AVP 0 8 101\r\n" // 音频、传输端口、传输类型、格式列表
             "a=rtpmap:0 PCMU/8000\r\n"       // 以下为具体描述格式列表中的
             "a=rtpmap:8 PCMA/8000\r\n"
             "a=rtpmap:101 telephone-event/8000\r\n"
             "a=fmtp:101 0-11\r\n"
             "m=video %d RTP/AVP 99\r\n"
             "a=rtpmap:99 H264/90000\r\n"
             "a=fmtp:99 profile-level-id=42801E; packetization-mode=1\r\n",
             local_name ,local_ip, local_ip, SIP_AUDIO_PORT, SIP_VIDEO_PORT);
//             local_name ,local_ip, local_ip, SIP_AUDIO_PORT);
    osip_message_set_body(answer, SDP_buf, strlen(SDP_buf));
    osip_message_set_content_type(answer, "application/sdp");
    return 0;
}

int SipCall::sdp_complete_200_ok(int did, osip_message_t *answer)
{
    (void)did;
    char local_name[64] = SIP_LOCAL_NAME;
    char local_ip[64] = {0};
    char SDP_buf[4096];
    // eXosip_guess_localip(ctx, AF_INET, local_ip, sizeof(local_ip)); // 获取本地IP
    get_eth0_ip(local_ip);

    // 构造SDP信息体
    snprintf(SDP_buf, 4096,
             "v=0\r\n"                  // SDP版本
             "o=%s 0 0 IN IP4 %s\r\n" // 用户名、ID、版本、网络类型、地址类型、IP地址
             "s=conversation\r\n"       // 会话名称
             "c=IN IP4 %s\r\n"
             "t=0 0\r\n"                      // 开始时间、结束时间。此处不需要设置
             "m=audio %d RTP/AVP 0 8 101\r\n" // 音频、传输端口、传输类型、格式列表
             "a=rtpmap:0 PCMU/8000\r\n"       // 以下为具体描述格式列表中的
             "a=rtpmap:8 PCMA/8000\r\n"
             "a=rtpmap:101 telephone-event/8000\r\n"
             "a=fmtp:101 0-11\r\n"
             "m=video %d RTP/AVP 99\r\n"
             "a=rtpmap:99 H264/90000\r\n"
             "a=fmtp:99 profile-level-id=42801E; packetization-mode=1\r\n",
             local_name ,local_ip, local_ip, SIP_AUDIO_PORT, SIP_VIDEO_PORT);
//             local_name ,local_ip, local_ip, SIP_AUDIO_PORT);
    osip_message_set_body(answer, SDP_buf, strlen(SDP_buf));
    osip_message_set_content_type(answer, "application/sdp");
    return 0;
}

//受到对方邀请，本地超时未处理后的回调函数
void SipCall::invite_timeout_handle(int sig, siginfo_t *si, void *uc)
{
    // 注意，超时后自动删除定时器，不用手动删除
    (void)sig;
    (void)uc;
    SipCall *sipCall = static_cast<SipCall *>(si->si_value.sival_ptr);
    sipCall->terminate();
    emit sipCall->hide_call_widget();
    sipCall->m_is_busy = false;
    printf("---timer time out!\n");
}


void *SipCall::sip_event(void *ptr)
{
    SipCall *sipCall = static_cast<SipCall *>(ptr);
    eXosip_t *ctx = sipCall->ctx;
    eXosip_event_t *evt;
    int remote_audio_port;
    int remote_video_port;


    while (1)
    {
        evt = eXosip_event_wait(ctx, 0, 50);
        eXosip_lock(ctx);
        eXosip_automatic_action(ctx);
        eXosip_unlock(ctx);

        if (evt == nullptr)
            continue;
        printf("Receive event type:%d\n", evt->type);

        switch (evt->type)
        {
        case EXOSIP_CALL_INVITE: //对方发出邀请
            sipCall->m_tid = evt->tid;
            sipCall->m_did = evt->did;
            sipCall->m_cid = evt->cid;
            printf("---evt->tid=%d\n", evt->tid);
            printf("---evt->did=%d\n", evt->did);
            printf("---evt->cid=%d\n", evt->cid);
            sipCall->m_remote_sdp_msg = eXosip_get_remote_sdp(ctx, sipCall->m_did);             //获取远程sdp
            sipCall->m_remote_sdp_audio = eXosip_get_audio_media(sipCall->m_remote_sdp_msg);    //获取远程sdp的音频信息
            sipCall->m_remote_sdp_video = eXosip_get_video_media(sipCall->m_remote_sdp_msg);    //获取远程sdp的视频信息
            
            pthread_mutex_lock(&sipCall->m_busy_lock);   // busy加锁
            if(sipCall->m_is_busy)
            {
                sipCall->send_answer_486_busyHere();
                printf("---receive an invite, but local is busy\n");
                pthread_mutex_unlock(&sipCall->m_busy_lock);   // busy解锁
                break;
            }
            sipCall->m_is_busy = true;
            g_sip_receive_invite = true;
            sipCall->send_answer_180_ringing();
            sipCall->send_answer_183_sessionProgress();
            // 开启邀请超时定时器
            printf("---start timer!\n");
            my_timer_create(&sipCall->invite_timeout_timer_id, INVITE_TIMEOUT_TIME * 1000, invite_timeout_handle, static_cast<void *>(sipCall));
            emit sipCall->show_call_widget_receive_call_page();
            pthread_mutex_unlock(&sipCall->m_busy_lock);   // busy解锁
            break;

        case EXOSIP_CALL_RINGING:        // 对方开始响铃
            sipCall->m_tid = evt->tid;
            sipCall->m_did = evt->did;
            sipCall->m_cid = evt->cid;
            printf("---evt->tid=%d\n", evt->tid);
            printf("---evt->did=%d\n", evt->did);
            printf("---evt->cid=%d\n", evt->cid);
            break;

        case EXOSIP_CALL_ANSWERED:           //对方接听
            sipCall->send_ack();
            emit sipCall->show_call_widget_calling_page();
            sipCall->m_remote_sdp_msg = eXosip_get_remote_sdp(ctx, sipCall->m_did);             //获取远程sdp
            sipCall->m_remote_sdp_audio = eXosip_get_audio_media(sipCall->m_remote_sdp_msg);    //获取远程sdp的音频信息
            if(sipCall->m_remote_sdp_audio != nullptr)
            {
                remote_audio_port = atoi(sipCall->m_remote_sdp_audio->m_port);
                g_mediaTrans.audio_trans_start(remote_audio_port);      //打开音频流
            }
            sipCall->m_remote_sdp_video = eXosip_get_video_media(sipCall->m_remote_sdp_msg);    //获取远程sdp的视频信息
            if(sipCall->m_remote_sdp_video != nullptr)
            {
                remote_video_port = atoi(sipCall->m_remote_sdp_video->m_port);
                g_mediaTrans.video_trans_start(remote_video_port);      //打开视频流
            }

            break;

        case EXOSIP_CALL_CANCELLED:     // 对方取消呼叫
            my_timer_delete(sipCall->invite_timeout_timer_id);
            g_sip_receive_invite = false;
            g_sip_need_invite = true;
            sipCall->m_is_busy = false;
            emit sipCall->hide_call_widget();
            break;
        case EXOSIP_CALL_REQUESTFAILURE:    // 对方拒接
        case EXOSIP_CALL_GLOBALFAILURE:     // 对方拒接/对方超时未接
            g_sip_receive_invite = false;
            g_sip_need_invite = true;
            sipCall->m_is_busy = false;
            emit sipCall->hide_call_widget();
            break;

        case EXOSIP_CALL_MESSAGE_NEW:
            if(MSG_IS_BYE(evt->request))    // 对方通话时挂断
            {
                g_sip_receive_invite = false;
                g_sip_need_invite = true;
                g_mediaTrans.audio_trans_stop();
                g_mediaTrans.video_trans_stop();
                sipCall->m_is_busy = false;
                emit sipCall->hide_call_widget();
			}
            else if(MSG_IS_INFO(evt->request))
            {
                char *msg;
                size_t msg_len;
                osip_message_to_str(evt->request, &msg, &msg_len);
                printf("get info!msg:%s\n", msg);
            }
            break;

        default:
            break;
        }

        // eXosip_execute(ctx);
        eXosip_event_free(evt);
        usleep(10 * 1000);
    }
}

int SipCall::init()
{
    int ret;
    pthread_t thread_id;
    ctx = eXosip_malloc();
    if (ctx == nullptr)
        return -1;
    ret = eXosip_init(ctx);
    if (ret != 0)
        return -1;
    ret = eXosip_listen_addr(ctx, IPPROTO_UDP, nullptr, SIP_LISTEN_PORT, AF_INET, 0);
    if (ret != 0)
    {
        eXosip_quit(ctx);
        fprintf(stderr, "could not initialize transport layer\n");
        return -1;
    }
    pthread_create(&thread_id, nullptr, sip_event, this);
    return 0;
}

int SipCall::invite()
{
    osip_message_t *invite;
    int cid;
    int ret;
    int id = 0;
    char from[128];
    char to[128];
    char SDP_buf[4096];
    char local_name[64] = SIP_LOCAL_NAME;
    char local_ip[64] = {0};
    char remote_name[64] = SIP_REMOTE_NAME;
    char remote_ip[64] = SIP_REMOTE_IP;

    pthread_mutex_lock(&m_busy_lock);   // busy加锁

    if(m_is_busy)
    {
        perror("---invite fail, is busy");
        ret = -1;
        goto LABEL_EXIT;
    }
    else
        m_is_busy = true;

    // eXosip_guess_localip(ctx, AF_INET, local_ip, sizeof(local_ip)); // 获取本地IP
    get_eth0_ip(local_ip);

    sprintf(from, "<sip:%s@%s>", local_name, local_ip);
    sprintf(to, "<sip:%s@%s>", remote_name, remote_ip);

    printf("---from=%s\n", from);
    printf("---to=%s\n", to);

    ret = eXosip_call_build_initial_invite(ctx, &invite, to, from,
                                           nullptr, // optional route header
                                           "This is a call for a conversation");
    if (ret != 0)
    {
        ret = -1;
        goto LABEL_EXIT;
    }
    osip_message_set_supported(invite, "100rel");

    // 构造SDP信息体
    snprintf(SDP_buf, 4096,
             "v=0\r\n"                  // SDP版本
             "o=%s 0 0 IN IP4 %s\r\n" // 用户名、ID、版本、网络类型、地址类型、IP地址
             "s=conversation\r\n"       // 会话名称
             "c=IN IP4 %s\r\n"
             "t=0 0\r\n"                      // 开始时间、结束时间。此处不需要设置
//             "m=audio %d RTP/AVP 0 8 101\r\n" // 音频、传输端口、传输类型、格式列表
             "m=audio %d RTP/AVP 0 101\r\n" // 音频、传输端口、传输类型、格式列表
             "a=rtpmap:0 PCMU/8000\r\n"       // 以下为具体描述格式列表中的
//             "a=rtpmap:8 PCMA/8000\r\n"
             "a=rtpmap:101 telephone-event/8000\r\n"
             "a=fmtp:101 0-11\r\n"
             "m=video %d RTP/AVP 99\r\n"
             "a=rtpmap:99 H264/90000\r\n"
             "a=fmtp:99 profile-level-id=42801E; packetization-mode=1\r\n",
             local_name ,local_ip, local_ip, SIP_AUDIO_PORT, SIP_VIDEO_PORT);
//             local_name ,local_ip, local_ip, SIP_AUDIO_PORT);

    osip_message_set_body(invite, SDP_buf, strlen(SDP_buf));
    osip_message_set_content_type(invite, "application/sdp");

    eXosip_lock(ctx);
    cid = eXosip_call_send_initial_invite(ctx, invite);
    if (cid > 0)
    {
        void *reference = nullptr;     // reference为要传到eXosip_event自定义的context
        eXosip_call_set_reference(ctx, id, reference);
    }
    eXosip_unlock(ctx);

    ret = id;
    emit show_call_widget_ringing_page();

LABEL_EXIT:
    pthread_mutex_unlock(&m_busy_lock);    // busy解锁
    return ret;
}

int SipCall::terminate()
{
    eXosip_lock(ctx);
    eXosip_call_terminate(ctx, m_cid, m_did);
    eXosip_unlock(ctx);
    return 0;
}




int SipCall::send_answer_180_ringing()
{
    eXosip_lock(ctx);
    eXosip_call_send_answer(ctx, m_tid, 180, nullptr);
    eXosip_unlock(ctx);
    return 0;
}

int SipCall::send_answer_183_sessionProgress()
{
    osip_message_t *answer = nullptr;
    int ret;
    eXosip_lock(ctx);
    ret = eXosip_call_build_answer(ctx, m_tid, 183, &answer);
    if (ret != 0)
    {
        eXosip_call_send_answer(ctx, m_tid, 400, nullptr);
    }
    else
    {
        ret = sdp_complete_183_sessionProgress(m_did, answer);
        if (ret != 0)
        {
            osip_message_free(answer);
            eXosip_call_send_answer(ctx, m_tid, 415, nullptr);
        }
        else
            eXosip_call_send_answer(ctx, m_tid, 183, answer);
    }
    eXosip_unlock(ctx);
    return 0;
}

// 本地接听对方呼叫
int SipCall::send_answer_200_ok()
{
    osip_message_t *answer = nullptr;
    int ret;
    int remote_audio_port;
    int remote_video_port;
    
    my_timer_delete(invite_timeout_timer_id);

    eXosip_lock(ctx);
    ret = eXosip_call_build_answer(ctx, m_tid, 200, &answer);
    if (ret != 0)
    {
        eXosip_call_send_answer(ctx, m_tid, 400, nullptr);
    }
    else
    {
        ret = sdp_complete_200_ok(m_did, answer);
        if (ret != 0)
        {
            osip_message_free(answer);
            eXosip_call_send_answer(ctx, m_tid, 415, nullptr);
        }
        else
            eXosip_call_send_answer(ctx, m_tid, 200, answer);
    }
    eXosip_unlock(ctx);
    if(m_remote_sdp_audio != nullptr)
    {
        remote_audio_port = atoi(m_remote_sdp_audio->m_port);
        g_mediaTrans.audio_trans_start(remote_audio_port);      //打开音频流
    }
    if(m_remote_sdp_video != nullptr)
    {
        remote_video_port = atoi(m_remote_sdp_video->m_port);
        g_mediaTrans.video_trans_start(remote_video_port);      //打开视频流
    }
    return 0;
}

int SipCall::send_answer_486_busyHere()
{
    eXosip_lock(ctx);
    eXosip_call_send_answer(ctx, m_tid, 486, nullptr);
    eXosip_unlock(ctx);
    return 0;
}


int SipCall::send_ack()
{
    int ret;
    osip_message_t *ack = nullptr;

    ret = eXosip_call_build_ack(ctx, m_tid, &ack);
    if(ret != 0)
        return -1;
    eXosip_lock(ctx);
    eXosip_call_send_ack(ctx, m_tid, ack);
    eXosip_unlock(ctx);
    return 0;
}


// 本地拒接
int SipCall::refuse_invite()
{
    terminate();
    my_timer_delete(invite_timeout_timer_id);
    m_is_busy = false;
    return 0;
}

// 本地通话时挂断
int SipCall::hang_out()
{
    terminate();
    g_mediaTrans.audio_trans_stop();
    g_mediaTrans.video_trans_stop();
    m_is_busy = false;
    return 0;
}

// 本地呼出时取消
int SipCall::cancel()
{
    terminate();
    m_is_busy = false;
    return 0;
}
