#include "Lx_Screencap.h"
#include <QDebug>

extern "C" {        // 用C规则编译指定的代码
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavdevice/avdevice.h"
}

Lx_Screencap::Lx_Screencap(QObject *parent) : QObject(parent)
{   
    this->moveToThread(&thread);
    // 初始化libavdevice并注册所有输入和输出设备。
    avdevice_register_all();
    FormatContext=0;
    video_stream=-1;
    video_avctx=0;
    video_codec=0;
    imgConvertContext=0;
    codecParameters=0;

}

Lx_Screencap::~Lx_Screencap()
{
    qDebug()<< "~Lx_Thread";
}


void Lx_Screencap::start(QString name)
{


    if(name=="read_thread")
       connect(&thread,&QThread::started,this,&Lx_Screencap::read_thread);


    thread.start();
}




void Lx_Screencap::read_thread()
{
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame=av_frame_alloc();

   stream_open("desktop");
qDebug()<<"111111";
   codecParameters=FormatContext->streams[video_stream]->codecpar;
   imgConvertContext=sws_getContext(codecParameters->width,codecParameters->height,
                        video_avctx->pix_fmt,codecParameters->width,codecParameters->height,
                                    AV_PIX_FMT_RGB24,SWS_BICUBIC,nullptr,nullptr,nullptr);
  image=QImage(codecParameters->width,codecParameters->height,QImage::Format_RGB888);


   while (1) {

       // 读取下一帧数据
       int readRet = av_read_frame(FormatContext, pkt);
       if(readRet < 0)
       {
           avcodec_send_packet(video_avctx, pkt); // 读取完成后向解码器中传如空AVPacket，否则无法读取出最后几帧
       }
       else
       {

           if(pkt->stream_index == video_stream)     // 如果是图像数据则进行解码
           {
               // 将读取到的原始数据包传入解码器
               int ret = avcodec_send_packet(video_avctx, pkt);
               if(ret < 0)
               {
                   //showError(ret);
               }
           }
       }
       av_packet_unref(pkt);  // 释放数据包，引用计数-1，为0时释放空间
       av_frame_unref(frame);
       int ret = avcodec_receive_frame(video_avctx, frame);
       if(ret < 0)
       {
           av_frame_unref(frame);
           if(readRet < 0)
           {
                   // 当无法读取到AVPacket并且解码器中也没有数据时表示读取完成
           }
          continue;
       }

       qDebug()<<"pts:"<<frame->pts;

       uint8_t *dst[]={image.bits()};
       sws_scale(imgConvertContext,frame->data,frame->linesize,0,codecParameters->height,dst,lineSize);


       emit sendImage(image);
   }
}

void Lx_Screencap::stream_open(QString path)
{
    AVDictionary* dict = nullptr;
    FormatContext=avformat_alloc_context();

    // 所有参数：https://ffmpeg.org/ffmpeg-devices.html
    av_dict_set(&dict, "framerate", "20", 0);          // 设置帧率，默认的是30000/1001，但是实际可能达不到30的帧率，所以最好手动设置
    av_dict_set(&dict, "draw_mouse", "1", 0);          // 指定是否绘制鼠标指针。0：不包含鼠标，1：包含鼠标
 // av_dict_set(&dict, "video_size", "500x400", 0);    // 录制视频的大小（宽高），默认为全屏
    const AVInputFormat*  inputFormat = av_find_input_format("gdigrab");
    int ret = avformat_open_input(&FormatContext,          // 返回解封装上下文
                                  "desktop",  // 打开视频地址
                                  inputFormat,             // 如果非null，此参数强制使用特定的输入格式。自动选择解封装器（文件格式）
                                  &dict);                    // 参数设置
    // 打开视频失败
    if(ret < 0)
    {
        qDebug()<<"avformat_open_input field";
    }

    // 释放参数字典
    if(dict)
    {
        av_dict_free(&dict);
    }

    //查找流信息
   if(avformat_find_stream_info(FormatContext,NULL)<0){
      qDebug()<<"avformat_find_stream_info field";
   }
   //查找音视频流索引
   video_stream=av_find_best_stream(FormatContext,AVMEDIA_TYPE_VIDEO,-1, -1, NULL, 0);
   if(video_stream<0){
     qDebug()<<"av_find_best_stream field";
   }

   stream_component_open(video_stream);


}

int Lx_Screencap::stream_component_open(int stream_index)
{
    AVCodecContext *avctx;
    AVCodec *codec;

    if (stream_index < 0 || stream_index >= FormatContext->nb_streams)
        return -1;
    //记得释放avctx
    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
        return AVERROR(ENOMEM);

    int ret = avcodec_parameters_to_context(avctx, FormatContext->streams[stream_index]->codecpar);
    if (ret < 0){
        qDebug()<<"avcodec_parameters_to_context field";
    }

    codec = (AVCodec *)avcodec_find_decoder(avctx->codec_id);
    avctx->pkt_timebase = FormatContext->streams[stream_index]->time_base;
    //查找解码器



    ret = avcodec_open2(avctx, codec, NULL);
    if(ret!=0){
       qDebug()<<"avcodec_open2 field";
    }


    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:{

    }
        break;
    case AVMEDIA_TYPE_VIDEO:
    {
        this->video_avctx=avctx;
        this->video_codec=codec;
        qDebug()<<"AVMEDIA_TYPE_VIDEO 视频解码器";
        this->video_avctx->flags2 |= AV_CODEC_FLAG2_FAST;    // 允许不符合规范的加速技巧。
        this->video_avctx->thread_count = 8;                 // 使用8线程解码

    }
       break;
    case AVMEDIA_TYPE_SUBTITLE:
    {
        qDebug()<<"AVMEDIA_TYPE_SUBTITLE avctx->codec_type field";
    }
        break;
    }
}









