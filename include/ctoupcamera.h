#ifndef CTOUPCAMERA_H
#define CTOUPCAMERA_H

#include <QObject>
#include <QImage>
#include <toupcam.h>
#include "qdebug.h"
#include "utils.h"

class CToupCamera : public QObject
{
    Q_OBJECT

public:
    explicit CToupCamera(ToupcamDeviceV2 cur, QObject *parent = nullptr);
    void open();
    void close();
    void pause();
    void read();

private:
    ToupcamDeviceV2 m_cur;
    HToupcam        m_hcam;
    uchar*          m_pData = nullptr;
    int             m_res;
    unsigned        m_imgWidth;
    unsigned        m_imgHeight;
    static void __stdcall eventCallBack(unsigned nEvent, void* pCallbackCtx);
    ToupcamFrameInfoV2* pInfo;
signals:
    void evtCallback(unsigned nEvent);
    void sendImage(QImage image);
    void sendFrame(cv::Mat frame);
};

#endif // CTOUPCAMERA_H
