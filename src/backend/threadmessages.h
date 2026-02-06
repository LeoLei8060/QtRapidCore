#ifndef THREADMESSAGES_H
#define THREADMESSAGES_H
/**
  * @brief: 自定义的线程间通信消息类
  * @author: Leo
  */

#include "core/messaging/ithreadmessage.h"
#include "core/messaging/threadmessagequeue.h"
#include <QSharedPointer>

/**
  * @example
  * @brief: 仿真指令消息：开始、停止、暂停、继续
class SimulationCmdMsg : public IThreadMessage
{
public:
    enum CmdType { kStartSimulation = 0, kStopSimulation, kPauseSimulation, kContinueSimulation };
    explicit SimulationCmdMsg() { setMsgType(ThdMSGType::kSimulationCmdMsg); }

    CmdType  m_cmdType;
    PlanInfo m_planInfo;
};
using SimulationCmdMsgPtr = QSharedPointer<SimulationCmdMsg>;
  */

#endif // THREADMESSAGES_H
