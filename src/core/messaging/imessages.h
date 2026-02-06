#ifndef IMESSAGES_H
#define IMESSAGES_H

/**
  * @brief: 发布订阅机制相关的消息基类和宏定义
  * @author: Leo
  */

#include <QObject>
#include <QSharedPointer>

/////////////////////////////// 后端服务发送给UI渲染的消息基类
enum class UiUpdateMsgType {
    // kXXXToUiMsg = 0,

    kOther
};

class IUIUpdateMsg : public QObject
{
    Q_OBJECT
public:
    explicit IUIUpdateMsg(QObject *parent = nullptr) {}
    virtual ~IUIUpdateMsg() {}

    void            setMsgType(UiUpdateMsgType type) { m_dataType = type; }
    UiUpdateMsgType getMsgType() const { return m_dataType; }

private:
    UiUpdateMsgType m_dataType{UiUpdateMsgType::kOther};
};
using IUIUpdateMsgPtr = QSharedPointer<IUIUpdateMsg>;

/////////////////////////////// UI发送给Presenter的消息基类
enum class PresenterMsgType {
    // kXXXToPMsg = 0,

    kOther
};

class IPresenterMsg : public QObject
{
    Q_OBJECT
public:
    explicit IPresenterMsg(QObject *parent = nullptr) {}
    virtual ~IPresenterMsg() {}

    void             setMsgType(PresenterMsgType type) { m_msgType = type; }
    PresenterMsgType getMsgType() const { return m_msgType; }

private:
    PresenterMsgType m_msgType{PresenterMsgType::kOther};
};
using IPresenterMsgPtr = QSharedPointer<IPresenterMsg>;

/**
  * @brief: 消息类型宏：后端服务处理的数据 发送给 界面的消息
  * @author: Leo
  * @note:
  *         宏对应的字符串规定了槽函数的后缀
  *         由MainWindowPresenter业务类进行处理
  * @param: QSharedPointer<IUIUpdateMsg>类型
  */
#define MSG_TOUI "toUIMsg"

/**
  * @brief: 消息类型宏：界面发送给Presenter的消息
  * @author: Leo
  * @note:
  *         宏对应的字符串规定了槽函数的后缀
  *         由MainWindowPresenter业务类进行处理
  * @param: QSharedPointer<IUIUpdateMsg>类型
  */
#define MSG_TOPRESENTER "toPresenterMsg"

#endif // IMESSAGES_H
