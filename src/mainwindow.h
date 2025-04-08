#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 路径选择按钮点击事件
    void on_browseButton_clicked();
    
    // 项目名称改变事件
    void on_projectNameLineEdit_textChanged(const QString &text);
    
    // 项目类型改变事件
    void on_projectTypeComboBox_currentIndexChanged(int index);
    
    // 框架预览按钮点击事件
    void on_previewButton_clicked();
    
    // 创建按钮点击事件
    void on_createButton_clicked();

private:
    Ui::MainWindow *ui;
    
    // 更新文件名预览
    void updateFileNamePreview();
    
    // 获取项目配置
    QMap<QString, QVariant> getProjectConfig();
};

#endif // MAINWINDOW_H
