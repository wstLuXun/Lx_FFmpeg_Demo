#include "CThread.h"
#include <QDebug>

CThread::CThread()
{
     thread =new  QThread;
     connect(this,&CThread::runFinish,thread,&QThread::quit);
     connect(thread,&QThread::finished,thread,&QThread::deleteLater);
     connect(thread,&QThread::finished,this,&CThread::deleteLater);
     this->moveToThread(thread);


     qDebug()<< "线程创建"<<QThread::currentThreadId();

}

CThread::~CThread()
{

    qDebug()<< "线程销毁"<<QThread::currentThreadId();
}

void CThread::start(std::function<void(void)>fun)
{
    connect(thread,&QThread::started,this,fun);
    thread->start();
}

void CThread::SendFileRun()
{

   emit runFinish();
}


