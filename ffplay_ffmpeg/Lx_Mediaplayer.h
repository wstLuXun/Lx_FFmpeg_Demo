#ifndef LX_MEDIAPLAYER_H
#define LX_MEDIAPLAYER_H

#include <Lx_AvPacketQueue.h>

#include <QImage>
extern "C"
{

    #include "libavfilter/buffersink.h"
    #include "libavfilter/buffersrc.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
    #include "libavutil/pixfmt.h"
    #include "libavutil/opt.h"
    #include "libavcodec/avfft.h"
    #include "libavutil/imgutils.h"
    #include "libswresample/swresample.h"
    #include <libavcodec/avcodec.h>
    #include "libavutil/time.h"

     #include <SDLinclude/SDL.h>

}

#undef main
#include <QObject>
class Lx_Mediaplayer : public QObject
{
    Q_OBJECT
public:
    explicit Lx_Mediaplayer(QObject *parent = nullptr);
    //友元类 让Lx_Thread对象访问Lx_Mediaplayer对象的私有成员
    //friend Lx_Thread;


signals:
    void video_display(QImage image);
    void readFinished();
    void video_time(qint64 time);
    void video_timeChange(double time);

public slots:
  void play(QString path);
  void setVolume(int volume);
  int  Volume();
  void setPause(bool pause);
  bool Pause();
  void setStop(bool stop);
  bool Stop();
  void setSeekProgress(qint64 seek_pos);


private:
    int volume=120;
    double speed;
    bool isplay=false;
    bool pause=false;
    bool stop=false;


    bool seek_req;
    qint64 seek_pos;
    double duration=0;
    bool isReadFinished=false;


private slots:
    void stream_open(QString path);
    int stream_component_open(int stream_index);
    int openAudio();
    void closeAudio();
    int decodeAudio();
    int decodeVideo(AVPacket& pkt,AVFrame& frame);
    double getAudioClock();
    double synchronize(AVFrame *frame, double pts);
    bool stream_seek(double pos,int stream_index);
    void clearData();


protected:
  static  void audio_callback(void *udata, Uint8 *stream, int len);
  static int read_thread(void *arg);
  static int video_thread(void *arg);

private:


    //url路径
    QString filepath;
    //核心结构体对象  用一个本地文件或网络文件来初始化该对象
    AVFormatContext* FormatContext;

    //解码器其解码器上下文
    AVCodecContext *audio_avctx;
    AVCodecContext *video_avctx;
    AVCodec *video_codec;
    AVCodec *audio_codec;
    // 流索引
    int video_stream;
    int audio_stream;
    int subtitle_stream;
    //音频数据

    Uint8 audioBuf1[19200];
    Uint8* audioBuf=0;

    int audioBufSize=0;
    int audioBufIndex=0;
    AVSampleFormat audioDstFmt;

    SDL_AudioSpec spec;
    quint8 audioDepth;


    qint64 audioDstChannelLayout;
    int audioSrcChannels;
    enum AVSampleFormat audioSrcFmt;
    int audioSrcFreq;
    double clock=0;
    double videoClk=0;
    int sendReturn=0;
    //音频数据 end

    SDL_mutex *wait_mutex=0;
    SDL_cond *continue_read_thread=0;

    Lx_AvPacketQueue video_pkts;
    Lx_AvPacketQueue audio_pkts;
    Lx_AvPacketQueue subtitle_pkts;
};

#endif // LX_MEDIAPLAYER_H
