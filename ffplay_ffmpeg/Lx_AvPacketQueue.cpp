#include "Lx_AvPacketQueue.h"

Lx_AvPacketQueue::Lx_AvPacketQueue()
{
    mutex   = SDL_CreateMutex();
    cond    = SDL_CreateCond();

}

void Lx_AvPacketQueue::enqueue(AVPacket* packet)
{
    SDL_LockMutex(mutex);

    queue.enqueue(packet);
    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);
}

AVPacket*  Lx_AvPacketQueue::dequeue()
{
    AVPacket* packet=nullptr;
    SDL_LockMutex(mutex);
    while (queue.isEmpty())  SDL_CondWait(cond, mutex);
    packet = queue.dequeue();
    SDL_UnlockMutex(mutex);
    return packet;
}

void Lx_AvPacketQueue::empty()
{
    //SDL_LockMutex(mutex);
    while (queue.size() > 0) {
        AVPacket* packet = queue.dequeue();
        av_packet_unref(packet);
        av_packet_free(&packet);
    }

    //SDL_UnlockMutex(mutex);
}

void Lx_AvPacketQueue::clear()
{
    //SDL_LockMutex(mutex);
    while (queue.size() > 0) {
        AVPacket* packet = queue.dequeue();
        av_packet_unref(packet);
        av_packet_free(&packet);
    }

    //SDL_UnlockMutex(mutex);
}

bool Lx_AvPacketQueue::isEmpty()
{
    return queue.isEmpty();
}

int Lx_AvPacketQueue::queueSize()
{
    return queue.size();
}

int Lx_AvPacketQueue::length()
{
    return queue.size();
}
