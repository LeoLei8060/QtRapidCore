#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QSharedMemory>
#include <QLocalServer>
#include <QFile>

class SingleApplication : public QApplication
{
    Q_OBJECT

public:
    SingleApplication(int &argc, char **argv, const QString &appKey = QString());
    ~SingleApplication() override;

    bool isRunning();
    bool sendMessage(const QString &message);
    
    // 加载样式表
    bool loadStyleSheet(const QString &path);
    
    // 加载字体资源
    bool loadFontResources();
    
    // 获取应用程序实例
    static SingleApplication* instance();

signals:
    void messageReceived(const QString &message);

private slots:
    void handleConnection();
    void receiveMessage();

private:
    bool initializeSharedMemory();
    void cleanupSharedMemory();

    QString appKey_;
    QSharedMemory sharedMemory_;
    QLocalServer *localServer_;
    static const int timeout_ = 1000;
    
    // 保存当前实例的指针
    static SingleApplication* m_instance;
};

#endif // SINGLEAPPLICATION_H
