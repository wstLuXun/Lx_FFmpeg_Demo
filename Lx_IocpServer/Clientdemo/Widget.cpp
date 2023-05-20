#include "Widget.h"
#include "ui_Widget.h"

#include "MessageHeader.h"
#include <winsock2.h>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);





        sdSocket= INVALID_SOCKET;

        memset(&this->maddr,0,sizeof(this->maddr));
        maddr.sin_family=AF_INET;
        maddr.sin_port=htons(14567);
        maddr.sin_addr.s_addr = inet_addr("127.0.0.1");



//    ui->pushButton_2->hide();
//    QTcpSocket *socket;
//    for(int i=0;i<=5000;i++)
//    {
//    socket=new QTcpSocket;
//    socket->connectToHost("127.0.0.1",13456);
//    QObject::connect(socket,&QTcpSocket::connected,[&](){
//        socket->write("1111999999");
//    });
//    QObject::connect(socket,&QTcpSocket::readyRead,[&](){
//       QByteArray arr=  socket->readAll();
//        ui->textEdit_2->append(arr);
//              socket->write(arr);
//    });
//    }



}

Widget::~Widget()
{

    if(s->isValid())
    {
        s->disconnectFromHost();
        s->close();
    }
    delete s;
    delete ui;
}

#include <QApplication>
#include <QFile>

void Widget::setTcpSocket(QTcpSocket *tcp)
{
    s=tcp;

    connect(s,&QTcpSocket::readyRead,[&](){

       QByteArray bytes =s->readAll();
      if(bytes.isEmpty()) return;

      MessageHeader Header;
      Header.Deserialize(bytes);

      if(Header.type==ClientMessage){

       MessageMsg  msg;
       msg.Deserialize(bytes);
       qDebug()<<"收到="<<msg.peernID<<"+"<<msg.msg;
        ui->textEdit_2->append(msg.peernID+":"+msg.msg);
      }

      else if(Header.type==ClientFile){

//          FileMsg  msg;
//          msg.Deserialize(bytes);
//         static qint64 recvsize=0;

//      static  QFile *file=new QFile(qApp->applicationDirPath()+"/"+ msg.FileName);

//      //delete file

//          if(msg.first)
//          {
//              bool ok = file->open(QIODevice::ReadWrite);

//              file->write(msg.rowcontent);
//              recvsize+=msg.rowcontent.length();

//             // if(recvsize>=msg.FileSize) {file->close(); recvsize=0;file->deleteLater();}

//          }else
//          {
//              //bool ok = file.open();
//              file->write(msg.rowcontent);
//              recvsize+=msg.rowcontent.length();


//          }


//          if(recvsize>=msg.FileSize) {file->close(); recvsize=0;file->deleteLater();}



      }
    });

}


#include <QFileInfo>
TRANSMIT_FILE_BUFFERS Buffer;
FileMsg Hmsg1,Hmsg2;
void Widget::on_pushButton_clicked()
{


    QString msgtext=ui->textEdit->toPlainText();
    qDebug()<<"msgtext"<<msgtext;

    QString nID=ui->lineEdit->text();
    static qint64 snedsize=0;
   //路径从左往右复制


   if(msgtext.isEmpty()||nID.isEmpty()) return;
    file.setFileName(msgtext);

    if(file.exists())
      {
       //方式1 传统io 发文件 效率低下
       //sendFile(nID,msgtext);

       // TransferFile 微软扩展接口发文件 效率最高
        sendTransferFile(nID,msgtext);

       //  文件内存映射 效率次之
       // sendMapFile(nID,msgtext);
 }else
     {
        MessageMsg msg;
        msg.peernID=nID;
        msg.msg=msgtext;
        QByteArray Array=msg.Serialize();
        //qDebug()<<"Array"<<Array<<"长度="<<Array.length();
        s->write(Array);
     }
}

extern QByteArray ArrayLogin;

void Widget::on_pushButton_2_clicked()
{
     static  bool connect=true;

     if(connect)
     {
         s->disconnectFromHost();
         s->close();
         ui->pushButton_2->setText("连接服务器");
     }else{
         s->connectToHost("127.0.0.1",13456);
         s->write(ArrayLogin);
         ui->pushButton_2->setText("断开连接");
     }


    connect=!connect;

}


#include <QFileDialog>

void Widget::on_pushButton_3_clicked()
{
   QString path= QFileDialog::getOpenFileName(0,"打开文件","C:\\Users\\Administrator\\Desktop");
   if(!path.isEmpty()) ui->textEdit->setText(path);
}




// 客户端和客户端之间传文件 共三种方法

// IO读写文件 循环读取文件file.read  效率低下
//
// TransmitFile扩展函数 不过得新建socket

void Widget::sendFile(QString nID,QString path)
{

     file.setFileName(path);

     if(file.exists())
     {

       static FileMsg msg;
       QFileInfo info(file);

//CreateFileMappingW创建文件映射
     qDebug()<<"打开文件"<< file.open(QIODevice::ReadOnly);

       msg.FileName=info.fileName();
       msg.FileSize=info.size();  //单位 字节
       msg.peernID=nID;
       msg.rowcontent="传输文件";
       msg.sendSize=0;

      qDebug()<<"文件信息="<<info.fileName()<<info.size();

      //文件名和大小不为空
     if( msg.rowcontent=="传输文件"){

         Timer=new QTimer;
         connect(Timer,&QTimer::timeout,[&](){

                int len=0;
               static int count=1;

                if(msg.sendSize>=msg.FileSize)
                {

                    if(file.isOpen())  {
                        file.close();
                      qDebug()<<"文件已关闭";
                      count=0;
                    }
                    file.setFileName("");
                    Timer->stop();
                    Timer->deleteLater();

                   return;
                }

               //读取文件内容
               msg.rowcontent=file.read(8*1024);
               //读取到的文件内容不为空
               if(!msg.rowcontent.isEmpty())  msg.sendSize+=msg.rowcontent.length();

               QByteArray Array= msg.Serialize();

               len= s->write(Array);
               qDebug()<<count<<"dwIoSize="<<len<<" 已发送大小="<<msg.sendSize<<" FileSize="<<"msg->size="<<msg.size;
               count++;

         });

        msg.sendSize=0;
        int len=0;
        Timer=new QTimer;
        QByteArray Array= msg.Serialize();

        //发送文件名和文件大小
        len= s->write(Array);

       //发送成功之后开始发送文件内容
       if(len) Timer->start(10);

     }

  }
}


void Widget::sendMapFile(QString nID,QString path)
{

    static FileMsg* msg=new FileMsg;
    QFileInfo info(path);

    msg->FileName=info.fileName();
    msg->FileSize=info.size();  //单位 字节
    msg->peernID=nID;
    msg->rowcontent="传输文件";
    msg->sendSize=0;

   std::wstring str= path.toStdWString();

    //创建文件映射
    hFile= CreateFileW(str.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    hFileMap =CreateFileMappingW(hFile,NULL,PAGE_READONLY,0,0,L"wstFileSocket");


      if(hFile==INVALID_HANDLE_VALUE) qDebug()<<"错误值="<<GetLastError();
     if(hFileMap==0) qDebug()<<"错误值="<<GetLastError();

      pFile=(char*) MapViewOfFile(hFileMap,FILE_MAP_READ ,0,0,0);

    //文件名和大小不为空
   if( msg->rowcontent=="传输文件"){
       Timer=new QTimer;
      msg->sendSize=0;
      int len=0;
      QByteArray Array= msg->Serialize();
      //socket->
      //发送文件名和文件大小
      len= s->write(Array);

      connect(Timer,&QTimer::timeout,[&](){

             int len=0;
            static int count=1;


       if(msg->sendSize>=msg->FileSize)
         {
                 Timer->stop();
                 count=0;
               qDebug()<<"文件发送完成 关闭文件";
                 UnmapViewOfFile(pFile);
                  if(hFile!=INVALID_HANDLE_VALUE) CloseHandle(hFile);
                if(hFileMap!=0)  CloseHandle(hFileMap);
                 Timer->deleteLater();
                 return;
         }


            //读取文件内容
        if(msg->FileSize-msg->sendSize<8*1024) msg->rowcontent=QByteArray((pFile+msg->sendSize),msg->FileSize-msg->sendSize);
              else
              msg->rowcontent=QByteArray((pFile+msg->sendSize),8*1024);
            //读取到的文件内容不为空
            if(!msg->rowcontent.isEmpty())  msg->sendSize+=msg->rowcontent.length();

            QByteArray Array= msg->Serialize();

            len= s->write(Array);

            qDebug()<<count<<"dwIoSize="<<len<<" 已发送大小="<<msg->sendSize<<" FileSize="<<"msg->size="<<msg->size;

            count++;

      });
     //发送成功之后开始发送文件内容
     if(len) Timer->start(10);

  }

}

void Widget::sendTransferFile(QString nID, QString path)
{

    QFileInfo info(path);

    path.toStdString();
    std::wstring ww=path.toStdWString();


    Transfer=CreateFileW(ww.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL |FILE_FLAG_SEQUENTIAL_SCAN,0);
   qDebug()<<"CreateFileW 错误 WSAGetLastError()="<<GetLastError();

      static bool _init=false;
     if(!_init){

         //创建socket
          sdSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);;

         // sdSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, 0);



         DWORD bytes = 0;
         GUID TransferFile_guid = WSAID_TRANSMITFILE;
         int nRet=-1;
         nRet = WSAIoctl(
            sdSocket,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
           &TransferFile_guid,
            sizeof(TransferFile_guid),
           &pTransferFile,
           sizeof(pTransferFile),
           &bytes,
            NULL,
            NULL
            );




        //WSAConnect(sdSocket,(sockaddr*)&maddr,len,0,0,0,0);

         qDebug()<<"nRet="<<nRet;
        _init=true;
     }



     //连接socket
     int len=sizeof(maddr);
     int err=::connect(sdSocket,(sockaddr*)&maddr,len);
     qDebug()<<"err="<<err<<"WSAGetLastError()="<<WSAGetLastError();

          qDebug()<<"WSAGetLastError()="<<WSAGetLastError();
         if(Transfer==INVALID_HANDLE_VALUE)  qDebug()<<"文件句柄"<<GetLastError();


          Hmsg1.FileName=info.fileName();
          Hmsg1.FileSize=info.size();
          Hmsg1.rowcontent=QByteArray("TransmitFile文件传输");
          Buffer.Head=Hmsg1.Serialize().data();
          Buffer.HeadLength=Hmsg1.Serialize().length();




          qDebug()<<"发送大小"<<"头部="<<Buffer.HeadLength<<"尾部="<<Buffer.HeadLength;
          BOOL SEND;
          SEND= pTransferFile(sdSocket,Transfer,0,0,NULL,&Buffer,TF_USE_DEFAULT_WORKER);
          qDebug()<<"55"<< WSAGetLastError()<<"SEND="<<SEND;

          CloseHandle(Transfer);


}

