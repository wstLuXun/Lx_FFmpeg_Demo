#include "MainFrom.h"
#include "ui_MainFrom.h"
#include <Widget.h>
#include "MessageHeader.h"

MainFrom::MainFrom(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainFrom)
{
    ui->setupUi(this);
    this->setWindowTitle("Login");
    this->setWindowFlags(windowFlags()|Qt::FramelessWindowHint);
    socket=new QTcpSocket;
    socket->connectToHost("127.0.0.1",13456);
    QObject::connect(socket,&QTcpSocket::connected,[&](){
         ui->label_3->setText("成功连接！！");
       });

 static  QMetaObject::Connection con= connect(socket,&QTcpSocket::readyRead,[&](){
        ResultMsg msg;
        QByteArray arr=socket->readAll();
         msg.Deserialize(arr);

        if(msg.type==RegisterSuccessful) qDebug()<<"注册成功！！ wst"<<msg.msg;
         if(msg.type==RegisterUnSuccessful) qDebug()<<"注册失败！！ wst"<<msg.msg;

        if(msg.type==LoginSuccessful)
        {
            qDebug()<<"登录成功！！ wst"<<msg.msg;

            Widget *w=new Widget;
            w->setTcpSocket(socket);
            w->setWindowTitle(QString("nID= %1").arg(ui->nID->text()));
            w->show();
            disconnect(con);
            this->close();
        }
        if(msg.type==LoginUnSuccessful)  qDebug()<<"登录失败！！ wst"<<msg.msg;




        ui->label_3->setText(msg.msg);
    });



}

MainFrom::~MainFrom()
{


    delete ui;
}

void MainFrom::mousePressEvent(QMouseEvent *event)
{
    if( (event->button() == Qt::LeftButton) ){
             mouse_press = true;
             mousePoint = event->globalPos() - this->pos();
    }else if(event->button() == Qt::RightButton){
           //如果是右键
         this->close();

    }
}

void MainFrom::mouseMoveEvent(QMouseEvent *event)
{
    if(mouse_press){
           move(event->globalPos() - mousePoint);
            event->accept();
    }
}

void MainFrom::mouseReleaseEvent(QMouseEvent *event)
{
    mouse_press = false;
}


 QByteArray ArrayLogin;
void MainFrom::on_pushButton_clicked()
{
    RegLoginterMsg msg;
    msg.type=Login;
    QString nID= ui->nID->text();
    QString password=ui->password->text();
   if(nID.isEmpty()||password.isEmpty()) return ;

    msg.nID=nID;
    msg.password=password;
    QByteArray Array=msg.Serialize();


    ArrayLogin=Array;
    qDebug()<<Array<<"长度="<<Array.length();

    socket->write(Array);

}


void MainFrom::on_pushButton_2_clicked()
{
      RegLoginterMsg msg;
      msg.type=Register;
      QString nID= ui->nID->text();
      QString password=ui->password->text();
     if(nID.isEmpty()||password.isEmpty()) return ;

     msg.nID=nID;
     msg.password=password;
     QByteArray Array=msg.Serialize();

      socket->write(Array);



}

