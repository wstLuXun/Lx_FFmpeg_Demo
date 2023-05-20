#ifndef CTHREAD_H
#define CTHREAD_H


#include <QThread>
#include <QMutex>
#include <QWaitCondition>
//#include <QObject>
#include <functional> //function头文件

class CThread : public QObject
{
    Q_OBJECT
public:
    //这种方式创建线程不能指定父对象
    explicit CThread();
    ~CThread();

public slots:
    //线程启动函数
    void start(std::function<void(void)>fun);

Q_SIGNALS:
    //线程执行完成信号
    void runFinish();

public slots:
   //线程函数1
    void SendFileRun();

protected:
   //线程对象
    QThread* thread;
   //多线程互斥锁
   //static QMutex mutex;
   //条件变量
   // static QWaitCondition cond;
};

#endif // CTHREAD_H
