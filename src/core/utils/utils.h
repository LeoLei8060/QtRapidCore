#ifndef UTILS_H
#define UTILS_H

#include <QObject>

namespace Core {
namespace Utils {

bool readFile(const QString &file, QString &context);

// 判断文件夹是否存在，不存在则创建， return true表示文件夹存在
bool ensureDirectoryExists(const QString &dirPath);

QString getAppDataPath();

} // namespace Utils
} // namespace Core

#endif // UTILS_H
