#ifndef APPCONFIGSERVICE_H
#define APPCONFIGSERVICE_H

/**
  * @brief: 系统配置服务
  * 单例类型服务，供其他业务类调用，提供setter和getter
  * @author: Leo
  */

#include "config/systemconfig.h"
#include <QObject>
#include <QScopedPointer>

class AppConfigService : public QObject
{
    Q_OBJECT
public:
    static AppConfigService *instance();

    bool loadFromSettings();
    bool saveToSettings();

    // TODO: 系统配置相关的setter/getter
    QString getIconfontPath() const { return m_systemConfig.iconFontPath; }
    void    setIconfontPath(const QString &path) { m_systemConfig.iconFontPath = path; }

private:
    AppConfigService(QObject *parent = nullptr);

    QString getConfigFilePath() const;

private:
    SystemConfig m_systemConfig;
};

#endif // APPCONFIGSERVICE_H
