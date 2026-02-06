#ifndef THREADMESSAGEQUEUE_H
#define THREADMESSAGEQUEUE_H

/**
  * @brief: 线程间消息队列模板类，暂时只支持1对1（一个生产者一个消费者）
  * @author: Leo
  */
#include "ithreadmessage.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>

template<typename T>
class ThreadMessageQueue
{
private:
    mutable std::mutex      m_mutex;
    std::queue<T>           m_queue;
    std::condition_variable m_condition;
    std::atomic<bool>       m_shutdown{false};

public:
    ThreadMessageQueue() = default;
    ThreadMessageQueue(const ThreadMessageQueue &) = delete;
    ThreadMessageQueue &operator=(const ThreadMessageQueue &) = delete;

    ~ThreadMessageQueue() { shutdown(); }

    // 生产者：向队列添加消息
    bool push(T item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_shutdown.load()) {
            return false; // 如果已关闭，则不再接受新消息
        }
        m_queue.push(std::move(item));
        m_condition.notify_one();
        return true;
    }

    // 消费者：从队列获取消息（阻塞）
    bool pop(T &item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // 等待直到有消息或者队列被关闭
        m_condition.wait(lock, [this] { return !m_queue.empty() || m_shutdown.load(); });

        if (m_queue.empty()) {
            return false; // 队列已关闭且为空
        }

        item = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    // 消费者：从队列获取消息（非阻塞）
    bool tryPop(T &item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        item = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shutdown.store(true);
        m_condition.notify_all();
    }

    bool isShutdown() const { return m_shutdown.load(); }
};

using ThreadMessageQueuePtr = std::shared_ptr<ThreadMessageQueue<IThreadMessagePtr>>;

#endif // THREADMESSAGEQUEUE_H
