#include "Widget.h"
#include "ui_Widget.h"
#include <QPainter>
#include <QDebug>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

   play=0;
   ismove=false;

   if(play) delete play;
   play=new Lx_Mediaplayer;
   connect(play,&Lx_Mediaplayer::video_display,this,&Widget::updateImage);

   connect(play,&Lx_Mediaplayer::video_time,[&](qint64 time){
       ui->sp->setMinimum(0);
       ui->sp->setMaximum(time); qDebug()<<"Lx_Mediaplayer::video_time="<<time;
   });

   connect(play,&Lx_Mediaplayer::video_timeChange,[&](double time){
       if(!ismove) ui->sp->setValue(time);
       QString timestr=QString::number((int)time)+"/"+QString::number(ui->sp->maximum());
       ui->label->setText(timestr);
   });

   connect(ui->sp,&QSlider::sliderPressed,[&](){
           ismove=true;
   });

   connect(ui->sp,&QSlider::sliderReleased,[&](){
       play->setSeekProgress(ui->sp->value());
       ismove=false;
   });

   //音量
   ui->volume->setMinimum(0);
   ui->volume->setMaximum(128);
   QString str=QString("音量(%1 / 128)").arg(play->Volume());
   ui->volumebtn->setText(str);
   connect(ui->volume,&QSlider::sliderReleased,[&](){
      play->setVolume(ui->volume->value());
      QString str=QString("音量(%1 / 128)").arg(ui->volume->value());
      ui->volumebtn->setText(str);
   });


}

Widget::~Widget()
{
    delete ui;
}

void Widget::updateImage(QImage image)
{
   this->image=image;
    update();
}

void Widget::paintEvent(QPaintEvent *event)
{
  QPainter p(this);
  if(!image.isNull())
  p.fillRect(QRect(0,0,width(),height()-55),image.scaled(width(),height()-55,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
}







void Widget::on_pause_clicked()
{
   static bool ispause=true;
   if(ispause)
      play->setPause(ispause);
    else
      play->setPause(ispause);

   ispause=!ispause;

}


void Widget::on_play_clicked()
{
    static bool isplay=true;
    if(isplay)
      play->play("..\\..\\music.mkv");
    else
      play->setStop(true);

     isplay=!isplay;
}







