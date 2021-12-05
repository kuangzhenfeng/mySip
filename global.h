#ifndef GLOBAL_H
#define GLOBAL_H


#include "myLib/log/myLog.h"
#include "sipCall.h"
#include "alsa_audio.h"
#include "mediaTrans/ortptrans.h"
#include "mediaTrans/mediatrans.h"
#include "v4l2_video.h"
#include "myLib/codec/myCodec.h"
#include "opencv_process.h"



extern bool g_sip_need_invite;      // 需要发起sip邀请
extern bool g_sip_receive_invite;   // 接收到sip邀请


extern SipCall g_sipCall;
extern ALSA_Audio g_alsa_audio;
extern OrtpTrans g_audio_ortpTrans;
extern OrtpTrans g_video_ortpTrans;
extern MediaTrans g_mediaTrans;
extern V4L2_Video g_v4l2_video;
extern VideoStreamReceiver g_videoStreamReceiver;
extern OpenCV_Process g_opencv_process;


#endif // GLOBAL_H
