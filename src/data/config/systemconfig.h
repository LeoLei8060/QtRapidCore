#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

/**
  * @brief: 系统配置数据结构
  * @author: Leo
  */
#include "serialization/json.h"
#include <QObject>

struct SystemConfig
{
    QString iconFontPath = "./res/iconfont/iconfont.ttf";
};
O_SERIALIZE_STRUCT(SystemConfig, iconFontPath)

#endif // SYSTEMCONFIG_H
