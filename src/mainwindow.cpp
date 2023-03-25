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
        if(ui->m_cbx_camera_type->currentText()=="USB"){
            appInit->webCamera->open();
            //1s读10帧
            int fps = 40;
//            m_timer->start(int(1000/fps));
            m_timer->setInterval(int(1000/fps));
//            app_init->appThread->start();
            //读取帧
            connect(m_timer, &QTimer::timeout, appInit->webCamera, &CUSBCamera::read);
            m_timer->start();
            //处理帧
//            connect(app_init->web_camera, &CUSBCamera::sendFrame, appEvent, &AppEvent::processFrame);
            connect(appInit->webCamera, &CUSBCamera::sendFrame, appInit->ncnnYolo, &CNcnn::detect);

            //显示帧
//            connect(appEvent, &AppEvent::sendProcessFrame, this, &MainWindow::showFrame);
            connect(appInit->ncnnYolo, &CNcnn::sendDectectImage, this, &::MainWindow::showFrame);
        }
        else if(ui->m_cbx_camera_type->currentText()=="TOUP")
        {
            //读取帧
            appInit->toupCamera->open();
            //显示帧
            connect(appInit->toupCamera, &CToupCamera::sendImage, this, &::MainWindow::showFrame);
        }
    }
    else
    {
        if(ui->m_cbx_camera_type->currentText()=="USB"){
//            app_init->appThread->quit();
//            app_init->appThread->wait();
            m_timer->stop();
            appInit->webCamera->close();
        }else if(ui->m_cbx_camera_type->currentText()=="TOUP")
        {
            appInit->toupCamera->close();
        }
        ui->m_btn_open_camera->setText("打开");
        ui->m_lbl_display->clear();
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
    }

}

/**
 * @brief 打开 Web Socket 信号槽
 * @param
 */
void MainWindow::on_m_btn_open_web_socket_clicked()
{
    appInit->webSocket->startListen();
}

/**
 * @brief 显示QImage
 * @param image    接收到的QImage
 */
void MainWindow::showFrame(QImage image)
{
//    qDebug() << "MainWindow:3.show frame.";
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



