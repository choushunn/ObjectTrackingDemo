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
    initWebSocket();
    //Onnx初始化
    initOnnx();
    //Ncnn初始化
    initNcnn();
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
 * @brief 获取可用相机列表
 * @param
 */
void getCameraList(std::vector<std::string> &camera_list,std::string& camera_type) {
    if(camera_type=="TOUP"){
        ToupcamDeviceV2 m_arr[TOUPCAM_MAX]; //所有相机
        int toupCamCount = Toupcam_EnumV2(m_arr);
        for (int i = 0; i < toupCamCount; ++i) {
            qDebug() << m_arr[i].id << m_arr[i].displayname;
#ifdef _WIN32
            camera_list.push_back(QString::fromWCharArray(m_arr[i].displayname).toStdString());
#else
            camera_list.push_back(m_arr[i].displayname);
#endif
        }
    }else{
        //查询可用相机
        QList<QCameraDevice> cameraList = QMediaDevices::videoInputs();
        for (const QCameraDevice &cameraDevice : cameraList) {
            camera_list.push_back(cameraDevice.description().toStdString());
        }
    }
}


/**
 * @brief 摄像头初始化
 * @param
 */
void AppInit::initCamera()
{
    //相机类型切换检测
    connect(mainwindowUi->m_cbx_camera_list,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
#ifdef _WIN32
                m_cameraIndex = index;
#else
            if(mainwindowUi->m_cbx_camera_type->currentText() == "USB"){
                QByteArray dev_name = m_cameraList[index].id();
                m_cameraIndex = findCameraIndex(dev_name);

            }else{
                m_cameraIndex = index;
            }
#endif
                camera = CCamera::createInstance(
                    camera_type,
                    m_cameraIndex);
                qDebug() << "AppInit:camera index changed:" << m_cameraIndex;
            });

    connect(mainwindowUi->m_cbx_camera_type,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)            {
                qDebug() << "AppInit:camera type changed:" << mainwindowUi->m_cbx_camera_type->currentText();
                camera_type = mainwindowUi->m_cbx_camera_type->currentText().toStdString();
                initCamera();
            });


    mainwindowUi->m_cbx_camera_list->clear();
    camera_type = mainwindowUi->m_cbx_camera_type->currentText().toStdString();

    // 读取相机列表
    std::vector <std::string> camera_list;
    getCameraList(camera_list, camera_type);


    if(camera_list.empty()){
        mainwindowUi->m_cbx_camera_list->setEditable(true);
        mainwindowUi->m_cbx_camera_list->setCurrentText("未检测到相机");
        mainwindowUi->m_cbx_camera_list->setDisabled(true);
        mainwindowUi->m_btn_open_camera->setDisabled(true);

        qDebug() << "AppInit:相机初始化失败." << "检测到"<< 0 << "个相机";
        return;
    }else{

        mainwindowUi->m_cbx_camera_list->setEditable(false);
        mainwindowUi->m_cbx_camera_list->setDisabled(false);
        mainwindowUi->m_btn_open_camera->setDisabled(false);

        for (const std::string &camera: camera_list) {
            mainwindowUi->m_cbx_camera_list->addItem(camera.c_str());
        }


        if(camera_type=="USB"){
#ifdef _WIN32
            m_cameraIndex = mainwindowUi->m_cbx_camera_list->currentIndex();
#else
            QByteArray dev_name = m_cameraList[m_cameraIndex].id();
            m_cameraIndex = findCameraIndex(dev_name);
#endif
        }else{

            m_cameraIndex = mainwindowUi->m_cbx_camera_list->currentIndex();
        }


        // 加载相机
        camera = CCamera::createInstance(camera_type, m_cameraIndex);
        qDebug() << "AppInit:相机初始化完成." << "检测到"<< camera_list.capacity() << "个摄像头.";


        if(camera == nullptr){
            qDebug() << "AppInit:相机初始化失败";
        }
    }
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

    connect(mainwindowUi->m_cbx_serial_port_list, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index)
            {
                qDebug() << "AppInit:serial port changed:" << mainwindowUi->m_cbx_serial_port_list->currentText();
                serialPort = new CSerialPort(mainwindowUi->m_cbx_serial_port_list->currentText(), 115200);
            });

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

    // 当从服务器收到消息时，将触发该信号
    QObject::connect(webSocket, &CWebSocket::sendBinaryMessage, [this](const QByteArray &message) {
        // 将收到的二进制数据解析为 JSON 对象
        QJsonDocument doc = QJsonDocument::fromJson(message);
        QJsonObject json = doc.object();
        QStringList keys = json.keys();
        for(int i = 0; i < keys.size(); i++){

        qDebug() << "key" << i << " is:" << keys.at(i) << json.value(keys.at(i)).toString();
            if (json.value("type").toString() == "image") {
                QString base64Image = json.value("data").toString();
                QByteArray imageData = QByteArray::fromBase64(base64Image.toLatin1());
                QPixmap pixmap;
                pixmap.loadFromData(imageData);
                mainwindowUi->m_lbl_display->setPixmap(pixmap);
            }

        }

    });
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

/**
 * @brief ONNX初始化
 * @param
 */
void AppInit::initOnnx()
{
    yolov8Onnx = new YoloV8Onnx();
//    yolov8Onnx->tracking()
    qDebug() << "AppInit:Onnx初始化完成.";
}


/**
 * @brief NCNN初始化
 * @param
 */
void AppInit::initNcnn()
{
    ncnnYolo = new CNcnn();
    qDebug() << "AppInit:Ncnn初始化完成.";
}




