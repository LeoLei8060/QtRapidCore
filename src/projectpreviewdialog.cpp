#include "projectpreviewdialog.h"
#include "ui_projectpreviewdialog.h"

#include <QIcon>

ProjectPreviewDialog::ProjectPreviewDialog(const QMap<QString, QVariant> &config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectPreviewDialog),
    m_config(config)
{
    ui->setupUi(this);
    
    // 设置窗口标题
    QString projectName = config["projectName"].toString();
    if (projectName.isEmpty()) {
        projectName = "MyProject";
    }
    setWindowTitle(tr("项目框架预览 - %1").arg(projectName));
    
    // 生成预览树
    generatePreviewTree();
}

ProjectPreviewDialog::~ProjectPreviewDialog()
{
    delete ui;
}

void ProjectPreviewDialog::generatePreviewTree()
{
    ui->previewTreeWidget->clear();
    
    QString projectName = m_config["projectName"].toString();
    if (projectName.isEmpty()) {
        projectName = "MyProject";
    }
    
    // 根节点
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(ui->previewTreeWidget);
    rootItem->setText(0, projectName);
    rootItem->setIcon(0, QIcon::fromTheme("folder"));
    
    // app目录
    QTreeWidgetItem *appItem = addTreeItem(rootItem, "app", true);
    
    // app/Application 或 app/SingleApplication
    bool singleInstance = m_config["singleInstance"].toBool();
    if (singleInstance) {
        QTreeWidgetItem *singleAppItem = addTreeItem(appItem, "SingleApplication", true);
        addTreeItem(singleAppItem, "singleapplication.h");
        addTreeItem(singleAppItem, "singleapplication.cpp");
    } else {
        QTreeWidgetItem *appDirItem = addTreeItem(appItem, "Application", true);
        addTreeItem(appDirItem, "application.h");
        addTreeItem(appDirItem, "application.cpp");
    }
    
    // app/main.cpp
    addTreeItem(appItem, "main.cpp");
    
    // infrastructure目录
    QTreeWidgetItem *infraItem = addTreeItem(rootItem, "infrastructure", true);
    
    // event目录
    QTreeWidgetItem *eventItem = addTreeItem(infraItem, "event", true);
    addTreeItem(eventItem, "qeventforwarder.h");
    addTreeItem(eventItem, "qeventforwarder.cpp");
    
    // fonts目录
    QTreeWidgetItem *fontsItem = addTreeItem(infraItem, "fonts", true);
    addTreeItem(fontsItem, "fontmanager.h");
    addTreeItem(fontsItem, "fontmanager.cpp");
    
    // logging目录
    QTreeWidgetItem *loggingItem = addTreeItem(infraItem, "logging", true);
    addTreeItem(loggingItem, "loghelper.h");
    addTreeItem(loggingItem, "loghelper.cpp");
    
    // data目录
    addTreeItem(rootItem, "data", true);
    
    // ui目录
    QTreeWidgetItem *uiItem = addTreeItem(rootItem, "ui", true);
    addTreeItem(uiItem, "widgets", true);
    addTreeItem(uiItem, "windows", true);
    
    // utils目录
    addTreeItem(rootItem, "utils", true);
    
    // 资源文件
    bool addResources = m_config["addResources"].toBool();
    if (addResources) {
        QTreeWidgetItem *resourcesItem = addTreeItem(rootItem, "resources", true);
        addTreeItem(resourcesItem, "img", true);
        addTreeItem(resourcesItem, "font", true);
        
        bool useStylesheet = m_config["useStylesheet"].toBool();
        if (useStylesheet) {
            addTreeItem(resourcesItem, "qss", true);
        }
        
        addTreeItem(rootItem, "resources.qrc");
    }
    
    // 第三方库目录
    addTreeItem(rootItem, "thirdparty", true);
    
    // 测试目录
    addTreeItem(rootItem, "tests", true);
    
    // CMakeLists.txt
    addTreeItem(rootItem, "CMakeLists.txt");
    
    // 展开所有项
    ui->previewTreeWidget->expandAll();
}

QTreeWidgetItem* ProjectPreviewDialog::addTreeItem(QTreeWidgetItem *parent, const QString &name, bool isDir)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    
    if (isDir) {
        item->setIcon(0, QIcon::fromTheme("folder"));
    } else {
        item->setIcon(0, QIcon::fromTheme("text-x-generic"));
    }
    
    return item;
}
