#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setWindowTitle(const QString &title);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void setupConnections();

private slots:
    void onCloseBtnClicked();
    void onMinBtnClicked();
    void onMaxBtnClicked();

private:
    Ui::MainWindow *ui;

    // 窗口操作
    QPoint m_dragPosition;
    bool   m_dragEnabled{false};
};

#endif // MAINWINDOW_H
