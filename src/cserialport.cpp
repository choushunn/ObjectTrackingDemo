#include "cserialport.h"

/**
 * @brief CSerialPort 构造函数
 * @param
 */
CSerialPort::CSerialPort(QString serialPortName, qint32 baud, QObject *parent)
    : QObject{parent}
{
    m_serialPort = new QSerialPort();
    this->m_baud = baud;
    this->m_serialPortName = serialPortName;
    connect(m_serialPort, &QSerialPort::readyRead, this, &CSerialPort::read);
}

/**
 * @brief 打开串口
 * @param
 */
bool CSerialPort::open() {
    // 参数配置
    // 波特率选择115200
    m_serialPort->setBaudRate(this->m_baud);
    //当前选择的串口名
    m_serialPort->setPortName(this->m_serialPortName);
    //用ReadWrite 的模式尝试打开串口，无法收发数据时，发出警告
    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "CSerialPort:串口打开失败";
        return false;
    } else {
        qDebug()<< "CSerialPort:串口打开成功,开始初始化设备";
        //初始化设备居中，135度
        QTimer::singleShot(10, this, [=]{
            sendData(1, 135);
        });
        QTimer::singleShot(50, this,[=]{
            sendData(2, 135);
        });

    }
    return true;
}

/**
 * @brief 关闭串口
 * @param
 */
void CSerialPort::close() {
    qDebug() <<"CSerialPort:正在关闭串口";
    if (m_serialPort->isOpen()) {
        //关闭时，初始化设备居中，设置为135度
        QTimer::singleShot(10, this, [=]{
            sendData(1, 135);
        });

        QTimer::singleShot(50, this, [=]{
            sendData(2, 135);
        });
        QTimer::singleShot(100, this, [=]{
            m_serialPort->clear();
            m_serialPort->close();
        });
    }
}

/**
 * @brief 给串口发送数据
 * @param
 */
void CSerialPort::sendData(int device_id = 1, int angle = 135) {
    int m_times=100;
    angle = abs(angle);
    if(device_id == 1 && (angle<45 || angle>225)){
        angle = 135;
    }else {
        angle %= 270;
    }

    //构造发送命令
    QString command = QString("#%1P%2T%3\r\n").arg(device_id).arg(Normalization(angle)).arg(m_times);
    QByteArray command_byte = command.toUtf8();

    //串口发送数据
    if (m_serialPort->isOpen()) {
        m_serialPort->write(command_byte);
        qDebug() << command_byte;
    } else {
        qDebug() << "CSerialPort:串口未打开";
    }

}

/**
 * @brief 接收串口数据
 * @param
 */
void CSerialPort::read(){
    QByteArray info = m_serialPort->readAll();
    if(!info.isEmpty()){
        qDebug()<<"CSerialPort:receive info:" << info;
    }
}
