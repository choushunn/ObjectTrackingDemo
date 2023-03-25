#ifndef YOLOV8ONNX_H
#define YOLOV8ONNX_H
#include <QObject>
//#include <onnxruntime_cxx_api.h>

class YoloV8Onnx : public QObject
{
    Q_OBJECT
public:
    explicit YoloV8Onnx(QObject *parent = nullptr);

signals:

};

#endif // YOLOV8ONNX_H
