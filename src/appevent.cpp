#include "appevent.h"

//    全局事件中转处理类 appevent.h 用来中转系统中各种跨多个UI以及多个类的事件。
//    此类必须是全局单例类，便于全局统一使用。
//    比如类a的父类是b，类b的父类是c，现在有个信号要发给类d，在没有事件中转处理的情况下的做法是将a信号发给b，b再发给c，c再发给d，如果父类嵌套层级越多越复杂，代码越难管理。
//    将类a的信号发给appevent类，然后类d直接关联appevent类进行处理就行。
//    项目越大，会越发现事件中转处理的必要性，代码清晰，管理方便。


AppEvent* AppEvent::m_instance = nullptr;

AppEvent* AppEvent::instance()
{
    if (m_instance == nullptr) {
        m_instance = new AppEvent();
    }
    return m_instance;
}

void AppEvent::sendEvent(const QString& eventName, const QVariantList& args)
{
    // 查找事件监听器列表中是否存在该事件
    if (m_eventListeners.contains(eventName)) {
        // 遍历事件监听器列表，发送事件给所有监听器
        for (QObject* listener : m_eventListeners[eventName]) {
            // 发送事件给监听器
            QMetaObject::invokeMethod(listener, qPrintable(eventName), Qt::QueuedConnection, Q_ARG(QVariantList, args));
        }
    }
}

void AppEvent::registerEvent(const QString& eventName, QObject* listener)
{
    // 如果事件监听器列表中不存在该事件，则创建一个空的监听器列表
    if (!m_eventListeners.contains(eventName)) {
        m_eventListeners[eventName] = QList<QObject*>();
    }

    // 将监听器添加到该事件的监听器列表中
    m_eventListeners[eventName].append(listener);
}

void AppEvent::unregisterEvent(const QString& eventName, QObject* listener)
{
    // 如果事件监听器列表中存在该事件，则从该事件的监听器列表中删除该监听器
    if (m_eventListeners.contains(eventName)) {
        m_eventListeners[eventName].removeAll(listener);
    }
}

AppEvent::AppEvent(QObject *parent)
    : QObject(parent)
{
    // 可以在构造函数中添加初始化代码
}
