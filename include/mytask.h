#ifndef MYTASK_H
#define MYTASK_H

#include <QObject>
#include <QRunnable>
#include <QThread>
#include <QDebug>
//#include "cncnn.h"

class MyTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit MyTask(QObject *parent = nullptr);
    void run() override;
//    CNcnn* nc = nullptr;
signals:

};

#endif // MYTASK_H
