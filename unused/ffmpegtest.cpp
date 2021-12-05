#include "ffmpegtest.h"
#include "ui_ffmpegtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

//ffmpeg库
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>

#ifdef __cplusplus
}
#endif


FfmpegTest::FfmpegTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FfmpegTest)
{
    ui->setupUi(this);


    AVFormatContext *fmtctx;
    AVFrame *pFrame;
    AVCodec *video_codec;
    enum AVCodecID codec_id;
    AVStream *video_st;
    AVFormatContext *oc;
    AVCodecContext *c;
    AVPacket pkt;

    QString SendIp = "192.168.8.200";
    int SendPort = 12345;
    int capWidth = 320;
    int capHeight = 200;
    int frameRate = 32;
    char sdp[256] = {0};
    int dstMat ,yMat,uMat,vMat;
    int got_output;






//1. 初始化, 依次完成以下工作
    av_register_all();

    //使用RTSP网络流时必须先执行
    avformat_network_init();
    pFrame = av_frame_alloc();
    fmtctx = avformat_alloc_context();

     //设置流格式为RTP
    fmtctx->oformat = av_guess_format("rtp", NULL, NULL);
    //用指定IP和端口构造输出流地址
    sprintf(fmtctx->filename,"rtp://%s:%d",SendIp.toLatin1().data(),SendPort);

    //打开输出流
    avio_open(&fmtctx->pb,fmtctx->filename, AVIO_FLAG_WRITE);

    //查找编码器
    video_codec = avcodec_find_encoder(codec_id);

    //初始化AVStream
    video_st = avformat_new_stream(oc, video_codec);

    //设置AVCodecContext编码参数
    avcodec_get_context_defaults3(c, video_codec);
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_VIDEO;
    c->width = capWidth;
    c->height = capHeight;
    c->time_base.den = frameRate;
    c->time_base.num = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;   //PIX_FMT_YUV420P;
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags|= AV_CODEC_FLAG_GLOBAL_HEADER;

    av_opt_set(c->priv_data, "preset", "slow", 0);
    av_opt_set(c->priv_data, "tune","zerolatency",0);
    av_opt_set(c->priv_data, "x264opts","crf=26:vbv-maxrate=728:vbv-bufsize=3640:keyint=25",0);

    //打开编码器
    avcodec_open2(video_st->codec, video_codec, NULL);

    //写文件头
    avformat_write_header(fmtctx, NULL);

    //打印SDP信息, 该信息可用于Rtp流接收解码
    av_sdp_create(&fmtctx,1, sdp, sizeof(sdp));
//2. 视频编码, rtp传输.  该处使用一个单独的线程完成.
    while (1)
    {
        //退出线程, 略

//        //获取RGB图像
//        cap>>srcMat;

//        //格式转换, BGR->YUV420P
//        convertMatBGR2YUV420(dstMat ,yMat,uMat,vMat);

        //构造AVPacket.
        av_init_packet(&pkt);

        pFrame->pts = video_st->codec->frame_number;

         //视频编码
        avcodec_encode_video2(c, &pkt,pFrame, &got_output);
        if (got_output) {
            if (c->coded_frame->key_frame)
                pkt.flags |= AV_PKT_FLAG_KEY;
            pkt.stream_index = video_st->index;

            //计算PTS
            if (pkt.pts != AV_NOPTS_VALUE ) {
                pkt.pts = av_rescale_q(pkt.pts,video_st->codec->time_base, video_st->time_base);
            }
            if(pkt.dts !=AV_NOPTS_VALUE ) {
                pkt.dts = av_rescale_q(pkt.dts,video_st->codec->time_base, video_st->time_base);
            }

            //写入一个AVPacket到输出文件, 这里是一个输出流
            av_interleaved_write_frame(oc,&pkt);
        }

        //销毁AVPacket.
        av_free_packet(&pkt);
    }

    //销毁资源
    av_frame_free(&pFrame);
    avcodec_close(video_st->codec);
    av_free(video_st->codec);
    avformat_free_context(fmtctx);


}

FfmpegTest::~FfmpegTest()
{
    delete ui;
}
