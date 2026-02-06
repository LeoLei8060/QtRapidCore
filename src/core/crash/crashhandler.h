#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

namespace Core {

class CrashHandler
{
public:
    /**
     * @brief 初始化崩溃处理
     * @details 在Windows下会设置UnhandledExceptionFilter来生成MiniDump
     */
    static void init();
};

} // namespace Core

#endif // CRASHHANDLER_H
