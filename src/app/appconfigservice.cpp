#include "appconfigservice.h"
#include "core/log/loghelper.h"
#include "serialization/json.h"
#include <QCoreApplication>
#include <QFileInfo>

using namespace OSerialize;

AppConfigService *AppConfigService::instance()
{
    static AppConfigService instance;
    return &instance;
}

bool AppConfigService::loadFromSettings()
{
    // 加载配置
    m_systemConfig = JSON::file_to_obj<SystemConfig>(getConfigFilePath().toStdString());

    LOGINFO("load system config: %1", "successful");
    return true;
}

bool AppConfigService::saveToSettings()
{
    // 保存配置
    JSON::obj_to_file(m_systemConfig, getConfigFilePath().toStdString());

    LOGINFO("save system config: %1", "successful");
    return true;
}

AppConfigService::AppConfigService(QObject *parent)
    : QObject{parent}
{}

QString AppConfigService::getConfigFilePath() const
{
    QString   appPath = QCoreApplication::applicationFilePath();
    QFileInfo fileInfo(appPath);

    QString path;
    path = QString("%1/%2.json").arg(fileInfo.absolutePath()).arg(fileInfo.baseName());
    return path;
}
