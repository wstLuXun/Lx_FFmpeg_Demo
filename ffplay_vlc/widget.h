#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE
#include <vlc/vlc.h>
#include <QTimer>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_btn_open_clicked();

    void on_time_slider_sliderReleased();

    void on_time_slider_sliderPressed();


    void on_audio_slider_sliderReleased();



    void on_rate_comboBox_currentIndexChanged(int index);

    void on_btn_pause_clicked();

#ifdef vlc_64
protected:
void resizeEvent(QResizeEvent *event);
#endif

private:
    Ui::Widget *ui;
    //当前vlc实例
    libvlc_instance_t *Instance;
    //播放器对象
    libvlc_media_player_t *Player;
    //媒体对象
    libvlc_media_t*  Media;
    //事件管理对象
    libvlc_event_manager_t* pEvent_manager;
    //播放器列表
    libvlc_media_list_player_t* list_player;
    //媒体列表
    libvlc_media_list_t* media_list=0;


    QTimer timer;
    bool ismove;
};
#endif // WIDGET_H
