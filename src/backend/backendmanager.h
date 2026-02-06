#pragma once
/**
  * @brief: 后端线程管理类
  * @author: Leo
  * @note: 主要是管理算法调用相关线程
  */

#include "core/event/qeventforwarder.h"
#include "core/thread/threadpool.h"
#include "data/touiupdatemsg.h"
#include <QDateTime>
#include <QDebug>
#include <QObject>

class BackendManager
{
public:
    static BackendManager *instance()
    {
        static BackendManager instance_;
        return &instance_;
    }

private:
    BackendManager();
    ~BackendManager();
    BackendManager(const BackendManager &) = delete;
    BackendManager &operator=(const BackendManager &) = delete;
};
