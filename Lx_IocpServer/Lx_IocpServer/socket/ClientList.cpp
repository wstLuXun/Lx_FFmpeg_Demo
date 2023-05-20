#include "ClientList.h"
#include "ui_ClientList.h"

#include <ItemDelegate.h>
ClientList::ClientList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CManageList)
{
    ui->setupUi(this);


    initFrom();

    qRegisterMetaType<PPER_SOCKET_CONTEXT>("PPER_SOCKET_CONTEXT");
    qRegisterMetaType<QVector<int>>("QVector<int>");

    CIocpServer::getCIocpServer()->run("127.0.0.1",13456);

    connect(CIocpServer::getCIocpServer(),&CIocpServer::NewConnectEx,this,&ClientList::AddItem);
    connect(CIocpServer::getCIocpServer(),&CIocpServer::ColseSocketEx,this,&ClientList::DeleteItem);
    connect(CIocpServer::getCIocpServer(),&CIocpServer::ClientLoginEx,[&](PPER_SOCKET_CONTEXT NEW_SOCKET,QString  msg){
       QStandardItem*temp=(QStandardItem*) NEW_SOCKET->pIOContext->SocketAccept;
       temp->setData(msg,Qt::UserRole +2);
    });
    connect(model,&QStandardItemModel::rowsInserted,this,&ClientList::UpdateOnlineNumber);
    connect(model,&QStandardItemModel::rowsRemoved,this,&ClientList::UpdateOnlineNumber);




    connect((ItemDelegate*)ui->listView->itemDelegate(),&ItemDelegate::buttonClick,this,&ClientList::BtnClick);


}

ClientList::~ClientList()
{
    delete ui;
}

void ClientList::setFirstItemText(QString text)
{
    firstItem->setData(text,Qt::UserRole + 2);
}

void ClientList::initFrom()
{
    model=new QStandardItemModel(ui->listView);
    firstItem=new QStandardItem;
    firstItem->setData(0);
    firstItem->setData(QString("当前在线人数：%1"),Qt::UserRole + 2);
    ui->listView->setItemDelegate(new ItemDelegate);
    ui->listView->setModel(model);
    model->appendRow(firstItem);

}
#include <QDebug>
void ClientList::AddItem(PPER_SOCKET_CONTEXT demo)
{

    QStandardItem *item=new QStandardItem;
    item->setData(QVariant::fromValue(demo));
    demo->pIOContext->SocketAccept=(int)item;
    model->appendRow(item);

}

void ClientList::DeleteItem(PPER_SOCKET_CONTEXT demo)
{

    QStandardItem *item=(QStandardItem *)demo->pIOContext->SocketAccept;
    model->removeRow(item->row());

}

void ClientList::UpdateOnlineNumber(const QModelIndex &parent, int first, int last)
{
    firstItem->setData(model->rowCount()-1);
}

void ClientList::BtnClick(const QModelIndex &index, int BtnType)
{
      switch(BtnType)
      {
          case 0:  qDebug()<<"0";break;
          case 1:  qDebug()<<"1";break;
          case 2:  qDebug()<<"2";break;
          case 3:  qDebug()<<"3";break;
          case 4:  qDebug()<<"4";break;
          case 5: model->removeRow(index.row());break;
      }
}
