#ifndef PLACEHOLDER_ENGINE_H
#define PLACEHOLDER_ENGINE_H

#include <QMap>
#include <QString>
#include <QStringList>

// 占位符替换引擎：将 {$KEY} 替换为参数表中的对应值
class PlaceholderEngine
{
public:
    // content：QSS 模板；args：键值映射；missingKeys：可选，收集未找到的 KEY
    static QString substitute(const QString                &content,
                              const QMap<QString, QString> &args,
                              QStringList                  *missingKeys = nullptr);
};

#endif
