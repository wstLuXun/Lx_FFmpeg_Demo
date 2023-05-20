#include "CWidget.h"

#include <QApplication>
#include <QFile>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CWidget w;
    w.show();
    w.resize(800,460);

    return a.exec();
}
