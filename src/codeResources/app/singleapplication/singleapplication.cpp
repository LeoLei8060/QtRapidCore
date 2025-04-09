#include "singleapplication.h"
#include <QLocalSocket>
#include <QDataStream>
#include <QFontDatabase>
#include <QDebug>
#include <QFile>

// 初始化静态成员变量
SingleApplication* SingleApplication::m_instance = nullptr;

SingleApplication::SingleApplication(int &argc, char **argv, const QString &appKey)
    : QApplication(argc, argv)
    , appKey_(appKey.isEmpty() ? applicationName() : appKey)
    , sharedMemory_(appKey.isEmpty() ? applicationName() : appKey)
    , localServer_(nullptr)
{
    // 保存实例指针
    m_instance = this;
    
    // 设置应用程序属性
    setOrganizationName(organizationName().isEmpty() ? applicationName() : organizationName());
    
    // 尝试初始化共享内存
    initializeSharedMemory();
}

SingleApplication::~SingleApplication()
{
    cleanupSharedMemory();
    if (localServer_) {
        localServer_->close();
        delete localServer_;
    }
    
    // 清除实例指针
    if (m_instance == this) {
        m_instance = nullptr;
    }
}

SingleApplication* SingleApplication::instance()
{
    return m_instance;
}

bool SingleApplication::initializeSharedMemory()
{
    // 如果已经存在共享内存，说明已经有实例在运行
    if (sharedMemory_.attach()) {
        return false;
    }

    // 创建共享内存
    if (!sharedMemory_.create(1)) {
        return false;
    }

    // 创建本地服务器用于进程间通信
    localServer_ = new QLocalServer(this);
    connect(localServer_, &QLocalServer::newConnection, this, &SingleApplication::handleConnection);

    // 确保之前的服务器实例被清理
    QLocalServer::removeServer(appKey_);
    
    // 启动服务器
    if (!localServer_->listen(appKey_)) {
        return false;
    }

    return true;
}

void SingleApplication::cleanupSharedMemory()
{
    if (sharedMemory_.isAttached()) {
        sharedMemory_.detach();
    }
}

bool SingleApplication::isRunning()
{
    return !localServer_;
}

bool SingleApplication::sendMessage(const QString &message)
{
    if (!isRunning()) {
        return false;
    }

    QLocalSocket socket;
    socket.connectToServer(appKey_, QIODevice::WriteOnly);

    if (!socket.waitForConnected(timeout_)) {
        return false;
    }

    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << message;

    socket.write(buffer);
    if (!socket.waitForBytesWritten(timeout_)) {
        return false;
    }

    socket.disconnectFromServer();
    return true;
}

void SingleApplication::handleConnection()
{
    QLocalSocket *socket = localServer_->nextPendingConnection();
    if (!socket) {
        return;
    }

    connect(socket, &QLocalSocket::readyRead, this, &SingleApplication::receiveMessage);
    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}

void SingleApplication::receiveMessage()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) {
        return;
    }

    QByteArray buffer = socket->readAll();
    QDataStream stream(buffer);
    QString message;
    stream >> message;

    emit messageReceived(message);
}

bool SingleApplication::loadStyleSheet(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "无法打开样式表文件:" << path;
        return false;
    }
    
    QString styleSheet = QLatin1String(file.readAll());
    file.close();
    
    if (styleSheet.isEmpty()) {
        qWarning() << "样式表文件内容为空:" << path;
        return false;
    }
    
    // 应用样式表
    setStyleSheet(styleSheet);
    
    qInfo() << "成功加载样式表:" << path;
    return true;
}

bool SingleApplication::loadFontResources()
{
    bool success = true;
    QStringList fontPaths = {
        ":/font/fontawesome.ttf"  // 默认字体路径，可根据实际项目调整
    };
    
    for (const QString &path : fontPaths) {
        QFile fontFile(path);
        if (fontFile.open(QIODevice::ReadOnly)) {
            int fontId = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
            if (fontId == -1) {
                qWarning() << "加载字体失败:" << path;
                success = false;
            } else {
                QStringList families = QFontDatabase::applicationFontFamilies(fontId);
                if (!families.isEmpty()) {
                    qInfo() << "成功加载字体:" << path << "字体家族:" << families;
                }
            }
            fontFile.close();
        } else {
            qWarning() << "无法打开字体文件:" << path;
            success = false;
        }
    }
    
    return success;
}
