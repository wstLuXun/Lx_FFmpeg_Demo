#include "DataBaseLogin.h"
#include "ui_DataBaseLogin.h"

DataBaseLogin::DataBaseLogin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataBaseLogin)
{
    ui->setupUi(this);
}

DataBaseLogin::~DataBaseLogin()
{
    delete ui;
}
