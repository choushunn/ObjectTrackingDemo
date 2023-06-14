#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <QMessageBox>
#include <QFileDialog>
#include "appinit.h"
#include "appevent.h"

#include <QPropertyAnimation>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *ui;


private:
    AppInit*    appInit;
    AppEvent*   appEvent;
    QThread*    appThread;
    QTimer*     m_timer = nullptr;
    QTimer*     m_searchTimer = nullptr;
    int         fps = 1;
    int m_servo_x = 135;
    int m_servo_y = 135;
    int m_count_no_objects = 0;
    QPropertyAnimation *m_propertyAnimation;
    QPropertyAnimation *m_propertyAnimation2;
    bool m_bSideflag = false;
private slots:
    void on_m_btn_open_camera_clicked(bool checked);
    void on_m_btn_open_serial_port_clicked(bool checked);
    void on_m_btn_open_web_socket_clicked();    
    void on_m_btn_serial_port_send_clicked();
    void on_m_btn_search_clicked();
    void on_m_slder_steer1_valueChanged(int value);
    void on_m_slder_steer2_valueChanged(int value);
    void on_timeoutSearch();
    void showFrame(QImage image);
    void showFrameServo(QImage image,cv::Point servo_xy);
    void on_pushButton_clicked();
    void on_action_2_triggered();
    void on_action_3_triggered();
    void on_action_3_triggered(bool checked);
    void on_action_4_triggered();
    void on_action_5_triggered();
    void on_action_triggered();
    void on_pushButton_2_clicked();
};
#endif // MAINWINDOW_H
