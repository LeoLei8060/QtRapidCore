#include "threadbase.h"

ThreadBase::ThreadBase()
    : m_stopFlag(false)
{}

ThreadBase::~ThreadBase()
{
    if (m_thread.joinable()) {
        // 安全终止策略：实际项目中建议记录错误日志
        std::cerr << "ERROR: Thread not stopped before destruction!" << std::endl;
        std::terminate(); // 或自定义安全处理逻辑
    }
}

void ThreadBase::start()
{
    if (m_thread.joinable()) {
        throw std::runtime_error("Thread already running");
    }
    m_stopFlag = false;
    m_thread = std::thread(&ThreadBase::threadFunc, this);
}

void ThreadBase::stop()
{
    requestStop();
    join();
}

void ThreadBase::join()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void ThreadBase::threadFunc()
{
    try {
        work(); // 执行子类工作逻辑
    } catch (const std::exception &e) {
        // 异常处理（实际项目应记录日志）
        std::cerr << "Thread exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown thread exception" << std::endl;
    }
}
