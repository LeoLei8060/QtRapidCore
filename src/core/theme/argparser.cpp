#include "argparser.h"
#include <QFile>
#include <QStringList>
#include <QTextStream>

// 去除首尾空白辅助函数
static inline QString trim(const QString &s)
{
    QString r = s;
    return r.trimmed();
}

QMap<QString, QString> ArgParser::parse(const QString &filePath)
{
    QMap<QString, QString> map;
    QFile                  f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 打不开文件时返回空映射
        return map;
    }

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString t = line.trimmed();
        if (t.isEmpty())
            continue; // 跳过空行
        if (t.startsWith('#') || t.startsWith("//"))
            continue; // 跳过注释行
        int eq = t.indexOf('=');
        if (eq < 0)
            continue;                        // 无等号行忽略
        QString key = trim(t.left(eq));      // KEY
        QString value = trim(t.mid(eq + 1)); // VALUE
        if (!key.isEmpty()) {
            map.insert(key, value); // 后出现的键覆盖之前的值
        }
    }
    return map;
}
