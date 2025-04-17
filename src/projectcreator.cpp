#include "projectcreator.h"
#include <QDir>

ProjectCreator::ProjectCreator() {}

ProjectCreator::~ProjectCreator() {}

bool ProjectCreator::createProject(const QMap<QString, QVariant> &config)
{
    // 获取项目路径和名称
    QString projectPath = config["projectPath"].toString();
    QString projectName = config["projectName"].toString();

    if (projectPath.isEmpty() || projectName.isEmpty()) {
        m_lastError = "项目路径或名称不能为空";
        return false;
    }

    // 项目目录
    QString projectDir = projectPath + "/" + projectName;
    QDir    projectDirObj(projectDir);

    if (!projectDirObj.exists() && !projectDirObj.mkpath(".")) {
        m_lastError = "无法创建项目目录: " + projectDir;
        return false;
    }

    // 创建目录结构
    if (!createDirectoryStructure(projectDir)) {
        return false;
    }

    // 创建应用层文件
    if (!createAppFiles(projectDir, config)) {
        return false;
    }

    // 创建基础设施层文件
    if (!createInfrastructureFiles(projectDir)) {
        return false;
    }

    // 创建UI层文件
    if (!createUIFiles(projectDir, config)) {
        return false;
    }

    // 创建资源文件
    if (config["addResources"].toBool()) {
        if (!createResourceFiles(projectDir, config)) {
            return false;
        }
    }

    // 创建CMakeLists.txt文件
    if (!createCMakeListsFile(projectDir, config)) {
        return false;
    }

    return true;
}

QString ProjectCreator::lastError() const
{
    return m_lastError;
}

bool ProjectCreator::createDirectoryStructure(const QString &projectDir)
{
    // 基本目录结构
    QStringList directories = {"app/application",
                               "app/singleapplication",
                               "infrastructure/event",
                               "infrastructure/fonts",
                               "infrastructure/logging",
                               "data",
                               "ui/widgets",
                               "ui/windows",
                               "utils",
                               "resources/img",
                               "resources/font",
                               "resources/qss",
                               "thirdparty",
                               "tests"};

    for (const QString &dir : directories) {
        QString fullPath = projectDir + "/" + dir;
        QDir    dirObj;
        if (!dirObj.mkpath(fullPath)) {
            m_lastError = "无法创建目录: " + fullPath;
            return false;
        }
    }

    return true;
}

bool ProjectCreator::ensureDirectoryExists(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

bool ProjectCreator::copyFile(const QString &sourceFilePath, const QString &targetFilePath)
{
    // 确保目标目录存在
    QFileInfo targetFileInfo(targetFilePath);
    ensureDirectoryExists(targetFileInfo.absolutePath());

    // 如果目标文件已存在，先删除
    if (QFile::exists(targetFilePath)) {
        QFile::remove(targetFilePath);
    }

    // 复制文件
    if (!QFile::copy(sourceFilePath, targetFilePath)) {
        m_lastError = "无法复制文件: " + sourceFilePath + " 到 " + targetFilePath;
        return false;
    }

    return true;
}

bool ProjectCreator::copyDirectory(const QString &sourceDir, const QString &destDir, bool recursive)
{
    QDir srcDir(sourceDir);
    QDir dstDir(destDir);

    // 确保目标目录存在
    if (!dstDir.exists()) {
        if (!dstDir.mkpath(".")) {
            m_lastError = "无法创建目录: " + destDir;
            return false;
        }
    }

    // 复制所有文件
    QStringList files = srcDir.entryList(QDir::Files);
    for (const QString &fileName : files) {
        QString srcFilePath = srcDir.filePath(fileName);
        QString dstFilePath = dstDir.filePath(fileName);

        if (!copyFile(srcFilePath, dstFilePath)) {
            return false;
        }
    }

    // 如果需要递归复制子目录
    if (recursive) {
        QStringList dirs = srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &dirName : dirs) {
            QString srcSubDir = srcDir.filePath(dirName);
            QString dstSubDir = dstDir.filePath(dirName);

            if (!copyDirectory(srcSubDir, dstSubDir, recursive)) {
                return false;
            }
        }
    }

    return true;
}

bool ProjectCreator::writeToFile(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建文件: " + filePath;
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;
    file.close();

    return true;
}

bool ProjectCreator::createAppFiles(const QString &projectDir, const QMap<QString, QVariant> &config)
{
    QString projectName = config["projectName"].toString();
    QString projectNameLower = projectName.toLower();
    bool    singleInstance = config["singleInstance"].toBool();
    QString projectType = config["projectType"].toString();
    bool    useForm = config["useForm"].toBool();

    // 创建app目录下需要的子目录
    if (!ensureDirectoryExists(projectDir + "/app/application")
        || !ensureDirectoryExists(projectDir + "/app/singleapplication")) {
        return false;
    }

    // 创建main.cpp
    QString mainContent;
    // 根据进程模式选择头文件，注意路径使用小写
    if (singleInstance) {
        mainContent += "#include \"singleapplication/singleapplication.h\"\n";
        mainContent += "#include <QMessageBox>\n";
    } else {
        mainContent += "#include \"application/application.h\"\n";
    }

    mainContent += "#include <QApplication>\n";

    // 根据项目类型选择头文件
    if (projectType == "QWidget") {
        mainContent += "#include \"ui/windows/" + projectNameLower + ".h\"\n\n";
    } else if (projectType == "QMainWindow") {
        mainContent += "#include \"ui/windows/" + projectNameLower + ".h\"\n\n";
    }

    mainContent += "int main(int argc, char *argv[])\n{\n";

    // 根据进程模式创建应用实例
    if (singleInstance) {
        mainContent += "    SingleApplication app(argc, argv);\n";

        mainContent += "    if (app.isRunning()) {\n";
        mainContent
            += "        QMessageBox::warning(nullptr, \"警告\", \"应用程序已经在运行！\");\n";
        mainContent += "        app.sendMessage(\"show\");\n";
        mainContent += "        return 0;\n";
        mainContent += "    }\n\n";
    } else {
        mainContent += "    Application app(argc, argv);\n";
    }

    // 根据项目类型创建主窗口
    if (projectType == "QWidget" || projectType == "QMainWindow") {
        mainContent += "\n    // 创建并显示主窗口\n";
        mainContent += "    " + projectName + " " + projectNameLower + ";\n";
        mainContent += "    " + projectNameLower + ".show();\n\n";
    }

    mainContent += "    return app.exec();\n}\n";

    if (!writeToFile(projectDir + "/app/main.cpp", mainContent)) {
        return false;
    }

    // 拷贝Application或SingleApplication类文件
    QString codeResourcesDir = QDir::currentPath() + "/codeResources";

    // 根据单实例选项复制相应的应用程序类文件
    // 复制SingleApplication模板文件
    QString srcDir = codeResourcesDir + "/app/singleapplication";
    QString destDir = projectDir + "/app/singleapplication";
    if (!copyDirectory(srcDir, destDir, false)) {
        return false;
    }
    // 复制Application模板文件
    srcDir = codeResourcesDir + "/app/application";
    destDir = projectDir + "/app/application";
    if (!copyDirectory(srcDir, destDir, false)) {
        return false;
    }

    // 复制thirdparty目录
    QString srcThirdPartyDir = codeResourcesDir + "/thirdparty";
    QString destThirdPartyDir = projectDir + "/thirdparty";
    if (!copyDirectory(srcThirdPartyDir, destThirdPartyDir, true)) {
        return false;
    }

    return true;
}

bool ProjectCreator::createInfrastructureFiles(const QString &projectDir)
{
    // 确保基础设施目录存在
    ensureDirectoryExists(projectDir + "/infrastructure/event");
    ensureDirectoryExists(projectDir + "/infrastructure/fonts");
    ensureDirectoryExists(projectDir + "/infrastructure/logging");

    // 从模板目录复制基础设施文件
    QString codeResourcesDir = QDir::currentPath() + "/codeResources";

    // 复制event目录
    QString srcEventDir = codeResourcesDir + "/infrastructure/event";
    QString destEventDir = projectDir + "/infrastructure/event";
    if (!copyDirectory(srcEventDir, destEventDir, false)) {
        return false;
    }

    // 复制fonts目录
    QString srcFontsDir = codeResourcesDir + "/infrastructure/fonts";
    QString destFontsDir = projectDir + "/infrastructure/fonts";
    if (!copyDirectory(srcFontsDir, destFontsDir, false)) {
        return false;
    }

    // 复制logging目录
    QString srcLoggingDir = codeResourcesDir + "/infrastructure/logging";
    QString destLoggingDir = projectDir + "/infrastructure/logging";
    if (!copyDirectory(srcLoggingDir, destLoggingDir, false)) {
        return false;
    }

    return true;
}

bool ProjectCreator::createUIFiles(const QString &projectDir, const QMap<QString, QVariant> &config)
{
    QString projectName = config["projectName"].toString();
    QString projectNameLower = projectName.toLower();
    QString projectType = config["projectType"].toString();
    bool    useForm = config["useForm"].toBool();

    // 创建主窗口文件
    QString mainWindowDir = projectDir + "/ui/windows";
    QString headerPath = mainWindowDir + "/" + projectNameLower + ".h";
    QString sourcePath = mainWindowDir + "/" + projectNameLower + ".cpp";
    QString formPath;

    if (useForm) {
        formPath = mainWindowDir + "/" + projectNameLower + ".ui";
    }

    // 创建头文件
    QString headerContent = "#ifndef " + projectName.toUpper() + "_H\n";
    headerContent += "#define " + projectName.toUpper() + "_H\n\n";

    if (projectType == "QWidget") {
        headerContent += "#include <QWidget>\n\n";
        if (useForm) {
            headerContent += "namespace Ui {\n";
            headerContent += "class " + projectName + ";\n";
            headerContent += "}\n\n";
        }
        headerContent += "class " + projectName + " : public QWidget\n";
    } else if (projectType == "QMainWindow") {
        headerContent += "#include <QMainWindow>\n\n";
        if (useForm) {
            headerContent += "namespace Ui {\n";
            headerContent += "class " + projectName + ";\n";
            headerContent += "}\n\n";
        }
        headerContent += "class " + projectName + " : public QMainWindow\n";
    }

    headerContent += "{\n";
    headerContent += "    Q_OBJECT\n\n";
    headerContent += "public:\n";
    headerContent += "    explicit " + projectName + "(QWidget *parent = nullptr);\n";
    headerContent += "    ~" + projectName + "();\n\n";
    headerContent += "private:\n";
    if (useForm) {
        headerContent += "    Ui::" + projectName + " *ui;\n";
    }
    headerContent += "};\n\n";
    headerContent += "#endif // " + projectName.toUpper() + "_H\n";

    if (!writeToFile(headerPath, headerContent)) {
        return false;
    }

    // 创建源文件
    QString sourceContent = "#include \"" + projectNameLower + ".h\"\n";

    if (useForm) {
        sourceContent += "#include \"ui_" + projectNameLower + ".h\"\n";
    }

    sourceContent += "\n";
    sourceContent += projectName + "::" + projectName + "(QWidget *parent) :\n";

    if (projectType == "QWidget") {
        sourceContent += "    QWidget(parent)";

    } else if (projectType == "QMainWindow") {
        sourceContent += "    QMainWindow(parent)";
    }
    if (useForm)
        sourceContent += ",\n";
    else
        sourceContent += "\n";

    if (useForm) {
        sourceContent += "    ui(new Ui::" + projectName + ")\n";
        sourceContent += "{\n";
        sourceContent += "    ui->setupUi(this);\n";
    } else {
        sourceContent += "{\n";
        sourceContent += "    // 手动设置UI\n";
        sourceContent += "    resize(800, 600);\n";
        sourceContent += "    setWindowTitle(tr(\"" + projectName + "\"));\n";
    }

    sourceContent += "}\n\n";
    sourceContent += projectName + "::~" + projectName + "()\n";
    sourceContent += "{\n";
    if (useForm) {
        sourceContent += "    delete ui;\n";
    }
    sourceContent += "}\n";

    if (!writeToFile(sourcePath, sourceContent)) {
        return false;
    }

    // 创建UI文件
    if (useForm) {
        QString formContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        formContent += "<ui version=\"4.0\">\n";
        formContent += " <class>" + projectName + "</class>\n";

        if (projectType == "QWidget") {
            formContent += " <widget class=\"QWidget\" name=\"" + projectName + "\">\n";
        } else if (projectType == "QMainWindow") {
            formContent += " <widget class=\"QMainWindow\" name=\"" + projectName + "\">\n";
        }

        formContent += "  <property name=\"geometry\">\n";
        formContent += "   <rect>\n";
        formContent += "    <x>0</x>\n";
        formContent += "    <y>0</y>\n";
        formContent += "    <width>800</width>\n";
        formContent += "    <height>600</height>\n";
        formContent += "   </rect>\n";
        formContent += "  </property>\n";
        formContent += "  <property name=\"windowTitle\">\n";
        formContent += "   <string>" + projectName + "</string>\n";
        formContent += "  </property>\n";

        if (projectType == "QMainWindow") {
            formContent += "  <widget class=\"QWidget\" name=\"centralwidget\"/>\n";
            formContent += "  <widget class=\"QMenuBar\" name=\"menubar\"/>\n";
            formContent += "  <widget class=\"QStatusBar\" name=\"statusbar\"/>\n";
        }

        formContent += " </widget>\n";
        formContent += " <resources/>\n";
        formContent += " <connections/>\n";
        formContent += "</ui>\n";

        if (!writeToFile(formPath, formContent)) {
            return false;
        }
    }

    return true;
}

bool ProjectCreator::createResourceFiles(const QString                 &projectDir,
                                         const QMap<QString, QVariant> &config)
{
    // 创建资源文件
    QString qrcContent = "<!DOCTYPE RCC>\n";
    qrcContent += "<RCC version=\"1.0\">\n";

    qrcContent += "    <qresource prefix=\"/\">\n";
    qrcContent += "    </qresource>\n";
    qrcContent += "</RCC>\n";

    if (!writeToFile(projectDir + "/resources.qrc", qrcContent)) {
        return false;
    }

    return true;
}

bool ProjectCreator::createCMakeListsFile(const QString                 &projectDir,
                                          const QMap<QString, QVariant> &config)
{
    QString projectName = config["projectName"].toString();
    bool    qt6Compatibility = config["qt6Compatibility"].toBool();
    bool    addResources = config["addResources"].toBool();
    QString projectType = config["projectType"].toString();
    bool    singleInstance = config["singleInstance"].toBool();
    bool    useForm = config["useForm"].toBool();

    // 读取模板文件
    QFile templateFile(QDir::currentPath() + "/templates/CMakeLists.txt.template");
    if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "无法打开CMakeLists.txt模板文件";
        return false;
    }

    QTextStream in(&templateFile);
    in.setCodec("UTF-8");
    QString cmakeContent = in.readAll();
    templateFile.close();

    // 替换项目名称
    cmakeContent.replace("${PROJECT_NAME}", projectName);

    // 设置Qt查找包内容
    QString qtFindPackage;
    if (qt6Compatibility) {
        qtFindPackage = "# 查找Qt包（Qt6或Qt5）\n";
        qtFindPackage += "find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets";
        if (singleInstance)
            qtFindPackage += " Network";
        qtFindPackage += ")\n";

        qtFindPackage += "find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets";
        if (singleInstance)
            qtFindPackage += " Network";
        qtFindPackage += ")";
    } else {
        qtFindPackage = "# 查找Qt5包\n";
        qtFindPackage += "find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets";
        if (singleInstance)
            qtFindPackage += " Network";
        qtFindPackage += ")";
    }
    cmakeContent.replace("${QT_FIND_PACKAGE}", qtFindPackage);

    // 设置应用层文件
    QString appFiles;
    if (singleInstance) {
        appFiles = "app/singleapplication/singleapplication.h\n";
        appFiles += "    app/singleapplication/singleapplication.cpp";
    } else {
        appFiles = "app/application/application.h\n";
        appFiles += "    app/application/application.cpp";
    }
    cmakeContent.replace("${APP_FILES}", appFiles);

    // 设置UI层文件
    QString uiFiles;
    uiFiles = "\n    # UI层\n";
    uiFiles += "    ui/windows/" + projectName.toLower() + ".h\n";
    uiFiles += "    ui/windows/" + projectName.toLower() + ".cpp";

    if (useForm) {
        uiFiles += "\n    ui/windows/" + projectName.toLower() + ".ui";
    }
    cmakeContent.replace("${UI_FILES}", uiFiles);

    // 设置资源文件
    QString resourceFiles;
    if (addResources) {
        resourceFiles = "\n    # 资源文件\n";
        resourceFiles += "    resources.qrc";
    }
    cmakeContent.replace("${RESOURCE_FILES}", resourceFiles);

    // 设置QML模块或可执行文件
    QString qmlModule;
    if (projectType == "QQuick2ApplicationWindow") {
        qmlModule = "# QML模块\n";
        qmlModule += "qt_add_qml_module(\n";
        qmlModule += "    ${PROJECT_NAME}\n";
        qmlModule += "    URI " + projectName + "\n";
        qmlModule += "    VERSION 1.0\n";
        qmlModule += "    QML_FILES\n";
        qmlModule += "        resources/qml/main.qml\n";
        qmlModule += ")";
    } else {
        qmlModule = "# 添加可执行文件\n";
        qmlModule += "add_executable(${PROJECT_NAME}\n";
        qmlModule += "    ${PROJECT_SOURCES}\n";
        qmlModule += ")";
    }
    cmakeContent.replace("${QML_MODULE}", qmlModule);

    // 设置Qt库
    QString qtLibraries;
    if (qt6Compatibility) {
        qtLibraries = "Qt${QT_VERSION_MAJOR}::Core\n";
        qtLibraries += "    Qt${QT_VERSION_MAJOR}::Gui\n";
        qtLibraries += "    Qt${QT_VERSION_MAJOR}::Widgets";

        if (singleInstance) {
            qtLibraries += "\n    Qt${QT_VERSION_MAJOR}::Network\n";
        }
    } else {
        qtLibraries = "Qt5::Core\n";
        qtLibraries += "    Qt5::Gui\n";
        qtLibraries += "    Qt5::Widgets";

        if (singleInstance) {
            qtLibraries += "\n    Qt5::Network\n";
        }
    }
    cmakeContent.replace("${QT_LIBRARIES}", qtLibraries);

    // 写入CMakeLists.txt文件
    if (!writeToFile(projectDir + "/CMakeLists.txt", cmakeContent)) {
        return false;
    }

    return true;
}
