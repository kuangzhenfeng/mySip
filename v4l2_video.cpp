#include "v4l2_video.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "mainwidget.h"




#define VIDEO_FILE "/dev/video8"


//#define IMAGEWIDTH      640
//#define IMAGEHEIGHT     480
//#define VIDEO_FPS       25

#define IMAGEWIDTH      640
#define IMAGEHEIGHT     480
#define VIDEO_FPS       15



#define ENCODE_QUEUE_FRAME_NUM       4
#define SEND_QUEUE_FRAME_NUM         ENCODE_QUEUE_FRAME_NUM * 8


V4L2_Video::V4L2_Video(QObject *parent) : QObject(parent)
{
    m_is_start = false;
    pthread_mutex_init(&m_encode_queue_lock, nullptr);
    pthread_mutex_init(&m_send_queue_lock, nullptr);
}



int V4L2_Video::init()
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_streamparm stream_parm;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    unsigned int buffer_n;
    int ret;


    fd = open(VIDEO_FILE, O_RDWR);
    if(fd == -1)
    {
        printf("Error opening V4L2 interface\n");
        return -1;
    }

    // 查询设备属性
    ioctl(fd, VIDIOC_QUERYCAP, &cap);
    printf("Driver Name:%s\nCard Name:%s\nBus info:%s\nDriver Version:%u.%u.%u\n",cap.driver,cap.card,cap.bus_info,(cap.version>>16)&0XFF, (cap.version>>8)&0XFF,cap.version&0XFF);

    //显示所有支持帧格式
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index=0;
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format:\n");
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
    {
        printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
        fmtdesc.index++;
    }

    // 设置帧格式
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;         // 传输流类型
    fmt.fmt.pix.width = IMAGEWIDTH;                 // 宽度
    fmt.fmt.pix.height = IMAGEHEIGHT;               // 高度
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;    // 采样类型
//    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;      // 采样区域
    ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if(ret < 0)
    {
        printf("Unable to set format\n");
        goto label_exit;
    }

    // 设置帧速率，设置为30帧（1秒采集30张图）
    stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_parm.parm.capture.timeperframe.denominator = VIDEO_FPS;
    stream_parm.parm.capture.timeperframe.numerator = 1;
    ret = ioctl(fd, VIDIOC_S_PARM, &stream_parm);
    if(ret < 0)
    {
        printf("Unable to set frame rate\n");
        goto label_exit;
    }


    // 申请帧缓冲
    req.count = ENCODE_QUEUE_FRAME_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(fd, VIDIOC_REQBUFS, &req);
    if(ret < 0)
    {
        printf("request for buffers error\n");
        goto label_exit;
    }

    // 内存映射
    video_buffer = static_cast<VideoBuffer *>(malloc(req.count * sizeof(VideoBuffer)));
    for(buffer_n = 0; buffer_n < ENCODE_QUEUE_FRAME_NUM; buffer_n++)
    {
//        memset(&buf, 0, sizeof(buf));
        buf.index = buffer_n;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl (fd, VIDIOC_QUERYBUF, &buf);
        if (ret < 0)
        {
            printf("query buffer error\n");
            goto label_exit;
        }
        video_buffer[buffer_n].start = static_cast<uint8_t *>(mmap(nullptr, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset));
        video_buffer[buffer_n].length = buf.length;
        if(video_buffer[buffer_n].start == MAP_FAILED)
        {
            ret = -1;
            printf("buffer map error\n");
            goto label_exit;
        }
        // 放入缓存队列
        ret = ioctl(fd, VIDIOC_QBUF, &buf);
        if (ret < 0)
        {
            printf("put in frame error\n");
            goto label_exit;
        }
    }

    return 0;

label_exit:
    close(fd);
    return ret;
}

int V4L2_Video::release()
{
    unsigned int buffer_n;
    enum v4l2_buf_type type;

    // 关闭流
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);

    // 关闭内存映射
    for(buffer_n=0; buffer_n < ENCODE_QUEUE_FRAME_NUM; buffer_n++)
    {
        munmap(video_buffer[buffer_n].start, video_buffer[buffer_n].length);
    }

    // 释放自己申请的内存
    free(video_buffer);

    // 关闭设备
    close(fd);
    return 0;
}

int V4L2_Video::frameHandle(VideoBuffer v_buffer)
{
    // 待编码视频帧入队
    pthread_mutex_lock(&m_encode_queue_lock);    // m_encode_queue加锁
    while(m_encode_queue.size() == ENCODE_QUEUE_FRAME_NUM)
    {
        pthread_mutex_unlock(&m_encode_queue_lock);   // m_encode_queue解锁
        printf("warning_2: m_encode_queue is overflow!\n");
        usleep(10 * 1000);
    }
    m_encode_queue.push(v_buffer);
    pthread_mutex_unlock(&m_encode_queue_lock);   // m_encode_queue解锁
    return 0;
}

void *V4L2_Video::frame_process_thread(void *ptr)
{
    V4L2_Video *v4l2_video = static_cast<V4L2_Video *>(ptr);
    enum v4l2_buf_type type;
    struct v4l2_buffer buf;
    int ret;

    // 开启视频流
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(v4l2_video->fd, VIDIOC_STREAMON, &type);
    if(ret < 0) {
          printf("VIDIOC_STREAMON failed (%d)\n", ret);
          return nullptr;
    }

    // 开始捕获摄像头数据
    while(v4l2_video->m_is_start)
    {
        if(v4l2_video->m_encode_queue.size() == ENCODE_QUEUE_FRAME_NUM)
        {
            printf("warning_1: m_encode_queue is overflow!\n");
            usleep(10 * 1000);
            continue;
        }
        // 从队列中得到一帧数据
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ret = ioctl(v4l2_video->fd, VIDIOC_DQBUF, &buf);
        if(ret < 0)
        {
            printf("get frame failed!\n");
            usleep(10 * 1000);
            continue;
        }
        // 帧处理程序
        v4l2_video->frameHandle(v4l2_video->video_buffer[buf.index]);
        // 将帧放回队列
        ret = ioctl(v4l2_video->fd, VIDIOC_QBUF, &buf);
        if(ret < 0)
        {
            printf("put back frame failed!");
        }
        usleep(10 * 1000);

    }
    return nullptr;
}


void V4L2_Video::start()
{
    pthread_t thread_id;
    std::queue<VideoBuffer> empty1;
    std::queue<VideoBuffer> empty2;
    // 清空待编码队列
    m_encode_queue.swap(empty1);
    // 清空待发送队列
    m_send_queue.swap(empty2);

    m_is_start = true;

    // 创建帧获取线程
    pthread_create(&thread_id, nullptr, frame_process_thread, this);
    // 创建编码线程
    pthread_create(&thread_id, nullptr, yuyv422_to_H264_thread, this);
}


void V4L2_Video::stop()
{
    m_is_start = false;
}


// 注：该转换函数来源于网上，尚未验证
void V4L2_Video::yuyv422ToYuv420p(int inWidth, int inHeight, uint8_t *pSrc, uint8_t *pDest)
{
    int i, j;
    //首先对I420的数据整体布局指定
    uint8_t *u = pDest + (inWidth * inHeight);
    uint8_t *v = u + (inWidth * inHeight) / 4;

    for (i = 0; i < inHeight/2; i++)
    {
        /*采取的策略是:在外层循环里面，取两个相邻的行*/
        uint8_t *src_l1 = pSrc + inWidth*2*2*i;//因为4:2:2的原因，所以占用内存，相当一个像素占2个字节，2个像素合成4个字节
        uint8_t *src_l2 = src_l1 + inWidth*2;//YUY2的偶数行下一行
        uint8_t *y_l1 = pDest + inWidth*2*i;//偶数行
        uint8_t *y_l2 = y_l1 + inWidth;//偶数行的下一行
        for (j = 0; j < inWidth/2; j++)//内层循环
        {
            // two pels in one go//一次合成两个像素
            //偶数行，取完整像素;Y,U,V;偶数行的下一行，只取Y
            *y_l1++ = src_l1[0];//Y
            *u++ = src_l1[1];//U
            *y_l1++ = src_l1[2];//Y
            *v++ = src_l1[3];//V
            //这里只有取Y
            *y_l2++ = src_l2[0];
            *y_l2++ = src_l2[2];
            //YUY2,4个像素为一组
            src_l1 += 4;
            src_l2 += 4;
        }
    }
}


// X264软件编码
void *V4L2_Video::yuyv422_to_H264_thread(void *ptr)
{
    V4L2_Video *v4l2_video = static_cast<V4L2_Video *>(ptr);
    x264_t *encoder;
    x264_picture_t pic_in;
    x264_picture_t pic_out;
    x264_param_t pParam;
    x264_nal_t *nal;
    int i_nal;
    int i_frame_size;
    VideoBuffer yuv_buffer;

    int ret;
    int i;
    int width = IMAGEWIDTH;
    int height = IMAGEHEIGHT;
    int csp = X264_CSP_I420;              // yuv 4:2:0 planar
    VideoBuffer h264_buffer[SEND_QUEUE_FRAME_NUM];
    int h264_buffer_index = 0;
    int64_t i_pts = 0;
    int need_keyframe = 0;

    uint8_t *I420_buffer = static_cast<uint8_t *>(malloc(width * height * 2 * sizeof(uint8_t)));

    for(int i= 0; i < SEND_QUEUE_FRAME_NUM; i++)
    {
        h264_buffer[i].start = static_cast<uint8_t *>(malloc(width * height * 2* sizeof(uint8_t)));
    }

    x264_param_default_preset(&pParam, "medium", NULL);

    // 设置编码器参数并打开编码器
    x264_param_default(&pParam);
    pParam.i_width = width;
    pParam.i_height = height;
    pParam.i_csp = csp;
    pParam.rc.b_mb_tree = 0;        // 为0可降低实时编码时的延迟
    pParam.i_slice_max_size = 1024;
    pParam.b_vfr_input = 0;
    pParam.i_fps_num = VIDEO_FPS;
    pParam.i_fps_den = 1;
//    pParam.i_keyint_max = 100;


    x264_param_apply_profile(&pParam, "baseline");

    x264_picture_alloc(&pic_in, pParam.i_csp, pParam.i_width, pParam.i_height);

    encoder = x264_encoder_open(&pParam);

    //    pParam.i_keyint_max = 10;

    while(v4l2_video->m_is_start)
    {
        pthread_mutex_lock(&v4l2_video->m_encode_queue_lock);   // m_encode_queue加锁
        if(v4l2_video->m_encode_queue.size() == 0)
        {
            pthread_mutex_unlock(&v4l2_video->m_encode_queue_lock);     // m_encode_queue解锁
            usleep(10 * 1000);
            continue;
        }
        yuv_buffer = v4l2_video->m_encode_queue.front();
        v4l2_video->yuyv422ToYuv420p(width, height, static_cast<uint8_t *>(yuv_buffer.start), I420_buffer);
        v4l2_video->m_encode_queue.pop();
        pthread_mutex_unlock(&v4l2_video->m_encode_queue_lock);     // m_encode_queue解锁

        memcpy(pic_in.img.plane[0], I420_buffer, width * height);
        memcpy(pic_in.img.plane[1], I420_buffer + width * height, width * height / 4);
        memcpy(pic_in.img.plane[2], I420_buffer + width * height + width * height / 4, width * height / 4);

        pic_in.i_pts = ++i_pts;
        if(need_keyframe < 5)
        {
            pic_in.i_type = X264_TYPE_KEYFRAME;
            need_keyframe++;
        }
        else
        {
            pic_in.i_type = X264_TYPE_AUTO;
        }
        i_frame_size = x264_encoder_encode(encoder, &nal, &i_nal, &pic_in, &pic_out);
        if(i_frame_size < 0)
        {
            printf("x264_encoder_encode error! i_frame_size = %d\n", i_frame_size);
            continue;
        }
        else if(i_frame_size == 0)
        {
            continue;
        }
        if(h264_buffer_index == SEND_QUEUE_FRAME_NUM)
            h264_buffer_index = 0;
        memcpy(h264_buffer[h264_buffer_index].start, nal->p_payload, static_cast<unsigned long>(i_frame_size));
        h264_buffer[h264_buffer_index].length = static_cast<unsigned int>(i_frame_size);
        h264_buffer[h264_buffer_index].type = static_cast<enum nal_unit_type_e>(nal->i_type);
        // 将编码好的帧放入发送队列
        pthread_mutex_lock(&v4l2_video->m_send_queue_lock);   // m_send_queue加锁
        if(v4l2_video->m_send_queue.size() < SEND_QUEUE_FRAME_NUM)
            v4l2_video->m_send_queue.push(h264_buffer[h264_buffer_index]);
        else
            printf("warning: m_send_queue is overflow!\n");
        pthread_mutex_unlock(&v4l2_video->m_send_queue_lock);   // m_send_queue解锁
        h264_buffer_index++;
    }


    free(I420_buffer);

    for(i = 0; i < SEND_QUEUE_FRAME_NUM; i++)
        free(h264_buffer[i].start);

    x264_picture_clean(&pic_in);
    x264_encoder_close(encoder);

    return 0;
}




int V4L2_Video::video_read(char **buffer, int *buffer_size, enum nal_unit_type_e *type)
{
    int ret;
    static unsigned char send_data_buffer[IMAGEWIDTH * IMAGEHEIGHT * 2];
    if(m_is_start == false)
    {
        printf("error: v4l2 video capture is stopped!\n");
        return -1;
    }

    pthread_mutex_lock(&m_send_queue_lock);   // m_send_queue加锁
    if(m_send_queue.size() == 0)
    {
        pthread_mutex_unlock(&m_send_queue_lock);   // m_send_queue解锁
//        printf("video send queue is empty!\n");
        return -1;
    }

    *buffer_size = static_cast<int>(m_send_queue.front().length);
    memcpy(send_data_buffer, m_send_queue.front().start, static_cast<unsigned int>(*buffer_size));
    *type = m_send_queue.front().type;
    m_send_queue.pop();
    pthread_mutex_unlock(&m_send_queue_lock);   // m_send_queue解锁


    *buffer = reinterpret_cast<char *>(send_data_buffer);


    return 0;
}


int V4L2_Video::get_width()
{
    return IMAGEWIDTH;
}

int V4L2_Video::get_height()
{
    return IMAGEHEIGHT;
}

int V4L2_Video::get_fps()
{
    return VIDEO_FPS;
}
