#include "mainwindowpresenter.h"
#include "core/event/qeventforwarder.h"
#include "core/log/loghelper.h"
#include "messages/topresentermsg.h"
#include "serialization/json.h"
#include "ui/widgets/mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QScreen>

MainWindowPresenter::MainWindowPresenter(QObject *parent)
    : QObject{parent}
    , m_mainWindow(new MainWindow())
{
    setupConnections();
}

MainWindowPresenter::~MainWindowPresenter() {}

void MainWindowPresenter::showWindow()
{
    m_mainWindow->showMaximized();
}

QWidget *MainWindowPresenter::getMainWindow()
{
    return m_mainWindow.get();
}

void MainWindowPresenter::onStart()
{
    // TODO: 开启服务流程
}

void MainWindowPresenter::onEvent_toUIMsg(QSharedPointer<IUIUpdateMsg> data)
{
    // TODO: 处理后端给UI的业务消息
}

void MainWindowPresenter::onEvent_toPresenterMsg(QSharedPointer<IPresenterMsg> data)
{
    // TODO: 处理ui给的业务消息
}

void MainWindowPresenter::setupConnections()
{
    Core::QEventForwarder::subscribe(this, MSG_TOUI);
    Core::QEventForwarder::subscribe(this, MSG_TOPRESENTER);
}
