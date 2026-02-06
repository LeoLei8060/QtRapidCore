#include "utils.h"
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace Core {
namespace Utils {

bool readFile(const QString &file, QString &context)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    context = f.readAll();

    f.close();
    return true;
}

bool ensureDirectoryExists(const QString &dirPath)
{
    QDir dir;
    if (!dir.exists(dirPath)) {
        return dir.mkpath(dirPath);
    }

    return true;
}

QString getAppDataPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir    dir;
    if (!dir.exists(path)) {
        dir.mkdir(".");
    }
    return path;
}

} // namespace Utils
} // namespace Core
