#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QImage>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE
#include <Lx_Mediaplayer.h>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
public slots:
    void updateImage(QImage image);
protected:
    QImage image;
    void paintEvent(QPaintEvent *event);


private slots:
    void on_pause_clicked();

    void on_play_clicked();


private:
    bool ismove;
    Lx_Mediaplayer* play;
    Ui::Widget *ui;
};
#endif // WIDGET_H
