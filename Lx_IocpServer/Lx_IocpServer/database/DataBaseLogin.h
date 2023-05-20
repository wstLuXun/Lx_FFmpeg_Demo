#ifndef DATABASELOGIN_H
#define DATABASELOGIN_H

#include <QWidget>

namespace Ui {
class DataBaseLogin;
}

class DataBaseLogin : public QWidget
{
    Q_OBJECT

public:
    explicit DataBaseLogin(QWidget *parent = nullptr);
    ~DataBaseLogin();

private:
    Ui::DataBaseLogin *ui;
};

#endif // DATABASELOGIN_H
