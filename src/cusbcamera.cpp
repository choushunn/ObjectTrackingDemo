#include "cusbcamera.h"

/**
 * @brief CUSBCamera 构造函数
 * @param
 */
CUSBCamera::CUSBCamera(int camIndex, QObject *parent)
    : QObject{parent}
    ,m_camIndex(camIndex)

{
    m_cap = new cv::VideoCapture();
//    qDebug() << "CUSBCamera" <<QThread::currentThreadId() << QThread::currentThread();
}




/**
 * @brief 打开相机
 * @param
 */
void CUSBCamera::open(){
    bool ret = m_cap->open(m_camIndex, cv::CAP_ANY);
    if(ret)
    {
        qDebug() << "CUSBCamera:open success.";
    }
    else
    {
        m_cap->release();
        return;
    }
}

/**
 * @brief 读取帧
 * @param
 */
void CUSBCamera::read(){
    cv::Mat frame;
    qDebug() << "CUSBCamera:1.read frame";
    qDebug() << "CUSBCamera" <<QThread::currentThreadId() << QThread::currentThread();
    if (m_cap->isOpened())
    {
        bool ret = m_cap->read(frame);
        if(ret)
        {
            emit sendFrame(frame);
        }
    }
}

/**
 * @brief 关闭相机
 * @param
 */
void CUSBCamera::close(){
    if(m_cap->isOpened()){
        m_cap->release();
        qDebug() << "CUSBCamera:close success.";
    }
}
