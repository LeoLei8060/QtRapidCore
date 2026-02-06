#pragma once
#ifdef USE_LOG4QT
#include "logger.h"

#include <QMutex>
#else
#include <QDebug>
#endif
/*
* 日志宏：
*   提供简单的日志处理
*   基础宏：LOGINFO、LOGDEBUG、LOGWARN、LOGERROR
*   扩展宏：LOGINFO、LOGDEBUG、LOGWARN、LOGERROR
*   基础宏暂时不支持输出类名、函数名、代码行号
*   扩展宏支持输出函数名和代码行号，但会影响性能
*   Release版本使用基础宏，Debug版本使用扩展宏
*
*   底层使用的是QString类型字符串，所以上层的字符串格式化采用的是%1
*
*  Example:（注意CMakeLists.txt要添加log4qt和LogHelper两个库）
*   LOGINFO("test")
*   LOGDEBUG("count %1", vec.count())
*   LOGDEBUG("test name: %1, len: %2", "name", 4)
*/

namespace Core {
#ifdef USE_LOG4QT
#ifdef QT_NO_DEBUG
#define LOGINFO(...)  Core::LogHelper::info(__VA_ARGS__)
#define LOGDEBUG(...) Core::LogHelper::debug(__VA_ARGS__)
#define LOGWARN(...)  Core::LogHelper::warn(__VA_ARGS__)
#define LOGERROR(...) Core::LogHelper::error(__VA_ARGS__)
#else

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOGINFO(message, ...) \
    Core::LogHelper::info(QString(message).append( \
                              QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                          ##__VA_ARGS__)
#define LOGDEBUG(message, ...) \
    Core::LogHelper::debug(QString(message).append( \
                               QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                           ##__VA_ARGS__)
#define LOGWARN(message, ...) \
    Core::LogHelper::warn(QString(message).append( \
                              QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                          ##__VA_ARGS__)
#define LOGERROR(message, ...) \
    Core::LogHelper::error(QString(message).append( \
                               QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                           ##__VA_ARGS__)
#endif

using namespace Log4Qt;
class LogHelper : public QObject
{
public:
    static LogHelper *instance()
    {
        if (!m_Instance) {
            m_Mutex.lock();
            if (!m_Instance)
                m_Instance = new LogHelper();
            m_Mutex.unlock();
        }
        return m_Instance;
    }

    static void info(const QString &msg) { instance()->m_LogAll->info(msg); }
    template<typename T, typename... Ts>
    static void info(const QString &message, T &&t, Ts &&...ts)
    {
        instance()->m_LogAll->info(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    static void debug(const QString &msg) { instance()->m_LogAll->debug(msg); }
    template<typename T, typename... Ts>
    static void debug(const QString &message, T &&t, Ts &&...ts)
    {
        instance()->m_LogAll->debug(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    static void warn(const QString &msg) { instance()->m_LogAll->warn(msg); }
    template<typename T, typename... Ts>
    static void warn(const QString &message, T &&t, Ts &&...ts)
    {
        instance()->m_LogAll->warn(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    static void error(const QString &msg) { instance()->m_LogAll->error(msg); }
    template<typename T, typename... Ts>
    static void error(const QString &message, T &&t, Ts &&...ts)
    {
        instance()->m_LogAll->error(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

private:
    LogHelper();
    void initLogConfig();

    Logger *m_LogAll;

    static LogHelper *m_Instance;
    static QMutex     m_Mutex;
};
#else
#define LOGINFO(...)  qInfo(__VA_ARGS__)
#define LOGDEBUG(...) qDebug(__VA_ARGS__)
#define LOGWARN(...)  qWarning(__VA_ARGS__)
#define LOGERROR(...) qCritical(__VA_ARGS__)
#endif

} // namespace Core
