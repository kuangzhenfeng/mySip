#include "alsa_audio.h"
#include "global.h"

#include <QDebug>

#include <math.h>
#include <inttypes.h>





ALSA_Audio::ALSA_Audio(QObject *parent) : QObject(parent)
{
    m_is_capture_start = false;
    m_is_playback_start = false;
}



int ALSA_Audio::set_hw_params(snd_pcm_t *pcm, unsigned int rate, snd_pcm_format_t format, unsigned int channels, snd_pcm_uframes_t frames)
{
    snd_pcm_uframes_t period_size;          // 一个处理周期需要的帧数
    snd_pcm_uframes_t hw_buffer_size;      // 硬件缓冲区大小
    snd_pcm_hw_params_t *hw_params;
    int ret;
    int dir = 0;



    // 初始化硬件参数结构体
    snd_pcm_hw_params_malloc(&hw_params);
    // 设置默认的硬件参数
    snd_pcm_hw_params_any(pcm, hw_params);

    // 以下为设置所需的硬件参数

    // 设置音频数据记录方式
    CHECK_RETURN(snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED));
    // 格式。使用16位采样大小，小端模式（SND_PCM_FORMAT_S16_LE）
    CHECK_RETURN(snd_pcm_hw_params_set_format(pcm, hw_params, format));
    // 设置音频通道数
    CHECK_RETURN(snd_pcm_hw_params_set_channels(pcm, hw_params, channels));
    // 采样频率，一次采集为一帧数据
    //CHECK_RETURN(snd_pcm_hw_params_set_rate_near(pcm, hw_params, &rate, &dir));          // 设置相近的值
    CHECK_RETURN(snd_pcm_hw_params_set_rate(pcm, hw_params, rate, dir));
    // 一个处理周期需要的帧数
    period_size = frames * 5;
    CHECK_RETURN(snd_pcm_hw_params_set_period_size_near(pcm, hw_params, &period_size, &dir)); // 设置相近的值
//    // 硬件缓冲区大小, 单位：帧（frame）
//    hw_buffer_size = period_size * 16;
//    CHECK_RETURN(snd_pcm_hw_params_set_buffer_size_near(pcm, hw_params, &hw_buffer_size));

    // 将参数写入pcm驱动
    CHECK_RETURN(snd_pcm_hw_params(pcm, hw_params));

    snd_pcm_hw_params_free(hw_params);     // 释放不再使用的hw_params空间

    printf("one frames=%ldbytes\n", snd_pcm_frames_to_bytes(pcm, 1));
    unsigned int val;
    snd_pcm_hw_params_get_channels(hw_params, &val);
    printf("channels=%d\n", val);

    if (ret < 0) {
        printf("error: unable to set hw parameters: %s\n", snd_strerror(ret));
        return -1;
    }
    return 0;
}


void ALSA_Audio::capture_start()
{
    m_capture_frames = 160;     // 此处160为固定值，发送接收均使用此值
    unsigned int rate = 8000;                               // 采样频率
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;        // 使用16位采样大小，小端模式
    unsigned int channels = 1;                              // 通道数
    int ret;

    if(m_is_capture_start)
    {
        printf("error: alsa audio capture is started!\n");
        return;
    }

    ret = snd_pcm_open(&m_capture_pcm, "plughw:1,0", SND_PCM_STREAM_CAPTURE, 0);       // 使用plughw:0,0
    if(ret < 0)
    {
        printf("snd_pcm_open error: %s\n", snd_strerror(ret));
        return;
    }

    // 设置硬件参数
    if(set_hw_params(m_capture_pcm, rate, format, channels, m_capture_frames) < 0)
    {
        return;
    }

    // 使用buffer保存一次处理得到的数据
    m_capture_buffer_size = m_capture_frames * static_cast<unsigned long>(snd_pcm_format_width(format) / 8 * static_cast<int>(channels));
    m_capture_buffer_size *= 5;         // * 5 表示使用5倍的缓存空间
    printf("snd_pcm_format_width(format):%d\n", snd_pcm_format_width(format));
    printf("m_capture_buffer_size:%ld\n", m_capture_buffer_size);
    m_capture_buffer = static_cast<char *>(malloc(sizeof(char) * m_capture_buffer_size));
    memset(m_capture_buffer, 0, m_capture_buffer_size);

    // 获取一次处理所需要的时间，单位us
    // 1/rate * frames * 10^6 = period_time， 即：采集一帧所需的时间 * 一次处理所需的帧数 * 10^6 = 一次处理所需的时间（单位us）
    // snd_pcm_hw_params_get_period_time(m_capture_hw_params, &m_period_time, &dir);

    m_is_capture_start = true;
}

void ALSA_Audio::capture_stop()
{
    if(m_is_capture_start == false)
    {
        printf("error: alsa audio capture is not start!");
        return;
    }

    m_is_capture_start = false;

    snd_pcm_drain(m_capture_pcm);
    snd_pcm_close(m_capture_pcm);
    free(m_capture_buffer);
}

int ALSA_Audio::audio_read(char **buffer, int *buffer_size, unsigned long *frames)
{
    int ret;
    if(m_is_capture_start == false)
    {
        printf("error: alsa audio capture is stopped!\n");
        return -1;
    }
    memset(m_capture_buffer, 0, m_capture_buffer_size);
    ret = static_cast<int>(snd_pcm_readi(m_capture_pcm, m_capture_buffer, m_capture_frames));
//    printf("strlen(m_capture_buffer)=%ld\n", strlen(m_capture_buffer));
    if (ret == -EPIPE)
    {
       /* EPIPE means overrun */
       printf("overrun occurred\n");
       snd_pcm_prepare(m_capture_pcm);
    }
    else if (ret < 0)
    {
       printf("error from read: %s\n", snd_strerror(ret));
    }
    else if (ret != static_cast<int>(m_capture_frames))
    {
       printf("short read, read %d frames\n", ret);
    }

    if(m_capture_buffer == nullptr)
    {
        printf("error: alsa audio capture_buffer is empty!\n");
        return -1;
    }
    *buffer = m_capture_buffer;
    *buffer_size = static_cast<int>(m_capture_buffer_size / 5);
    *frames = m_capture_frames;

    return 0;
}



void ALSA_Audio::playback_start()
{
    m_playback_frames = 160;     // 此处160为固定值，发送接收均使用此值
    unsigned int rate = 8000;                               // 采样频率
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;        // 使用16位采样大小，小端模式
    unsigned int channels = 1;                              // 通道数
    int ret;


    if(m_is_playback_start)
    {
        printf("error: alsa audio playback is started!\n");
        return;
    }

    ret = snd_pcm_open(&m_playback_pcm, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);      // 使用plughw:0,0
    if(ret < 0)
    {
        printf("snd_pcm_open error: %s\n", snd_strerror(ret));
        return;
    }

    // 设置硬件参数
    if(set_hw_params(m_playback_pcm, rate, format, channels, m_playback_frames) < 0)
    {
        return;
    }


    m_is_playback_start = true;

}

void ALSA_Audio::playback_stop()
{
    if(m_is_playback_start == false)
    {
        printf("error: alsa audio playback is not start!");
        return;
    }

    m_is_playback_start = false;

    snd_pcm_drain(m_playback_pcm);
    snd_pcm_close(m_playback_pcm);
}


int ALSA_Audio::audio_write(char *buffer)
{
    long ret;
    if(m_is_playback_start == false)
    {
        printf("error: alsa audio playback is stopped!\n");
        return -1;
    }
    else
    {
        ret = snd_pcm_writei(m_playback_pcm, buffer, m_playback_frames);
        if(ret == -EPIPE)
        {
            /* EPIPE means underrun  */
            printf("underrun occurred\n");
            snd_pcm_prepare(m_playback_pcm);
        }
        else if (ret < 0)
        {
           printf("error from write: %s\n", snd_strerror(static_cast<int>(ret)));
        }
        else if (ret != static_cast<long>(m_playback_frames))
        {
           printf("short write, write %ld frames\n", ret);
        }
    }
    return 0;
}







