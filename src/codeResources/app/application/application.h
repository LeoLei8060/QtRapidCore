#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QFile>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application() override;
    
    // 加载样式表
    bool loadStyleSheet(const QString &path);
    
    // 加载字体资源
    bool loadFontResources();
    
    // 获取应用程序实例
    static Application* instance();

private:
    // 保存当前实例的指针
    static Application* m_instance;
};

#endif // APPLICATION_H
