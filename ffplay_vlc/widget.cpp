#include "widget.h"
#include "ui_widget.h"

#include <windows.h>
#include <vlc/vlc.h>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);


        libvlc_instance_t *vlcInstance;
        libvlc_media_player_t *vlcPlayer= NULL;
        libvlc_media_t*  vlcMedia;


        vlcInstance = libvlc_new(0, 0);
        if (vlcInstance == NULL) exit(1);


            if (vlcPlayer && libvlc_media_player_is_playing(vlcPlayer))
            {
                //stop
                 //libvlc_media_player_stop(vlcPlayer);

                /* release the media player */
                libvlc_media_player_release(vlcPlayer);

                /* Reset application values */
                vlcPlayer = NULL;

            }

#ifdef vlc_64
            //64位
            vlcMedia = libvlc_media_new_path("..\\..\\1.mkv");
            /* Create a new libvlc player */
            //64位
            vlcPlayer = libvlc_media_player_new_from_media (vlcInstance,vlcMedia);
#endif

#ifdef vlc_32
            //32位
            //vlcMedia = libvlc_media_new_path(vlcInstance, "..\\..\\1.mkv");
            //32位
            //vlcPlayer = libvlc_media_player_new_from_media (vlcMedia);
#endif

            if (!vlcMedia) return ;

            libvlc_media_player_set_hwnd(vlcPlayer, (HWND)winId());

            /* Release the media */
            libvlc_media_release(vlcMedia);

            /* And start playback */
            libvlc_media_player_play (vlcPlayer);






}

Widget::~Widget()
{
    delete ui;
}

