#include "mainwindow.h"
#include "common/iconfonts.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setMouseTracking(true);

    setupUI();
    setupConnections();
}

void MainWindow::setWindowTitle(const QString &title)
{
    QWidget::setWindowTitle(title);
    if (ui && ui->label_systemTitle) {
        ui->label_systemTitle->setText(title);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !isMaximized()) {
        QPoint titleWidgetPos = ui->widget_systemTitle->mapFrom(this, event->pos());
        if (ui->widget_systemTitle->rect().contains(titleWidgetPos)) {
            m_dragEnabled = true;
            m_dragPosition = event->globalPos();
        }
    }
    QWidget::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && !isMaximized() && m_dragEnabled) {
        auto nowPos = event->globalPos();
        move(nowPos - m_dragPosition + pos());
        m_dragPosition = nowPos;
    }
    QWidget::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragEnabled) {
        m_dragEnabled = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (isMaximized())
        ui->max_button->setIconText(NORMALBTN_TXT);
    else
        ui->max_button->setIconText(MAXBTN_TXT);
}

void MainWindow::setupUI()
{
    ui->close_button->setIconText(CLOSEBTN_TXT);
    ui->min_button->setIconText(MINBTN_TXT);
    ui->max_button->setIconText(MAXBTN_TXT);
    ui->setting_button->setIconText(SETTINGBTN_TXT);
}

void MainWindow::setupConnections()
{
    connect(ui->close_button, &QPushButton::clicked, this, &MainWindow::onCloseBtnClicked);
    connect(ui->min_button, &QPushButton::clicked, this, &MainWindow::onMinBtnClicked);
    connect(ui->max_button, &QPushButton::clicked, this, &MainWindow::onMaxBtnClicked);
}

void MainWindow::onCloseBtnClicked()
{
    qApp->quit();
}

void MainWindow::onMinBtnClicked()
{
    showMinimized();
}

void MainWindow::onMaxBtnClicked()
{
    if (isMaximized())
        showNormal();
    else
        showMaximized();
}
