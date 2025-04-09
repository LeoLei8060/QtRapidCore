#include "application.h"
#include <QFontDatabase>
#include <QDebug>

// 初始化静态成员变量
Application* Application::m_instance = nullptr;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
    // 保存实例指针
    m_instance = this;
    
    // 设置应用程序属性
    setOrganizationName(organizationName().isEmpty() ? applicationName() : organizationName());
}

Application::~Application()
{
    // 清除实例指针
    if (m_instance == this) {
        m_instance = nullptr;
    }
}

Application* Application::instance()
{
    return m_instance;
}

bool Application::loadStyleSheet(const QString &path)
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

bool Application::loadFontResources()
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
