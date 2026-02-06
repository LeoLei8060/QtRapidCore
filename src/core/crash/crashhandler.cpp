#include "crashhandler.h"
#include <QDir>
#include <QCoreApplication>
#include <QDateTime>

#ifdef Q_OS_WIN
#include <windows.h>
// NOTE: windows.h必须在dbghelp.h前面
#include <dbghelp.h>
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

void CrashHandler::init()
{
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(generateDump);
#endif
}

} // namespace Core
