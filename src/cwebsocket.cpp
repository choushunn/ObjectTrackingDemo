#include "cwebsocket.h"

/**
 * @brief CWebSocket 构造函数
 * @param
 */
CWebSocket::CWebSocket(quint16 port, bool debug, QObject *parent)
    : QObject{parent},
      m_debug(debug),
      m_port(port)
{
    //初始化
    m_webSocketServer = new QWebSocketServer(QStringLiteral("WebSocketServer"), QWebSocketServer::NonSecureMode, this);
    //在线测试地址
    //https://www.hake.cc/tools/websocket/
}


/**
 * @brief 启动监听
 * @param
 */
void CWebSocket::startListen()
{
    if(m_webSocketServer->listen(QHostAddress::Any, m_port))
    {
        qDebug() << "Chat Server listening on port " << m_port;
        connect(m_webSocketServer, &QWebSocketServer::newConnection, this, &CWebSocket::onNewConnection);
    }
}

/**
 * @brief 解析客户端信息
 * @param peer    接收到的QWebSocket
 */
QString CWebSocket::getIdentifier(QWebSocket *peer)
{
    return QStringLiteral("%1:%2").arg(peer->peerAddress().toString(),
                                       QString::number(peer->peerPort()));
}

/**
 * @brief 建立新连接槽函数
 * @param
 */
void CWebSocket::onNewConnection(){
    auto pSocket = m_webSocketServer->nextPendingConnection();
    qDebug() << "Client:" << getIdentifier(pSocket) << "connected!";
    pSocket->setParent(this);
    //对连接进来的每一个进行信号槽连接绑定
    connect(pSocket, &QWebSocket::textMessageReceived, this, &CWebSocket::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &CWebSocket::processBinaryMessage);
    //断开
    connect(pSocket, &QWebSocket::disconnected, this, &CWebSocket::socketDisconnected);
    // 使用 list 进行管理，方便断开
    m_clients << pSocket;
}


/**
 * @brief 接收文本信息槽函数
 * @param
 */
void CWebSocket::processTextMessage(const QString &message)
{
    emit sendTextMessage(message);
    QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());
    if (m_debug){
        qDebug() << "Message received:" << message;
    }
    for (QWebSocket *pClient : qAsConst(m_clients)) {
        if (pClient == pSender) //don't echo message back to sender
        {
            pClient->sendTextMessage("Host Echo@" + message);
            qDebug() << "Peer Address:" << pClient->peerAddress().toString();
        }
    }
}


/**
 * @brief 接收二进制信息槽函数
 * @param
 */
void CWebSocket::processBinaryMessage(QByteArray message)
{
    emit sendBinaryMessage(message);
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Binary Message received:" << message;
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}


/**
 * @brief 断开连接
 * @param
 */
void CWebSocket::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << getIdentifier(pClient) << " disconnected!";
    if (pClient)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
