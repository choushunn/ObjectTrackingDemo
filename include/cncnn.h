#ifndef CNCNN_H
#define CNCNN_H

#include <QObject>
#include <QtConcurrent>
#include <ncnn/layer.h>
#include <ncnn/net.h>
#include "utils.h"

#define YOLOV5_V60 1 //YOLOv5 v6.0

#define MAX_STRIDE 64



class CNcnn : public QObject,public ncnn::Layer
{
    Q_OBJECT
public:
    explicit CNcnn(QObject *parent = nullptr);
    int detectYolov5(const cv::Mat& bgr, std::vector<Object>& objects);
private:
    void drawObjects(const cv::Mat& bgr, const std::vector<Object>& objects);

private:
    static const char* m_classNames[];
    ncnn::Net* yolov5;

public slots:
    void detect(cv::Mat m);

signals:
    void sendDectectImage(QImage image);
};

#endif // CNCNN_H
