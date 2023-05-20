#include "widget.h"
#include "ui_widget.h"

#include <windows.h>
#include "QDebug"
#include <QFileDialog>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),Instance(0),Player(0),Media(0),ismove(false)
{
    ui->setupUi(this);
    //初始化vlc实例对象
    Instance = libvlc_new(0, 0);
    if(!Instance) exit(1);
    //创建列表
    list_player=libvlc_media_list_player_new(Instance);
    //创建播放器
    Player=libvlc_media_player_new(Instance);

    connect(&timer,&QTimer::timeout,[=](){
        //Player是否正在播放
        if(Player&&libvlc_media_player_is_playing(Player)){
          //获取视频总时长
          libvlc_time_t time=  libvlc_media_player_get_length(Player)/1000;
          //获取视频当前已播放时长
          libvlc_time_t currtime= libvlc_media_player_get_time(Player)/1000;

            ui->time_label->setText(QString("%1/%2").arg(currtime).arg(time));
            ui->time_slider->setMaximum(time);
            if(!ismove)
               ui->time_slider->setValue(currtime);
        }
    });




}

Widget::~Widget()
{
    //释放vlc实例
    libvlc_release(Instance);
    libvlc_media_list_player_release(list_player);
    delete ui;
}


void Widget::on_btn_open_clicked()
{
    QString path=QFileDialog::getOpenFileName();
    if(path.isEmpty()) return;
    // 把路径中的正斜杠/转化成反斜杠\\形式
    path=QDir::toNativeSeparators(path);

    qDebug()<<"++++++++++++++++++++++++++++++++++++++path"<<path<<"---------------------------------------------";

    QByteArray array=path.toLatin1();
    //Player是否正在播放
    if(Player&&libvlc_media_player_is_playing(Player)){
//        libvlc_media_player_pause(Player);
//        libvlc_media_player_stop_async(Player);
        //释放Media
        //libvlc_media_release(Media);
       // libvlc_media_player_release(Player);
    }

    if(timer.isActive()){
        timer.stop();
    }

 //64位
#ifdef vlc_64
   //文件路径获得媒体Media
    Media = libvlc_media_new_path(array.data());
    if(Media) this->setWindowTitle(path);
    //方式一创建获得播放器
    //媒体创建获得播放器
    //Player = libvlc_media_player_new_from_media (Instance,Media);


    //方式二创建获得播放器
    //Player=libvlc_media_player_new(Instance);
    //将媒体和播放器关联
    libvlc_media_player_set_media(Player,Media);

    //释放媒体对象 关联之后就可释放媒体对象
    libvlc_media_release(Media);
Media=0;
#endif

#ifdef vlc_32
    //32位
    //vlcMedia = libvlc_media_new_path(vlcInstance, array.data());
    //32位
    //vlcPlayer = libvlc_media_player_new_from_media (vlcMedia);
#endif



    if(Player){
      //libvlc_video_set_callbacks 回调获取每一帧
      //不能拿多个播放器对象关联同一个窗口上,血坑(播放器对象播放完之后删除  重新new播放器对象 再次关联这个窗口对象 此操作会导致播放失败)
        //按照可以播放 播放器对象和窗口对象一一对应思路,可播放多路视频
      libvlc_media_player_set_hwnd(Player, (HWND)ui->video_widget->winId());
      libvlc_media_player_play (Player);
      //事件管理
      pEvent_manager=libvlc_media_player_event_manager(Player);
      //注册事件
      //libvlc_event_attach(pEvent_manager,libvlc_MediaPlayerMediaChanged,0,0);
      //取消注册
      //libvlc_event_detach(pEvent_manager,libvlc_MediaPlayerMediaChanged,0,0);
      //启动定时器
      timer.start(500);
    }





}


void Widget::on_time_slider_sliderReleased()
{
    int time=ui->time_slider->value()*1000;
    if(Player){
       if(!libvlc_media_player_set_time(Player,time,false))
          qDebug()<<"seek成功";
    }
    ismove=false;
}


void Widget::on_time_slider_sliderPressed()
{
    ismove=true;
}



void Widget::on_audio_slider_sliderReleased()
{
    int volume=ui->audio_slider->value();
    if(Player){
       libvlc_audio_set_volume(Player,volume);
       ui->audio_label->setText(QString("音量：%1").arg(volume));
    }
}





void Widget::on_rate_comboBox_currentIndexChanged(int index)
{
    double rate=ui->rate_comboBox->currentText().toFloat();
    if(Player){
       libvlc_media_player_set_rate(Player,rate);
    }

}


void Widget::on_btn_pause_clicked()
{

    if(Player&&libvlc_media_player_is_playing(Player))
    {
       libvlc_media_player_pause(Player);
       ui->btn_pause->setText("resume");
    }else{
       libvlc_media_player_set_pause(Player,0);
       ui->btn_pause->setText("pause");
    }

}






