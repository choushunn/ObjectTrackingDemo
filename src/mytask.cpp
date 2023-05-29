#include "mytask.h"

MyTask::MyTask(QObject *parent)
    : QObject{parent}
{
//    nc = new CNcnn();
}


void MyTask::run() {
    qDebug() << "Task running in thread:" << QThread::currentThreadId();
}
