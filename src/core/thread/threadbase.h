#pragma once

/**
  * @brief: 工作线程的基类
  * @author: Leo
  */

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>

class ThreadBase
{
public:
    ThreadBase();
    virtual ~ThreadBase();

    // 删除拷贝构造和赋值
    ThreadBase(const ThreadBase &) = delete;
    ThreadBase &operator=(const ThreadBase &) = delete;

    void start();

    void stop();

    void requestStop() { m_stopFlag = true; }

    void join();

    bool shouldStop() const { return m_stopFlag; }

protected:
    virtual void work() = 0; // 纯虚函数，子类实现具体逻辑

private:
    void threadFunc();

    std::thread       m_thread;
    std::atomic<bool> m_stopFlag;
};
