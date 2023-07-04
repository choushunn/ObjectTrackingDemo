#include "ccamera.h"
#include "ctoupcamera.h"
#include "cusbcamera.h"

CCamera::CCamera()
{

}

CCamera* CCamera::createInstance(const std::string type, const int index) {

    if (type == "USB") {
        qDebug() << "创建摄像头 USB"<< &type << index;
        return new CUSBCamera(index);

    } else if (type == "TOUP") {
        qDebug() << "创建摄像头 TOUP"<< &type << index;
        return new CToupCamera(index);
    } else {
        qDebug() << "创建摄像头失败"<< &type << index;
        return nullptr;
    }


}
