#include "mainwindow.h"
#include "projectcreator.h"
#include "projectpreviewdialog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置初始项目路径为当前目录
    ui->projectPathLineEdit->setText(QDir::currentPath());

    // 初始化文件名预览
    updateFileNamePreview();

    // 项目类型变更时更新UI状态
    connect(ui->projectTypeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &MainWindow::on_projectTypeComboBox_currentIndexChanged);

    // 初始化UI状态
    on_projectTypeComboBox_currentIndexChanged(ui->projectTypeComboBox->currentIndex());

    ui->useIconFontCheckBox->setDisabled(true);
    ui->useStylesheetCheckBox->setDisabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browseButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("选择项目目录"),
                                                    ui->projectPathLineEdit->text(),
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->projectPathLineEdit->setText(dir);
    }
}

void MainWindow::on_projectNameLineEdit_textChanged(const QString &text)
{
    updateFileNamePreview();
}

void MainWindow::on_projectTypeComboBox_currentIndexChanged(int index)
{
    // 更新文件名预览
    updateFileNamePreview();
}

void MainWindow::updateFileNamePreview()
{
    QString projectName = ui->projectNameLineEdit->text().toLower();
    if (projectName.isEmpty()) {
        projectName = "myproject";
    }

    ui->headerFilePreviewLabel->setText(projectName + ".h");
    ui->sourceFilePreviewLabel->setText(projectName + ".cpp");

    int  projectType = ui->projectTypeComboBox->currentIndex();
    bool useForm = ui->useFormCheckBox->isChecked();

    ui->formFilePreviewLabel->setText(useForm ? projectName + ".ui" : tr("(无)"));
}

void MainWindow::on_previewButton_clicked()
{
    QMap<QString, QVariant> config = getProjectConfig();

    ProjectPreviewDialog preview(config, this);
    preview.exec();
}

void MainWindow::on_createButton_clicked()
{
    QString projectPath = ui->projectPathLineEdit->text();
    QString projectName = ui->projectNameLineEdit->text();

    if (projectName.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("项目名称不能为空！"));
        return;
    }

    QDir dir(projectPath);
    if (!dir.exists()) {
        QMessageBox::warning(this, tr("警告"), tr("项目路径不存在！"));
        return;
    }

    // 创建项目文件夹
    QString projectDir = projectPath + "/" + projectName;
    QDir    projectDirObj(projectDir);

    if (projectDirObj.exists()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("确认"),
                                      tr("项目目录已存在，是否继续？这可能会覆盖现有文件。"),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    } else {
        if (!dir.mkdir(projectName)) {
            QMessageBox::critical(this, tr("错误"), tr("无法创建项目目录！"));
            return;
        }
    }

    // 获取项目配置
    QMap<QString, QVariant> config = getProjectConfig();

    // 创建项目
    ProjectCreator creator;
    bool           success = creator.createProject(config);

    if (success) {
        QMessageBox::information(this, tr("成功"), tr("项目创建成功！\n项目位置：") + projectDir);
    } else {
        QMessageBox::critical(this, tr("错误"), tr("项目创建失败！\n") + creator.lastError());
    }
}

QMap<QString, QVariant> MainWindow::getProjectConfig()
{
    QMap<QString, QVariant> config;

    // 基本信息
    config["projectPath"] = ui->projectPathLineEdit->text();
    config["projectName"] = ui->projectNameLineEdit->text();

    // 项目类型
    int     projectTypeIndex = ui->projectTypeComboBox->currentIndex();
    QString projectType;
    switch (projectTypeIndex) {
    case 0:
        projectType = "QWidget";
        break;
    case 1:
        projectType = "QMainWindow";
        break;
    default:
        projectType = "QWidget";
    }
    config["projectType"] = projectType;

    // Form文件
    config["useForm"] = ui->useFormCheckBox->isChecked() && projectTypeIndex != 2;

    // 附加选项
    config["addResources"] = ui->addResourcesCheckBox->isChecked();
    config["qt6Compatibility"] = ui->qt6CompatibilityCheckBox->isChecked();
    config["singleInstance"] = ui->singleInstanceRadioButton->isChecked();
    config["useStylesheet"] = ui->useStylesheetCheckBox->isChecked();
    config["useIconFont"] = ui->useIconFontCheckBox->isChecked();

    return config;
}
