#ifndef CSERIALPORT_H
#define CSERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <qdebug.h>
#include "utils.h"


class CSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit CSerialPort(const QString serialPortName, qint32 baud, QObject *parent = nullptr);
    bool open();
    void close();
    void read();
    void sendData(int device_id, int angle);

private:
    QSerialPort* m_serialPort;
    qint32       m_baud;
    QString      m_serialPortName;
signals:

};

#endif // CSERIALPORT_H
