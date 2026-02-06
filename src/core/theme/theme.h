#ifndef THEME_H
#define THEME_H

#include <QDateTime>
#include <QMap>
#include <QString>

// 单个主题的元数据与编译结果
class Theme
{
public:
    QString                id;                // 主题 ID（目录名）
    QString                name;              // 主题显示名
    QString                dirPath;           // 主题目录路径
    QString                argPath;           // 参数文件路径（.arg）
    QString                themePath;         // 模板文件路径（.theme）
    QMap<QString, QString> args;              // 解析后的参数映射
    QString                compiledQss;       // 编译生成的 QSS 文本
    QDateTime              lastArgModified;   // 参数文件修改时间
    QDateTime              lastThemeModified; // 模板文件修改时间

    // 编译主题：读取 .arg 与 .theme 并替换占位符
    bool compile(QStringList *missingKeys = nullptr);
};

#endif // THEME_H
