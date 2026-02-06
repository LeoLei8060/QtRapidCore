#include "apppresenter.h"
#include "appconfigservice.h"
#include "common/appdefines.h"
#include "core/log/loghelper.h"
#include "ui/components/fontmanager.h"
#include "utils/utils.h"
#include <QDirIterator>

using namespace Core;

AppPresenter::AppPresenter(Core::SingleApplication *app, QObject *parent)
    : QObject{parent}
    , m_app(app)
    , m_themeManager(new ThemeManager(app, this))
{
    m_themeManager->discover();
}

AppPresenter::~AppPresenter()
{
    AppConfigService::instance()->saveToSettings();
    LOGINFO("Application quit ...");
}

void AppPresenter::showWindow()
{
    // NOTE: 通过MainWindowPresenter管理主界面
    if (!m_mainPresenter)
        m_mainPresenter = new MainWindowPresenter(this);
    m_mainPresenter->showWindow();
}

QString AppPresenter::getErrs()
{
    return m_errs.join("\n");
}

void AppPresenter::initSystemWorkDir()
{
    // TODO: 初始化系统工作目录
}

bool AppPresenter::initTheme()
{
    connect(m_themeManager.get(), &ThemeManager::sigErrorOccurred, this, [](const QString &message) {
        LOGWARN(message);
    });
    m_themeManager->discover();
    QString defaultId = "default";
    return m_themeManager->apply(defaultId);
}

bool AppPresenter::initSystem()
{
    LOGINFO("Application initializing ...");

    // 初始化系统相关工作目录
    initSystemWorkDir();

    // 初始化加载系统配置文件
    bool configState = AppConfigService::instance()->loadFromSettings();
    if (!configState)
        m_errs.append("系统配置文件加载失败.");

    // 初始化加载Icon字体库
    bool fontState = FontManager::instance()
                         ->addThirdpartyFont(AppConfigService::instance()->getIconfontPath(),
                                             FontManager::kIcon);
    if (!fontState)
        m_errs.append("Iconfont文件加载失败.");

    // 初始化主题
    bool themeState = initTheme();
    if (!themeState)
        m_errs.append("主题加载失败.");

    m_initState = configState && fontState && themeState;

    LOGINFO("Application initialize: %1", m_initState ? "successful" : "failed");
    return m_initState;
}

bool AppPresenter::loadStyles()
{
    LOGINFO("loading application style files ...");
    bool    result = false;
    QString stylesDir = QString("%1/styles/%2")
                            .arg(QApplication::applicationDirPath())
                            .arg(QApplication::applicationName());
    QString      stylesContext;
    QDirIterator it(stylesDir, QDir::Files | QDir::NoDotAndDotDot);
    if (!it.hasNext())
        LOGERROR("application style files not found.");
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        QString   fileContext;
        if (Utils::readFile(fileInfo.filePath(), fileContext))
            stylesContext += fileContext;
        result = true;
    }
    m_app->setStyleSheet(stylesContext);

    LOGINFO("load application style files: %1", result ? "successful" : "failed");
    return result;
}
