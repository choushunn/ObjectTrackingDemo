#ifndef APPINIT_H
#define APPINIT_H

#include <QMainWindow>
#include <QMediaDevices>
#include <QCamera>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "ui_mainwindow.h"
#include "cwebsocket.h"
#include "cusbcamera.h"
#include "cserialport.h"
#include "ctoupcamera.h"
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
    CUSBCamera*     webCamera  = nullptr;
    CToupCamera*    toupCamera = nullptr;
    CWebSocket*     webSocket  = nullptr;
    CSerialPort*    serialPort = nullptr;
    CNcnn*          ncnnYolo   = nullptr;
    QThread*        appThread  = nullptr;

private:
    Ui::MainWindow       *mainwindowUi;
    QList<QCameraDevice> m_cameraList;
    int m_cameraIndex;

private:
    void initMainWindowUI();
    void initCamera();
    void initToupCamera();
    void initSerialPort();
    void initWebSocket();
    void initOnnx();
    void initNcnn();

signals:

private slots:


};

#endif // APPINIT_H
