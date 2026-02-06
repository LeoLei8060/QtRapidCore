#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#define LFQ_NODE_COUNT 1024
#define LFQ_NODE_SIZE  3000

/**
 * @brief: 单生产者单消费者无锁队列 (SPSC Lock-Free Queue)
 * @author: Leo
 * 
 * 特性：
 * - 支持单生产者单消费者模式
 * - 使用连续内存布局，每个节点包含4字节头部+用户数据
 * - 头部4字节使用原子操作，高位表示状态，低31位表示数据长度
 * - 环形缓冲区设计
 */
class LockFreeQueue
{
public:
    /**
     * @brief 构造函数
     */
    LockFreeQueue();

    /**
     * @brief 析构函数
     */
    ~LockFreeQueue();

    /**
     * @brief 初始化队列
     * @param node_count 节点数量
     * @param node_size 每个节点的数据大小（不包含4字节头部）
     * @return true 初始化成功，false 初始化失败
     */
    bool initialize(uint32_t node_count, uint32_t node_size);

    /**
     * @brief 获取待消费的节点数
     * @return 待消费的节点数
     */
    uint32_t get_pending_count() const;

    /**
     * @brief 获取生产者指针
     * @return 生产者数据写入指针，如果队列满返回nullptr
     */
    char *get_producer_pointer();

    /**
     * @brief 设置数据大小并移动生产者指针
     * @param data_size 实际有效数据的大小
     */
    void set_data_size(uint32_t data_size);

    /**
     * @brief 获取消费者指针
     * @return 消费者数据读取指针，如果没有数据返回nullptr
     */
    char *get_consumer_pointer();

    /**
     * @brief 移动消费者指针
     */
    void move_consumer_pointer();

    /**
     * @brief 重置队列
     */
    void reset();

    /**
     * @brief 检查队列是否为空
     * @return true 队列为空，false 队列不为空
     */
    bool is_empty() const;

    /**
     * @brief 检查队列是否已满
     * @return true 队列已满，false 队列未满
     */
    bool is_full() const;

    /**
     * @brief 获取队列容量
     * @return 队列容量（节点数量）
     */
    uint32_t capacity() const;

private:
    // 节点结构
    struct Node
    {
        std::atomic<uint32_t> size_and_state; // 状态+数据长度
        char                  data[0];        // 柔性数组，实际数据区
    };

    // 状态常量
    static const uint32_t STATE_MASK = 0x80000000;  // 最高位：状态标志
    static const uint32_t SIZE_MASK = 0x7FFFFFFF;   // 低31位：数据长度
    static const uint32_t STATE_READY = 0x80000000; // 数据就绪状态

    // 成员变量
    char    *m_buffer;          // 队列内存缓冲区
    uint32_t m_node_count;      // 节点数量
    uint32_t m_node_size;       // 单个节点数据大小
    uint32_t m_total_node_size; // 单个节点总大小（包含头部）

    Node *m_head; // 队列头节点（不移动）
    Node *m_tail; // 队列尾节点（不移动）

    std::atomic<uint32_t> m_producer_index; // 生产者索引
    std::atomic<uint32_t> m_consumer_index; // 消费者索引

    /**
     * @brief 根据索引获取节点指针
     * @param index 节点索引
     * @return 节点指针
     */
    Node *get_node_by_index(uint32_t index) const;

    /**
     * @brief 检查数据是否就绪
     * @param node 节点指针
     * @param data_size 输出参数：数据大小
     * @return true 数据就绪，false 数据未就绪
     */
    bool is_data_ready(Node *node, uint32_t &data_size) const;

    /**
     * @brief 标记节点为已消费
     * @param node 节点指针
     */
    void mark_consumed(Node *node);
};

using LockFreeQueuePtr = std::shared_ptr<LockFreeQueue>;
