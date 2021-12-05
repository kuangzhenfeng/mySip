#include "myCodec.h"
#include <pthread.h>
#include <QDebug>
#include <QDateTime>
#include "global.h"

#ifdef __cplusplus
extern "C"
{
#endif

//ffmpeg库
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif





VideoStreamReceiver::VideoStreamReceiver(QObject *parent) : QObject(parent)
{

    m_i_frameFinished = 0;

}

VideoStreamReceiver::~VideoStreamReceiver()
{

}



void VideoStreamReceiver::setUrl(QString url)
{
    m_str_url = url;
}

void VideoStreamReceiver::startStream()
{
    videoStreamIndex=-1;
    av_register_all();//注册库中所有可用的文件格式和解码器
    avformat_network_init();//初始化网络流格式,使用RTSP网络流时必须先执行
    pAVFormatContext = avformat_alloc_context();//申请一个AVFormatContext结构的内存,并进行简单初始化
    pAVFrame=av_frame_alloc();
    this->init();
}

void VideoStreamReceiver::stopStream()
{
    avformat_close_input(&pAVFormatContext);
    avformat_free_context(pAVFormatContext);
    av_frame_free(&pAVFrame);
    sws_freeContext(pSwsContext);
}





bool VideoStreamReceiver::init()
{
    int result;

    videoWidth=g_v4l2_video.get_width();
    videoHeight=g_v4l2_video.get_height();

    avpicture_alloc(&pAVPicture, AV_PIX_FMT_RGB24, videoWidth, videoHeight);

    AVCodec *pAVCodec;

    av_init_packet(&pAVPacket);

    //获取视频流解码器
//    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    pAVCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    pAVCodecContext = avcodec_alloc_context3(pAVCodec);
    pSwsContext = sws_getContext(videoWidth,videoHeight,AV_PIX_FMT_YUV420P,videoWidth,videoHeight,AV_PIX_FMT_RGB24,SWS_BICUBIC,0,0,0);

    m_pCodecParserCtx=av_parser_init(AV_CODEC_ID_H264);
    if (!m_pCodecParserCtx){
        printf("Could not allocate video parser context\n");
        return -1;
    }


    //打开对应解码器
    result=avcodec_open2(pAVCodecContext,pAVCodec,NULL);
    if (result<0){
        qDebug()<<"打开解码器失败";
        return false;
    }

    qDebug()<<"初始化视频流成功";
    return true;
}


int VideoStreamReceiver::h264_decode(char* h264_buffer, int h264_buffer_len)
{
    int paser_len;
    int len;
    char *current_ptr = h264_buffer;
    int current_size = h264_buffer_len;
    static unsigned char sps_pps_buffer[1024] = {0};


    if(h264_buffer == NULL || h264_buffer_len == 0)
    {
        return -1;
    }

//    FILE *f1;
//    f1 = fopen("/mnt/f1.h264", "a");
//    fwrite(h264_buffer, h264_buffer_len, 1, f1);
//    fclose(f1);

//    printf("%02x %02x %02x %02x %02x %02x\n", h264_buffer[0], h264_buffer[1], h264_buffer[2], h264_buffer[3], h264_buffer[4], h264_buffer[5]);



    while(current_size > 0)
    {
        paser_len = av_parser_parse2(m_pCodecParserCtx, pAVCodecContext, &pAVPacket.data, &pAVPacket.size, (uint8_t *)h264_buffer, h264_buffer_len,
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        current_ptr += paser_len;
        current_size -= paser_len;

        if(pAVPacket.size == 0)
        {
            //printf("Error pAVPacket.size is 0\n");
            continue;
        }
        //一帧一帧读取视频
//        qDebug()<<"开始解码"<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        len = avcodec_decode_video2(pAVCodecContext, pAVFrame, &m_i_frameFinished, &pAVPacket);
        if(len < 0)
        {
            printf("Error while decoding frame\n");
            return -1;
        }
        if (m_i_frameFinished)
        {
            mutex.lock();
            sws_scale(pSwsContext,(const uint8_t* const *)pAVFrame->data,pAVFrame->linesize,0,videoHeight,pAVPicture.data,pAVPicture.linesize);
            //发送获取一帧图像信号
            QImage image(pAVPicture.data[0],videoWidth,videoHeight,QImage::Format_RGB888);
            emit get_image(image);
            mutex.unlock();
        }
    }

//    av_free_packet(&pAVPacket);//释放资源,否则内存会一直上升
    return 0;
}

