#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include "appinit.h"
#include "appevent.h"

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
    int         fps = 30;
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

};
#endif // MAINWINDOW_H
