#ifndef CCAMERA_H
#define CCAMERA_H
#include <opencv2/opencv.hpp>

class CCamera
{
public:
    CCamera();
    virtual ~CCamera() {}
    virtual bool isOpened() const = 0;
    virtual int open() = 0;
    virtual void close() = 0;
    virtual bool read(cv::Mat& frame) = 0;
    //工厂方法
    static CCamera* createInstance(const std::string type, const int index);
};

#endif // CCAMERA_H
