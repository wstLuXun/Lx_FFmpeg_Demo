#ifndef LX_SCREENCAP_H
#define LX_SCREENCAP_H

#include <QThread>
#include <QImage>
using namespace std;

struct AVCodecParameters;
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVFrame;
struct AVPacket;
struct AVOutputFormat;
struct SwsContext;
struct AVCodec;

class Lx_Screencap : public QObject
{
    Q_OBJECT
public:
    explicit Lx_Screencap(QObject *parent = nullptr);
     ~Lx_Screencap();
signals:
   void sendImage(QImage& image);

public slots:
   void start(QString name);
   void read_thread();
   void stream_open(QString path);
   int stream_component_open(int stream_index);


protected:
    QThread thread;
    AVFormatContext*FormatContext;
    int video_stream;
    AVCodecContext* video_avctx;
    AVCodec* video_codec;
    SwsContext *imgConvertContext;
    QImage image;
    int lineSize[4];
    AVCodecParameters *codecParameters;


};

#endif // LX_SCREENCAP_H
