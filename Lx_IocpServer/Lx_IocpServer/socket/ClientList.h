#ifndef CMANAGELIST_H
#define CMANAGELIST_H

#include <QWidget>

namespace Ui {
class CManageList;
}
#include <QStandardItemModel>
#include <CIocpServer.h>

Q_DECLARE_METATYPE(PPER_SOCKET_CONTEXT)

class ClientList : public QWidget
{
    Q_OBJECT

public:
    explicit ClientList(QWidget *parent = nullptr);
    ~ClientList();
    void setFirstItemText(QString text);

private:
    Ui::CManageList *ui;
    QStandardItemModel* model;
    QStandardItem* firstItem;

    void initFrom();
public slots:
    void AddItem(PPER_SOCKET_CONTEXT demo);
    void DeleteItem(PPER_SOCKET_CONTEXT demo);
    void UpdateOnlineNumber(const QModelIndex &parent, int first, int last);
    void BtnClick(const QModelIndex &index,int BtnType);
};

#endif // CMANAGELIST_H
