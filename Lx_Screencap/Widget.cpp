#include "Widget.h"
#include <QPainter>
#include <QDebug>
#include <QTimer>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    avdevice_register_all();                                    //初始化所有设备
        formatContext=avformat_alloc_context();                     //分配format上下文
        const AVInputFormat *inputFormat=av_find_input_format("gdigrab"); //寻找输入设备【gdigrab】
        AVDictionary* options = NULL;
        av_dict_set(&options,"framerate","60",0);                   //设置帧数为60
        if(avformat_open_input(&formatContext,"desktop",inputFormat,&options)){ //开启输入设备
            qDebug()<<"cant`t open input stream.";
            return ;
        }
        if(avformat_find_stream_info(formatContext,nullptr)){       //加载流中存储的信息
            qDebug()<<"can`t find stream information.";
            return ;
        }

        videoIndex=-1;                                              //寻找视频流
        for(uint i=0;i<formatContext->nb_streams;i++){
            if(formatContext->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
                videoIndex=i;
                break;
            }
        }
        if(videoIndex==-1){
            qDebug()<<"can`t find video stream.";
            return ;
        }
        codecParameters=formatContext->streams[videoIndex]->codecpar;
        codecContext=avcodec_alloc_context3(nullptr);
        avcodec_parameters_to_context(codecContext,codecParameters);
        const AVCodec* codec=avcodec_find_decoder(codecParameters->codec_id);

        if(codec==nullptr){
            qDebug()<<"can`t find codec";
            return ;
        }
        if(avcodec_open2(codecContext,codec,nullptr)){
            qDebug()<<"can`t open codec";
            return ;
        }

        packet=av_packet_alloc();
        frame=av_frame_alloc();

        imgConvertContext=sws_getContext(codecParameters->width,codecParameters->height,codecContext->pix_fmt,codecParameters->width,codecParameters->height,AV_PIX_FMT_RGB24,SWS_BICUBIC,nullptr,nullptr,nullptr);
        image=QImage(codecParameters->width,codecParameters->height,QImage::Format_RGB888);
        av_image_fill_linesizes(lineSize,AV_PIX_FMT_RGB24,codecParameters->width);

        QTimer *timer=new QTimer;           //定时刷新
        connect(timer,&QTimer::timeout,this,static_cast<void (QWidget::*)()>(&QWidget::repaint));
        timer->setInterval(20);
        timer->start();

        resize(codecParameters->width*0.6,codecParameters->height*0.6);


}

Widget::~Widget()
{
}

void Widget::paintEvent(QPaintEvent *event){
    if(av_read_frame(formatContext,packet)){
            return ;
        }
        if(packet->stream_index==videoIndex){
            if(avcodec_send_packet(codecContext,packet))
                return;
            if(avcodec_receive_frame(codecContext,frame))
                return;
            uint8_t* dst[]={image.bits()};
            sws_scale(imgConvertContext,frame->data,frame->linesize,0,codecParameters->height,dst,lineSize);
            av_packet_unref(packet);                //清空数据包

            QPainter painter(this);
            painter.fillRect(rect(),image.scaled(width(),height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
        }

}
