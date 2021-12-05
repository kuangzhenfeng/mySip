#include "global.h"

bool g_sip_need_invite = false;
bool g_sip_receive_invite = false;


SipCall g_sipCall;
ALSA_Audio g_alsa_audio;
OrtpTrans g_audio_ortpTrans;
OrtpTrans g_video_ortpTrans;
MediaTrans g_mediaTrans;
V4L2_Video g_v4l2_video;
VideoStreamReceiver g_videoStreamReceiver;
OpenCV_Process g_opencv_process;

