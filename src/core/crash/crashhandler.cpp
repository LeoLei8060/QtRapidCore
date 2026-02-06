#include "crashhandler.h"
#include <QDir>
#include <QCoreApplication>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <windows.h>
// NOTE: windows.h必须在dbghelp.h前面
#include <dbghelp.h>
#endif

#ifdef Q_OS_LINUX
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#endif

namespace Core {

#ifdef Q_OS_WIN
/**
* MINIDUMP_TYPE 说明：
*   MiniDumpNormal：生成基础的调试信息，比如所有线程的栈信息
*   MiniDumpWithDataSegs: 生成更多的上下文信息，比如全局变量
*   MiniDumpWithFullMemory: 提供最全面的信息，生成的文件也最大，不推荐使用
*   MiniDumpWithHandleData: 关于资源泄漏的信息
*   MiniDumpWithThreadInfo: 关于线程创建和退出时间的额外信息
*   MiniDumpWithUnloadedModules: 关于动态加载和卸载的模块信息
**/
LONG WINAPI generateDump(EXCEPTION_POINTERS *pExceptionPointers)
{
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "app";
    }
    
    QString dumpName = QString("%1_%2.dmp")
        .arg(appName)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    HANDLE hDumpFile
        = CreateFile((LPCSTR)dumpName.toLocal8Bit().constData(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
    if (hDumpFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = pExceptionPointers;
        exceptionInfo.ClientPointers = TRUE;

        MiniDumpWriteDump(GetCurrentProcess(),
                          GetCurrentProcessId(),
                          hDumpFile,
                          MiniDumpWithProcessThreadData, 
                          &exceptionInfo,
                          NULL,
                          NULL);

        CloseHandle(hDumpFile);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef Q_OS_LINUX
void handleCrashSignal(int sig)
{
    // 1. 获取程序名称和时间戳
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "app";
    }

    QString dumpName = QString("%1_%2.trace")
                           .arg(appName)
                           .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    // 2. 打开文件
    int fd = open(dumpName.toLocal8Bit().constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        // 3. 写入头信息
        char header[256];
        snprintf(header, sizeof(header), "Crash Signal: %d\nTime: %s\n\nStack Trace:\n", 
                 sig, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toLocal8Bit().constData());
        write(fd, header, strlen(header));

        // 4. 获取并写入堆栈信息
        void *array[100];
        int size = backtrace(array, 100);
        backtrace_symbols_fd(array, size, fd);

        close(fd);
    }

    // 5. 恢复默认信号处理并重新触发信号，以便系统生成core dump或正常终止
    signal(sig, SIG_DFL);
    raise(sig);
}
#endif

void CrashHandler::init()
{
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(generateDump);
#endif

#ifdef Q_OS_LINUX
    // 注册常见的崩溃信号
    signal(SIGSEGV, handleCrashSignal); // 段错误
    signal(SIGFPE, handleCrashSignal);  // 浮点异常
    signal(SIGILL, handleCrashSignal);  // 非法指令
    signal(SIGABRT, handleCrashSignal); // 中止信号
    signal(SIGBUS, handleCrashSignal);  // 总线错误
#endif
}

} // namespace Core
