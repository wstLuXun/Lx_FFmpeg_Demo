#ifndef DATABASEMANAGE_H
#define DATABASEMANAGE_H

#include <QWidget>
#include <QSqlTableModel>
#include <QMenu>

namespace Ui {
class DataBaseManage;
}

class DataBaseManager : public QWidget
{
    Q_OBJECT

public:
    explicit DataBaseManager(QWidget *parent = nullptr);
    ~DataBaseManager();

protected:
    void initmuen();
    void resizeEvent(QResizeEvent *event);

private slots:
    void MenuRequested(QPoint pos);

private:
    Ui::DataBaseManage *ui;
    QSqlTableModel* model;
    QMenu muen;

};

#endif // DATABASEMANAGE_H
