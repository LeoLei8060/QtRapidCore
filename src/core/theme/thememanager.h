#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

/**
  * @brief: 主题管理器
  * @author: Leo
  */

#include "theme.h"
#include "utils/singleapplication.h"
#include <QApplication>
#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QStringList>

namespace Core {

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(SingleApplication *app, QObject *parent = nullptr);

    // 添加主题搜索根目录
    void addSearchRoot(const QString &path);
    // 扫描根目录发现主题
    int discover();
    // 返回可用主题 ID 列表
    QStringList availableThemes() const;
    // 应用主题到根窗口
    bool apply(const QString &themeId);
    // 重新编译指定主题
    bool reload(const QString &themeId);
    // 重新编译所有主题
    void reloadAll();
    // 当前已应用主题 ID
    QString currentThemeId() const;
    // 设置是否自动热更新
    void setAutoReloadEnabled(bool enabled);

signals:
    void sigThemesChanged();
    void sigThemeApplied(const QString &id);
    void sigErrorOccurred(const QString &message);

private slots:
    void onFileChanged(const QString &path);

private:
    void watchTheme(const Theme &t);                        // 注册文件监听
    bool loadThemeMeta(const QString &dirPath, Theme &out); // 加载主题元信息
    void enableWatcher();
    void disableWatcher();

private:
    QStringList                  m_roots;              // 搜索根目录列表
    QMap<QString, Theme>         m_themes;             // 已发现主题集合
    QString                      m_currentThemeId;     // 当前应用主题 ID
    QPointer<QFileSystemWatcher> m_watcher;            // 文件系统监控器
    bool                         m_autoReload = false; // 自动热更新开关
    SingleApplication           *m_rootApp{nullptr};   // 根窗口指针
};

} // namespace Core
#endif // THEME_MANAGER_H
