#include "simulationworker.h"
#include <memory>

SimulationWorker::SimulationWorker() {}

SimulationWorker::~SimulationWorker() {}

void SimulationWorker::work()
{
    IThreadMessagePtr msg;
    while (!shouldStop()) {
        if (m_msgQueuePtr && m_msgQueuePtr->tryPop(msg)) {
            if (msg->getMsgType() == ThdMSGType::kSimulationCmdMsg) {
                SimulationCmdMsgPtr simMsg = msg.dynamicCast<SimulationCmdMsg>();
                handleSimulationCmdMsg(simMsg);
            }
        }
    }
}

void SimulationWorker::handleSimulationCmdMsg(SimulationCmdMsgPtr msg) {}
