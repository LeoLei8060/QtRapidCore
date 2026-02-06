#include "placeholderengine.h"
#include <QRegularExpression>

// 使用正则表达式查找 {$KEY} 并替换成参数映射中的值
QString PlaceholderEngine::substitute(const QString                &content,
                                      const QMap<QString, QString> &args,
                                      QStringList                  *missingKeys)
{
    QRegularExpression              re("\\{\\$([A-Za-z_]\\w*)\\}");
    QRegularExpressionMatchIterator it = re.globalMatch(content);
    QString                         out;
    out.reserve(content.size());
    int lastIndex = 0; // 记录上一个片段结束位置
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        int                     start = m.capturedStart();
        int                     end = m.capturedEnd();
        out += content.mid(lastIndex, start - lastIndex); // 追加前置原文
        QString key = m.captured(1);
        if (args.contains(key)) {
            out += args.value(key); // 找到值则替换
        } else {
            if (missingKeys)
                missingKeys->append(key); // 记录缺失键
            out += m.captured(0);         // 保留原占位符
        }
        lastIndex = end;
    }
    out += content.mid(lastIndex); // 追加末尾原文
    return out;
}
