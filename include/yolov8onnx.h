#ifndef YOLOV8ONNX_H
#define YOLOV8ONNX_H
#include <QObject>

#include "layer.h"
#include "net.h"

//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <iostream>
#include "BYTETracker.h"
#include "utils.h"




class YoloV8Onnx : public QObject,public ncnn::Layer
{
    Q_OBJECT
public:
    explicit YoloV8Onnx(QObject *parent = nullptr);
    void initTracker();
    void deleteTracker();
private:
    int detect_yolox(const cv::Mat& bgr, std::vector<Object>& objects);
    static const char* class_names[];
    ncnn::Net* yolox;
    BYTETracker* tracker;
    vector<STrack> output_stracks;

public slots:
    void tracking(cv::Mat m);

signals:
    void sendDectectImage(QImage image, cv::Point servo_xy);
};

#endif // YOLOV8ONNX_H
