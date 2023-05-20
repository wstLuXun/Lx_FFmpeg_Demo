#include "DataBaseManager.h"
#include "ui_DataBaseManager.h"

#include <CIocpServer.h>
#include <QSqlQuery>

#include <QDebug>

DataBaseManager::DataBaseManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataBaseManage)
{
    ui->setupUi(this);
    //添加右键菜单
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    initmuen();
    connect(ui->tableView,&QTableView::customContextMenuRequested,this,&DataBaseManager::MenuRequested);

    connect(CIocpServer::getCIocpServer(),&CIocpServer::ClientRegterEx,[&](){model->select();});


    model=new QSqlTableModel(ui->tableView,CIocpServer::getCIocpServer()->db);
    model->setTable("userinfo");
    model->select();
    ui->tableView->setModel(model);



    ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView::section{"
                                                     "text-align:center;"
                                                     "padding:3px;"
                                                     "margin:0px;"
                                                     "color:#DCDCDC;"
                                                     "border:1px solid #242424;"
                                                     "border-left-width:0px;"
                                                     "border-right-width:1px;"
                                                     "border-top-width:0px;"
                                                     "border-bottom-width:1px;"
                                                     "background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 #646464,stop:1 #525252);"
                                                 "}"


                                                  );

//    QSqlQuery Query(CIocpServer::getCIocpServer()->db);
//   qDebug()<< Query.exec("drop table userinfo");
//   qDebug()<< Query.exec("create table userinfo( UserName text(32),Password text(32),allowLogin text(12),state text(20),regTime text(50),blacklist text,token text(100),message blob)");

}

DataBaseManager::~DataBaseManager()
{
    delete ui;
}

void DataBaseManager::initmuen()
{
    QAction *Action;

//    Action=new QAction("增行");
//    muen.addAction(Action);


//    Action=new QAction("修改");
//    muen.addAction(Action);

//    Action=new QAction("删行");
//    muen.addAction(Action);

}

void DataBaseManager::resizeEvent(QResizeEvent *event)
{
   int Header=ui->tableView->horizontalHeader()->width();

    ui->tableView->setColumnWidth(0,Header*0.15);
    ui->tableView->setColumnWidth(1,Header*0.15);
    ui->tableView->setColumnWidth(2,Header*0.12);
    ui->tableView->setColumnWidth(3,Header*0.1);
    ui->tableView->setColumnWidth(4,Header*0.14);
    ui->tableView->setColumnWidth(5,Header*0.1);
    ui->tableView->setColumnWidth(6,Header*0.1);
    ui->tableView->setColumnWidth(7,Header*0.14);
}

void DataBaseManager::MenuRequested(QPoint pos)
{
   muen.exec(mapToGlobal(pos));
}
