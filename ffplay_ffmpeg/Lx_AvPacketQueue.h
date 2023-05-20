#ifndef LX_AVPACKETQUEUE_H
#define LX_AVPACKETQUEUE_H

#include <QQueue>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#include "SDLinclude/SDL.h"

class Lx_AvPacketQueue
{
public:
    explicit Lx_AvPacketQueue();

    void enqueue(AVPacket* packet);

    AVPacket* dequeue();

    bool isEmpty();

    void empty();
    void clear();

    int queueSize();
    int length();

private:
    SDL_mutex *mutex;
    SDL_cond * cond;


    QQueue<AVPacket*> queue;
};

#endif // LX_AVPACKETQUEUE_H
