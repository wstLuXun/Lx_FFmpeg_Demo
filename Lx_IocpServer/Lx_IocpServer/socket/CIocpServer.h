#ifndef CIOCPSERVER_H
#define CIOCPSERVER_H

#include <QObject>
#include <winsock2.h>
#include "mswsock.h"
#include <QSqlDatabase>
enum IO_OPERATION{
     ClientIoAccept=1,
     ClientIoRead,
     ClientIoWrite,};

#define MAX_BUFF_SIZE 18*1024


//单IO操作数据
 typedef struct _PER_IO_CONTEXT{
      OVERLAPPED overlapped;

      char      Buffer[MAX_BUFF_SIZE];
      WSABUF    wsabuf;
      int       nTotalBytes;
      int       nSentBytes;  
      IO_OPERATION   operatorType;            //操作类型,可以为ACCEPT/SEND/RECV三种
      SOCKET    SocketAccept;            //监听socket ACCEPT才使用此成员 非监听socket 则存储QListView item的值
      //int item;
 }PER_IO_CONTEXT,*PPER_IO_CONTEXT;

//单句柄数据定义
#include <QDebug>
 typedef struct _PER_SOCKET_CONTEXT
 {
      SOCKET           socket;
      PPER_IO_CONTEXT  pIOContext;
      ~_PER_SOCKET_CONTEXT()
      {
          qDebug()<<"释放"<<"PPER_SOCKET_CONTEXT";
      }

 }PER_SOCKET_CONTEXT,*PPER_SOCKET_CONTEXT;


/**
 *
 *  LIBS+=-lws2_32
 *  io多路复用模型(iocp、epoll)
 *  基于windows iocp
 *
 *  微软官方例子： https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/netds/winsock/iocp/serverex
 *  CIocpServer 单例类
 */
#include <QMap>
#include <QQueue>
#include <QMutex>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>


class CIocpServer : public QObject
{
    Q_OBJECT
public:
     static CIocpServer* getCIocpServer();
     void run(char* ipaddress,int port);
     void stoprun();
     QSqlDatabase db;

private:
    explicit CIocpServer(QObject *parent = 0);
    SOCKET CreateSocket();
    void CreateListenSocket(char* ipaddress,int port);
    PPER_SOCKET_CONTEXT UpdateCompletionPort(SOCKET s,IO_OPERATION ClientIo);
    PPER_SOCKET_CONTEXT CtxtAllocate(SOCKET s, IO_OPERATION ClientIO);//移除QuitList队列头

    BOOL PostAcceptSocket();
    VOID CloseClient(PPER_SOCKET_CONTEXT lpPerSocketContext,BOOL bGraceful);//加入QuitList队列尾  从而实现客户端socket关闭连接时 不释放其所对应的PER_SOCKET_CONTEXT实例对象
    static  DWORD  WINAPI icopServerThread (LPVOID lpParam);                //当下一个客户端连接时直接从QuitList取 PER_SOCKET_CONTEXT实例对象再初始化 避免了反复xmalloc xfree的后果



private:
    void InitFileSocket();
    QTcpServer FileServer;
    QTcpSocket *socket;



private:
    bool g_bEndServer=false;
    QMutex mutex;
    SOCKET ServerSocket;
    HANDLE hIocp;
    LPFN_ACCEPTEX   mpAcceptEx;
    PPER_SOCKET_CONTEXT ClientIoAccept_SOCKET_CONTEXT;
    QQueue<PPER_SOCKET_CONTEXT>QuitList;

    QMap<QString,PPER_SOCKET_CONTEXT>ClientMap;






signals:
    void NewConnectEx(PPER_SOCKET_CONTEXT s);
    void ColseSocketEx(PPER_SOCKET_CONTEXT s);
    void ClientLoginEx(PPER_SOCKET_CONTEXT s,QString  msg);
    void ClientRegterEx();

};











#endif // CIOCPSERVER_H
