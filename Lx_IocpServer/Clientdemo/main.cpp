#include <MainFrom.h>

#include <QApplication>
#include <QLibraryInfo>
#include <QSettings>


#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainFrom w;
     w.show();

    return a.exec();
}
