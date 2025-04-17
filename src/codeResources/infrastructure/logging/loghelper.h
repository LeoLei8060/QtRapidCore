#pragma once

#include "log4qt/logger.h"

#include <QMutex>

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

#ifdef QT_NO_DEBUG
#define LOGINFO(...)  Log::LogHelper::info(__VA_ARGS__)
#define LOGDEBUG(...) Log::LogHelper::debug(__VA_ARGS__)
#define LOGWARN(...)  Log::LogHelper::warn(__VA_ARGS__)
#define LOGERROR(...) Log::LogHelper::error(__VA_ARGS__)
#else

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOGINFO(message, ...) \
    Log::LogHelper::info(QString(message).append( \
                             QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                         ##__VA_ARGS__)
#define LOGDEBUG(message, ...) \
    Log::LogHelper::debug(QString(message).append( \
                              QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                          ##__VA_ARGS__)
#define LOGWARN(message, ...) \
    Log::LogHelper::warn(QString(message).append( \
                             QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                         ##__VA_ARGS__)
#define LOGERROR(message, ...) \
    Log::LogHelper::error(QString(message).append( \
                              QString("  [%1:%2(%3)]").arg(__FILENAME__).arg(__func__).arg(__LINE__)), \
                          ##__VA_ARGS__)
#endif

namespace Log {
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

    Logger *m_LogAll; // TODO: 暂时不需要

    static LogHelper *m_Instance;
    static QMutex     m_Mutex;
};
} // namespace Log
