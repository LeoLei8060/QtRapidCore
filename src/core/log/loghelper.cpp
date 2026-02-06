#include "loghelper.h"
#include "propertyconfigurator.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <QDir>
#include <QFileInfo>
#include <QtCore/QCoreApplication>

#define LOGCONFIG_PATH "./log.conf"
#define LOGCONFIG_NAME "log.conf"

#ifdef USE_LOG4QT
namespace Core {
QMutex     LogHelper::m_Mutex;
LogHelper *LogHelper::m_Instance = nullptr;
LogHelper::LogHelper()
    : m_LogAll(nullptr)
{
    initLogConfig();
    m_LogAll = Logger::logger("FEMLogger");
}

void LogHelper::initLogConfig()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QString   confPath = "";
    QFileInfo f(LOGCONFIG_PATH);
    if (f.isFile() && f.fileName() == LOGCONFIG_NAME)
        confPath = f.absoluteFilePath();
    else
        confPath = QCoreApplication::applicationDirPath() + "/" + LOGCONFIG_NAME;

    Log4Qt::PropertyConfigurator::configure(confPath);
}
} // namespace Core
#endif
