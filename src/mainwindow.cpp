#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QHostAddress>
/**
 * @brief 构造函数
 * @param
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/img/logo.ico"));
    appInit  = new AppInit(ui);
//    appEvent = new AppEvent(this);
    m_timer  = new QTimer();
    m_searchTimer = new QTimer();
    m_searchTimer->setInterval(120);
    connect(m_searchTimer, &QTimer::timeout, this, &MainWindow::on_timeoutSearch);

    qDebug() << "MainWindow" <<QThread::currentThreadId() << QThread::currentThread();
    ui->tabWidget->setVisible(false);
    ui->pushButton_2->raise();


    // 注册事件
    appEvent->registerEvent("myEvent", this);

    // 连接按钮的点击事件信号到Lambda表达式
    connect(ui->pushButton_4, &QPushButton::clicked, [=]() {
        // 发送事件到AppEvent
        QVariantList args;
        args << "Event received!";
        AppEvent::instance()->sendEvent("myEvent", args);
    });

    // 监听"myEvent"事件
    QObject::connect(appEvent, &AppEvent::myEvent, this, [=](const QVariantList& args) {
        // 处理事件
        statusBar()->showMessage(args[0].toString());
    });


    // 监听"myEvent"事件
    connect(AppEvent::instance(), SIGNAL(myEvent(QVariantList)), this, SLOT(onMyEventReceived(QVariantList)));

}

void MainWindow::onMyEventReceived(const QVariantList& args)
{
    // 处理事件
    QString message = args[0].toString();

    statusBar()->showMessage(args[0].toString());
}


//读取图像槽函数
void MainWindow::readFrame() {
    cv::Mat image;
    appInit->camera->read(image);
    if (!image.empty()) {
        emit sendFrame(image);
//        this->showResultFrame(image);
    }
}


/**
* @brief 显示frame
* @param frame
*/
void MainWindow::showResultFrame(cv::Mat frame) {
    cv::resize(frame, frame, cv::Size(640, 480));
    QSize size = ui->m_lbl_display->size();
    QImage qimage1(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
    QPixmap pixmap1 = QPixmap::fromImage(qimage1);
    pixmap1 = pixmap1.scaled(size, Qt::KeepAspectRatio);
    ui->m_lbl_display->setPixmap(pixmap1);
    cv::Mat output_image = frame.clone();
}



/**
 * @brief 打开相机信号槽
 * @param
 */
void MainWindow::on_m_btn_open_camera_clicked(bool checked)
{
    if(checked){
        ui->m_btn_open_camera->setText("关闭");
        //1s读10帧
        fps = ui->lineEdit_FPS->text().toInt();
        m_timer->setInterval(int(1000/fps));

            appInit->camera->open();
            appInit->yolov8Onnx->initTracker();
            m_timer->start();
            //读取帧
//            connect(m_timer, &QTimer::timeout, appInit->camera, &CCamera::read);
            connect(m_timer, &QTimer::timeout, this, &MainWindow::readFrame);
            //处理帧
            //            connect(appInit->webCamera, &CUSBCamera::sendFrame, appEvent, &AppEvent::processFrame);
            connect(this, &MainWindow::sendFrame, appInit->ncnnYolo, &CNcnn::detect);
//            connect(this, &MainWindow::sendFrame, appInit->yolov8Onnx, &YoloV8Onnx::tracking);
            //显示帧
            connect(appInit->ncnnYolo, &CNcnn::sendDectectImage, this, &::MainWindow::showFrame);
            connect(appInit->yolov8Onnx, &YoloV8Onnx::sendDectectImage, this, &::MainWindow::showFrameServo);
        ui->m_cbx_camera_list->setDisabled(true);
        ui->m_cbx_camera_type->setDisabled(true);
    }
    else
    {

        m_timer->stop();
        appInit->camera->close();
        appInit->yolov8Onnx->deleteTracker();
        ui->m_btn_open_camera->setText("打开");
        ui->m_lbl_display->clear();
        ui->m_cbx_camera_list->setDisabled(false);
        ui->m_cbx_camera_type->setDisabled(false);
    }
}

/**
 * @brief 打开串口信号槽
 * @param
 */
void MainWindow::on_m_btn_open_serial_port_clicked(bool checked)
{

    if(checked){
        appInit->serialPort->open();
        ui->m_btn_open_serial_port->setText("关闭");
        ui->m_slder_steer1->setDisabled(false);
        ui->m_slder_steer2->setDisabled(false);
        ui->m_btn_serial_port_send->setDisabled(false);
        ui->m_btn_search->setDisabled(false);
        ui->m_cbx_serial_port_list->setDisabled(true);
    }
    else
    {
        appInit->serialPort->close();
        ui->m_btn_open_serial_port->setText("打开");
        ui->m_slder_steer1->setDisabled(true);
        ui->m_slder_steer2->setDisabled(true);
        ui->m_btn_serial_port_send->setDisabled(true);
        ui->m_btn_search->setDisabled(true);
        ui->m_slder_steer1->setSliderPosition(135);
        ui->m_slder_steer2->setSliderPosition(135);
        ui->m_cbx_serial_port_list->setDisabled(false);
    }

}

/**
 * @brief 打开 Web Socket 信号槽
 * @param
 */
void MainWindow::on_m_btn_open_web_socket_clicked()
{
    if(!appInit->webSocket){
        appInit->initWebSocket();
        appInit->webSocket->startListen();
        ui->m_btn_open_web_socket->setText("关闭");
    }
    else
    {
        appInit->webSocket->disconnect();
        ui->m_btn_open_web_socket->setText("打开");
    }

}

/**
 * @brief 显示QImage
 * @param image    接收到的QImage
 */
void MainWindow::showFrame(QImage image)
{
        qDebug() << "MainWindow:3.show frame.";
    //    QImage new_image = image.scaled(900, 700, Qt::KeepAspectRatio, Qt::FastTransformation);
    QImage new_image = image.scaled(ui->m_lbl_display->width(), ui->m_lbl_display->height(), Qt::KeepAspectRatio, Qt::FastTransformation);
    ui->m_lbl_display->setPixmap(QPixmap::fromImage(new_image));
}

/**
 * @brief 显示QImage
 * @param image    接收到的QImage
 */
void MainWindow::showFrameServo(QImage image,cv::Point servo_xy)
{
        qDebug() << "MainWindow:3.show frame.";
    //    QImage new_image = image.scaled(900, 700, Qt::KeepAspectRatio, Qt::FastTransformation);
    QImage new_image = image.scaled(ui->m_lbl_display->width(), ui->m_lbl_display->height(), Qt::KeepAspectRatio, Qt::FastTransformation);
    ui->m_lbl_display->setPixmap(QPixmap::fromImage(new_image));
    qDebug() << servo_xy.x << "x" << servo_xy.y;
    if(servo_xy.x==-135 && servo_xy.y==135)
    {
        qDebug() << "没有检测到物体";
        m_count_no_objects +=1;
        if(m_count_no_objects>=10){
            m_servo_x = 135;
            QTimer::singleShot(50, this, [=]{
                //2# 控制上下
                appInit->serialPort->sendData(1, 135);
            });
            appInit->serialPort->sendData(2, 135);
            m_count_no_objects=0;
        }
    }
    else
    {
        m_servo_x -= servo_xy.x*0.1;
        m_servo_y += servo_xy.y*0.08;
        //1# 控制左右
        appInit->serialPort->sendData(2, m_servo_x);
        QTimer::singleShot(50, this, [=]{
            //2# 控制上下
            appInit->serialPort->sendData(1, m_servo_y);
        });

        ui->m_lbl_slider2_value->setText(QString::number(servo_xy.y));
    }
}

/**
 * @brief 析构函数
 * @param
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief 搜索槽函数
 * @param
 */
void MainWindow::on_timeoutSearch()
{
    int x1 = 0 + rand() % (180 - 0 + 1);
    int y = 0 + rand() % (2 - 0 + 1);
    appInit->serialPort->sendData(y, x1);
}

/**
 * @brief 发送串口测试信息信号槽函数
 * @param
 */
void MainWindow::on_m_btn_serial_port_send_clicked()
{
    int value;
    value = ui->m_line_angle->text().toInt();
    int device_id = ui->m_cbx_device_list->currentIndex() + 1;
    appInit->serialPort->sendData(device_id, value);
    qDebug() << "MainWindow:发送测试数据";
}

/**
 * @brief 开始搜索信号槽函数
 * @param
 */
void MainWindow::on_m_btn_search_clicked()
{
    if(m_searchTimer->isActive())
    {
        m_searchTimer->stop();
        ui->m_btn_search->setText("搜索");
    }
    else
    {
        m_searchTimer->start();
        ui->m_btn_search->setText("停止搜索");
    }
}

/**
 * @brief 舵机1
 * @param
 */
void MainWindow::on_m_slder_steer1_valueChanged(int value)
{
    appInit->serialPort->sendData(1, value);
    ui->m_lbl_slider1_value->setText(QString::number(value - 45));
}

/**
 * @brief 舵机2
 * @param
 */
void MainWindow::on_m_slder_steer2_valueChanged(int value)
{
    appInit->serialPort->sendData(2, value);
    ui->m_lbl_slider2_value->setText(QString::number(value));
}




void MainWindow::on_pushButton_clicked()
{
    this->fps = ui->lineEdit_FPS->text().toInt();
    m_timer->setInterval(int(1000/fps));
    qDebug() << "set fps: " << this->fps;
}

/**
 * @brief 退出
 * @param
 */
void MainWindow::on_action_2_triggered()
{
    int ret = QMessageBox::warning(this, "退出", "是否退出程序", QMessageBox::Ok, QMessageBox::Cancel);
    switch(ret)
    {
    case QMessageBox::Ok:
        qDebug() <<"退出程序";
        QApplication::quit();
        break;
    case QMessageBox::Cancel:
        qDebug() <<"取消退出程序";
        break;
    default:
        break;
    }
}


void MainWindow::on_action_3_triggered()
{
    if(MainWindow::isFullScreen()){
        MainWindow::showNormal();
    }else{
        MainWindow::showFullScreen();
    }

}


void MainWindow::on_action_3_triggered(bool checked)
{
    //    if(checked){
    //        MainWindow::showFullScreen();
    //    }else{
    //        MainWindow::showNormal();
    //    }
}


void MainWindow::on_action_4_triggered()
{
    if(MainWindow::isMaximized()){
        MainWindow::showNormal();
    }else{
        MainWindow::showMaximized();
    }
}


void MainWindow::on_action_5_triggered()
{

    MainWindow::showNormal();

}


void MainWindow::on_action_triggered()
{
    QString path = QDir::currentPath();
    QString fileName = QFileDialog::getOpenFileName(this, "选择一个文件", path,  "图像文件 (*.jpg *.png *.bmp)");

    // 如果选择了文件，则加载图像文件并显示在标签控件中
    if (!fileName.isEmpty())
    {
        QPixmap pixmap(fileName);
        ui->m_lbl_display->setPixmap(pixmap);
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    if(ui->tabWidget->isVisible()){
        ui->tabWidget->setVisible(false);
    }else{
        ui->tabWidget->setVisible(true);
    }
}


void MainWindow::on_pushButton_3_clicked()
{

}

