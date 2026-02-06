#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <QMap>
#include <QString>

// 解析 .arg 参数文件（键值对），返回键到值的映射
class ArgParser
{
public:
    // 从文件路径读取并解析，忽略空行与注释（#、//）
    static QMap<QString, QString> parse(const QString &filePath);
};

#endif // ARG_PARSER_H
