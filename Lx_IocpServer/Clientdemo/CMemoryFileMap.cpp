#include "CMemoryFileMap.h"

CMemoryFileMap::CMemoryFileMap(QObject *parent)
    : QObject{parent}
{
    hFile=INVALID_HANDLE_VALUE;
    hFileMap=0;
    pFile=nullptr;
}

CMemoryFileMap::~CMemoryFileMap()
{
    close();
}

bool CMemoryFileMap::Open(char *path, OpenMode mode)
{

    if(mode==ReadOnly){
        hFile= CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_READONLY,NULL);
        hFileMap=CreateFileMappingA(hFile,NULL,PAGE_READONLY,0,0,"wstFileSocket");
    }

    else if(mode==WriteOnly){

        hFile= CreateFileA(path,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        hFileMap=CreateFileMappingA(hFile,NULL,PAGE_READONLY,0,0,"wstFileSocket");

    }

    else{
        //GENERIC_READ|GENERIC_WRITE
        hFile= CreateFileA(path,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        hFileMap=CreateFileMappingA(hFile,NULL,PAGE_READWRITE,0,0,"wstFileSocket");

    }


    if(hFile!=INVALID_HANDLE_VALUE&&hFileMap!=0){
        return true;
    }

    else{
       if(hFile!=INVALID_HANDLE_VALUE)  CloseHandle(hFile);
       if(hFileMap!=0)  CloseHandle(hFileMap);
       return false;
    }

}

void CMemoryFileMap::close()
{
    if(hFile!=INVALID_HANDLE_VALUE)  CloseHandle(hFile);
    if(hFileMap!=0)  CloseHandle(hFileMap);
}
