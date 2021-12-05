#ifndef ALSA_AUDIO_H
#define ALSA_AUDIO_H

#include <QObject>

#include <alsa/asoundlib.h>

class ALSA_Audio : public QObject
{
    Q_OBJECT
public:
    explicit ALSA_Audio(QObject *parent = nullptr);


    void capture_start();
    void capture_stop();
    /**
     * @brief 读取音频数据
     * @param buffer 音频数据
     * @param buffer_size 音频数据大小
     * @param frames 读取的音频帧数
     * @return 0 成功，-1 失败
     */
    int audio_read(char **buffer, int *buffer_size, unsigned long *frames);

    void playback_start();
    void playback_stop();
    /**
     * @brief audio_write 播放音频
     * @param buffer 音频数据
     * @param frames 播放的音频帧数
     * @return 0 成功，-1 失败
     */
    int audio_write(char *buffer);



private:
    bool m_is_capture_start;
    snd_pcm_t *m_capture_pcm;
    char *m_capture_buffer;
    unsigned long m_capture_buffer_size;
    snd_pcm_uframes_t m_capture_frames;       // 一次读的帧数


    bool m_is_playback_start;
    snd_pcm_t *m_playback_pcm;
    snd_pcm_uframes_t m_playback_frames;       // 一次写的帧数

    /**
     * @brief ALSA_Audio::set_hw_params
     * @param pcm
     * @param hw_params
     * @param rate 采样频率
     * @param format 格式
     * @param channels 通道数
     * @param frames 一次读写的帧数
     * @return
     */
    int set_hw_params(snd_pcm_t *pcm, unsigned int rate, snd_pcm_format_t format, unsigned int channels, snd_pcm_uframes_t frames);



signals:

public slots:
};

#endif // ALSA_AUDIO_H
