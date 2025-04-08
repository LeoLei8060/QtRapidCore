#ifndef PROJECTPREVIEWDIALOG_H
#define PROJECTPREVIEWDIALOG_H

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QTreeWidget>

namespace Ui {
class ProjectPreviewDialog;
}

class ProjectPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectPreviewDialog(const QMap<QString, QVariant> &config, QWidget *parent = nullptr);
    ~ProjectPreviewDialog();

private:
    Ui::ProjectPreviewDialog *ui;
    QMap<QString, QVariant> m_config;
    
    // 生成框架预览树
    void generatePreviewTree();
    
    // 添加树节点
    QTreeWidgetItem* addTreeItem(QTreeWidgetItem *parent, const QString &name, bool isDir = false);
};

#endif // PROJECTPREVIEWDIALOG_H
