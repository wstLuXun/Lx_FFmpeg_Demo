#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavdevice/avdevice.h"
    #include "libavformat/avio.h"
    #include "libavutil/imgutils.h"
}


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    QImage image;
        int lineSize[4];
        AVFormatContext *formatContext;
        AVCodecParameters *codecParameters;
        int videoIndex;
        AVCodecContext* codecContext;
        AVPacket *packet;
        AVFrame *frame;
        SwsContext *imgConvertContext;


    void paintEvent(QPaintEvent *event);
};
#endif // WIDGET_H
