#include "LogHelper.h"
#include "propertyconfigurator.h"

#include <windows.h>
#include <QFileInfo>
#include <QtCore/QCoreApplication>

#define LOGCONFIG_PATH "./log.conf"
#define LOGCONFIG_NAME "log.conf"

namespace Log {
QMutex     LogHelper::m_Mutex;
LogHelper *LogHelper::m_Instance = nullptr;
LogHelper::LogHelper()
    : m_LogAll(nullptr)
{
    initLogConfig();
    //    m_LogInfo = Logger::logger("info");
    //    m_LogDebug = Logger::logger("debug");
    //    m_LogWarn = Logger::logger("warn");
    //    m_LogError = Logger::logger("error");
    m_LogAll = Logger::logger("FEMLogger");
}

void LogHelper::initLogConfig()
{
    SetCurrentDirectory(QCoreApplication::applicationDirPath().toStdString().c_str());

    QString   confPath = "";
    QFileInfo f(LOGCONFIG_PATH);
    if (f.isFile() && f.fileName() == LOGCONFIG_NAME)
        confPath = f.absoluteFilePath();
    else
        confPath = QCoreApplication::applicationDirPath() + "/" + LOGCONFIG_NAME;

    Log4Qt::PropertyConfigurator::configure(confPath);
}
} // namespace Log
