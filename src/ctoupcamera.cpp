#include "ctoupcamera.h"
#include <QString>

/**
 * @brief CToupCamera 构造函数
 * @param
 */
CToupCamera::CToupCamera(int index)
    :m_index(index)
{
    Toupcam_EnumV2(m_arr);
}

CToupCamera::~CToupCamera() {
    // 析构函数
    close();
}

bool CToupCamera::isOpened() const {
    // 检查是否已经打开
    if(m_hcam){
        return true;
    }
    else{
        return false;
    }
}


int CToupCamera::open() {
    // 打开
    if (!isOpened()) {
        m_hcam = Toupcam_Open(m_arr[m_index].id);
        if (m_hcam) {
            Toupcam_get_eSize(m_hcam, (unsigned *) &m_res);
            m_imgWidth = m_arr[m_index].model->res[m_res].width;
            m_imgHeight = m_arr[m_index].model->res[m_res].height;
            //初始化Toup设置
            //1:BGR,2:RGB,Qimage use RGB byte order
            Toupcam_put_Option(m_hcam, TOUPCAM_OPTION_BYTEORDER, 0);
            //自动曝光 0:不启用,1:连续模式,2:单次模式
            Toupcam_put_AutoExpoEnable(m_hcam, 1);
            if (m_pData) {
                delete[] m_pData;
                m_pData = nullptr;
            }
            //启动相机
            if (SUCCEEDED(Toupcam_StartPullModeWithCallback(m_hcam, eventCallBack, this))) {
                qDebug() << "CToupCam:打开成功.";
                m_pData = new unsigned char[TDIBWIDTHBYTES(m_imgWidth * 24) * m_imgHeight];
                pInfo = new ToupcamFrameInfoV2();
                return 200;
            } else {
                this->close();
                qDebug() << "CToupCam:打开失败，拉取图像失败。";
                return 404;
            }
        }
    }
    return isOpened();
}

void CToupCamera::close(){
    // 关闭
    if (isOpened()) {
        Toupcam_Close(m_hcam);
        m_hcam = nullptr;
        if (m_pData) {
            delete[] m_pData;
            m_pData = nullptr;
        }
        qDebug() << "CToupCam:关闭相机成功.";
    }
}

bool CToupCamera::read(cv::Mat& frame) {
    // 读取帧
    if (isOpened()) {
        HRESULT hr = Toupcam_PullImageV2(m_hcam, m_pData, 24, pInfo);
        if (SUCCEEDED(hr)) {
            qDebug() << "CToupCam:读取图像成功。" << pInfo->width << "x" << pInfo->height;
            // 将图像数据和大小信息存储到 Mat 对象中
            cv::Mat image(m_imgHeight, m_imgWidth, CV_8UC3, m_pData);
            frame = image.clone();
            // 将图像数据和大小信息存储到 ImageData 对象中
            // image.data = m_pData;
            // image.width = m_imgWidth;
            // image.height = m_imgHeight;
            return true;
        }
        qDebug() << "CToupCam:读取图像失败。" << FAILED(hr);
    }
    return false;
}

/**
 * @brief 回调函数
 * @param
 */
void __stdcall CToupCamera::eventCallBack(unsigned nEvent, void *pCallbackCtx) {
    if (TOUPCAM_EVENT_IMAGE == nEvent) {
        qDebug() << "CToupCam:handleEvent:pull image ok" << nEvent;
    } else {
        qDebug() << "CToupCam:handleEvent" << nEvent;
    }
}

