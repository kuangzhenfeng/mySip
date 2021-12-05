#include "ortptrans.h"
#include "sipCall.h"


OrtpTrans::OrtpTrans(QObject *parent) : QObject(parent)
{
    ortp_init();
    ortp_scheduler_init();
    // ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);
    // ortp_set_log_level_mask(0xFF);
}



void OrtpTrans::start(int rtp_local_media_port, const char *rtp_remote_addr, int rtp_remote_port, int payload_type)
{
    rtpSession = rtp_session_new(RTP_SESSION_SENDRECV);
    // 是否使用rtp session的系统调度功能
    // 1. 可以使用session_set_select在多个rtp会话之间进行选择，根据时间戳判定某个会话是否到达了收发的时间。
    // 2. 可以使用rtp_session_set_blocking_mode()设置是否使用阻塞模式来进行rtp包的发送和接收。
    rtp_session_set_scheduling_mode(rtpSession, 1);
    // 是否使用阻塞模式
    // 1. rtp_session_recv_with_ts()会一直阻塞直到接收RTP包的时间点到达（这个时间点由该函数参数中所定义的时间戳来决定），当接收完RTP数据包后，该函数才会返回。
    // 2. 同样，rtp_session_send_with_ts()也会一直阻塞直到需要被发送的RTP包的时间点到达，发送结束后，函数才返回。
    rtp_session_set_blocking_mode(rtpSession, 1);

    rtp_session_set_connected_mode(rtpSession, TRUE);
    rtp_session_set_local_addr(rtpSession, "0.0.0.0", rtp_local_media_port);
    rtp_session_set_remote_addr(rtpSession, rtp_remote_addr, rtp_remote_port);
    rtp_profile_set_payload(&av_profile, 99, &payload_type_h264);       // 将96和h264关联
    rtp_session_set_payload_type(rtpSession, payload_type);             // 设置载荷类型   0:pcmu8000, 8:pcma:8000




    m_ssrc=getenv("SSRC");
    if (m_ssrc!=NULL) {
        printf("using SSRC=%i.\n",atoi(m_ssrc));
        // 设置输出流的SSRC。不做此步的话将会给个随机值
        rtp_session_set_ssrc(rtpSession, static_cast<unsigned int>(atoi(m_ssrc)));
    }
}

void OrtpTrans::stop()
{
    rtp_session_destroy(rtpSession);
//    ortp_exit();
    ortp_global_stats_display();
}

int OrtpTrans::audio_send(const char *buffer, int len, uint32_t user_ts)
{
    int sendBytes = 0;
    sendBytes = rtp_session_send_with_ts(rtpSession, reinterpret_cast<const unsigned char *>(buffer), len, user_ts);
//    printf("sendBytes:%d\n", sendBytes);
    return sendBytes;
}

int OrtpTrans::video_send(const char *buffer, int len, uint32_t user_ts, bool isSetMark)
{
    mblk_t *m;
    int err;
    m = rtp_session_create_packet(rtpSession, RTP_FIXED_HEADER_SIZE, (uint8_t*)buffer,len);
    rtp_set_markbit(m, isSetMark);
    err=rtp_session_sendm_with_ts(rtpSession, m, user_ts);
    return err;

}

int OrtpTrans::audio_receive(char *buffer, int len, uint32_t ts, int *have_more)
{
    return rtp_session_recv_with_ts(rtpSession, reinterpret_cast<unsigned char *>(buffer), len, ts, have_more);
}

int OrtpTrans::video_receive(char *buffer, int len, uint32_t ts, int *have_more)
{
    // 由rtp_session_recv_with_ts()修改而来，添加了头部"00 00 00 01"
    mblk_t *mp=NULL;
    int plen,blen=0;
    *have_more=0;
    int type;
    while(1){
        if (rtpSession->pending){
            mp=rtpSession->pending;
            rtpSession->pending=NULL;
        }else {
            mp=rtp_session_recvm_with_ts(rtpSession,ts);
            if (mp!=NULL) rtp_get_payload(mp,&mp->b_rptr);
        }
        if (mp){
            plen=mp->b_wptr-mp->b_rptr;
            type = mp->b_rptr[0] & 0x1F;
            if(type == 0x01 || type == 0x05 || type == 0x06 || type == 0x07 || type == 0x08)
            {
                sprintf(buffer, "%c%c%c%c", 0x00, 0x00, 0x00, 0x01);    // 加上头部"00 00 00 01"
                buffer+=4;
                blen+=4;
                len-=4;
            }
            if (plen<=len){
                memcpy(buffer,mp->b_rptr,plen);
                buffer+=plen;
                blen+=plen;
                len-=plen;
                freemsg(mp);
                mp=NULL;
            }else{
                memcpy(buffer,mp->b_rptr,len);
                mp->b_rptr+=len;
                buffer+=len;
                blen+=len;
                len=0;
                rtpSession->pending=mp;
                *have_more=1;
                break;
            }
        }else break;
    }
    return blen;
}


