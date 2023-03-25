#ifndef CUSBCAMERA_H
#define CUSBCAMERA_H

#include <QObject>
#include <QMediaDevices>
#include <QCamera>
#include <QThread>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

class CUSBCamera : public QObject
{
    Q_OBJECT
public:
    explicit CUSBCamera(int camIndex=0, QObject *parent = nullptr);
    void open();
    void close();

public slots:
    void read();

private:
    QList<QCameraDevice> m_cameraList;
    cv::VideoCapture* m_cap;
    int m_camIndex;

signals:
    void sendFrame(cv::Mat frame);
};

#endif // CUSBCAMERA_H
