#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("Qt快速开发框架生成器");
    QApplication::setOrganizationName("QtRapidCore");
    QApplication::setApplicationVersion("1.0.0");
    
    // 创建并加载翻译
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QtRapidCore_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
    
    // 确保程序可以正确找到相对路径下的文件
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    
    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
