#ifndef MESSAGEHEADER_H
#define MESSAGEHEADER_H

enum MessageType
{
   Base=1,
   Register,
   RegisterSuccessful,
   RegisterUnSuccessful,
   Login,
   LoginSuccessful,
   LoginUnSuccessful,
   ClientMessage,
   ClientFile

};

#include <QString>
#include <QDataStream>

struct MessageHeader
{
   int  type;
   int  size;


   friend QDataStream &operator<<(QDataStream& input, MessageHeader& iteam)
   {
       input <<iteam.type<<iteam.size;
       return input;
   }

   friend QDataStream &operator>>(QDataStream& output,MessageHeader& iteam)
   {
       output >>iteam.type>>iteam.size;
       return output;
   }
   //序列化
   QByteArray Serialize()
   {
       QByteArray temp,Array;
       QDataStream tempStream(&temp,QIODevice::WriteOnly);
       tempStream<<*this;
       this->size=temp.length();
       QDataStream Stream(&Array,QIODevice::WriteOnly);
       Stream<<*this;
       return Array;
   }
   //反序列化
   void  Deserialize(QByteArray& Array)
   {
       QDataStream Stream(&Array,QIODevice::ReadOnly);
       Stream>>*this;
   }

};


struct ResultMsg :public MessageHeader
{
    ResultMsg()
    {
      type=LoginUnSuccessful;
      size=sizeof(ResultMsg);
    }
    QString msg;

    //序列化
    friend QDataStream &operator<<(QDataStream& input,const ResultMsg &iteam)
    {
       input<< iteam.type<<iteam.size<<iteam.msg;
       return input;
    }
    //反序列化
    friend QDataStream &operator>>(QDataStream& output,ResultMsg& iteam)
    {
        output >>iteam.type>>iteam.size>>iteam.msg;
        return output;
    }
     QByteArray Serialize()
     {
         QByteArray temp,Array;
         QDataStream tempStream(&temp,QIODevice::WriteOnly);
         tempStream<<*this;
         this->size=temp.length();
         QDataStream Stream(&Array,QIODevice::WriteOnly);
         Stream<<*this;
         return Array;
     }

     void  Deserialize(QByteArray& Array)
     {
         QDataStream Stream(&Array,QIODevice::ReadOnly);
         Stream>>*this;
     }


};

struct RegLoginterMsg :public MessageHeader
{
    RegLoginterMsg()
    {
      type=Login;
      size=sizeof(RegLoginterMsg);
    }
    QString nID;
    QString password;


    friend QDataStream &operator<<(QDataStream& input,const RegLoginterMsg &iteam)
    {
       input<< iteam.type<<iteam.size<<iteam.nID<<iteam.password;
       return input;
    }

    friend QDataStream &operator>>(QDataStream& output,RegLoginterMsg& iteam)
    {
        output >>iteam.type>>iteam.size>>iteam.nID>>iteam.password;
        return output;
    }
   //序列化
     QByteArray Serialize()
     {
         QByteArray temp,Array;
         QDataStream tempStream(&temp,QIODevice::WriteOnly);
         tempStream<<*this;
         this->size=temp.length();
         QDataStream Stream(&Array,QIODevice::WriteOnly);
         Stream<<*this;
         return Array;
     }
   //反序列化
     void  Deserialize(QByteArray& Array)
     {
         QDataStream Stream(&Array,QIODevice::ReadOnly);
         Stream>>*this;
     }

};


struct MessageMsg:public MessageHeader
{
    MessageMsg()
    {
      type=ClientMessage;
      size=sizeof(MessageMsg);
    }
    QString peernID;
    QString msg;

    friend QDataStream &operator<<(QDataStream& input,const MessageMsg &iteam)
    {
       input<< iteam.type<<iteam.size<<iteam.peernID<<iteam.msg;
       return input;
    }
    //反序列化
    friend QDataStream &operator>>(QDataStream& output,MessageMsg& iteam)
    {
        output >>iteam.type>>iteam.size>>iteam.peernID>>iteam.msg;
        return output;
    }

     QByteArray Serialize()
     {
         QByteArray temp,Array;
         QDataStream tempStream(&temp,QIODevice::WriteOnly);
         tempStream<<*this;
         this->size=temp.length();
         QDataStream Stream(&Array,QIODevice::WriteOnly);
         Stream<<*this;
         return Array;
     }

     void  Deserialize(QByteArray& Array)
     {
         QDataStream Stream(&Array,QIODevice::ReadOnly);
         Stream>>*this;
     }


};

struct FileMsg:public MessageHeader
{
    FileMsg()
    {
      type=ClientFile;
      size=sizeof(FileMsg);
    }

    QString peernID;
    QString FileName;
    qint64  FileSize;
    qint64 sendSize;
    QByteArray rowcontent;//文件内容

    friend QDataStream &operator<<(QDataStream& input,const FileMsg &iteam)
    {
       input<< iteam.type<<iteam.size<<iteam.peernID<<iteam.FileName<<iteam.FileSize<<iteam.sendSize<<iteam.rowcontent;
       return input;
    }

    //反序列化
    friend QDataStream &operator>>(QDataStream& output,FileMsg& iteam)
    {
        output >>iteam.type>>iteam.size>>iteam.peernID>>iteam.FileName>>iteam.FileSize>>iteam.sendSize>>iteam.rowcontent;
        return output;
    }

    QByteArray Serialize()
    {
        QByteArray temp,Array;
        QDataStream tempStream(&temp,QIODevice::WriteOnly);
        tempStream<<*this;
        this->size=temp.length();
        QDataStream Stream(&Array,QIODevice::WriteOnly);
        Stream<<*this;
        return Array;
    }

    void  Deserialize(QByteArray& Array)
    {
        QDataStream Stream(&Array,QIODevice::ReadOnly);
        Stream>>*this;
    }
};


#define BUFF_SIZE 20481
//  循环队列
struct loopQueue{

    char Buffer[BUFF_SIZE];
    int front;
    int rear;
    loopQueue(){
        Clear();
    }

    // 将Q清为空队列
    void Clear() {
       front =rear = 0;
    }
    int Length() {
        return (rear - front + BUFF_SIZE)%BUFF_SIZE;
    }
    // 若队列Q为空队列，则返回1；否则返回-1
    bool isEmpty() {
        if (front == rear) // 队列空的标志
            return true;
        else
            return false;
    }
    bool isFull()
    {
        if (front == (rear+1)%BUFF_SIZE) // 队列空的标志
            return true;
        else
            return false;
    }

    void enqueue(char c)
    {
        if(isFull()) return;

        Buffer[rear]=c;
        rear=(rear + 1) % BUFF_SIZE;

    }

    char dequeue(){
         if(isEmpty()) return -1;
         char c=Buffer[front];
         front = (front + 1) % BUFF_SIZE;
         return c;

    }

    char at(int i){

        if(i<Length()){
           return Buffer[(front + i) % BUFF_SIZE];
        }else return -1;
    }

};









#endif // MESSAGEHEADER_H
