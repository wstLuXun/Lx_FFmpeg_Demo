#ifndef CWIDGET_H
#define CWIDGET_H

#include <FramelessWidget.h>

namespace Ui {
class CWidget;
}

class CWidget : public FramelessWidget
{
    Q_OBJECT

public:
    explicit CWidget(QWidget *parent = nullptr);
    ~CWidget();


private slots:
    void on_btn_min_clicked();

    void on_btn_max_clicked();

    void on_btn_close_clicked();

    void buttonClick();


    void on_toolButton_clicked();

    void on_toolButton_3_clicked();

protected:
   void initForm();


private:
    Ui::CWidget *ui;


};



#endif // CWIDGET_H
