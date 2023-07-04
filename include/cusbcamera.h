#ifndef CUSBCAMERA_H
#define CUSBCAMERA_H

#include "ccamera.h"

class CUSBCamera : public CCamera
{
public:
    CUSBCamera(int index);
    ~CUSBCamera();
    bool isOpened() const override;
    int open() override;
    void close() override;
    bool read(cv::Mat& frame) override;
private:
    int m_index;
    cv::VideoCapture m_capture;
};

#endif // CUSBCAMERA_H
