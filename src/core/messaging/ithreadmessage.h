#ifndef ITHREADMESSAGE_H
#define ITHREADMESSAGE_H

/**
 * @brief: 线程间通信的消息定义，所有自定义消息类型都必须继承IThreadMessage
 * @author: Leo
 */
#include <QSharedPointer>

enum class ThdMSGType {
    kSimulationCmdMsg = 0, // 目标信息数据库相关消息

    kUnknow
};

// 消息基类
class IThreadMessage
{
public:
    virtual ~IThreadMessage() = default;

    void       setMsgType(const ThdMSGType &type) { m_type = type; }
    ThdMSGType getMsgType() const { return m_type; }

private:
    ThdMSGType m_type;
};
using IThreadMessagePtr = QSharedPointer<IThreadMessage>;

#endif // ITHREADMESSAGE_H
