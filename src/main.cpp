#include "app/apppresenter.h"
#include "utils/singleapplication.h"
#include "core/crash/crashhandler.h"
#include <QMessageBox>
#include <QWidget>

int main(int argc, char *argv[])
{
    Core::CrashHandler::init();

    Core::SingleApplication app(argc, argv, "App");

    if (app.isRunning()) {
        QMessageBox::warning(nullptr, "警告", "应用程序已经在运行！");
        app.sendMessage("show");
        return 0;
    }

    AppPresenter appPresenter(&app);
    if (!appPresenter.initSystem()) {
        QString infos = QString("系统初始化失败，是否继续运行？\n%1").arg(appPresenter.getErrs());
        auto ret = QMessageBox::warning(nullptr, "警告", infos, QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No)
            return 0;
    }
    appPresenter.showWindow();

    return app.exec();
}
