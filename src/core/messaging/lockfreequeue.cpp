#include "lockfreequeue.h"
#include <cstdlib>
#include <new>
#include <QDebug>

#ifdef _WIN32
#include <malloc.h> // for _aligned_malloc/_aligned_free on Windows
#else
#include <cstdlib> // for posix_memalign on Unix/Linux
#endif

/**
 * @brief 跨平台的内存对齐分配函数
 * @param alignment 对齐字节数，必须是2的幂
 * @param size 要分配的内存大小
 * @return 对齐的内存指针，失败返回nullptr
 */
static void *aligned_malloc(size_t alignment, size_t size)
{
#ifdef _WIN32
    // Windows: 使用_aligned_malloc
    return _aligned_malloc(size, alignment);
#elif defined(__APPLE__) || defined(__linux__)
    // macOS/Linux: 使用posix_memalign
    void *ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) == 0) {
        return ptr;
    }
    return nullptr;
#else
    // 其他平台：手动对齐实现
    // 分配额外的内存用于对齐
    size_t total_size = size + alignment + sizeof(void *);
    void  *raw_ptr = std::malloc(total_size);
    if (!raw_ptr) {
        return nullptr;
    }

    // 计算对齐后的地址
    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw_ptr);
    uintptr_t aligned_addr = (raw_addr + sizeof(void *) + alignment - 1) & ~(alignment - 1);
    void     *aligned_ptr = reinterpret_cast<void *>(aligned_addr);

    // 在对齐地址前存储原始指针，用于释放
    void **ptr_storage = reinterpret_cast<void **>(aligned_addr) - 1;
    *ptr_storage = raw_ptr;

    return aligned_ptr;
#endif
}

/**
 * @brief 跨平台的对齐内存释放函数
 * @param ptr 要释放的对齐内存指针
 */
static void aligned_free(void *ptr)
{
    if (!ptr) {
        return;
    }

#ifdef _WIN32
    _aligned_free(ptr);
#elif defined(__APPLE__) || defined(__linux__)
    std::free(ptr);
#else
    // 其他平台：从对齐地址前获取原始指针并释放
    void **ptr_storage = reinterpret_cast<void **>(ptr) - 1;
    std::free(*ptr_storage);
#endif
}

LockFreeQueue::LockFreeQueue()
    : m_buffer(nullptr)
    , m_node_count(0)
    , m_node_size(0)
    , m_total_node_size(0)
    , m_head(nullptr)
    , m_tail(nullptr)
    , m_producer_index(0)
    , m_consumer_index(0)
{}

LockFreeQueue::~LockFreeQueue()
{
    if (m_buffer) {
        aligned_free(m_buffer);
        m_buffer = nullptr;
    }
}

bool LockFreeQueue::initialize(uint32_t node_count, uint32_t node_size)
{
    // 参数校验
    if (node_count == 0 || node_size == 0) {
        return false;
    }

    // 避免重复初始化
    if (m_buffer != nullptr) {
        return false;
    }

    // 计算单个节点总大小（4字节头部 + 用户数据大小）
    m_total_node_size = sizeof(uint32_t) + node_size;

    // 内存对齐到8字节边界，提高性能
    if (m_total_node_size % 8 != 0) {
        m_total_node_size = (m_total_node_size + 7) & ~7;
    }

    // 计算总内存大小
    size_t total_size = static_cast<size_t>(node_count) * m_total_node_size;

    // 分配内存
    m_buffer = static_cast<char *>(aligned_malloc(64, total_size)); // 64字节对齐，避免false sharing
    if (!m_buffer) {
        return false;
    }

    // 初始化成员变量
    m_node_count = node_count;
    m_node_size = node_size;

    // 设置头尾指针
    m_head = reinterpret_cast<Node *>(m_buffer);
    m_tail = reinterpret_cast<Node *>(m_buffer + (node_count - 1) * m_total_node_size);

    // 重置队列状态
    reset();

    return true;
}

void LockFreeQueue::reset()
{
    if (!m_buffer) {
        return;
    }

    // 将所有节点的状态设置为空（size_and_state = 0）
    for (uint32_t i = 0; i < m_node_count; ++i) {
        Node *node = get_node_by_index(i);
        node->size_and_state.store(0, std::memory_order_relaxed);
    }

    // 重置生产者和消费者索引
    m_producer_index.store(0, std::memory_order_relaxed);
    m_consumer_index.store(0, std::memory_order_relaxed);
}

LockFreeQueue::Node *LockFreeQueue::get_node_by_index(uint32_t index) const
{
    // 环形索引计算
    uint32_t real_index = index % m_node_count;
    char    *node_ptr = m_buffer + real_index * m_total_node_size;
    return reinterpret_cast<Node *>(node_ptr);
}

bool LockFreeQueue::is_data_ready(Node *node, uint32_t &data_size) const
{
    uint32_t value = node->size_and_state.load(std::memory_order_acquire);
    if (value & STATE_MASK) {
        data_size = value & SIZE_MASK;
        return true;
    }
    return false;
}

void LockFreeQueue::mark_consumed(Node *node)
{
    node->size_and_state.store(0, std::memory_order_release);
}

char *LockFreeQueue::get_producer_pointer()
{
    if (!m_buffer) {
        return nullptr;
    }

    uint32_t current_producer = m_producer_index.load(std::memory_order_relaxed);
    uint32_t current_consumer = m_consumer_index.load(std::memory_order_acquire);

    // 检查队列是否已满
    // 如果生产者索引的下一个位置等于消费者索引，说明队列满了
    uint32_t next_producer = (current_producer + 1) % m_node_count;
    if (next_producer == current_consumer) {
        return nullptr; // 队列满
    }

    Node *producer_node = get_node_by_index(current_producer);
    return producer_node->data;
}

void LockFreeQueue::set_data_size(uint32_t data_size)
{
    if (!m_buffer) {
        return;
    }

    // 数据大小不能超过最大限制（31位）
    if (data_size > SIZE_MASK) {
        data_size = SIZE_MASK;
    }

    uint32_t current_producer = m_producer_index.load(std::memory_order_relaxed);
    Node    *producer_node = get_node_by_index(current_producer);

    // 使用原子操作设置数据就绪状态和数据长度
    uint32_t value = STATE_READY | (data_size & SIZE_MASK);
    producer_node->size_and_state.store(value, std::memory_order_release);

    // 移动生产者指针到下一个节点
    uint32_t next_producer = (current_producer + 1) % m_node_count;
    m_producer_index.store(next_producer, std::memory_order_release);
}

char *LockFreeQueue::get_consumer_pointer()
{
    if (!m_buffer) {
        return nullptr;
    }

    uint32_t current_consumer = m_consumer_index.load(std::memory_order_relaxed);
    Node    *consumer_node = get_node_by_index(current_consumer);

    uint32_t data_size;
    if (is_data_ready(consumer_node, data_size)) {
        return consumer_node->data;
    }

    return nullptr; // 没有数据可消费
}

void LockFreeQueue::move_consumer_pointer()
{
    if (!m_buffer) {
        return;
    }

    uint32_t current_consumer = m_consumer_index.load(std::memory_order_relaxed);
    Node    *consumer_node = get_node_by_index(current_consumer);

    // 标记当前节点为已消费
    mark_consumed(consumer_node);

    // 移动消费者指针到下一个节点
    uint32_t next_consumer = (current_consumer + 1) % m_node_count;
    m_consumer_index.store(next_consumer, std::memory_order_release);
}

uint32_t LockFreeQueue::get_pending_count() const
{
    if (!m_buffer) {
        return 0;
    }

    uint32_t producer_idx = m_producer_index.load(std::memory_order_acquire);
    uint32_t consumer_idx = m_consumer_index.load(std::memory_order_acquire);

    if (producer_idx >= consumer_idx) {
        return producer_idx - consumer_idx;
    } else {
        // 环形缓冲区，生产者索引回绕的情况
        return m_node_count - consumer_idx + producer_idx;
    }
}

bool LockFreeQueue::is_empty() const
{
    if (!m_buffer) {
        return true;
    }

    uint32_t producer_idx = m_producer_index.load(std::memory_order_acquire);
    uint32_t consumer_idx = m_consumer_index.load(std::memory_order_acquire);

    return producer_idx == consumer_idx;
}

bool LockFreeQueue::is_full() const
{
    if (!m_buffer) {
        return false;
    }

    uint32_t producer_idx = m_producer_index.load(std::memory_order_acquire);
    uint32_t consumer_idx = m_consumer_index.load(std::memory_order_acquire);

    uint32_t next_producer = (producer_idx + 1) % m_node_count;
    return next_producer == consumer_idx;
}

uint32_t LockFreeQueue::capacity() const
{
    return m_node_count;
}
