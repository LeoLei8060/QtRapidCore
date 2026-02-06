#include "app/apppresenter.h"
#include "utils/singleapplication.h"
#include <QMessageBox>
#include <QWidget>

#ifdef Q_OS_WIN
#include <windows.h>
// NOTE: windows.h必须在dbghelp.h前面
#include <dbghelp.h>
#endif

#ifdef Q_OS_WIN
LONG WINAPI generateDump(EXCEPTION_POINTERS *pExceptionPointers);
#endif
int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter(generateDump);
#endif

    Core::SingleApplication app(argc, argv, "App");

    if (app.isRunning()) {
        QMessageBox::warning(nullptr, "警告", "应用程序已经在运行！");
        app.sendMessage("show");
        return 0;
    }

    AppPresenter appPresenter(&app);
    if (!appPresenter.initSystem()) {
        QString infos = QString("系统初始化失败，是否继续运行？\n%1").arg(appPresenter.getErrs());
        auto ret = QMessageBox::warning(nullptr, "警告", infos, QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No)
            return 0;
    }
    appPresenter.showWindow();

    return app.exec();
}

#ifdef Q_OS_WIN
LONG WINAPI generateDump(EXCEPTION_POINTERS *pExceptionPointers)
{
    HANDLE hDumpFile
        = CreateFile("app.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = pExceptionPointers;
        exceptionInfo.ClientPointers = TRUE;

        MiniDumpWriteDump(GetCurrentProcess(),
                          GetCurrentProcessId(),
                          hDumpFile,
                          MiniDumpWithProcessThreadData, // 参考后续枚举说明
                          &exceptionInfo,
                          NULL,
                          NULL);

        CloseHandle(hDumpFile);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/**
* MINIDUMP_TYPE 说明：
*   MiniDumpNormal：生成基础的调试信息，比如所有线程的栈信息
*   MiniDumpWithDataSegs: 生成更多的上下文信息，比如全局变量
*   MiniDumpWithFullMemory: 提供最全面的信息，生成的文件也最大，不推荐使用
*   MiniDumpWithHandleData: 关于资源泄漏的信息
*   MiniDumpWithThreadInfo: 关于线程创建和退出时间的额外信息
*   MiniDumpWithUnloadedModules: 关于动态加载和卸载的模块信息
**/
