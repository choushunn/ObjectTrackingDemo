#include "appevent.h"

AppEvent::AppEvent(QObject *parent)
    : QObject{parent}
{
    //    全局事件中转处理类 appevent.h 用来中转系统中各种跨多个UI以及多个类的事件。
    //    此类必须是全局单例类，便于全局统一使用。
    //    比如类a的父类是b，类b的父类是c，现在有个信号要发给类d，在没有事件中转处理的情况下的做法是将a信号发给b，b再发给c，c再发给d，如果父类嵌套层级越多越复杂，代码越难管理。
    //    将类a的信号发给appevent类，然后类d直接关联appevent类进行处理就行。
    //    项目越大，会越发现事件中转处理的必要性，代码清晰，管理方便。
}

/**
 * @brief 处理frame
 * @param frame
 */
void AppEvent::processFrame(cv::Mat frame)
{
    qDebug() << "AppEvent:process frame.";
//    cv::blur(frame, frame,cv::Size(5, 5));
    emit sendProcessFrame(cvMatToQImage(frame));
}
