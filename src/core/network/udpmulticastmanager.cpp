#include "udpmulticastmanager.h"
#include "app/appconfigservice.h"
#include "core/log/loghelper.h"
#include <condition_variable>
#include <QDebug>
#include <QHostAddress>

namespace Core {

// #define USE_HEARTBEATTIMER

// 静态成员初始化
bool       UdpMulticastManager::s_networkInitialized = false;
std::mutex UdpMulticastManager::s_initMutex;

UdpMulticastManager::UdpMulticastManager(QObject *parent)
    : QObject(parent)
    , m_socket(INVALID_SOCKET)
    , m_port(0)
    , m_connected(false)
    , m_stopThreads(true)
    , m_heartbeatTimer(new QTimer(this))
{
    // 初始化网络
    initializeNetwork();

#ifdef USE_HEARTBEATTIMER
    // 设置心跳定时器
    m_heartbeatTimer->setInterval(5000); // 5秒心跳
    connect(m_heartbeatTimer, &QTimer::timeout, this, &UdpMulticastManager::onHeartbeatTimer);
#endif

    LOGINFO("UdpMulticastManager created");
}

UdpMulticastManager::~UdpMulticastManager()
{
    disconnectFromMulticast();
    cleanupNetwork();
    LOGINFO("UdpMulticastManager destroyed");
}

bool UdpMulticastManager::initializeNetwork()
{
    std::lock_guard<std::mutex> lock(s_initMutex);

    if (s_networkInitialized) {
        return true;
    }

#ifdef _WIN32
    WSADATA wsaData;
    int     result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        LOGWARN("WSAStartup failed with error: %1", result);
        return false;
    }
#endif

    s_networkInitialized = true;
    LOGINFO("Network initialized successfully");
    return true;
}

void UdpMulticastManager::cleanupNetwork()
{
    std::lock_guard<std::mutex> lock(s_initMutex);

    if (!s_networkInitialized) {
        return;
    }

#ifdef _WIN32
    WSACleanup();
#endif

    s_networkInitialized = false;
    LOGINFO("Network cleaned up");
}

bool UdpMulticastManager::connectToMulticast(const QString &multicastAddress, int port)
{
    if (m_connected) {
        return false;
    }

    if (!isValidMulticastAddress(multicastAddress)) {
        emit errorOccurred("Invalid multicast address: " + multicastAddress);
        return false;
    }

    if (port <= 0 || port > 65535) {
        emit errorOccurred("Invalid port: " + QString::number(port));
        return false;
    }

    m_multicastAddress = multicastAddress;
    m_port = port;

    // 创建socket
    if (!createSocket()) {
        emit errorOccurred("Failed to create socket");
        return false;
    }

    // 加入组播组
    if (!joinMulticastGroup()) {
        closeSocket();
        emit errorOccurred("Failed to join multicast group");
        return false;
    }

    LOGINFO("Connected to multicast %1:%2", multicastAddress, port);
    return true;
}

void UdpMulticastManager::disconnectFromMulticast()
{
    if (!m_connected) {
        return;
    }

    LOGINFO("Disconnecting from multicast...");

#ifdef USE_HEARTBEATTIMER
    // 停止心跳定时器
    m_heartbeatTimer->stop();
#endif

    // 停止线程
    m_stopThreads.store(true);
    m_sendCondition.notify_all();

    if ((m_role & kReceiver) && m_receiveThread && m_receiveThread->joinable()) {
        m_receiveThread->join();
    }

    if ((m_role & kSender) && m_sendThread && m_sendThread->joinable()) {
        m_sendThread->join();
    }

    // 清理网络资源
    leaveMulticastGroup();
    closeSocket();

    // 清空发送队列
    {
        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        while (!m_sendQueue.empty()) {
            m_sendQueue.pop();
        }
    }

    m_connected = false;
    emit disconnected();

    LOGINFO("Disconnected from multicast");
}

bool UdpMulticastManager::startThread()
{
    if (!m_stopThreads.load())
        return false;
    // 启动线程
    m_stopThreads.store(false);
    if (m_role & kReceiver)
        m_receiveThread = std::make_unique<std::thread>(&UdpMulticastManager::receiveThreadFunc,
                                                        this);
    if (m_role & kSender)
        m_sendThread = std::make_unique<std::thread>(&UdpMulticastManager::sendThreadFunc, this);

#ifdef USE_HEARTBEATTIMER
    // 启动心跳定时器
    m_heartbeatTimer->start();
#endif

    m_connected = true;
    emit connected();
    return true;
}

bool UdpMulticastManager::createSocket()
{
    // 创建UDP socket
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == INVALID_SOCKET) {
        LOGWARN("Failed to create socket: %1", getLastErrorString());
        return false;
    }

    // 设置socket选项：允许地址重用
    int reuse = 1;
    if (setsockopt(m_socket,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   reinterpret_cast<const char *>(&reuse),
                   sizeof(reuse))
        == SOCKET_ERROR) {
        LOGWARN("Failed to set SO_REUSEADDR: %1", getLastErrorString());
        closeSocket();
        return false;
    }

    // 绑定到本地地址
    sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(m_port);

    if (bind(m_socket, reinterpret_cast<sockaddr *>(&localAddr), sizeof(localAddr))
        == SOCKET_ERROR) {
        LOGWARN("Failed to bind socket: %1", getLastErrorString());
        closeSocket();
        return false;
    }

    LOGINFO("Socket created and bound successfully");
    return true;
}

void UdpMulticastManager::closeSocket()
{
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        LOGINFO("Socket closed");
    }
}

bool UdpMulticastManager::joinMulticastGroup()
{
    // 准备组播地址结构
    memset(&m_multicastAddr, 0, sizeof(m_multicastAddr));
    m_multicastAddr.sin_family = AF_INET;
    m_multicastAddr.sin_port = htons(m_port);

    // 使用inet_addr替代inet_pton以提供更好的跨平台兼容性
    m_multicastAddr.sin_addr.s_addr = inet_addr(m_multicastAddress.toUtf8().constData());
    if (m_multicastAddr.sin_addr.s_addr == INADDR_NONE) {
        LOGWARN("Invalid multicast address format: %1", m_multicastAddress);
        return false;
    }

    // 设置接收缓存
    int recvBufferSize = 64 * 1024 * 1024;
    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *) &recvBufferSize, sizeof(recvBufferSize))
        == SOCKET_ERROR) {
        LOGWARN("Failed to set recv buffer size: %1", getLastErrorString());
        return false;
    }

    // 加入组播组
    ip_mreq mreq;
    mreq.imr_multiaddr = m_multicastAddr.sin_addr;
#ifndef BIND_LOCAL_IP
    mreq.imr_interface.s_addr = INADDR_ANY;
#else
    mreq.imr_interface.s_addr = inet_addr(
        AppConfigService::instance()->getLocalAddress().toStdString().c_str());
#endif

    if (setsockopt(m_socket,
                   IPPROTO_IP,
                   IP_ADD_MEMBERSHIP,
                   reinterpret_cast<const char *>(&mreq),
                   sizeof(mreq))
        == SOCKET_ERROR) {
        LOGWARN("Failed to join multicast group: %1", getLastErrorString());
        return false;
    }

    // 设置组播TTL
    int ttl = 64;
    if (setsockopt(m_socket,
                   IPPROTO_IP,
                   IP_MULTICAST_TTL,
                   reinterpret_cast<const char *>(&ttl),
                   sizeof(ttl))
        == SOCKET_ERROR) {
        LOGWARN("Failed to set multicast TTL: %1", getLastErrorString());
        // 不是致命错误，继续执行
    }

    // 设置组播回环
    int loop = 1;
    if (setsockopt(m_socket,
                   IPPROTO_IP,
                   IP_MULTICAST_LOOP,
                   reinterpret_cast<const char *>(&loop),
                   sizeof(loop))
        == SOCKET_ERROR) {
        LOGWARN("Failed to set multicast loop: %1", getLastErrorString());
        // 不是致命错误，继续执行
    }

// 设置发送组播的本地端口
#ifdef BIND_LOCAL_IP
    struct in_addr local_interface;
    local_interface.s_addr = inet_addr(
        AppConfigService::instance()->getLocalAddress().toStdString().c_str());
    if (setsockopt(m_socket,
                   IPPROTO_IP,
                   IP_MULTICAST_IF,
                   (char *) &local_interface,
                   sizeof(local_interface))
        == SOCKET_ERROR) {
        LOGWARN("Failed to set IP_MULTICAST_IF: %1", getLastErrorString());
    }
#endif

    LOGINFO("Joined multicast group successfully");
    return true;
}

void UdpMulticastManager::leaveMulticastGroup()
{
    if (m_socket == INVALID_SOCKET) {
        return;
    }

    ip_mreq mreq;
    mreq.imr_multiaddr = m_multicastAddr.sin_addr;
#ifndef BIND_LOCAL_IP
    mreq.imr_interface.s_addr = INADDR_ANY;
#else
    mreq.imr_interface.s_addr = inet_addr(
        AppConfigService::instance()->getLocalAddress().toStdString().c_str());
#endif

    if (setsockopt(m_socket,
                   IPPROTO_IP,
                   IP_DROP_MEMBERSHIP,
                   reinterpret_cast<const char *>(&mreq),
                   sizeof(mreq))
        == SOCKET_ERROR) {
        LOGWARN("Failed to leave multicast group: %1", getLastErrorString());
    } else {
        LOGINFO("Leave multicast group successfully");
    }
}

void UdpMulticastManager::receiveThreadFunc()
{
    char *buffer = nullptr;

    while (!m_stopThreads.load()) {
        if ((buffer = m_dataQueue->get_producer_pointer()) == nullptr) {
            LOGINFO("The producer thread queue is full.");
            // 优化：队列满时休眠5毫秒，避免忙等待
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        sockaddr_in senderAddr;
        socklen_t   senderAddrLen = sizeof(senderAddr);

        // 设置接收超时，避免线程无法停止
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(m_socket, &readSet);

        timeval timeout;
        timeout.tv_sec = 0; // 1秒超时
        timeout.tv_usec = 1000;

        int selectResult = select(m_socket + 1, &readSet, nullptr, nullptr, &timeout);

        if (selectResult == SOCKET_ERROR) {
            if (!m_stopThreads.load()) {
                emit errorOccurred("Select error: " + getLastErrorString());
            }
            continue;
        }

        if (selectResult <= 0) {
            // 超时，继续循环检查停止标志
            continue;
        }

        // TODO: 这里接收数据的缓存需要通过环形队列来得到
        // 接收数据
        int bytesReceived = recvfrom(m_socket,
                                     buffer,
                                     LFQ_NODE_SIZE,
                                     0,
                                     reinterpret_cast<sockaddr *>(&senderAddr),
                                     &senderAddrLen);

        if (bytesReceived == SOCKET_ERROR) {
            if (!m_stopThreads.load()) {
                emit errorOccurred("Receive error: " + getLastErrorString());
            }
            continue;
        }

        if (bytesReceived > 0) {
            // TODO: 环形队列的生产者线程移动到下个节点
            m_dataQueue->set_data_size(bytesReceived);
        }
    }
}

void UdpMulticastManager::sendThreadFunc()
{
    while (!m_stopThreads.load()) {
        std::unique_lock<std::mutex> lock(m_sendQueueMutex);

        // 等待有数据要发送或者线程停止
        m_sendCondition.wait(lock, [this] { return !m_sendQueue.empty() || m_stopThreads.load(); });

        if (m_stopThreads.load()) {
            break;
        }

        // 取出要发送的数据
        QByteArray data = m_sendQueue.front();
        m_sendQueue.pop();
        lock.unlock();

        // 发送数据
        int bytesSent = sendto(m_socket,
                               data.constData(),
                               data.size(),
                               0,
                               reinterpret_cast<const sockaddr *>(&m_multicastAddr),
                               sizeof(m_multicastAddr));

        if (bytesSent == SOCKET_ERROR) {
            if (!m_stopThreads.load()) {
                emit errorOccurred("Send error: " + getLastErrorString());
            }
        } else if (bytesSent != data.size()) {
            LOGWARN("Partial send: sent %1 of %2 bytes", bytesSent, data.size());
        } else {
            LOGINFO("Sent %1 bytes successfully", bytesSent);
        }
    }
}

bool UdpMulticastManager::sendData(const QByteArray &data)
{
    if (!m_connected || !(m_role & kSender)) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        m_sendQueue.push(data);
    }

    m_sendCondition.notify_one();
    return true;
}

void UdpMulticastManager::onHeartbeatTimer()
{
    if (m_connected) {
        LOGINFO("Heartbeat: Connected to %1:%2", m_multicastAddress, m_port);
    }
}

QString UdpMulticastManager::getLastErrorString()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return QString("Error code: %1").arg(error);
#else
    return QString::fromLocal8Bit(strerror(errno));
#endif
}

bool UdpMulticastManager::isValidMulticastAddress(const QString &address)
{
    QHostAddress hostAddr(address);
    if (hostAddr.isNull()) {
        return false;
    }

    quint32 addr = hostAddr.toIPv4Address();
    // 组播地址范围：224.0.0.0 到 239.255.255.255
    return (addr >= 0xE0000000) && (addr <= 0xEFFFFFFF);
}
} // namespace Core
