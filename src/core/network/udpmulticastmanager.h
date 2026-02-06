#ifndef UDPMULTICASTMANAGER_H
#define UDPMULTICASTMANAGER_H

/**
 * @brief UDP组播管理器类
 * @details 负责UDP组播数据的接收和发送，使用多线程避免阻塞
 */

#include "core/messaging/lockfreequeue.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <QObject>
#include <QString>
#include <QTimer>
#include <queue>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // Windows Vista or later for advanced multicast support
#endif
#ifndef WINVER
#define WINVER 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET         int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define closesocket    close
#endif

namespace Core {

class UdpMulticastManager : public QObject
{
    Q_OBJECT

public:
    enum Role {
        kNone = 0,
        kReceiver = 1 << 0,                // 接收者，只启动接收线程
        kSender = 1 << 1,                  // 发送者，只启动发送线程
        kTransceiver = kReceiver | kSender // 收发者，接收和发送线程都启动
    };
    explicit UdpMulticastManager(QObject *parent = nullptr);
    ~UdpMulticastManager();

    // 连接和断开
    bool connectToMulticast(const QString &multicastAddress, int port);
    void disconnectFromMulticast();

    bool startThread();

    void setRole(Role role) { m_role = role; }
    Role getRole() const { return m_role; }

    // 关联接收数据的环形无锁队列
    void setDataQueue(LockFreeQueuePtr queue) { m_dataQueue = queue; }

    // 发送数据
    bool sendData(const QByteArray &data);

    // 状态查询
    bool    isConnected() const { return m_connected; }
    QString getMulticastAddress() const { return m_multicastAddress; }
    int     getPort() const { return m_port; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);
    void dataReceived(const QByteArray &data);

private slots:
    void onHeartbeatTimer();

private:
    // 网络初始化和清理
    bool initializeNetwork();
    void cleanupNetwork();

    // Socket操作
    bool createSocket();
    void closeSocket();
    bool joinMulticastGroup();
    void leaveMulticastGroup();

    // 线程函数
    void receiveThreadFunc();
    void sendThreadFunc();

    // 工具函数
    QString getLastErrorString();
    bool    isValidMulticastAddress(const QString &address);

private:
    // 网络相关
    SOCKET            m_socket;
    sockaddr_in       m_multicastAddr;
    QString           m_multicastAddress;
    int               m_port;
    std::atomic<bool> m_connected;

    // 线程相关
    std::unique_ptr<std::thread> m_receiveThread;
    std::unique_ptr<std::thread> m_sendThread;
    std::atomic<bool>            m_stopThreads;
    Role                         m_role{kTransceiver}; // 默认为收发者

    LockFreeQueuePtr m_dataQueue;

    // 发送队列
    std::queue<QByteArray>  m_sendQueue;
    std::mutex              m_sendQueueMutex;
    std::condition_variable m_sendCondition;

    // 心跳定时器
    QTimer *m_heartbeatTimer;

    // 网络初始化标志
    static bool       s_networkInitialized;
    static std::mutex s_initMutex;
};

using UdpMulticastManagerPtr = QSharedPointer<UdpMulticastManager>;
} // namespace Core

#endif // UDPMULTICASTMANAGER_H
