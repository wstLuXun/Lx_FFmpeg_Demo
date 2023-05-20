#include "CIocpServer.h"
#include<windows.h>

#define xmalloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#define xfree(p)   HeapFree(GetProcessHeap(),0,(p))

#include<QMetaType>
#define logText QString("文件名：%1  行号：%2 函数名：%3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)
#include <MessageHeader.hpp>

#include <QSqlQuery>
#include <QApplication>

#include <QHostAddress>
#include <QDateTime>

CIocpServer *CIocpServer::getCIocpServer()
{
    static  CIocpServer Server;
    return  &Server;
}

void CIocpServer::run(char *ipaddress, int port)
{

    static bool inRun=false;
    if(inRun)
    {
        inRun=!inRun;
        return;
    }

    CreateListenSocket(ipaddress,port);


}

void CIocpServer::stoprun()
{
    mutex.lock();
    g_bEndServer=true;
    mutex.unlock();
}

CIocpServer::CIocpServer(QObject *parent) : QObject(parent)
{
   g_bEndServer=false;
   this->ClientMap.clear();
   this->QuitList.clear();

   qRegisterMetaType<PPER_SOCKET_CONTEXT>("PPER_SOCKET_CONTEXT");
   db=QSqlDatabase::addDatabase("QSQLITE");
   db.setDatabaseName("./../Lx_IocpServer/user.db");//qApp->applicationDirPath()+
   db.open();


   InitFileSocket();
}

void CIocpServer::CreateListenSocket(char* ipaddress, int port)
{



    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    //创建socket
    ServerSocket=WSASocket(AF_INET,SOCK_STREAM,0,0,0,WSA_FLAG_OVERLAPPED);


    //ServerSocket=CreateSocket();

    //绑定ip info
    SOCKADDR_IN si;
    si.sin_family = AF_INET;
    si.sin_port = ntohs(port);
    si.sin_addr.S_un.S_addr = inet_addr(ipaddress);
    bind(ServerSocket,(sockaddr*)&si, sizeof(si));

    //监听socket
    listen(ServerSocket,SOMAXCONN);//SOMAXCONN


    //线程数目=系统进程数目的两倍 创建iocp完成端口并投递一个连接事件
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);


    hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    ClientIoAccept_SOCKET_CONTEXT=UpdateCompletionPort(ServerSocket, ClientIoAccept);

    DWORD   dwThreadId;

    for (ulong i = 0;i < SystemInfo.dwNumberOfProcessors * 2; i++)
        {

            HANDLE hThreadId = CreateThread(NULL, 0, icopServerThread,hIocp, 0, &dwThreadId);

            if (hThreadId)
            {
                //CloseHandle(hThreadId);
            }
        }

    PostAcceptSocket();


}

BOOL CIocpServer::PostAcceptSocket()
{
    int nRet =-1;
    DWORD dwRecvNumBytes = 0;
    DWORD bytes = 0;

    //
    // GUID to Microsoft specific extensions
    //
    static BOOL fUpdateIOCP=TRUE;
    if( fUpdateIOCP ) {



         GUID acceptex_guid = WSAID_ACCEPTEX;

         nRet = WSAIoctl(
            ServerSocket,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
           &acceptex_guid,
            sizeof(acceptex_guid),
           &mpAcceptEx,
           sizeof(mpAcceptEx),
           &bytes,
            NULL,
            NULL
            );




         fUpdateIOCP=FALSE;

        if (nRet!=0)  return (FALSE);

    }

    ClientIoAccept_SOCKET_CONTEXT->pIOContext->SocketAccept = CreateSocket();
    if( ClientIoAccept_SOCKET_CONTEXT->pIOContext->SocketAccept == INVALID_SOCKET)
    {
       return(FALSE);
    }



    nRet=-1;
    nRet = mpAcceptEx(ServerSocket, ClientIoAccept_SOCKET_CONTEXT->pIOContext->SocketAccept,
                    (LPVOID)(ClientIoAccept_SOCKET_CONTEXT->pIOContext->Buffer),
                    0,// MAX_BUFF_SIZE - (2 * (sizeof(SOCKADDR_STORAGE) + 16)),//SOCKADDR_STORAGE//这里一定要填0 血的教训
                    sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16,
                    &dwRecvNumBytes,
                    (LPOVERLAPPED)&ClientIoAccept_SOCKET_CONTEXT->pIOContext->overlapped);


    if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) )
    {
        return(FALSE);
    }
        return(TRUE);

}
#include <QDebug>
void CIocpServer::CloseClient(PPER_SOCKET_CONTEXT lpPerSocketContext,BOOL bGraceful)
{


      mutex.lock();

    if( lpPerSocketContext) {

        if(WSAGetLastError()==WSAENOTSOCK) { mutex.unlock();return;}
        if( !bGraceful ){

            //
            // force the subsequent closesocket to be abortative.
            //
            LINGER  lingerStruct;

            lingerStruct.l_onoff = 1;
            lingerStruct.l_linger = 0;
            setsockopt(lpPerSocketContext->socket, SOL_SOCKET, SO_LINGER,
                       (char *)&lingerStruct, sizeof(lingerStruct) );
        }

        if(WSAGetLastError()==WSAENOTSOCK) { mutex.unlock();return;}

        if(!QuitList.contains(lpPerSocketContext))
        {
            QuitList.enqueue(lpPerSocketContext);

           // qDebug()<<"add QuitList.count()="<<QuitList.count()<<"错误信息："<<WSAGetLastError();
        }
        if(ClientMap.keys(lpPerSocketContext).count())
        {
             ClientMap.remove(ClientMap.keys(lpPerSocketContext).at(0));
             qDebug()<<"remove ClientMap.count()="<<ClientMap.count();
        }

        if(WSAGetLastError()==WSAENOTSOCK) { mutex.unlock();return;}

        //当socket为监听socket才执行此代码 非监听socket存储qlistview item的地址
        if( lpPerSocketContext->pIOContext->SocketAccept==ServerSocket&&lpPerSocketContext==ClientIoAccept_SOCKET_CONTEXT) {
            closesocket(lpPerSocketContext->pIOContext->SocketAccept);
            lpPerSocketContext->pIOContext->SocketAccept = INVALID_SOCKET;
        }
        if(lpPerSocketContext->socket!=INVALID_SOCKET&&lpPerSocketContext!=ClientIoAccept_SOCKET_CONTEXT){
           closesocket(lpPerSocketContext->socket);
           lpPerSocketContext->socket = INVALID_SOCKET;
         }
         emit ColseSocketEx(lpPerSocketContext);

 //     qDebug()<<"当前错误WSAGetLastError()1="<<WSAGetLastError()<<"当前线程id="<<GetCurrentThreadId()<<"当前行数:"<<__LINE__;



    }
    mutex.unlock();

}



PPER_SOCKET_CONTEXT CIocpServer::UpdateCompletionPort(SOCKET s, IO_OPERATION ClientIo)
{
    PPER_SOCKET_CONTEXT lpPerSocketContext;

   lpPerSocketContext=CtxtAllocate(s,ClientIo);

    HANDLE newhIocp=CreateIoCompletionPort((HANDLE)s,hIocp,(ULONG_PTR)lpPerSocketContext,0);
//WSAIoctl()
     qDebug()<<__FUNCTION__<<__FILE__;
    if(newhIocp!=hIocp)
    {
        if( lpPerSocketContext->pIOContext )
            xfree(lpPerSocketContext->pIOContext);
        xfree(lpPerSocketContext);
        return (NULL);
    }



    return lpPerSocketContext;
}

PPER_SOCKET_CONTEXT CIocpServer::CtxtAllocate(SOCKET s, IO_OPERATION ClientIO)
{
    PPER_SOCKET_CONTEXT lpPerSocketContext;
   //加锁
    mutex.lock();

    if(QuitList.count()!=0){

        lpPerSocketContext= QuitList.dequeue();
        qDebug()<<"delete QuitList.count()="<<QuitList.count();

        lpPerSocketContext->socket=s;
        lpPerSocketContext->pIOContext->nTotalBytes = 0;
        lpPerSocketContext->pIOContext->nSentBytes  = 0;
        lpPerSocketContext->pIOContext->operatorType=ClientIO;
        lpPerSocketContext->pIOContext->wsabuf.buf=lpPerSocketContext->pIOContext->Buffer;
        lpPerSocketContext->pIOContext->wsabuf.len=sizeof(lpPerSocketContext->pIOContext->Buffer);
        //死在这一步
        memset(&lpPerSocketContext->pIOContext->overlapped,0,sizeof(OVERLAPPED));
        //注意
        lpPerSocketContext->pIOContext->SocketAccept=INVALID_SOCKET;

      }else{

    lpPerSocketContext = (PPER_SOCKET_CONTEXT)xmalloc(sizeof(PER_SOCKET_CONTEXT));
    if( lpPerSocketContext) {
        lpPerSocketContext->pIOContext = (PPER_IO_CONTEXT)xmalloc(sizeof(PER_IO_CONTEXT));
        if( lpPerSocketContext->pIOContext ) {
            lpPerSocketContext->socket=s;
            lpPerSocketContext->pIOContext->nTotalBytes = 0;
            lpPerSocketContext->pIOContext->nSentBytes  = 0;
            lpPerSocketContext->pIOContext->operatorType=ClientIO;
            lpPerSocketContext->pIOContext->wsabuf.buf=lpPerSocketContext->pIOContext->Buffer;
            lpPerSocketContext->pIOContext->wsabuf.len=sizeof(lpPerSocketContext->pIOContext->Buffer);
            //死在这一步
            memset(&lpPerSocketContext->pIOContext->overlapped,0,sizeof(OVERLAPPED));
            //注意
            lpPerSocketContext->pIOContext->SocketAccept=INVALID_SOCKET;
        } else {
            xfree(lpPerSocketContext);
        }

    } else {

        return(NULL);
    }

    }
    //解锁
    mutex.unlock();
    return(lpPerSocketContext);
}

SOCKET CIocpServer::CreateSocket()
{
    int nRet =SOCKET_ERROR,rRet=SOCKET_ERROR;
    int nZero = 20*1024;
    SOCKET sdSocket = INVALID_SOCKET;

    sdSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if( sdSocket == INVALID_SOCKET){
        return(sdSocket);
    }


//设置发送缓冲区大小为0   SO_SNDBUF
    nZero = 0;
    nRet = setsockopt(sdSocket, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));
    rRet = setsockopt(sdSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nZero, sizeof(nZero));
    if( nRet == SOCKET_ERROR||rRet == SOCKET_ERROR) {
        return(sdSocket);
    }
    unsigned long ul = 1;
    if (SOCKET_ERROR == ioctlsocket(sdSocket, FIONBIO, &ul)) {
       perror("FAILED TO SET NONBLOCKING SOCKET");
       closesocket(sdSocket);

    }

    return(sdSocket);
}





QFile*  file;
SOCKET sss;


DWORD CIocpServer::icopServerThread(LPVOID lpParam)
{

    HANDLE hIOCP = (HANDLE)lpParam;
    int nRet = -1;
    DWORD dwIoSize = 0;
    LPWSAOVERLAPPED lpOverlapped = NULL;
    PPER_SOCKET_CONTEXT lpPerSocketContext = NULL;
    PPER_SOCKET_CONTEXT lpAcceptSocketContext = NULL;
    PPER_IO_CONTEXT lpIOContext = NULL;
    BOOL bSuccess = FALSE;

    WSABUF buffRecv;
    WSABUF buffSend;
    DWORD dwRecvNumBytes = 0;
    DWORD dwSendNumBytes = 0;

    DWORD dwFlags = 0;


    while (true)
    {

        bSuccess = GetQueuedCompletionStatus(hIOCP,
                                            &dwIoSize,
                                            (PDWORD_PTR)&lpPerSocketContext,
                                            (LPOVERLAPPED*)&lpOverlapped,
                                            INFINITE
                                            );


        if(!bSuccess)
            qDebug()<<QString("GetQueuedCompletionStatus() failed: %1\n").arg(WSAGetLastError());

        if( lpPerSocketContext == NULL) continue;

        if(getCIocpServer()->g_bEndServer)  return NULL;

        lpIOContext = (PPER_IO_CONTEXT)lpOverlapped;

        if(lpIOContext->operatorType != ClientIoAccept &&WSAGetLastError()!=64) {
            if(!bSuccess||(bSuccess && (0 == dwIoSize)) ) {
                //客户端连接丢失
                //qDebug()<<"客户端连接丢失"<<bSuccess<<dwIoSize<<"行数"<<__LINE__;
                //qDebug()<<"CloseClient 所在行数"<<__LINE__<<WSAGetLastError();
                getCIocpServer()->CloseClient(lpPerSocketContext,FALSE);
                qDebug()<<"CloseClient 所在行数"<<__LINE__<<WSAGetLastError()<<"当前线程id"<<GetCurrentThreadId()
                        <<"bSuccess="<<bSuccess<<"dwIoSize="<<dwIoSize;
                continue;
            }
        }




        switch (lpIOContext->operatorType) {
          //注意:在ClientIoAccept这里ClientIoAccept_SOCKET_CONTEXT和lpPerSocketContext相同(只读不做修改)
        case ClientIoAccept:

         nRet = setsockopt(
                     lpPerSocketContext->pIOContext->SocketAccept,
                     SOL_SOCKET,
                     SO_UPDATE_ACCEPT_CONTEXT,
                     (char *)&getCIocpServer()->ServerSocket,
                     sizeof(getCIocpServer()->ServerSocket)
                     );

        lpAcceptSocketContext=nullptr;
        lpAcceptSocketContext=getCIocpServer()->UpdateCompletionPort(lpPerSocketContext->pIOContext->SocketAccept,ClientIoAccept);
        if(lpAcceptSocketContext){

        emit getCIocpServer()->NewConnectEx(lpAcceptSocketContext);

            //
            // AcceptEx completes but doesn't read any data so we need to post
            // an outstanding overlapped read.
            //
            lpAcceptSocketContext->pIOContext->operatorType = ClientIoRead;
            dwRecvNumBytes = 0;
            dwFlags = 0;
            buffRecv.buf = lpAcceptSocketContext->pIOContext->Buffer,
            buffRecv.len =MAX_BUFF_SIZE;

            nRet = WSARecv(
                          lpAcceptSocketContext->socket,
                          &buffRecv,1,
                          &dwRecvNumBytes,
                          &dwFlags,
                         &lpAcceptSocketContext->pIOContext->overlapped,NULL);


         if( nRet== SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()))
            {
                 getCIocpServer()->CloseClient(lpAcceptSocketContext,FALSE);
                 qDebug()<<"CloseClient 所在行数="<<__LINE__<<"错误值="<<WSAGetLastError();
            }
        }
           //投递一个新连接
           CIocpServer::getCIocpServer()->PostAcceptSocket();



             break;
        case ClientIoRead:

        {
             lpIOContext->operatorType = ClientIoWrite;
             dwFlags = 0;


             MessageHeader msg;
          //开始
            QByteArray temp2(lpIOContext->Buffer,dwIoSize);
            msg.Deserialize(temp2);

            switch(msg.type)
            {
                //注册消息
            case Register:
                 {

                       RegLoginterMsg regter;
                       regter.Deserialize(temp2);



                       QString sql= QString("select 1 from userinfo where UserName=%1 and Password=%2 limit 1 ").arg(regter.nID).arg(regter.password);
                       QSqlQuery query(getCIocpServer()->db);
                       int value=0;
                       bool result =query.exec(sql);
                       //qDebug()<<query.lastError().text()<<"=text";

                       if(result)
                       {
                         query.next();
                        if(query.isValid()) value=query.value(0).toInt();
                       }

                       ResultMsg resultMsg;

                       if(result&&value)
                       {
                           resultMsg.msg=QString("注册失败!!");
                           resultMsg.type=RegisterUnSuccessful;
                       }else{

                           sql=QString("insert into userinfo (UserName,Password,regTime) values ('%1','%2','%3')").arg(regter.nID).arg(regter.password)
                                      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                           query.exec(sql);
                           resultMsg.msg=QString("注册成功!!");
                           resultMsg.type=RegisterSuccessful;
                           emit getCIocpServer()->ClientRegterEx();
                       }


                       lpIOContext->operatorType = ClientIoWrite;
                       dwFlags = 0;

                       QByteArray arr= resultMsg.Serialize();
                       lpIOContext->nTotalBytes = resultMsg.size;
                       lpIOContext->nSentBytes  = 0;
                       lpIOContext->wsabuf.len  = resultMsg.size;


                         memcpy(lpIOContext->Buffer,arr.data(),arr.length());

                    nRet = WSASend(
                              lpPerSocketContext->socket,
                              &lpIOContext->wsabuf, 1,
                              &dwSendNumBytes,
                              dwFlags,
                              &(lpIOContext->overlapped), NULL);

                       break;
                   }
                //登录消息
               case Login:
                   {

                          RegLoginterMsg Login;
                          Login.Deserialize(temp2);
                          ResultMsg resultMsg;

                          QSqlQuery query(getCIocpServer()->db);
                          QString sql=QString("select 1 from userinfo where UserName=%1 and Password=%2 limit 1 ").arg(Login.nID).arg(Login.password);

                          int value=0;
                          bool result= query.exec(sql);

                          if(result)
                          {
                            query.next();
                            if(query.isValid())value=query.value(0).toInt();
                          }

                          if(result&&value)
                          {
                              resultMsg.msg=QString("登录成功!!");
                              resultMsg.type=LoginSuccessful;

                              //登录成功时插入nID映射表中
                              getCIocpServer()->mutex.lock();

                              if(getCIocpServer()->ClientMap.contains(Login.nID))
                                                                  getCIocpServer()->ClientMap[Login.nID]=lpPerSocketContext;
                                                                 else
                                     getCIocpServer()->ClientMap.insert(Login.nID,lpPerSocketContext);

                                   getCIocpServer()->mutex.unlock();

                                qDebug()<<"add getCIocpServer()->ClientMap.count()="<<getCIocpServer()->ClientMap.count()<<"Login.nID="<<Login.nID;

                                emit getCIocpServer()->ClientLoginEx(lpPerSocketContext,Login.nID);

                          }else{
                               resultMsg.msg=QString("登录失败!! 账号或密码错误!!");
                               resultMsg.type=LoginUnSuccessful;
                          }



                                QByteArray arr=resultMsg.Serialize();

                                lpIOContext->operatorType = ClientIoWrite;
                                dwFlags = 0;

                                lpIOContext->nTotalBytes = resultMsg.size;
                                lpIOContext->nSentBytes  = 0;
                                lpIOContext->wsabuf.len  = resultMsg.size;

                                memcpy(lpIOContext->Buffer,arr.data(),arr.length());

                                nRet = WSASend(
                                              lpPerSocketContext->socket,
                                              &lpIOContext->wsabuf, 1,
                                              &dwSendNumBytes,
                                              dwFlags,
                                              &(lpIOContext->overlapped), NULL);


                        break;
                   }
                //客户端与客户端通信
               case ClientFile:
                   {


                   static int count=0;
                  static  FileMsg message;
                 message.Deserialize(temp2);


                   if(message.rowcontent=="传输文件"){
                        count=0;
                       file=new QFile;
                       file->setFileName((qApp->applicationDirPath()+"/"+message.FileName));
                     if(!file->isOpen())
                     {

                         file->open(QIODevice::ReadWrite);
                         qDebug()<<"---------------------------------------------------------------------"<<endl;
                         qDebug()<<"FileName="<<message.FileName<<"FileSize="<<message.FileSize<<message.sendSize;
                         qDebug()<<"---------------------------------------------------------------------"<<endl;
                     }

                   }

                   else{

                     file->write(message.rowcontent);
                       count++;
                     qDebug()<<"count"<<count<<"dwIoSize="<<dwIoSize<<" 已接收大小="<<message.sendSize<<" Filesize="<<message.FileSize;

                   }

                   if(message.sendSize==message.FileSize)
                   {

                       file->close();
                       file->deleteLater();
                       qDebug()<<"文件接收完成 关闭文件";
                   }





//               qDebug()<<"当前错误WSAGetLastError()="<<WSAGetLastError()<<"行号："<<__FILE__;



//                     //不用memcpy
//                     PPER_SOCKET_CONTEXT temp=nullptr;
//                     temp=getCIocpServer()->ClientMap[nID];
//                     if(!temp)  break ;
//                     memcpy(temp->pIOContext->Buffer,mmsg.data(),mmsg.length());

//                      temp->pIOContext->operatorType=ClientIoWrite;

//                      temp->pIOContext->nTotalBytes = message.size;
//                      temp->pIOContext->nSentBytes  = 0;
//                      temp->pIOContext->wsabuf.len  =  message.size;
//                      dwFlags = 0;
//                      nRet=-1;
//                      if(temp->socket!=INVALID_SOCKET)
//                      nRet = WSASend(
//                           temp->socket,
//                           &temp->pIOContext->wsabuf, 1,
//                           &dwSendNumBytes,
//                           dwFlags,
//                           &(temp->pIOContext->overlapped), NULL);
//                      if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {

//                           getCIocpServer()->CloseClient(temp,FALSE);
//                           qDebug()<<"CloseClient 所在行数"<<__LINE__<<  WSAGetLastError();
//                      }


//----------------------------------------------------------------------------------------




                }


                  break;

               case ClientMessage:
                   {
                        MessageMsg message;
                        message.Deserialize(temp2);

                        QString nID=message.peernID;

                      message.peernID=getCIocpServer()->ClientMap.keys(lpPerSocketContext).at(0);

                      if(nID==message.peernID) break;

                      QByteArray mmsg=message.Serialize();


                        //不用memcpy
                        PPER_SOCKET_CONTEXT temp=nullptr;
                        temp=getCIocpServer()->ClientMap[nID];
                        if(!temp)  break ;
                        memcpy(temp->pIOContext->Buffer,mmsg.data(),mmsg.length());

                         temp->pIOContext->operatorType=ClientIoWrite;
                         dwFlags = 0;
                         temp->pIOContext->nTotalBytes = message.size;
                         temp->pIOContext->nSentBytes  = 0;
                         temp->pIOContext->wsabuf.len  =  message.size;

                         nRet=-1;
                         if(temp->socket!=INVALID_SOCKET)
                         nRet = WSASend(
                              temp->socket,
                              &temp->pIOContext->wsabuf, 1,
                              &dwSendNumBytes,
                              dwFlags,
                              &(temp->pIOContext->overlapped), NULL);
                         if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {

                              getCIocpServer()->CloseClient(temp,FALSE);
                              qDebug()<<"CloseClient 所在行数"<<__LINE__;
                         }


//----------------------------------------------------------------------------------------
                         lpIOContext->operatorType = ClientIoRead;
                         dwRecvNumBytes = 0;
                         buffRecv.buf = lpIOContext->Buffer,
                         buffRecv.len = MAX_BUFF_SIZE;
                         dwFlags = 0;
                         nRet=-1;
                        if(lpPerSocketContext->socket!=INVALID_SOCKET)
                         nRet = WSARecv(
                                       lpPerSocketContext->socket,
                                       &buffRecv, 1, &dwRecvNumBytes,
                                       &dwFlags,
                                       &lpIOContext->overlapped, NULL);
                         if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
                              getCIocpServer()->CloseClient(lpPerSocketContext,FALSE);
                              qDebug()<<"CloseClient 所在行数"<<__LINE__;
                         }


                       qDebug()<<"当前错误WSAGetLastError()="<<WSAGetLastError()<<"行号："<<__FILE__;

                       break;
                   }


            }


//            if(nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {
//                getCIocpServer()->CloseClient(lpPerSocketContext,FALSE);
//                qDebug()<<"CloseClient 所在行数"<<__LINE__<<WSAGetLastError();
//            }




            lpIOContext->operatorType = ClientIoRead;
            dwRecvNumBytes = 0;
            buffRecv.buf = lpIOContext->Buffer,
            buffRecv.len = MAX_BUFF_SIZE;
            dwFlags = 0;
            nRet=-1;
           if(lpPerSocketContext->socket!=INVALID_SOCKET)
            nRet = WSARecv(
                          lpPerSocketContext->socket,
                          &buffRecv, 1, &dwRecvNumBytes,
                          &dwFlags,
                          &lpIOContext->overlapped, NULL);


            if(nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())){
                 getCIocpServer()->CloseClient(lpPerSocketContext,FALSE);
                 qDebug()<<"CloseClient 所在行数"<<__LINE__;
            }
        }


            break;
        case ClientIoWrite:

            lpIOContext->operatorType = ClientIoWrite;
            lpIOContext->nSentBytes  += dwIoSize;
            dwFlags = 0;
            if( lpIOContext->nSentBytes < lpIOContext->nTotalBytes ) {

                //
                // the previous write operation didn't send all the data,
                // post another send to complete the operation
                //
                buffSend.buf = lpIOContext->Buffer + lpIOContext->nSentBytes;
                buffSend.len = lpIOContext->nTotalBytes - lpIOContext->nSentBytes;
                nRet = WSASend(
                               lpPerSocketContext->socket,
                               &buffSend, 1, &dwSendNumBytes,
                               dwFlags,
                               &(lpIOContext->overlapped), NULL);
                if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()) ) {

                    getCIocpServer()->CloseClient(lpPerSocketContext,FALSE);
                    qDebug()<<"CloseClient 所在行数"<<__LINE__;
                }
            } else {

                //
                // previous write operation completed for this socket, post another recv
                //
                lpIOContext->operatorType = ClientIoRead;
                dwRecvNumBytes = 0;
                dwFlags = 0;
                buffRecv.buf = lpIOContext->Buffer,
                buffRecv.len = MAX_BUFF_SIZE;
                nRet = WSARecv(
                              lpPerSocketContext->socket,
                              &buffRecv, 1, &dwRecvNumBytes,
                              &dwFlags,
                              &lpIOContext->overlapped, NULL);
                if( nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()))
                {

                    getCIocpServer()->CloseClient(lpPerSocketContext,FALSE);
                    qDebug()<<"CloseClient 所在行数"<<__LINE__;
                }
            }
            break;

        }

    }



}

#include <QApplication>

void CIocpServer::InitFileSocket()
{
     FileServer.listen(QHostAddress("127.0.0.1"),14567);


     this->connect(&FileServer,&QTcpServer::newConnection,[&](){

          socket= FileServer.nextPendingConnection();
         qDebug()<<"客户端成功连接！！";

       this->connect(socket,&QTcpSocket::readyRead,[&](){

                   QByteArray Array=socket->readAll();
                   FileMsg msg;
                   msg.Deserialize(Array);
                   static int FileSize=0;
                   static int RecvSize=0;
                   static QFile file;



                   if(msg.rowcontent=="TransmitFile文件传输"){

                      file.setFileName(qApp->applicationDirPath()+"/"+msg.FileName);
                      if(!file.isOpen()){
                          file.open(QIODevice::ReadWrite);
                          qDebug()<<"打开文件！！"<<"文件名="<<msg.FileName<<"文件大小="<<msg.FileSize;
                          FileSize=msg.FileSize;
                          RecvSize=0;


                          RecvSize+=Array.length()-msg.size;
                          char *p=Array.data();
                         file.write((char*)(p+msg.size),RecvSize);


                         if(RecvSize==FileSize){
                             file.close();
                             qDebug()<<"关闭文件 RecvSize="<<RecvSize<<"文件大小="<<FileSize;
                         }

                      }

                   }

                   else{
                       file.write(Array);
                       RecvSize+=Array.length();

                       if(RecvSize==FileSize){
                           file.close();
                           qDebug()<<"关闭文件 RecvSize="<<RecvSize<<"文件大小="<<FileSize;
                       }

                   }

         });

     });
}







