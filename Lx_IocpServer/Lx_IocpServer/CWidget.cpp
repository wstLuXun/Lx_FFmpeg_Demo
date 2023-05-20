#include "CWidget.h"
#include "ui_CWidget.h"

#ifdef Q_OS_WIN
#include "windows.h"
#include "windowsx.h"
#endif

#include <QMenu>

#include <windows.h>
#pragma execution_character_set("utf-8")
CWidget::CWidget(QWidget *parent) :
    FramelessWidget(parent),
    ui(new Ui::CWidget)
{
    ui->setupUi(this);   
    initForm();



}

CWidget::~CWidget()
{
    delete ui;
}

void CWidget::initForm()
{
    ui->titleBar->setProperty("form", "title");
    ui->widgetTop->setProperty("nav", "top");

    //设置标题栏控件
    ui->labTitle->setText("IocpServer");

    QFont font;
    font.setPixelSize(24);
    ui->labTitle->setFont(font);

    const QPixmap* Appico=ui->lmainIco->pixmap();
    this->setWindowIcon(QIcon(*Appico));
    this->setWindowTitle(ui->labTitle->text());
    this->setTitleBar(ui->titleBar);
    //当前窗口及子控件均不响应鼠标事件 穿透到父窗口
    ui->lmainIco->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->labTitle->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    //关联信号
//    connect(this, SIGNAL(titleDblClick()), this, SLOT(titleDblClick()));
//    connect(this, SIGNAL(windowStateChange(bool)), this, SLOT(windowStateChange(bool)));

    //设置样式表
    QStringList qss;
    qss << "#titleBar{background:#ffffff;}";//bbbbbb
    qss << "#titleBar{border-top-left-radius:0px;border-top-right-radius:0px;}";
    qss << "#widgetMain{border:0px solid #c4e0f7;background:#ffffff;}";//bbbbbb
    qss << "#widgetMain{border-bottom-left-radius:0px;border-bottom-right-radius:0px;}";

    qss << "#btn_close{border-top-right-radius:0px;background:transparent;} #btn_close:hover{background-color: red}";
    qss << "#btn_max{border-top-right-radius:0px;background:transparent;}  #btn_max:hover{background-color: blue;}"; //
    qss << "#btn_min{border-top-right-radius:0px;background:transparent;} #btn_min:hover{background-color: blue;}";
    qss << "#btn_menu{border-top-right-radius:0px;background:transparent;} #btn_menu:hover{background-color: blue;}";


    //单独设置指示器大小
    int addWidth = 20;
    int addHeight = 10;
    int rbtnWidth = 15;
    int ckWidth = 13;
    int scrWidth = 12;
    int borderWidth = 3;


    qss << QString("QComboBox::drop-down,QDateEdit::drop-down,QTimeEdit::drop-down,QDateTimeEdit::drop-down{width:%1px;}").arg(addWidth);
    qss << QString("QComboBox::down-arrow,QDateEdit[calendarPopup=\"true\"]::down-arrow,QTimeEdit[calendarPopup=\"true\"]::down-arrow,"
                   "QDateTimeEdit[calendarPopup=\"true\"]::down-arrow{width:%1px;height:%1px;right:2px;}").arg(addHeight);
    qss << QString("QRadioButton::indicator{width:%1px;height:%1px;}").arg(rbtnWidth);
    qss << QString("QCheckBox::indicator,QGroupBox::indicator,QTreeWidget::indicator,QListWidget::indicator{width:%1px;height:%1px;}").arg(ckWidth);
    qss << QString("QScrollBar:horizontal{min-height:%1px;border-radius:%2px;}QScrollBar::handle:horizontal{border-radius:%2px;}"
                   "QScrollBar:vertical{min-width:%1px;border-radius:%2px;}QScrollBar::handle:vertical{border-radius:%2px;}").arg(scrWidth).arg(scrWidth / 2);
    qss << QString("QWidget#widget_top>QToolButton:pressed,QWidget#widget_top>QToolButton:hover,"
                   "QWidget#widget_top>QToolButton:checked,QWidget#widget_top>QLabel:hover{"
                   "border-width:0px 0px %1px 0px;}").arg(borderWidth);
    qss << QString("QWidget#widgetleft>QPushButton:checked,QWidget#widgetleft>QToolButton:checked,"
                   "QWidget#widgetleft>QPushButton:pressed,QWidget#widgetleft>QToolButton:pressed{"
                   "border-width:0px 0px 0px %1px;}").arg(borderWidth);

    this->setStyleSheet(qss.join(""));


    QSize icoSize(32,32);
    int icoWidth =85;





}



void CWidget::on_btn_min_clicked()
{
    this->showMinimized();
}


void CWidget::on_btn_max_clicked()
{
    if (this->isMaximized()) {
        this->showNormal();
        ui->btn_max->setIcon(QIcon(":/ico/image/maximize1.png"));
    } else {
        this->showMaximized();
        ui->btn_max->setIcon(QIcon(":/ico/image/maximize2.png"));
    }
}


void CWidget::on_btn_close_clicked()
{
    this->close();
}

void CWidget::buttonClick()
{


}




void CWidget::on_toolButton_clicked()
{

     ui->stackedWidgetMain->setCurrentWidget(ui->ClienListWidget);
}

void CWidget::on_toolButton_3_clicked()
{
    ui->stackedWidgetMain->setCurrentWidget(ui->page);
}
