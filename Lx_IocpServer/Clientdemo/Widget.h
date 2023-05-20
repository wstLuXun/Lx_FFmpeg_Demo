#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QFile>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE


#include <QTcpSocket>

#include "mswsock.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void setTcpSocket(QTcpSocket* tcp);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();

    void sendFile(QString nID,QString path);
    void sendMapFile(QString nID,QString path);
    void sendTransferFile(QString nID,QString path);

private:
    Ui::Widget *ui;
    QTcpSocket*s;
    QTimer *Timer; //定时器
    QFile file;

    HANDLE hFile;
    HANDLE hFileMap;
    char* pFile;


    LPFN_TRANSMITFILE   pTransferFile ;
    SOCKET sdSocket;
    struct sockaddr_in maddr;
    HANDLE Transfer;


};
#endif // WIDGET_H

















