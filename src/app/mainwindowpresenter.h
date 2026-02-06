#ifndef MAINWINDOWPRESENTER_H
#define MAINWINDOWPRESENTER_H

/**
  * @brief: 主界面的Presenter
  * 控制和管理主界面的业务流程
  * @author: Leo
  */

#include "data/imessages.h"
#include <QObject>
#include <QScopedPointer>

class MainWindow;

class MainWindowPresenter : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowPresenter(QObject *parent = nullptr);
    ~MainWindowPresenter();

    void     showWindow();
    QWidget *getMainWindow();

private slots:
    void onStart();
    void onEvent_toUIMsg(QSharedPointer<IUIUpdateMsg> data);
    void onEvent_toPresenterMsg(QSharedPointer<IPresenterMsg> data);

private:
    void setupConnections();

private:
    QScopedPointer<MainWindow> m_mainWindow;
};

#endif // MAINWINDOWPRESENTER_H
