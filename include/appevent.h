#ifndef APPEVENT_H
#define APPEVENT_H

#include <QObject>
#include <qdebug.h>
#include "utils.h"

class AppEvent : public QObject
{
    Q_OBJECT
public:
    explicit AppEvent(QObject *parent = nullptr);

public slots:
    void processFrame(cv::Mat frame);

signals:
    void sendProcessFrame(QImage image);
};

#endif // APPEVENT_H
