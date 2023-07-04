#include "cusbcamera.h"
#include "QDebug"
/**
 * @brief CUSBCamera 构造函数
 * @param
 */
CUSBCamera::CUSBCamera(int index)
    :m_index(index)
{

}

CUSBCamera::~CUSBCamera() {
    // 析构函数
    close();
}

bool CUSBCamera::isOpened() const {
    // 检查是否已经打开
    return m_capture.isOpened();
}

int CUSBCamera::open() {

    // 打开
    if (!isOpened()) {
        m_capture.open(m_index);
        qDebug() <<"CUSBCamera:打开成功！";
    }
    return isOpened();
}

void CUSBCamera::close(){
    // 关闭
    if (isOpened()) {
        m_capture.release();
    }
}

bool CUSBCamera::read(cv::Mat& frame) {
    // 读取帧
    if (isOpened()) {
        return m_capture.read(frame);
    }
    return false;
}




