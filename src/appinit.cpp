#include "appinit.h"
#include "cncnn.h"


/**
 * @brief AppInit 构造函数
 * @param
 */
AppInit::AppInit()
{
    //    读取配置文件。
    //    设置全局字体。
    //    设置全局样式表，建议先读取通用的样式表，然后将额外的样式表内容加到后面一起设置。
    //    设置项目编码。
    //    设置翻译文件，可以加载多个，包括qt内置的qt_zh_CN.qm，用户自己的翻译文件等。
    //    初始化随机数种子。
    //    新建项目中需要的目录，防止没有目录无法保存文件到目录。
    //    初始化数据库，包括打开数据库，载入基础数据比如用户表、设备表等。
    //    启动日志输出类用来启动日志服务。
    //    启动运行时间记录类用来记录每次软件运行开始时间和结束时间。
    //    关联全局事件过滤器处理自定义无边框UI拖动、全局按键处理等。
}

/**
 * @brief findCameraIndex 根据ID查询/dev/video的序号
 * @param cameraId 读取的相机id
 */
int findCameraIndex(QByteArray cameraId){
    std::string dev_name_str(cameraId.data(), cameraId.size());
    std::size_t pos = dev_name_str.find("/dev/video");
    int dev_num =0;
    if (pos != std::string::npos) {
        std::string num_str = dev_name_str.substr(pos + 10);
        dev_num = std::stoi(num_str);
        qDebug() << "Device number: " << dev_num;
        return dev_num;
    }
    return 0;
}

/**
 * @brief AppInit 构造函数
 * @param
 */
AppInit::AppInit(Ui::MainWindow *ui)
    :mainwindowUi(ui)
{
    //UI初始化
    initMainWindowUI();
    //摄像头初始化
    initCamera();
    //串口初始化
    initSerialPort();
    //WebSocket初始化
    //    initWebSocket();
    //Onnx初始化
    //    initOnnx();
    //Ncnn初始化
    initNcnn();
    //相机类型切换检测
    connect(mainwindowUi->m_cbx_camera_list, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index)
    {
#ifdef _WIN32
        m_cameraIndex = index;
#else
        if(mainwindowUi->m_cbx_camera_type->currentText() == "USB"){
            QByteArray dev_name = m_cameraList[index].id();
            m_cameraIndex = findCameraIndex(dev_name);
            //切换先删除初始化相机
            if(webCamera){
                delete webCamera;
            }
            webCamera = new CUSBCamera(m_cameraIndex);
        }else{
            m_cameraIndex = index;
        }
#endif

        qDebug() << "AppInit:camera index changed:" << m_cameraIndex;
    });

    connect(mainwindowUi->m_cbx_camera_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index)
    {
        qDebug() << "AppInit:camera type changed:" << mainwindowUi->m_cbx_camera_type->currentText();
        if(mainwindowUi->m_cbx_camera_type->currentText() == "USB"){
            initCamera();
        }else if(mainwindowUi->m_cbx_camera_type->currentText() == "TOUP"){
            initToupCamera();
        }
        else
        {
            qDebug() <<"AppInit:Camera Failed.";
        }
    });



    connect(mainwindowUi->m_cbx_serial_port_list, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index)
    {
        qDebug() << "AppInit:serial port changed:" << mainwindowUi->m_cbx_serial_port_list->currentText();
        serialPort = new CSerialPort(mainwindowUi->m_cbx_serial_port_list->currentText(), 115200);
    });

}

/**
 * @brief 界面初始化
 * @param
 */
void AppInit::initMainWindowUI()
{
    qDebug() << "AppInit:UI初始化完成.";
}


/**
 * @brief USB摄像头初始化
 * @param
 */
void AppInit::initCamera()
{
    mainwindowUi->m_cbx_camera_list->clear();
    //查询可用相机
    m_cameraList = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : m_cameraList) {
        //qDebug()<<cameraDevice.id() << cameraDevice.isNull() << cameraDevice.position() << cameraDevice.isDefault();
        mainwindowUi->m_cbx_camera_list->addItem(cameraDevice.description());
    }

    if(m_cameraList.count()==0){
        mainwindowUi->m_cbx_camera_list->setEditable(true);
        mainwindowUi->m_cbx_camera_list->setCurrentText("未检测到相机");
        mainwindowUi->m_cbx_camera_list->setDisabled(true);
        mainwindowUi->m_btn_open_camera->setDisabled(true);
        qDebug() << "AppInit:USB摄像头初始化失败." << "检测到"<< m_cameraList.count() << "个USB摄像头.";
        return;
    }else{
        mainwindowUi->m_cbx_camera_list->setEditable(false);
        mainwindowUi->m_cbx_camera_list->setDisabled(false);
        mainwindowUi->m_btn_open_camera->setDisabled(false);
        m_cameraIndex = mainwindowUi->m_cbx_camera_list->currentIndex();

#ifdef _WIN32

        qDebug() << "Windows";
#else
        QByteArray dev_name = m_cameraList[m_cameraIndex].id();
        m_cameraIndex = findCameraIndex(dev_name);
#endif
        //qDebug() << "默认0：" << m_cameraIndex;
        webCamera = new CUSBCamera(m_cameraIndex);
        //        appThread = new QThread();
        //        camera->moveToThread(appThread);
        //        appThread->start();
    }
    qDebug() << "AppInit:USB摄像头初始化完成." << "检测到"<< m_cameraList.count() << "个USB摄像头.";
}


/**
 * @brief Toup摄像头初始化
 * @param
 */
void AppInit::initToupCamera()
{
    mainwindowUi->m_cbx_camera_list->clear();
    // 显示可用相机
    ToupcamDeviceV2 arr[TOUPCAM_MAX];
    unsigned toupCamCount = Toupcam_EnumV2(arr);

    if (0 == toupCamCount){
        mainwindowUi->m_cbx_camera_list->setEditable(true);
        mainwindowUi->m_cbx_camera_list->setCurrentText("未检测到相机");
        mainwindowUi->m_cbx_camera_list->setDisabled(true);
        mainwindowUi->m_btn_open_camera->setDisabled(true);
        qDebug() << "AppInit:Toup摄像头初始化失败." << "检测到"<< 0 << "个Toup摄像头.";
        return;
    }
    else
    {
        mainwindowUi->m_cbx_camera_list->setEditable(false);
        mainwindowUi->m_cbx_camera_list->setDisabled(false);
        mainwindowUi->m_btn_open_camera->setDisabled(false);
        for (unsigned i = 0; i < toupCamCount; ++i)
        {
            //循环每个相机
#ifdef _WIN32
            mainwindowUi->m_cbx_camera_list->addItem(QString::fromWCharArray(arr[i].displayname));
#else
            mainwindowUi->m_cbx_camera_list->addItem(arr[i].displayname);
#endif
        }
        m_cameraIndex = mainwindowUi->m_cbx_camera_list->currentIndex();
        qDebug() <<"ToupCame id：" << m_cameraIndex;
        toupCamera = new CToupCamera(arr[m_cameraIndex]);
    }
    qDebug() << "AppInit:Toup摄像头初始化完成." << "检测到"<< toupCamCount << "个Toup摄像头.";
}




/**
 * @brief 串口初始化
 * @param
 */
void AppInit::initSerialPort()
{
    //获取可以用的串口
    QList <QSerialPortInfo> serialPortInfos = QSerialPortInfo::availablePorts();
    //将所有可以使用的串口设备添加到ComboBox中
    for (const QSerialPortInfo &serialPortInfo: serialPortInfos) {
        mainwindowUi->m_cbx_serial_port_list->addItem(serialPortInfo.portName());
    }
    mainwindowUi->m_slder_steer1->setDisabled(true);
    mainwindowUi->m_slder_steer2->setDisabled(true);
    mainwindowUi->m_btn_serial_port_send->setDisabled(true);
    mainwindowUi->m_btn_search->setDisabled(true);
    if (serialPortInfos.count() == 0) {
        mainwindowUi->m_cbx_serial_port_list->setEditable(true);
        mainwindowUi->m_cbx_serial_port_list->setCurrentText("未检测到串口");
        mainwindowUi->m_cbx_serial_port_list->setDisabled(true);
        mainwindowUi->m_btn_open_serial_port->setDisabled(true);
        qDebug() << "AppInit:串口初始化失败." << "检测到"<< serialPortInfos.count() <<"个串口.";
        return;
    }
    else
    {
        mainwindowUi->m_cbx_serial_port_list->setEditable(false);
        mainwindowUi->m_btn_open_serial_port->setDisabled(false);
        serialPort = new CSerialPort(mainwindowUi->m_cbx_serial_port_list->currentText(), 115200);
    }
    mainwindowUi->m_lbl_slider1_value->setText(QString::number(mainwindowUi->m_slder_steer1->value()-45));
    mainwindowUi->m_lbl_slider2_value->setText(QString::number(mainwindowUi->m_slder_steer2->value()));
    mainwindowUi->statusbar->showMessage("Ready");
    qDebug() << "AppInit:串口初始化完成." << "检测到"<< serialPortInfos.count() <<"个串口.";

    mainwindowUi->m_cbx_ip_list->addItem("127.0.0.1");
}


/**
 * @brief WebSocket初始化
 * @param
 */
void AppInit::initWebSocket()
{
    //显示可用IP
    //    mainwindowUi->m_cbx_ip_list->addItem("127.0.0.1");
    //默认端口
    quint32  port = mainwindowUi->m_line_port->text().toInt();
    webSocket = new CWebSocket(port);
    connect(webSocket,&CWebSocket::sendTextMessage, this, &AppInit::showTextMessage);
    //检测是否打开
    qDebug() << "AppInit:WebSocket初始化完成.";
}


/**
 * @brief WebSocket显示接收信息
 * @param
 */
void AppInit::showTextMessage(const QString &message){
    QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间
    QString dataTimeStr = dateTime .toString("hh:mm:ss");//格式化时间
    mainwindowUi->textBrowser2->append(dataTimeStr+"-->"+message);
}

///**
// * @brief ONNX初始化
// * @param
// */
//void AppInit::initOnnx()
//{
//    yolov8Onnx = new YoloV8Onnx();
//    //    appThread = new QThread();
//    //    nc->moveToThread(appThread);
//    //    appThread->start();
//        qDebug() << "AppInit:Onnx初始化完成.";
//}


/**
 * @brief NCNN初始化
 * @param
 */
void AppInit::initNcnn()
{
    //    ncnnYolo = new CNcnn();
    //    appThread = new QThread();
    //    nc->moveToThread(appThread);
    //    appThread->start();
    qDebug() << "AppInit:Ncnn初始化完成.";
}




