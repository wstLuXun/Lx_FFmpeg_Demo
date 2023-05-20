#ifndef MAINFROM_H
#define MAINFROM_H

#include <QWidget>

namespace Ui {
class MainFrom;
}
#include <QTcpSocket>
#include<QHostAddress>
#include<QMouseEvent>

class MainFrom : public QWidget
{
    Q_OBJECT

public:
    explicit MainFrom(QWidget *parent = nullptr);
    ~MainFrom();
    QTcpSocket *socket;


protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainFrom *ui;
    QPoint mousePoint;
    bool mouse_press;
};

#endif // MAINFROM_H
