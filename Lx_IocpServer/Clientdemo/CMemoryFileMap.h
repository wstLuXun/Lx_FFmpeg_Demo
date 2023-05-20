#ifndef CMEMORYFILEMAP_H
#define CMEMORYFILEMAP_H

#include <QObject>
#include <windows.h>



class CMemoryFileMap : public QObject
{
    Q_OBJECT
public:

enum OpenMode{
     ReadOnly=1,
     WriteOnly,
     ReadWrite
};

    explicit CMemoryFileMap(QObject *parent = nullptr);
     ~CMemoryFileMap();
    //文件路径 打开方式
    bool Open(char* path,OpenMode mode);
    void close();
private:
    HANDLE hFile;
    HANDLE hFileMap;
    char * pFile;

};

#endif // CMEMORYFILEMAP_H
