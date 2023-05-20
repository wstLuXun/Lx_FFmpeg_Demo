#include "ItemDelegate.h"

#include <QPushButton>
#include <QPainter>
#include <QEvent>
#include <QProgressBar>
#include <QApplication>
#include <QMouseEvent>

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index)const
{
    Q_UNUSED(parent);
    Q_UNUSED(option);
    Q_UNUSED(index);
    return nullptr;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(),35);
}


void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   //int id=index.data(Qt::UserRole + 1).toInt();

    if(index.row()==0){

        painter->save();
        QPen pen;
        pen.setWidth(5);
        pen.setBrush(Qt::red);
        painter->setPen(pen);
        painter->setFont(QFont("宋体", 11));
        QString str=index.data(Qt::UserRole + 2).toString().arg(index.data(Qt::UserRole + 1).toInt());

        painter->drawText(option.rect,Qt::AlignVCenter|Qt::AlignHCenter,str);
        painter->restore();
    }else{

    //抗锯齿
    painter->setRenderHints(QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing|QPainter::Antialiasing);
   //画按钮1
    QRect icoRect=QRect(option.rect.x(),option.rect.y(),32,option.rect.height());
    painter->drawImage(icoRect,QImage(":/ico/socket/avatar2.png"));


   //画文字
   painter->save();
   QRect Textrect=QRect(icoRect.x()+icoRect.width(),option.rect.y(),option.rect.width()-7*32,option.rect.height());
  // painter->fillRect(Textrect,Qt::blue);
   QPen pen;
   pen.setWidth(5);
   pen.setBrush(Qt::blue);
   painter->setPen(pen);
   painter->setFont(QFont("宋体", 11));
   QString str=QString("第%1个item节点:nID= %2").arg(index.row()).arg(index.data(Qt::UserRole +2).toString()) ;
   painter->drawText(Textrect,Qt::AlignVCenter|Qt::AlignHCenter,str);
   painter->restore();






   //画按钮2
   //painter->fillRect(loveRect,Qt::green);
   painter->save();
   QPainterPath path;
   QRect loveRect=QRect(Textrect.x()+Textrect.width(),option.rect.y(),32,option.rect.height());
   path.addEllipse(loveRect.center(),9,9);
   painter->setBrush(Qt::cyan);
   painter->setPen(QPen( Qt::SolidLine));
   path.setFillRule(Qt::WindingFill);
   painter->drawPath(path);
   painter->restore();

   //绘制文字
   painter->save();
   QRect Textrect1=QRect(loveRect.x()+loveRect.width(),option.rect.y(),32*2,option.rect.height());
  // painter->fillRect(Textrect,Qt::blue);
   pen.setWidth(5);
   pen.setBrush(Qt::blue);
   painter->setPen(pen);
   painter->setFont(QFont("宋体", 11));
   str="在线";
   painter->drawText(Textrect1,Qt::AlignLeft|Qt::AlignVCenter,str);
   painter->restore();


   QRect messageRect=QRect(Textrect1.x()+Textrect1.width(),option.rect.y(),32,option.rect.height());
   painter->drawImage(messageRect,QImage(":/ico/socket/message.png"));

  //画按钮3
   QRect colseRect=QRect(messageRect.x()+messageRect.width()+32,option.rect.y(),32,option.rect.height());
  // painter->fillRect(colseRect,Qt::lightGray);
   painter->drawImage(colseRect,QImage(":/ico/socket/lahei.png"));


    }

    return QStyledItemDelegate::paint(painter,option,index);

}


bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(index.row()==0) return QStyledItemDelegate::editorEvent(event,model,option,index);

   if(event->type()==QEvent::MouseButtonPress)
   {
        QMouseEvent* MouseEvent=static_cast<QMouseEvent*>(event);
        QPointF itemPoint=MouseEvent->localPos();

        QRectF Rect[6];
        Rect[0]=QRectF(option.rect.x(),option.rect.y(),32,option.rect.height());
        Rect[1]=QRectF(Rect[0].x()+Rect[0].width(),option.rect.y(),option.rect.width()-7*32,option.rect.height());
        Rect[2]=QRectF(Rect[1].x()+Rect[1].width(),option.rect.y(),32,option.rect.height());
        Rect[3]=QRectF(Rect[2].x()+Rect[2].width(),option.rect.y(),32*2,option.rect.height());
        Rect[4]=QRectF(Rect[3].x()+Rect[3].width(),option.rect.y(),32,option.rect.height());
        Rect[5]=QRectF(Rect[4].x()+Rect[4].width()+32,option.rect.y(),32,option.rect.height());





        if(Rect[0].contains(itemPoint)) emit buttonClick(index,0);
        if(Rect[1].contains(itemPoint)) emit buttonClick(index,1);
        if(Rect[2].contains(itemPoint)) emit buttonClick(index,2);
        if(Rect[3].contains(itemPoint)) emit buttonClick(index,3);
        if(Rect[4].contains(itemPoint)) emit buttonClick(index,4);
        if(Rect[5].contains(itemPoint)) emit buttonClick(index,5);

   }

   return QStyledItemDelegate::editorEvent(event,model,option,index);
}
