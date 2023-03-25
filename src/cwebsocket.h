#ifndef CWEBSOCKET_H
#define CWEBSOCKET_H

#include <QObject>
#include <QtWebSockets>
#include <QWebSocketServer>

class CWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit CWebSocket(quint16 port, bool debug = false, QObject *parent = nullptr);
    void     startListen();

private:
    QWebSocketServer    *m_webSocketServer;
    QString             m_serverName = "WebSocketServer";
    QList<QWebSocket *> m_clients;
    bool                m_debug;
    quint16             m_port;
    QString getIdentifier(QWebSocket *peer);

private slots:
    void onNewConnection();
    void processTextMessage(const QString &message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

signals:
    void sendTextMessage(const QString &message);
    void sendBinaryMessage(QByteArray message);

};

#endif // CWEBSOCKET_H
