#include "mainwindow.h"
#include "./ui_mainwindow.h"


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
    appEvent = new AppEvent(this);
    //appEvent线程
    //    appEventThread = new QThread();
    //    appEvent->moveToThread(appEventThread);
    //    appEventThread->start();
    m_timer  = new QTimer();
    m_searchTimer = new QTimer();
    m_searchTimer->setInterval(120);
    connect(m_searchTimer, &QTimer::timeout, this, &MainWindow::on_timeoutSearch);
    qDebug() << "MainWindow" <<QThread::currentThreadId() << QThread::currentThread();
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
        //        fps = ui->lineEdit_FPS->text().toInt();
        m_timer->setInterval(int(1000/fps));
        connect(appInit->ncnnYolo, &CNcnn::sendDectectImage, this, &::MainWindow::showFrame);
        connect(appInit->yolov8Onnx, &YoloV8Onnx::sendDectectImage, this, &::MainWindow::showFrame);
        if(ui->m_cbx_camera_type->currentText() == "USB"){
            appInit->webCamera->open();

            m_timer->start();
            //读取帧
            connect(m_timer, &QTimer::timeout, appInit->webCamera, &CUSBCamera::read);
            //处理帧
            //            connect(appInit->webCamera, &CUSBCamera::sendFrame, appEvent, &AppEvent::processFrame);
//            connect(appInit->webCamera, &CUSBCamera::sendFrame, appInit->ncnnYolo, &CNcnn::detect);
            connect(appInit->webCamera, &CUSBCamera::sendFrame, appInit->yolov8Onnx, &YoloV8Onnx::tracking);
            //显示帧
            //            connect(appEvent, &AppEvent::sendProcessFrame, this, &MainWindow::showFrame);
        }
        else if(ui->m_cbx_camera_type->currentText() == "TOUP")
        {
            //打开摄像头,判断是否打开成功
            appInit->toupCamera->open();

            m_timer->start();
            //读取帧
            connect(m_timer, &QTimer::timeout, appInit->toupCamera, &CToupCamera::read);
            //处理帧
            //            connect(appInit->toupCamera, &CToupCamera::sendFrame, appInit->ncnnYolo, &CNcnn::detect);
            //显示帧
            connect(appInit->toupCamera, &CToupCamera::sendImage, this, &::MainWindow::showFrame);
        }
        ui->m_cbx_camera_list->setDisabled(true);
        ui->m_cbx_camera_type->setDisabled(true);
    }
    else
    {
        if(ui->m_cbx_camera_type->currentText()=="USB"){
            m_timer->stop();
            appInit->webCamera->close();
        }else if(ui->m_cbx_camera_type->currentText()=="TOUP")
        {
            appInit->toupCamera->close();
        }
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
    //    qDebug() << "MainWindow:3.show frame.";
    //    QImage new_image = image.scaled(900, 700, Qt::KeepAspectRatio, Qt::FastTransformation);
    QImage new_image = image.scaled(ui->m_lbl_display->width(), ui->m_lbl_display->height(), Qt::KeepAspectRatio, Qt::FastTransformation);
    ui->m_lbl_display->setPixmap(QPixmap::fromImage(new_image));
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

