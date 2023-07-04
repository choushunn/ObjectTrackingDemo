#ifndef APPEVENT_H
#define APPEVENT_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <qdebug.h>


class AppEvent : public QObject
{
    Q_OBJECT

public:
    static AppEvent* instance(); // 获取单例实例
    void sendEvent(const QString& eventName, const QVariantList& args); // 发送事件
    void registerEvent(const QString& eventName, QObject* listener); // 注册事件
    void unregisterEvent(const QString& eventName, QObject* listener); // 注销事件

private:
    AppEvent(QObject *parent = nullptr); // 构造函数私有化，只能通过instance()获取实例
    static AppEvent* m_instance; // 单例实例
    QMap<QString, QList<QObject*>> m_eventListeners; // 事件监听器列表

    // 禁止拷贝和赋值构造函数
    AppEvent(const AppEvent&) = delete;
    AppEvent& operator=(const AppEvent&) = delete;
};


#endif // APPEVENT_H
