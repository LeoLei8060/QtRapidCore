#include "theme.h"
#include "argparser.h"
#include "placeholderengine.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

// 将 .arg 与 .theme 编译为最终 QSS
bool Theme::compile(QStringList *missingKeys)
{
    QFileInfo argInfo(argPath);
    QFileInfo themeInfo(themePath);
    lastArgModified = argInfo.lastModified();
    lastThemeModified = themeInfo.lastModified();

    args = ArgParser::parse(argPath); // 解析参数映射
    QFile f(themePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false; // 模板不可读
    }
    QTextStream in(&f);
    QString     content = in.readAll();
    compiledQss = PlaceholderEngine::substitute(content, args, missingKeys); // 执行占位符替换
    return true;
}
