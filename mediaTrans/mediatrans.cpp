#include "mediatrans.h"
#include "global.h"
#include "g711/g711_encode.h"
#include <errno.h>


MediaTrans::MediaTrans(QObject *parent) : QObject(parent)
{

    m_is_audio_trans_start = false;
    m_is_video_trans_start = false;


}

void *MediaTrans::audio_send_thread(void *ptr)
{
//    MediaTrans *mediaTrans = static_cast<MediaTrans *>(ptr);

//    char *data = nullptr;       // data指针空间由g_alsa_audio.capture_stop()中的free(m_capture_buffer)释放
//    int data_size;
//    unsigned long frames;
//    uint32_t send_audio_ts = 0;
//    char *send_data = nullptr;
//    int send_data_size;

//    while(mediaTrans->m_is_audio_trans_start)
//    {
//        g_alsa_audio.audio_read(&data, &data_size, &frames);
//        g711_encode(data, data_size, &send_data, &send_data_size);
//        g_audio_ortpTrans.audio_send(send_data, send_data_size, send_audio_ts);
//        send_audio_ts += frames;

//        free(send_data);
//        usleep(1000);
//    }
    return nullptr;
}

void *MediaTrans::audio_recv_thread(void *ptr)
{
//    MediaTrans *mediaTrans = static_cast<MediaTrans *>(ptr);
//    char recv_data[160];
//    uint32_t ts = 0;    // 设置接收的第一个数据包的时间戳为0
//    int have_more;
//    char *data = nullptr;
//    int data_size;
//    int recv_data_size, len_temp;

//    while(mediaTrans->m_is_audio_trans_start)
//    {
//        have_more = 1;
//        recv_data_size = 0;
//        while (have_more){
//            len_temp = g_audio_ortpTrans.audio_receive(recv_data + recv_data_size, 160 * 16 / 8 * 1 / 2, ts, &have_more);
////            printf("******len_temp = %d\n", len_temp);
//            if(len_temp > 0)
//                recv_data_size += len_temp;
//        }
//        g711_decode(recv_data, recv_data_size, &data, &data_size);
////        printf("******recv_data_size = %d\n", recv_data_size);
////        printf("******data_size = %d\n", data_size);
//        g_alsa_audio.audio_write(data);
//        free(data);

//        ts += 160;      // 此处160为固定值，发送接收均使用此值
//        usleep(1000);
//    }
    return nullptr;
}

void MediaTrans::audio_trans_start(int remote_audio_port)
{
    if(m_is_audio_trans_start)
    {
        printf("error!audio trans is started! wait for audio trans stop!");
        m_is_audio_trans_start = false;
        usleep(100000);
    }

    g_alsa_audio.capture_start();
    g_alsa_audio.playback_start();
    g_audio_ortpTrans.start(SIP_AUDIO_PORT, SIP_REMOTE_IP, remote_audio_port, 0);    // 0->pcmu8000
    m_is_audio_trans_start = true;
    pthread_create(&m_aduio_send_thread_id, 0, audio_send_thread, this);
    pthread_create(&m_aduio_recv_thread_id, 0, audio_recv_thread, this);
}

void MediaTrans::audio_trans_stop()
{
    if(!m_is_audio_trans_start)
    {
        printf("error! audio trans is stoped!");
        return;
    }

    m_is_audio_trans_start = false;

    // 等待音频发送线程和接收线程结束
    pthread_join(m_aduio_send_thread_id, 0);
    pthread_join(m_aduio_recv_thread_id, 0);

    g_audio_ortpTrans.stop();
    g_alsa_audio.capture_stop();
    g_alsa_audio.playback_stop();

}




void *MediaTrans::video_send_thread(void *ptr)
{
    MediaTrans *mediaTrans = static_cast<MediaTrans *>(ptr);

    uint32_t send_video_ts = 0;
    char *data = nullptr;      // 此处不用释放内存，该指针指向一个static的数组
    int data_size;
    enum nal_unit_type_e nalu_type;
    int type;
    char *send_data = nullptr;
    int send_data_size;
    int ret;
    char nalu_header1[] = {0x00, 0x00, 0x00, 0x01};
    char nalu_header2[] = {0x00, 0x00, 0x01};
    char *next_nalu;
    bool isSetMark;

    remove("/root/f1.h264");

    while(mediaTrans->m_is_video_trans_start)
    {
        ret = g_v4l2_video.video_read(&data, &data_size, &nalu_type);
        if(ret < 0)
        {
            usleep(10 * 1000);
            continue;
        }


        FILE *f1;
        f1 = fopen("/root/f1.h264", "a");
        fwrite(data, data_size, 1, f1);
        fclose(f1);

//        printf("%02x %02x %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3], data[4], data[5]);


//        if(nalu_type == NAL_SPS)
//        {
//            // 去掉NALU的开始码"00 00 00 01"
//            data += 4;
//            data_size -= 4;
//        }

//        printf("type=%d\n", nalu_type);



        while(data_size > 0)
        {
            if(data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x01)
            {
                // 去掉NALU的开始码"00 00 00 01"
                data += 4;
                data_size -= 4;
            }
            else if(data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01)
            {
                // 去掉NALU的开始码"00 00 01"
                data += 3;
                data_size -= 3;
            }
            else
            {
                printf("error: this is not a nalu!\n");
                break;
            }

            type = data[0] & 0x1F;

            if(type == 7)     // SPS，则下个NALU的开始码为"00 00 00 01"
                next_nalu = (char *)memmem(data, data_size, nalu_header1, 4);
            else
                next_nalu = (char *)memmem(data, data_size, nalu_header2, 3);

            send_data = data;
            if(next_nalu == nullptr)
            {
                send_data_size = data_size;
                data_size = 0;
                isSetMark = true;
            }
            else
            {
                send_data_size = next_nalu - data;
                data = next_nalu;
                data_size -= send_data_size;
                isSetMark = false;
            }
            ret = g_video_ortpTrans.video_send(send_data, send_data_size, send_video_ts, isSetMark);
            if(ret < 0)
                printf("video send error!\n");
            usleep(500);
        }
        send_video_ts += static_cast<unsigned int>(90000/g_v4l2_video.get_fps());
    }
    return nullptr;
}

void *MediaTrans::video_recv_thread(void *ptr)
{
    MediaTrans *mediaTrans = static_cast<MediaTrans *>(ptr);
    char *recv_data;
    uint32_t ts = 0;    // 设置接收的第一个数据包的时间戳为0
    int have_more;
    char *data = nullptr;
    int data_size;
    int recv_data_size, len_temp;

    // 为防止栈溢出，此处使用动态申请内存
    recv_data = static_cast<char *>(malloc(1024 * 80 * sizeof(char)));


    while(mediaTrans->m_is_video_trans_start)
    {
        have_more = 1;
        recv_data_size = 0;

        // 加上头部“00 00 00 01”
//        sprintf(recv_data, "%c%c%c%c", 0x00, 0x00, 0x00, 0x01);
//        recv_data_size += 4;

        while (have_more){
            len_temp = g_video_ortpTrans.video_receive(recv_data + recv_data_size, 1024 * 80, ts, &have_more);     // TODO:这里怎么算
            if(len_temp > 0)
                recv_data_size += len_temp;
        }
        if(recv_data_size > 0)
            g_videoStreamReceiver.h264_decode(recv_data, recv_data_size);
        ts += 6000;      // 此处90000/30为固定值，发送接收均使用此值
        usleep(1000);
    }
    free(recv_data);
    return nullptr;
}

void MediaTrans::video_trans_start(int remote_video_port)
{
    if(m_is_video_trans_start)
    {
        printf("error! video trans is started! wait for video trans stop!");
        m_is_video_trans_start = false;
        usleep(100000);
    }

    g_v4l2_video.start();
    g_video_ortpTrans.start(SIP_VIDEO_PORT, SIP_REMOTE_IP, remote_video_port, 99);     // 96->h264
    g_videoStreamReceiver.startStream();
    m_is_video_trans_start = true;
    pthread_create(&m_video_send_thread_id, 0, video_send_thread, this);
//    pthread_create(&m_video_recv_thread_id, 0, video_recv_thread, this);
}

void MediaTrans::video_trans_stop()
{
    if(!m_is_video_trans_start)
    {
        printf("error! video trans is stoped!");
        return;
    }

    m_is_video_trans_start = false;

    // 等待视频发送线程和接收线程结束
    pthread_join(m_video_send_thread_id, 0);
//    pthread_join(m_video_recv_thread_id, 0);

    g_videoStreamReceiver.stopStream();
    g_video_ortpTrans.stop();
    g_v4l2_video.stop();

}

