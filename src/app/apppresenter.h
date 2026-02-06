#ifndef APPPRESENTER_H
#define APPPRESENTER_H

/**
  * @brief: 系统层次的Presenter
  * 控制和管理整个软件的业务流程：
  * 1. 登录
  * 2. 多个界面切换显示（显示模式切换）
  * @author: Leo
  */

#include "core/theme/thememanager.h"
#include "core/utils/singleapplication.h"
#include "mainwindowpresenter.h"
#include <QObject>
#include <QScopedPointer>

class AppPresenter : public QObject
{
    Q_OBJECT
public:
    explicit AppPresenter(Core::SingleApplication *app, QObject *parent = nullptr);
    ~AppPresenter();

    /**
      * @brief: 初始化系统
      * @return: 返回false表示初始化失败，需要弹窗提示并退出程序
      */
    bool initSystem();

    void showWindow();

    QString getErrs();

private:
    void initSystemWorkDir();
    bool initTheme();
    bool loadStyles();

private:
    MainWindowPresenter               *m_mainPresenter{nullptr};
    bool                               m_initState{false};
    Core::SingleApplication           *m_app;
    QScopedPointer<Core::ThemeManager> m_themeManager;

    QStringList m_errs;
};

#endif //APPPRESENTER_H
