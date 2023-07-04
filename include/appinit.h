#ifndef APPINIT_H
#define APPINIT_H

#include <QMainWindow>
#include <QMediaDevices>
#include <QCamera>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <toupcam.h>
#include "ui_mainwindow.h"
#include "cwebsocket.h"
#include "cserialport.h"
#include "ccamera.h"
#include "cncnn.h"
#include "yolov8onnx.h"
#include "utils.h"

namespace Ui{
class MainWindow;
}

class AppInit : public QMainWindow
{
    Q_OBJECT
public:
    AppInit();
    explicit AppInit(Ui::MainWindow *ui);
    CCamera*     camera  = nullptr;
    CWebSocket*     webSocket  = nullptr;
    CSerialPort*    serialPort = nullptr;
    CNcnn*          ncnnYolo   = nullptr;

    QThread*        appThread  = nullptr;
    YoloV8Onnx*     yolov8Onnx = nullptr;


private:
    Ui::MainWindow       *mainwindowUi;
    QList<QCameraDevice> m_cameraList;
    int m_cameraIndex = 0;
    std::string camera_type = "USB";

public:
    void initWebSocket();

private:
    void initMainWindowUI();
    void initCamera();
//    void initToupCamera();
    void initSerialPort();

    void initOnnx();
    void initNcnn();

signals:

public slots:
    void showTextMessage(const QString &message);


};

#endif // APPINIT_H
