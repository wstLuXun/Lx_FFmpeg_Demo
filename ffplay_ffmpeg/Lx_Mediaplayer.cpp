#include "Lx_Mediaplayer.h"

#include <QDebug>
#include <QThread>
/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

Lx_Mediaplayer::Lx_Mediaplayer(QObject *parent) : QObject(parent)
{

   avformat_network_init();
   //FormatContext=avformat_alloc_context();
   FormatContext=0;

}

void Lx_Mediaplayer::play(QString path)
{
    this->isplay=true;
    setStop(false);
    setPause(false);
    this->filepath=path;

    clearData();


    SDL_CreateThread(&Lx_Mediaplayer::read_thread, "read_thread", this);
}

void Lx_Mediaplayer::stream_open(QString path)
{

   video_stream=audio_stream=-1;
   clock=0;
   videoClk=0;

   qDebug()<<"read_thread id="<<QThread::currentThreadId();
   QByteArray array=filepath.toLatin1();
   //打开输入文件
   FormatContext=avformat_alloc_context();
   if(avformat_open_input(&FormatContext,array.data(),0,0)!=0){
       qDebug()<<"avformat_open_input field";
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
  audio_stream=av_find_best_stream(FormatContext,AVMEDIA_TYPE_AUDIO,-1, -1, NULL, 0);
  if(audio_stream<0){
    qDebug()<<"av_find_best_stream field";
  }
  subtitle_stream=av_find_best_stream(FormatContext,AVMEDIA_TYPE_SUBTITLE,-1, -1, NULL, 0);
  if(audio_stream<0){
    qDebug()<<"av_find_best_stream field";
  }
  //获取文件时长 单位s
  duration=FormatContext->duration/AV_TIME_BASE;
  emit video_time(duration);
  //打开这个流
  if(audio_stream>=0)
    stream_component_open(audio_stream);

  if(video_stream>=0)
    stream_component_open(video_stream);


}

void Lx_Mediaplayer::setVolume(int volume)
{
    this->volume=volume%SDL_MIX_MAXVOLUME;
}

int Lx_Mediaplayer::Volume()
{
    return this->volume;
}

void Lx_Mediaplayer::setPause(bool pause)
{
    this->pause=pause;
}

bool Lx_Mediaplayer::Pause()
{
    return this->pause;
}

void Lx_Mediaplayer::setStop(bool stop)
{
    this->stop=stop;
    if(stop){
        videoClk=0;
        clock=0;

    }
}

bool Lx_Mediaplayer::Stop()
{
    return  stop;
}

void Lx_Mediaplayer::setSeekProgress(qint64 seek_pos)
{
    if(!seek_req){

       this->seek_pos=seek_pos;
       SDL_CondSignal(continue_read_thread);
       seek_req=true;
    }
}


int Lx_Mediaplayer::stream_component_open(int stream_index)
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
        this->audio_avctx=avctx;
        this->audio_codec=codec;
        if(openAudio()==0) qDebug()<<"openAudio 成功";


    }
        break;
    case AVMEDIA_TYPE_VIDEO:
    {
        this->video_avctx=avctx;
        this->video_codec=codec;
        SDL_CreateThread(&Lx_Mediaplayer::video_thread, "video_thread", this);
    }
       break;
    case AVMEDIA_TYPE_SUBTITLE:
    {
        qDebug()<<"AVMEDIA_TYPE_SUBTITLE avctx->codec_type field";
    }
        break;
    }
//    avcodec_close(avctx);
//    avcodec_free_context(&avctx);
}




int Lx_Mediaplayer::openAudio()
{
    AVCodec *codec;
    SDL_AudioSpec wantedSpec;

    int wantedNbChannels;
    quint32 audioDeviceFormat=AUDIO_S16SYS;  // audio device sample format

    const char *env;

    /*  soundtrack array use to adjust */
    int nextNbChannels[]   = {0, 0, 1, 6, 2, 6, 4, 6};
    int nextSampleRates[]  = {0, 44100, 48000, 96000, 192000};
    int nextSampleRateIdx = FF_ARRAY_ELEMS(nextSampleRates) - 1;

    stop = false;
    pause = false;
    isReadFinished = false;

    audioSrcFmt = AV_SAMPLE_FMT_NONE;
    //audioSrcChannelLayout = 0;
    audioSrcFreq = 0;

    FormatContext->streams[audio_stream]->discard = AVDISCARD_DEFAULT;



    env = SDL_getenv("SDL_AUDIO_CHANNELS");
    if (env) {
        qDebug() << "SDL audio channels";
        wantedNbChannels = atoi(env);
        audioDstChannelLayout = av_get_default_channel_layout(wantedNbChannels);
    }

    wantedNbChannels = audio_avctx->channels;
    if (!audioDstChannelLayout ||
        (wantedNbChannels != av_get_channel_layout_nb_channels(audioDstChannelLayout))) {
        audioDstChannelLayout = av_get_default_channel_layout(wantedNbChannels);
        audioDstChannelLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    wantedSpec.channels    = av_get_channel_layout_nb_channels(audioDstChannelLayout);
    wantedSpec.freq        = audio_avctx->sample_rate;
    if (wantedSpec.freq <= 0 || wantedSpec.channels <= 0) {
        avcodec_free_context(&audio_avctx);
        qDebug() << "Invalid sample rate or channel count, freq: " << wantedSpec.freq << " channels: " << wantedSpec.channels;
        return -1;
    }

    while (nextSampleRateIdx && nextSampleRates[nextSampleRateIdx] >= wantedSpec.freq) {
        nextSampleRateIdx--;
    }

    wantedSpec.format      = audioDeviceFormat;
    wantedSpec.silence     = 0;
    wantedSpec.samples     = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wantedSpec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wantedSpec.callback    = &Lx_Mediaplayer::audio_callback;
    wantedSpec.userdata    = this;

    /* This function opens the audio device with the desired parameters, placing
     * the actual hardware parameters in the structure pointed to spec.
     */
    while (1) {
        while (SDL_OpenAudio(&wantedSpec, &spec) < 0) {
            qDebug() << QString("SDL_OpenAudio (%1 channels, %2 Hz): %3")
                    .arg(wantedSpec.channels).arg(wantedSpec.freq).arg(SDL_GetError());
            wantedSpec.channels = nextNbChannels[FFMIN(7, wantedSpec.channels)];
            if (!wantedSpec.channels) {
                wantedSpec.freq = nextSampleRates[nextSampleRateIdx--];
                wantedSpec.channels = wantedNbChannels;
                if (!wantedSpec.freq) {
                    avcodec_free_context(&audio_avctx);
                    qDebug() << "No more combinations to try, audio open failed";
                    return -1;
                }
            }
            audioDstChannelLayout = av_get_default_channel_layout(wantedSpec.channels);
        }

        if (spec.format != audioDeviceFormat) {
            qDebug() << "SDL audio format: " << wantedSpec.format << " is not supported"
                     << ", set to advised audio format: " <<  spec.format;
            wantedSpec.format = spec.format;
            audioDeviceFormat = spec.format;
            SDL_CloseAudio();
        } else {
            break;
        }
    }

    if (spec.channels != wantedSpec.channels) {
        audioDstChannelLayout = av_get_default_channel_layout(spec.channels);
        if (!audioDstChannelLayout) {
            avcodec_free_context(&audio_avctx);
            qDebug() << "SDL advised channel count " << spec.channels << " is not supported!";
            return -1;
        }
    }

    /* set sample format */
    switch (audioDeviceFormat) {
    case AUDIO_U8:
        audioDstFmt    = AV_SAMPLE_FMT_U8;
        audioDepth = 1;
        break;

    case AUDIO_S16SYS:
        audioDstFmt    = AV_SAMPLE_FMT_S16;
        audioDepth = 2;
        break;

    case AUDIO_S32SYS:
        audioDstFmt    = AV_SAMPLE_FMT_S32;
        audioDepth = 4;
        break;

    case AUDIO_F32SYS:
        audioDstFmt    = AV_SAMPLE_FMT_FLT;
        audioDepth = 4;
        break;

    default:
        audioDstFmt    = AV_SAMPLE_FMT_S16;
        audioDepth = 2;
        break;
    }

    /* open sound */
    SDL_PauseAudio(0);

    return 0;
}

void Lx_Mediaplayer::closeAudio()
{


    SDL_LockAudio();
    SDL_CloseAudio();
    SDL_UnlockAudio();

    avcodec_close(audio_avctx);
    avcodec_free_context(&audio_avctx);
}

int Lx_Mediaplayer::decodeAudio()
{
    static struct SwrContext *au_convert_ctx=0;
    static qint64 audioSrcChannelLayout=0;
    int ret;
    AVFrame *frame = av_frame_alloc();
    AVPacket* packet;
    int resampledDataSize;

    if (!frame) {
        qDebug() << "Decode audio frame alloc failed.";
        return -1;
    }

    if (stop) {
        return -1;
    }

    if (audio_pkts.length() <= 0) {
        if (isReadFinished) {
            stop = true;
            SDL_Delay(100);
            //emit playFinished();
        }
        return -1;
    }

    /* get new packet whiel last packet all has been resolved */
    if (sendReturn != AVERROR(EAGAIN)) {
        packet=audio_pkts.dequeue();
    }
    if(!packet){
       return -1;
    }

    if (!strcmp((char*)packet->data, "FLUSH")) {
        avcodec_flush_buffers(audio_avctx);
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet=nullptr;
        av_frame_free(&frame);
        sendReturn = 0;
        qDebug() << "seek audio";
        return -1;
    }

    /* while return -11 means packet have data not resolved,
     * this packet cannot be unref
     */
    sendReturn = avcodec_send_packet(audio_avctx, packet);
    if ((sendReturn < 0) && (sendReturn != AVERROR(EAGAIN)) && (sendReturn != AVERROR_EOF)) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet=nullptr;
        av_frame_free(&frame);
        qDebug() << "Audio send to decoder failed, error code: " << sendReturn;
        return sendReturn;
    }

    ret = avcodec_receive_frame(audio_avctx, frame);
    if ((ret < 0) && (ret != AVERROR(EAGAIN))) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet=nullptr;
        av_frame_free(&frame);
        qDebug() << "Audio frame decode failed, error code: " << ret;
        return ret;
    }

    if (frame->pts != AV_NOPTS_VALUE) {
        clock = av_q2d(FormatContext->streams[audio_stream]->time_base) * frame->pts;
//        qDebug() << "no pts";
    }

    /* get audio channels */
    qint64 inChannelLayout = (frame->channel_layout && frame->channels == av_get_channel_layout_nb_channels(frame->channel_layout)) ?
                frame->channel_layout : av_get_default_channel_layout(frame->channels);

    if (frame->format       != audioSrcFmt              ||
        inChannelLayout     != audioSrcChannelLayout    ||
        frame->sample_rate  != audioSrcFreq             ||
        !au_convert_ctx) {
        if (au_convert_ctx) {
            swr_free(&au_convert_ctx);
        }

        /* init swr audio convert context */
        au_convert_ctx = swr_alloc_set_opts(nullptr, audioDstChannelLayout, audioDstFmt, spec.freq,
                inChannelLayout, (AVSampleFormat)frame->format , frame->sample_rate, 0, NULL);
        if (!au_convert_ctx || (swr_init(au_convert_ctx) < 0)) {
            av_packet_unref(packet);
            av_packet_free(&packet);
            packet=nullptr;
            av_frame_free(&frame);
            return -1;
        }

        audioSrcFmt             = (AVSampleFormat)frame->format;
        audioSrcChannelLayout   = inChannelLayout;
        audioSrcFreq            = frame->sample_rate;
        audioSrcChannels        = frame->channels;
    }

    if (au_convert_ctx) {
        const quint8 **in   = (const quint8 **)frame->extended_data;
        uint8_t *out[] = {audioBuf1};

        int outCount = sizeof(audioBuf1) / spec.channels / av_get_bytes_per_sample(audioDstFmt);

        int sampleSize = swr_convert(au_convert_ctx, out, outCount, in, frame->nb_samples);
        if (sampleSize < 0) {
            qDebug() << "swr convert failed";
            av_packet_unref(packet);
            av_packet_free(&packet);
            packet=nullptr;
            av_frame_free(&frame);
            return -1;
        }

        if (sampleSize == outCount) {
            qDebug() << "audio buffer is probably too small";
            if (swr_init(au_convert_ctx) < 0) {
                swr_free(&au_convert_ctx);
            }
        }

        audioBuf = audioBuf1;
        resampledDataSize = sampleSize * spec.channels * av_get_bytes_per_sample(audioDstFmt);
    } else {
        audioBuf = frame->data[0];
        resampledDataSize = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, static_cast<AVSampleFormat>(frame->format), 1);
    }

    clock += static_cast<double>(resampledDataSize) / (audioDepth * audio_avctx->channels * audio_avctx->sample_rate);

    if (sendReturn != AVERROR(EAGAIN)) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet=nullptr;
    }
    if(packet) av_packet_free(&packet);
    av_frame_free(&frame);

    return resampledDataSize;

}

int Lx_Mediaplayer::decodeVideo(AVPacket &pkt, AVFrame &frame)
{
    /* flush codec buffer while received flush packet */
    if (!strcmp((char *)pkt.data, "FLUSH")) {
        qDebug() << "Seek video";
        avcodec_flush_buffers(video_avctx);
        av_packet_unref(&pkt);
        return -1;
    }

   int  ret = avcodec_send_packet(video_avctx, &pkt);
    if ((ret < 0) && (ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF)) {
        qDebug() << "Video send to decoder failed, error code: " << ret;
        av_packet_unref(&pkt);
        return -1;
    }
    ret = avcodec_receive_frame(video_avctx, &frame);

    if ((ret < 0) && (ret != AVERROR_EOF)) {
        qDebug() << "Video frame decode failed, error code: " << (ret==AVERROR(EAGAIN));
        av_packet_unref(&pkt);
        return -1;
    }

    return ret;
}

double Lx_Mediaplayer::getAudioClock()
{
    if (audio_avctx&&!pause) {
        /* control audio pts according to audio buffer data size */
        int hwBufSize   = audioBufSize - audioBufIndex;
        int bytesPerSec = audio_avctx->sample_rate * audio_avctx->channels * audioDepth;

        clock -= static_cast<double>(hwBufSize) / bytesPerSec;
    }

    return clock;
}


double Lx_Mediaplayer::synchronize(AVFrame *frame, double pts)
{
    double delay;

    if (pts != 0) {
        videoClk = pts; // Get pts,then set video clock to it
    } else {
        pts = videoClk; // Don't get pts,set it to video clock
    }

    delay = av_q2d(FormatContext->streams[video_stream]->time_base);
    delay += frame->repeat_pict * (delay * 0.5);

    videoClk += delay;

    return pts;
}

bool Lx_Mediaplayer::stream_seek(double pos, int stream_index)
{
//    int64_t target_ts = av_rescale_q(pos * AV_TIME_BASE, AV_TIME_BASE_Q, FormatContext->streams[stream_index]->time_base);

//       int64_t start_ts = av_rescale_q(0, AV_TIME_BASE_Q, FormatContext->streams[stream_index]->time_base);
//       int64_t end_ts = av_rescale_q(FormatContext->duration, FormatContext->streams[stream_index]->time_base, AV_TIME_BASE_Q);

//       if (avformat_seek_file(FormatContext, -1, start_ts, target_ts, end_ts, AVSEEK_FLAG_BACKWARD) < 0) {
//           // 定位失败
//           return false;
//       }

       if (avformat_seek_file(FormatContext, -1, INT64_MIN, pos*AV_TIME_BASE, INT64_MAX, AVSEEK_FLAG_BACKWARD) < 0) {
           // 定位失败
           qDebug()<<"avformat_seek_file";
           return false;
       }
     if(seek_req){

       video_pkts.clear();
       audio_pkts.clear();
       avcodec_flush_buffers(audio_avctx);
       avcodec_flush_buffers(video_avctx);

       //av_usleep(5000);
     }
       return true;
}

void Lx_Mediaplayer::clearData()
{
    video_stream = -1,
    audio_stream = -1,
    subtitle_stream = -1,

    duration = 0;
    seek_pos=0;

    stop  = false;
    pause = false;
    seek_req  = false;
    isReadFinished      = false;
    //isDecodeFinished    = false;



    videoClk = 0;

    audioBuf = nullptr;

    audioBufIndex = 0;
    audioBufSize = 0;


    clock = 0;

    sendReturn = 0;

    video_pkts.clear();
    audio_pkts.clear();
}





void Lx_Mediaplayer::audio_callback(void *udata, Uint8 *stream, int len)
{
    //首先使用SDL_memset()将stream中的数据设置为0

    Lx_Mediaplayer *decoder = (Lx_Mediaplayer *)udata;


     int decodedSize;
    /* SDL_BufSize means audio play buffer left size
     * while it greater than 0, means counld fill data to it
     */
    while (len > 0) {
        if (decoder->stop) {
           qDebug()<<"22222222";
            break;
        }

        if (decoder->pause) {
            SDL_Delay(10);
            continue;
        }

        if(decoder->audio_pkts.isEmpty()) break;

        /* no data in buffer */
        if (decoder->audioBufIndex >= decoder->audioBufSize) {

            decodedSize = decoder->decodeAudio();
            /* if error, just output silence */
            if (decodedSize < 0) {
                /* if not decoded data, just output silence */
                decoder->audioBufSize = 1024;
                decoder->audioBuf = nullptr;
            } else {
                decoder->audioBufSize = decodedSize;
            }
            decoder->audioBufIndex = 0;
        }

        /* calculate number of data that haven't play */
        int left = decoder->audioBufSize - decoder->audioBufIndex;
        if (left > len) {
            left = len;
        }

        if (decoder->audioBuf) {
            memset(stream, 0, left);

            SDL_MixAudio(stream, decoder->audioBuf + decoder->audioBufIndex, left, decoder->volume);//decoder->volume
        }

        len -= left;
        stream += left;
        decoder->audioBufIndex += left;
    }

}

int Lx_Mediaplayer::read_thread(void *arg)
{
    Lx_Mediaplayer* data=(Lx_Mediaplayer*)arg;
    AVPacket *pkt =nullptr;

    data->wait_mutex = SDL_CreateMutex();
    data->continue_read_thread = SDL_CreateCond();

    data->stream_open(data->filepath);

    while (true) {
        if (data->stop) {
            break;
        }

        /* do not read next frame & delay to release cpu utilization */
        if (data->pause) {
            SDL_Delay(10);
            continue;
        }

        if(data->seek_req){
           if(!data->stream_seek(data->seek_pos,data->audio_stream)){
              data->seek_req=false;
              continue;
           }
            data->videoClk=data->seek_pos;
            data->seek_req=false;
        }


        if(data->FormatContext&&(data->audio_pkts.length()+data->video_pkts.length())>200){

            /* wait 10 ms */
            SDL_LockMutex(data->wait_mutex);
            SDL_CondWaitTimeout(data->continue_read_thread,data->wait_mutex, 10);
            SDL_UnlockMutex(data->wait_mutex);
            continue;

        }
        //这里申请空间 留其他解码线程释放
        //不要直接用AVPacket pkt;这种方式申请空间
        pkt=nullptr;
        pkt =av_packet_alloc();
        if(!pkt) continue;

        int ret=av_read_frame(data->FormatContext,pkt);
        if(ret<0){
            if(ret==AVERROR_EOF){
                qDebug() << "Read file completed.";
                data->isReadFinished = true;
                emit data->readFinished();
                SDL_Delay(10);
                break;
            }
            continue;
        }



        //区分包类型是音频还是视频
       if(pkt->stream_index==data->video_stream){
         data->video_pkts.enqueue(pkt);
         //qDebug()<<"data->video_pkts.count()==" <<data->video_pkts.count();
       }
       if(pkt->stream_index==data->audio_stream){
          data->audio_pkts.enqueue(pkt);
          //qDebug()<<"data->audio_pkts.count()==" <<data->audio_pkts.count();
       }
       if(pkt->stream_index==data->subtitle_stream){
          data->subtitle_pkts.enqueue(pkt);
       }

     }

    //av_packet_free(&pkt);
    SDL_DestroyCond(data->continue_read_thread);
    SDL_DestroyMutex(data->wait_mutex);
    qDebug() << "read_thread finished.";

}

QImage avframe_to_qimage(const AVFrame *frame)
{
    QImage image;
    int width = frame->width;
    int height = frame->height;
    image = QImage(width, height, QImage::Format_RGB888);

    // Initialize YUV parameters
    const uint8_t *y = frame->data[0];
    const uint8_t *u = frame->data[1];
    const uint8_t *v = frame->data[2];
    int y_stride = frame->linesize[0];
    int u_stride = frame->linesize[1];
    int v_stride = frame->linesize[2];

    // Convert YUV to RGB
    for (int y_pos = 0; y_pos < height; ++y_pos) {
        for (int x_pos = 0; x_pos < width; ++x_pos) {
            int y_val = y[y_pos * y_stride + x_pos];
            int u_val = u[(y_pos/2) * u_stride + (x_pos/2)];
            int v_val = v[(y_pos/2) * v_stride + (x_pos/2)];

            int r = y_val + 1.13983*(v_val-128);
            int g = y_val - 0.39465*(u_val-128) - 0.5806*(v_val-128);
            int b = y_val + 2.03211*(u_val-128);

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            QRgb *pixel = reinterpret_cast<QRgb*>(image.scanLine(y_pos) + x_pos * 3);
            *pixel = qRgb(r, g, b);
        }
    }

    return image;
}


int Lx_Mediaplayer::video_thread(void *arg)
{
    int ret;
    double pts;
    AVPacket* packet;
    Lx_Mediaplayer *data = (Lx_Mediaplayer *)arg;
    AVFrame *pFrame  = av_frame_alloc();

    while (true) {
        if (data->stop) {
            break;
        }

        if (data->pause) {
            SDL_Delay(10);
            continue;
        }

        if (data->video_pkts.length() <= 0) {
            /* while video file read finished exit decode thread,
             * otherwise just delay for data input
             */
            if (data->isReadFinished) {
                break;
            }
            SDL_Delay(1);
            continue;
        }

        packet=data->video_pkts.dequeue();
        if(!packet) continue;

        if(data->decodeVideo(*packet,*pFrame)<0) continue;



        if ((pts = pFrame->pts) == AV_NOPTS_VALUE) {
            pts = 0;
        }

        pts *= av_q2d(data->FormatContext->streams[data->video_stream]->time_base);
        pts =  data->synchronize(pFrame, pts);

        if (data->audio_stream >= 0) {
            while (1) {
                if (data->stop) {
                    break;
                }

                double audioClk = data->getAudioClock();
                emit data->video_timeChange(audioClk);
                pts = data->videoClk;

                if (pts <= audioClk) {
                     break;
                }
                int delayTime = (pts - audioClk) * 1000;

                delayTime = delayTime > 5 ? 5 : delayTime;

                SDL_Delay(delayTime);
            }

            emit data->video_display(avframe_to_qimage(pFrame));
        }


        av_frame_unref(pFrame);
        av_packet_unref(packet);
      if(packet)av_packet_free(&packet);
    }

    av_frame_free(&pFrame);

    if (!data->stop) {
        data->stop = true;
    }

    qDebug() << "video_thread finished.";
    data->closeAudio();
    data->clearData();
    avcodec_close(data->audio_avctx);
    avcodec_close(data->video_avctx);
    avcodec_free_context(&data->audio_avctx);
    avcodec_free_context(&data->video_avctx);
    avformat_close_input(&data->FormatContext);
    avformat_free_context(data->FormatContext);
    SDL_Delay(100);

//   data->isDecodeFinished = true;

//    if (decoder->gotStop) {
//        decoder->setPlayState(Decoder::STOP);
//    } else {
//        decoder->setPlayState(Decoder::FINISH);
//    }

    return 0;
}



