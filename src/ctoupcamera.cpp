#include "ctoupcamera.h"


/**
 * @brief CToupCamera 构造函数
 * @param
 */
CToupCamera::CToupCamera(ToupcamDeviceV2 cur, QObject *parent)
    : QObject{parent}
    ,m_cur(cur)
{
    //全局事件响应
    connect(this, &CToupCamera::evtCallback, this, [this](unsigned nEvent)
    {
        //todo:
        //热插拔设备检测
        //qDebug() << "handleEvent" << nEvent;
        //运行时事件
        if (m_hcam)
        {
            if (TOUPCAM_EVENT_IMAGE == nEvent)
            {
                //读取图像
                //read();
            }
            else if(TOUPCAM_EVENT_EXPOSURE == nEvent)
            {
                //曝光事件
            }
            else if (TOUPCAM_EVENT_TEMPTINT == nEvent)
            {
                //handleTempTintEvent();
            }
            else if (TOUPCAM_EVENT_STILLIMAGE == nEvent)
            {
                //handleStillImageEvent();
            }
            else if (TOUPCAM_EVENT_ERROR == nEvent)
            {
                close();

            }
            else if (TOUPCAM_EVENT_DISCONNECTED == nEvent)
            {
                close();

            }
            else
            {

            }
        }
    });

}

/**
 * @brief 暂停Toup相机
 * @param
 */
void CToupCamera::pause()
{
    if (m_hcam)
    {
        //0:继续,1:暂停
        Toupcam_Pause(m_hcam, 0);
    }
}


/**
 * @brief 关闭Toup相机
 * @param
 */
void CToupCamera::close()
{
    if (m_hcam)
    {
        Toupcam_Close(m_hcam);
        m_hcam = nullptr;
    }
    delete[] m_pData;
    m_pData = nullptr;
}


/**
 * @brief 事件调用
 * @param
 */
void CToupCamera::eventCallBack(unsigned nEvent, void* pCallbackCtx)
{
    CToupCamera* pThis = reinterpret_cast<CToupCamera*>(pCallbackCtx);
    emit pThis->evtCallback(nEvent);
}


/**
 * @brief 打开Toup相机
 * @param
 */
void CToupCamera::open(){
    m_hcam = Toupcam_Open(m_cur.id);
    if (m_hcam)
    {
        Toupcam_get_eSize(m_hcam, (unsigned*)&m_res);
        m_imgWidth = m_cur.model->res[m_res].width;
        m_imgHeight = m_cur.model->res[m_res].height;
        //初始化Toup设置
        //1:BGR,2:RGB,Qimage use RGB byte order
        Toupcam_put_Option(m_hcam, TOUPCAM_OPTION_BYTEORDER, 0);
        //自动曝光 0:不启用,1:连续模式,2:单次模式
        Toupcam_put_AutoExpoEnable(m_hcam, 1);
        if (m_pData)
        {
            delete[] m_pData;
            m_pData = nullptr;
        }
        //启动相机
        if (SUCCEEDED(Toupcam_StartPullModeWithCallback(m_hcam, eventCallBack, this)))
        {
            m_pData = new uchar[TDIBWIDTHBYTES(m_imgWidth * 24) * m_imgHeight];
            pInfo = new ToupcamFrameInfoV2();
            qDebug()<<"CToupCamera:ToupCam打开成功.";
        }
        else
        {
            qDebug()<<"CToupCamera:ToupCam打开失败.";
            close();
        }
    }
}


/**
 * @brief 读取图像
 * @param
 */
void CToupCamera::read()
{
    if(SUCCEEDED(Toupcam_PullImageV2(m_hcam, m_pData, 24, pInfo))){
//        QImage image(m_pData, pInfo->width, pInfo->height, QImage::Format_RGB888);
        //        emit sendImage(image);
        cv::Mat frame(pInfo->height,pInfo->width, CV_8UC3, m_pData);
        emit sendFrame(frame);
    }
}

