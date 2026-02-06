#ifndef SIMULATIONWORKER_H
#define SIMULATIONWORKER_H

#include "core/messaging/ithreadmessage.h"
#include "core/messaging/threadmessagequeue.h"
#include "core/thread/threadbase.h"
#include "threadmessages.h"

class SimulationWorker : public ThreadBase
{
public:
    SimulationWorker();
    ~SimulationWorker();

    void setMsgQueue(ThreadMessageQueuePtr queue) { m_msgQueuePtr = queue; }

protected:
    void work() override;

    void handleSimulationCmdMsg(SimulationCmdMsgPtr msg);

private:
    ThreadMessageQueuePtr m_msgQueuePtr;
};

#endif // SIMULATIONWORKER_H
