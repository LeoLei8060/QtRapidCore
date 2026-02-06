#include "thememanager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QWidget>

namespace Core {

// 初始化管理器，设置文件监控与默认搜索路径
ThemeManager::ThemeManager(SingleApplication *app, QObject *parent)
    : QObject(parent)
    , m_rootApp(app)
{
    // 默认添加当前exe目录下的themes文件夹
    addSearchRoot(QCoreApplication::applicationDirPath() + "/themes");
}

void ThemeManager::addSearchRoot(const QString &path)
{
    if (!m_roots.contains(path))
        m_roots.append(path);
}

bool ThemeManager::loadThemeMeta(const QString &dirPath, Theme &out)
{
    QDir d(dirPath);
    if (!d.exists())
        return false;
    QString baseName = QFileInfo(dirPath).fileName();
    QString arg = d.filePath(baseName + ".arg");
    QString theme = d.filePath(baseName + ".theme");
    if (!QFileInfo::exists(arg) || !QFileInfo::exists(theme))
        return false;
    out.id = baseName;
    out.name = baseName;
    out.dirPath = dirPath;
    out.argPath = arg;
    out.themePath = theme;
    return true;
}

void ThemeManager::enableWatcher()
{
    if (!m_watcher) {
        m_watcher = new QFileSystemWatcher(this);
        connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &ThemeManager::onFileChanged);
    }
    for (const Theme &t : m_themes) {
        watchTheme(t);
    }
    m_autoReload = true;
}

void ThemeManager::disableWatcher()
{
    if (m_watcher) {
        m_watcher->deleteLater();
        m_watcher = nullptr;
    }
    m_autoReload = false;
}

// 扫描所有根目录，收集有效主题（同时有 .arg/.theme）
int ThemeManager::discover()
{
    QMap<QString, Theme> found;
    for (const QString &root : m_roots) {
        QDir rd(root);
        if (!rd.exists())
            continue;
        QFileInfoList subs = rd.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &fi : subs) {
            Theme t;
            if (loadThemeMeta(fi.filePath(), t)) {
                found.insert(t.id, t);
            }
        }
    }
    m_themes = found;
    for (const Theme &t : m_themes)
        watchTheme(t);
    return m_themes.count();
}

// 返回已经发现的主题 ID 列表
QStringList ThemeManager::availableThemes() const
{
    return m_themes.keys();
}

// 编译并应用指定主题到 root 窗口
bool ThemeManager::apply(const QString &themeId)
{
    if (!m_themes.contains(themeId)) {
        emit sigErrorOccurred(QString::fromLocal8Bit("主题不存在: %1").arg(themeId));
        return false;
    }
    Theme      &t = m_themes[themeId];
    QStringList missing;
    if (!t.compile(&missing)) {
        emit sigErrorOccurred(QString::fromLocal8Bit("主题编译失败: %1").arg(themeId));
        return false;
    }
    if (!missing.isEmpty()) {
        emit sigErrorOccurred(QString::fromLocal8Bit("缺失参数: %1").arg(missing.join(", ")));
        return false;
    }
    if (m_rootApp) {
        m_rootApp->setStyleSheet(t.compiledQss);
    }
    m_currentThemeId = themeId;
    emit sigThemeApplied(themeId);
    return true;
}

bool ThemeManager::reload(const QString &themeId)
{
    if (!m_themes.contains(themeId))
        return false;
    QStringList missing;
    Theme      &t = m_themes[themeId];
    bool        ok = t.compile(&missing);
    if (!ok || !missing.isEmpty())
        return false;
    return true;
}

void ThemeManager::reloadAll()
{
    for (auto it = m_themes.begin(); it != m_themes.end(); ++it) {
        QStringList missing;
        it.value().compile(&missing);
    }
}

QString ThemeManager::currentThemeId() const
{
    return m_currentThemeId;
}

void ThemeManager::setAutoReloadEnabled(bool enabled)
{
    if (m_autoReload == enabled)
        return;
    if (enabled) {
        enableWatcher();
    } else {
        disableWatcher();
    }
}

// 将主题的 .arg/.theme 文件加入监控列表
void ThemeManager::watchTheme(const Theme &t)
{
    if (!m_watcher)
        return;
    QStringList paths;
    paths << t.argPath << t.themePath;
    for (const QString &p : paths) {
        if (!m_watcher->files().contains(p) && QFileInfo::exists(p)) {
            m_watcher->addPath(p);
        }
    }
}

// 文件变更时重新编译，必要时重新应用到当前根窗口
void ThemeManager::onFileChanged(const QString &path)
{
    if (!m_autoReload)
        return;
    QString changedId;
    for (auto it = m_themes.begin(); it != m_themes.end(); ++it) {
        const Theme &t = it.value();
        if (t.argPath == path || t.themePath == path) {
            changedId = it.key();
            break;
        }
    }
    if (changedId.isEmpty())
        return;
    QStringList missing;
    Theme      &t = m_themes[changedId];
    bool        ok = t.compile(&missing);
    if (!ok) {
        emit sigErrorOccurred(QString::fromLocal8Bit("主题重新编译失败: %1").arg(changedId));
        return;
    }
    if (!missing.isEmpty()) {
        emit sigErrorOccurred(QString::fromLocal8Bit("缺失参数: %1").arg(missing.join(", ")));
        return;
    }
    if (changedId == m_currentThemeId && m_rootApp) {
        m_rootApp->setStyleSheet(t.compiledQss);
        emit sigThemeApplied(changedId);
    }
}

} // namespace Core
