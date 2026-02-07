# QtRapidCore

基于 Qt 的快速开发框架，旨在简化 C++ Qt 应用程序的开发流程，提供通用的基础设施和工具模块。

## 平台支持

- [x] Windows
- [ ] Linux

## Qt 版本支持

- [x] Qt 5
- [ ] Qt 6

## 核心功能与使用指南

### 1. 日志系统 (Log System)

集成 `Log4Qt`，支持丰富的日志输出格式和策略，并提供了便捷的宏定义封装。

**配置：**
项目根目录下需要包含 `log.conf` 配置文件，用于定义 Appender（输出目的地）、Layout（格式）等。

**代码使用：**

```cpp
#include "core/log/loghelper.h"

// 记录普通信息
LOGINFO("Application started successfully.");

// 记录带参数的信息 (支持 Qt 风格的 arg 占位符)
int count = 10;
LOGINFO("Current user count: %1", count);

// 记录警告信息
LOGWARN("Resource usage is high.");

// 记录错误信息
LOGERROR("Failed to connect to database.");

// 记录调试信息
LOGDEBUG("Variable x = %1", x);
```

### 2. 序列化支持 (Serialization)

提供统一的接口支持 XML、JSON、INI 格式的数据序列化与反序列化，通过宏定义极大简化了结构体与数据格式之间的映射。

**支持格式：**
- JSON (`serialization/json.h`)
- XML (`serialization/xml.h`)
- INI (`serialization/ini.h`)

**使用示例 (以 JSON 为例)：**

1. **定义数据结构**：使用 `O_SERIALIZE_STRUCT` 宏注册需要序列化的成员变量。

```cpp
#include "serialization/json.h"
#include <QString>

struct SystemConfig
{
    QString appName;
    int version;
    bool enableLog;
};

// 注册结构体成员
O_SERIALIZE_STRUCT(SystemConfig, appName, version, enableLog)
```

2. **序列化与反序列化**：

```cpp
using namespace OSerialize;

SystemConfig config;
config.appName = "QtRapidCore Demo";
config.version = 1;
config.enableLog = true;

// 序列化：对象 -> 文件
JSON::obj_to_file(config, "config.json");

// 反序列化：文件 -> 对象
SystemConfig loadedConfig = JSON::file_to_obj<SystemConfig>("config.json");
```

### 3. 主题管理 (Theme Management)

支持动态切换应用主题，采用 **模板替换** 的方式实现。主题由 `.theme` (样式模板) 和 `.arg` (样式变量) 两个文件组成。

**文件结构：**
主题文件通常存放在 `themes/` 目录下，例如：
- `themes/default/default.theme`：包含 QSS 样式代码，变量使用 `{$VariableName}` 占位。
- `themes/default/default.arg`：定义变量的具体值 (颜色、图片路径、字体等)。

**代码使用：**

```cpp
#include "core/theme/thememanager.h"

// 1. 初始化 ThemeManager
ThemeManager *themeMgr = new ThemeManager(qApp, this);

// 2. 扫描主题目录 (自动加载 themes 文件夹下的主题)
themeMgr->discover();

// 3. 应用主题 (传入主题 ID，通常是文件夹名称)
bool success = themeMgr->apply("default");

// 4. 监听错误信号
connect(themeMgr, &ThemeManager::sigErrorOccurred, [](const QString &msg){
    LOGWARN(msg);
});
```

**主题文件示例 (.arg)：**
```ini
// 定义按钮背景色变量
BtnBackColor = #333333
BtnTextColor = #FFFFFF
```

**主题文件示例 (.theme)：**
```css
QPushButton {
    /* 使用变量 */
    background-color: {$BtnBackColor};
    color: {$BtnTextColor};
    border: none;
}
```

### 4. 线程与消息通信 (Threading & Messaging)

提供了一套基于消息驱动的线程处理机制，方便开发者实现多线程任务处理和线程间通信。

**核心类：**
- `ThreadBase`: 线程基类，封装了 `std::thread`，提供启动、停止、暂停等基本操作。
- `IThreadMessage`: 消息接口，所有跨线程消息需继承此类。
- `ThreadMessageQueue`: 线程安全的消息队列，支持阻塞和非阻塞获取消息。

**使用示例：**

1. **定义消息**：

   首先需要在 `src/core/messaging/ithreadmessage.h` 的 `ThdMSGType` 枚举中添加自定义的消息类型。

```cpp
// src/core/messaging/ithreadmessage.h
enum class ThdMSGType {
    // ... 原有类型
    kProcessData, // 新增的自定义类型
    // ...
};
```

   然后定义具体的消息类：

```cpp
#include "core/messaging/ithreadmessage.h"

class DataMessage : public IThreadMessage {
public:
    DataMessage(const QString &data) : m_data(data) {
        setMsgType(ThdMSGType::kProcessData);
    }
    QString m_data;
};
using DataMessagePtr = QSharedPointer<DataMessage>;
```

2. **创建工作线程**：

```cpp
#include "core/thread/threadbase.h"
#include "core/messaging/threadmessagequeue.h"

class WorkerThread : public ThreadBase {
public:
    // 关联两个线程间的队列
    void setMsgQueue(ThreadMessageQueuePtr queue) { m_msgQueuePtr = queue; }


protected:
    // 线程执行函数
    void work() override {
        std::shared_ptr<IThreadMessage> msg;
        while (!shouldStop()) {
            // 阻塞等待消息
            if (m_queuePtr && m_queuePtr->pop(msg)) {
                if (msg->getMsgType() == ThdMSGType::kProcessData) {
                    DataMessagePtr dataMsg = msg.dynamicCast<DataMessage>();
                    // 处理对应消息逻辑
                }
            }
        }
    }

private:
    ThreadMessageQueuePtr m_queuePtr;
};
```

### 5. 其他核心模块

- **崩溃处理 (CrashHandler)**:
  内置 `CrashHandler`，在 `main` 函数入口调用 `Core::CrashHandler::init()` 即可开启。程序崩溃时会自动捕获并在 `dump` 目录下生成堆栈信息文件。

- **高性能队列 (LockFreeQueue)**:
  提供单生产者单消费者 (SPSC) 的无锁队列 `LockFreeQueue`，用于极高性能要求的线程间通信场景。

- **单例应用 (SingleApplication)**:
  防止程序多开，确保同一时间只有一个应用程序实例运行。如果尝试启动第二个实例，可以通过 `sendMessage` 唤醒已运行的实例。

## 构建与编译

本项目使用 CMake 构建，建议使用 Qt Creator 打开 `CMakeLists.txt` 进行配置和编译。

**依赖环境：**
- Qt 5.15+ (MinGW64 推荐)
- CMake 3.14+

### 构建输出与资源部署

CMake 配置会自动处理依赖文件和资源的拷贝，构建生成的可执行文件位于 `build/bin` 目录下。

**自动拷贝的资源包括：**
- **DLL 依赖**: 根据构建模式 (Debug/Release) 和架构 (x64/x86) 自动从 `3rdparty/lib/` 拷贝对应的动态库。
- **主题文件**: 将 `themes/` 目录拷贝至输出目录的 `themes/` 子目录。
- **资源文件**: 将 `res/` 目录拷贝至输出目录的 `res/` 子目录。
- **配置文件**: 将根目录下的 `log.conf` 拷贝至输出目录。

> **注意**: 资源拷贝发生在 CMake 配置阶段。如果修改了资源文件但未生效，请尝试重新运行 CMake 配置。

## 后续计划 (Roadmap)

- [ ] **网络模块**: 完善 HTTP、TCP、UDP 通信组件
- [ ] **数据库ORM**: 实现更强大的对象关系映射系统
- [ ] **插件管理系统**: 支持动态加载和卸载功能插件
