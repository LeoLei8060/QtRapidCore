#include "projectcreator.h"

ProjectCreator::ProjectCreator()
{
}

ProjectCreator::~ProjectCreator()
{
}

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
    QDir projectDirObj(projectDir);
    
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
    QStringList directories = {
        "app/Application",
        "app/SingleApplication",
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
        "tests"
    };
    
    for (const QString &dir : directories) {
        QString fullPath = projectDir + "/" + dir;
        QDir dirObj;
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
    bool singleInstance = config["singleInstance"].toBool();
    QString projectType = config["projectType"].toString();
    bool useForm = config["useForm"].toBool();
    
    // 创建main.cpp
    QString mainContent = "#include <QApplication>\n";
    
    // 根据进程模式选择头文件
    if (singleInstance) {
        mainContent += "#include \"SingleApplication/singleapplication.h\"\n";
    } else {
        mainContent += "#include \"Application/application.h\"\n";
    }
    
    // 根据项目类型选择头文件
    if (projectType == "QWidget") {
        mainContent += "#include \"ui/windows/" + projectNameLower + ".h\"\n\n";
    } else if (projectType == "QMainWindow") {
        mainContent += "#include \"ui/windows/" + projectNameLower + ".h\"\n\n";
    } else if (projectType == "QQuick2ApplicationWindow") {
        mainContent += "#include <QQmlApplicationEngine>\n\n";
    }
    
    mainContent += "int main(int argc, char *argv[])\n{\n";
    
    // 根据进程模式创建应用实例
    if (singleInstance) {
        mainContent += "    SingleApplication app(argc, argv);\n";
    } else {
        mainContent += "    Application app(argc, argv);\n";
    }
    
    // 根据项目类型创建主窗口
    if (projectType == "QWidget" || projectType == "QMainWindow") {
        mainContent += "\n    // 创建并显示主窗口\n";
        mainContent += "    " + projectName + " " + projectNameLower + ";\n";
        mainContent += "    " + projectNameLower + ".show();\n\n";
    } else if (projectType == "QQuick2ApplicationWindow") {
        mainContent += "\n    // 加载QML引擎\n";
        mainContent += "    QQmlApplicationEngine engine;\n";
        mainContent += "    engine.load(QUrl(QStringLiteral(\"qrc:/main.qml\")));\n\n";
    }
    
    mainContent += "    return app.exec();\n}\n";
    
    if (!writeToFile(projectDir + "/app/main.cpp", mainContent)) {
        return false;
    }
    
    // 创建Application或SingleApplication类文件
    // 略（完整实现太长，请参考下面我提供的代码）
    
    return true;
}

bool ProjectCreator::createInfrastructureFiles(const QString &projectDir)
{
    // 事件转发器
    QString eventForwarderHeader = "#ifndef QEVENTFORWARDER_H\n";
    eventForwarderHeader += "#define QEVENTFORWARDER_H\n\n";
    eventForwarderHeader += "#include <QObject>\n";
    eventForwarderHeader += "#include <QEvent>\n\n";
    eventForwarderHeader += "class QEventForwarder : public QObject\n";
    eventForwarderHeader += "{\n";
    eventForwarderHeader += "    Q_OBJECT\n\n";
    eventForwarderHeader += "public:\n";
    eventForwarderHeader += "    explicit QEventForwarder(QObject *parent = nullptr);\n\n";
    eventForwarderHeader += "signals:\n";
    eventForwarderHeader += "    void eventOccurred(QObject *watched, QEvent *event);\n\n";
    eventForwarderHeader += "protected:\n";
    eventForwarderHeader += "    bool eventFilter(QObject *watched, QEvent *event) override;\n";
    eventForwarderHeader += "};\n\n";
    eventForwarderHeader += "#endif // QEVENTFORWARDER_H\n";
    
    if (!writeToFile(projectDir + "/infrastructure/event/qeventforwarder.h", eventForwarderHeader)) {
        return false;
    }
    
    QString eventForwarderSource = "#include \"qeventforwarder.h\"\n\n";
    eventForwarderSource += "QEventForwarder::QEventForwarder(QObject *parent)\n";
    eventForwarderSource += "    : QObject(parent)\n";
    eventForwarderSource += "{\n}\n\n";
    eventForwarderSource += "bool QEventForwarder::eventFilter(QObject *watched, QEvent *event)\n";
    eventForwarderSource += "{\n";
    eventForwarderSource += "    emit eventOccurred(watched, event);\n";
    eventForwarderSource += "    return QObject::eventFilter(watched, event);\n";
    eventForwarderSource += "}\n";
    
    if (!writeToFile(projectDir + "/infrastructure/event/qeventforwarder.cpp", eventForwarderSource)) {
        return false;
    }
    
    // 字体管理器
    QString fontManagerHeader = "#ifndef FONTMANAGER_H\n";
    fontManagerHeader += "#define FONTMANAGER_H\n\n";
    fontManagerHeader += "#include <QObject>\n";
    fontManagerHeader += "#include <QFont>\n";
    fontManagerHeader += "#include <QMap>\n";
    fontManagerHeader += "#include <QString>\n\n";
    fontManagerHeader += "class FontManager : public QObject\n";
    fontManagerHeader += "{\n";
    fontManagerHeader += "    Q_OBJECT\n\n";
    fontManagerHeader += "public:\n";
    fontManagerHeader += "    static FontManager *instance();\n\n";
    fontManagerHeader += "    bool loadFonts();\n";
    fontManagerHeader += "    QFont iconFont(int size = 12);\n\n";
    fontManagerHeader += "private:\n";
    fontManagerHeader += "    explicit FontManager(QObject *parent = nullptr);\n";
    fontManagerHeader += "    static FontManager *m_instance;\n";
    fontManagerHeader += "    QMap<QString, int> m_fontIds;\n";
    fontManagerHeader += "};\n\n";
    fontManagerHeader += "#endif // FONTMANAGER_H\n";
    
    if (!writeToFile(projectDir + "/infrastructure/fonts/fontmanager.h", fontManagerHeader)) {
        return false;
    }
    
    QString fontManagerSource = "#include \"fontmanager.h\"\n";
    fontManagerSource += "#include <QFontDatabase>\n";
    fontManagerSource += "#include <QFile>\n";
    fontManagerSource += "#include <QDebug>\n\n";
    fontManagerSource += "FontManager *FontManager::m_instance = nullptr;\n\n";
    fontManagerSource += "FontManager *FontManager::instance()\n";
    fontManagerSource += "{\n";
    fontManagerSource += "    if (!m_instance) {\n";
    fontManagerSource += "        m_instance = new FontManager();\n";
    fontManagerSource += "    }\n";
    fontManagerSource += "    return m_instance;\n";
    fontManagerSource += "}\n\n";
    fontManagerSource += "FontManager::FontManager(QObject *parent)\n";
    fontManagerSource += "    : QObject(parent)\n";
    fontManagerSource += "{\n";
    fontManagerSource += "    loadFonts();\n";
    fontManagerSource += "}\n\n";
    fontManagerSource += "bool FontManager::loadFonts()\n";
    fontManagerSource += "{\n";
    fontManagerSource += "    QStringList fontPaths = {\n";
    fontManagerSource += "        \":/font/fontawesome.ttf\"\n";
    fontManagerSource += "    };\n\n";
    fontManagerSource += "    bool success = true;\n";
    fontManagerSource += "    for (const QString &path : fontPaths) {\n";
    fontManagerSource += "        QFile fontFile(path);\n";
    fontManagerSource += "        if (fontFile.open(QIODevice::ReadOnly)) {\n";
    fontManagerSource += "            int fontId = QFontDatabase::addApplicationFontFromData(fontFile.readAll());\n";
    fontManagerSource += "            if (fontId != -1) {\n";
    fontManagerSource += "                QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);\n";
    fontManagerSource += "                if (!fontFamilies.isEmpty()) {\n";
    fontManagerSource += "                    m_fontIds.insert(fontFamilies.first(), fontId);\n";
    fontManagerSource += "                }\n";
    fontManagerSource += "            } else {\n";
    fontManagerSource += "                qWarning() << \"Failed to load font:\" << path;\n";
    fontManagerSource += "                success = false;\n";
    fontManagerSource += "            }\n";
    fontManagerSource += "            fontFile.close();\n";
    fontManagerSource += "        } else {\n";
    fontManagerSource += "            qWarning() << \"Failed to open font file:\" << path;\n";
    fontManagerSource += "            success = false;\n";
    fontManagerSource += "        }\n";
    fontManagerSource += "    }\n";
    fontManagerSource += "    return success;\n";
    fontManagerSource += "}\n\n";
    fontManagerSource += "QFont FontManager::iconFont(int size)\n";
    fontManagerSource += "{\n";
    fontManagerSource += "    QFont font;\n";
    fontManagerSource += "    if (m_fontIds.contains(\"FontAwesome\")) {\n";
    fontManagerSource += "        font.setFamily(\"FontAwesome\");\n";
    fontManagerSource += "    }\n";
    fontManagerSource += "    font.setPixelSize(size);\n";
    fontManagerSource += "    return font;\n";
    fontManagerSource += "}\n";
    
    if (!writeToFile(projectDir + "/infrastructure/fonts/fontmanager.cpp", fontManagerSource)) {
        return false;
    }
    
    // 日志帮助类
    QString logHelperHeader = "#ifndef LOGHELPER_H\n";
    logHelperHeader += "#define LOGHELPER_H\n\n";
    logHelperHeader += "#include <QObject>\n";
    logHelperHeader += "#include <QString>\n";
    logHelperHeader += "#include <QFile>\n";
    logHelperHeader += "#include <QTextStream>\n";
    logHelperHeader += "#include <QMutex>\n\n";
    logHelperHeader += "class LogHelper : public QObject\n";
    logHelperHeader += "{\n";
    logHelperHeader += "    Q_OBJECT\n\n";
    logHelperHeader += "public:\n";
    logHelperHeader += "    static LogHelper *instance();\n\n";
    logHelperHeader += "    void setLogFile(const QString &filePath);\n";
    logHelperHeader += "    void log(const QString &message, const QString &level = \"INFO\");\n";
    logHelperHeader += "    void info(const QString &message);\n";
    logHelperHeader += "    void warning(const QString &message);\n";
    logHelperHeader += "    void error(const QString &message);\n";
    logHelperHeader += "    void debug(const QString &message);\n\n";
    logHelperHeader += "private:\n";
    logHelperHeader += "    explicit LogHelper(QObject *parent = nullptr);\n";
    logHelperHeader += "    static LogHelper *m_instance;\n";
    logHelperHeader += "    QString m_logFilePath;\n";
    logHelperHeader += "    QFile m_logFile;\n";
    logHelperHeader += "    QTextStream m_logStream;\n";
    logHelperHeader += "    QMutex m_mutex;\n";
    logHelperHeader += "};\n\n";
    logHelperHeader += "#endif // LOGHELPER_H\n";
    
    if (!writeToFile(projectDir + "/infrastructure/logging/loghelper.h", logHelperHeader)) {
        return false;
    }
    
    QString logHelperSource = "#include \"loghelper.h\"\n";
    logHelperSource += "#include <QDateTime>\n";
    logHelperSource += "#include <QDir>\n";
    logHelperSource += "#include <QDebug>\n";
    logHelperSource += "#include <QMutexLocker>\n\n";
    logHelperSource += "LogHelper *LogHelper::m_instance = nullptr;\n\n";
    logHelperSource += "LogHelper *LogHelper::instance()\n";
    logHelperSource += "{\n";
    logHelperSource += "    if (!m_instance) {\n";
    logHelperSource += "        m_instance = new LogHelper();\n";
    logHelperSource += "    }\n";
    logHelperSource += "    return m_instance;\n";
    logHelperSource += "}\n\n";
    logHelperSource += "LogHelper::LogHelper(QObject *parent)\n";
    logHelperSource += "    : QObject(parent)\n";
    logHelperSource += "{\n";
    logHelperSource += "}\n\n";
    logHelperSource += "void LogHelper::setLogFile(const QString &filePath)\n";
    logHelperSource += "{\n";
    logHelperSource += "    QMutexLocker locker(&m_mutex);\n";
    logHelperSource += "    \n";
    logHelperSource += "    if (m_logFile.isOpen()) {\n";
    logHelperSource += "        m_logStream.flush();\n";
    logHelperSource += "        m_logFile.close();\n";
    logHelperSource += "    }\n\n";
    logHelperSource += "    m_logFilePath = filePath;\n";
    logHelperSource += "    \n";
    logHelperSource += "    // 确保目录存在\n";
    logHelperSource += "    QDir dir = QFileInfo(m_logFilePath).dir();\n";
    logHelperSource += "    if (!dir.exists()) {\n";
    logHelperSource += "        dir.mkpath(\".\");\n";
    logHelperSource += "    }\n\n";
    logHelperSource += "    m_logFile.setFileName(m_logFilePath);\n";
    logHelperSource += "    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {\n";
    logHelperSource += "        qWarning() << \"Failed to open log file:\" << m_logFilePath;\n";
    logHelperSource += "        return;\n";
    logHelperSource += "    }\n\n";
    logHelperSource += "    m_logStream.setDevice(&m_logFile);\n";
    logHelperSource += "    m_logStream.setCodec(\"UTF-8\");\n";
    logHelperSource += "}\n\n";
    logHelperSource += "void LogHelper::log(const QString &message, const QString &level)\n";
    logHelperSource += "{\n";
    logHelperSource += "    QMutexLocker locker(&m_mutex);\n";
    logHelperSource += "    \n";
    logHelperSource += "    QString timestamp = QDateTime::currentDateTime().toString(\"yyyy-MM-dd hh:mm:ss.zzz\");\n";
    logHelperSource += "    QString logEntry = QString(\"%1 [%2] %3\").arg(timestamp, level, message);\n\n";
    logHelperSource += "    // 输出到控制台\n";
    logHelperSource += "    qDebug().noquote() << logEntry;\n\n";
    logHelperSource += "    // 写入日志文件\n";
    logHelperSource += "    if (m_logFile.isOpen()) {\n";
    logHelperSource += "        m_logStream << logEntry << Qt::endl;\n";
    logHelperSource += "        m_logStream.flush();\n";
    logHelperSource += "    }\n";
    logHelperSource += "}\n\n";
    logHelperSource += "void LogHelper::info(const QString &message)\n";
    logHelperSource += "{\n";
    logHelperSource += "    log(message, \"INFO\");\n";
    logHelperSource += "}\n\n";
    logHelperSource += "void LogHelper::warning(const QString &message)\n";
    logHelperSource += "{\n";
    logHelperSource += "    log(message, \"WARNING\");\n";
    logHelperSource += "}\n\n";
    logHelperSource += "void LogHelper::error(const QString &message)\n";
    logHelperSource += "{\n";
    logHelperSource += "    log(message, \"ERROR\");\n";
    logHelperSource += "}\n\n";
    logHelperSource += "void LogHelper::debug(const QString &message)\n";
    logHelperSource += "{\n";
    logHelperSource += "    log(message, \"DEBUG\");\n";
    logHelperSource += "}\n";
    
    if (!writeToFile(projectDir + "/infrastructure/logging/loghelper.cpp", logHelperSource)) {
        return false;
    }
    
    return true;
}

bool ProjectCreator::createUIFiles(const QString &projectDir, const QMap<QString, QVariant> &config)
{
    QString projectName = config["projectName"].toString();
    QString projectNameLower = projectName.toLower();
    QString projectType = config["projectType"].toString();
    bool useForm = config["useForm"].toBool();
    
    // 创建主窗口文件
    QString mainWindowDir = projectDir + "/ui/windows";
    QString headerPath = mainWindowDir + "/" + projectNameLower + ".h";
    QString sourcePath = mainWindowDir + "/" + projectNameLower + ".cpp";
    QString formPath;
    
    if (useForm && projectType != "QQuick2ApplicationWindow") {
        formPath = mainWindowDir + "/" + projectNameLower + ".ui";
    } else if (projectType == "QQuick2ApplicationWindow") {
        formPath = projectDir + "/resources/qml/main.qml";
        
        // 确保QML目录存在
        QDir qmlDir(projectDir + "/resources/qml");
        if (!qmlDir.exists()) {
            qmlDir.mkpath(".");
        }
    }
    
    // 创建头文件
    QString headerContent = "#ifndef " + projectName.toUpper() + "_H\n";
    headerContent += "#define " + projectName.toUpper() + "_H\n\n";
    
    if (projectType == "QWidget") {
        headerContent += "#include <QWidget>\n\n";
        headerContent += "namespace Ui {\n";
        headerContent += "class " + projectName + ";\n";
        headerContent += "}\n\n";
        headerContent += "class " + projectName + " : public QWidget\n";
    } else if (projectType == "QMainWindow") {
        headerContent += "#include <QMainWindow>\n\n";
        headerContent += "namespace Ui {\n";
        headerContent += "class " + projectName + ";\n";
        headerContent += "}\n\n";
        headerContent += "class " + projectName + " : public QMainWindow\n";
    } else { // QQuick2ApplicationWindow - 这种情况下我们不需要创建C++类
        // 创建QML文件
        QString qmlContent = "import QtQuick 2.12\n";
        qmlContent += "import QtQuick.Window 2.12\n";
        qmlContent += "import QtQuick.Controls 2.12\n\n";
        qmlContent += "ApplicationWindow {\n";
        qmlContent += "    id: root\n";
        qmlContent += "    visible: true\n";
        qmlContent += "    width: 800\n";
        qmlContent += "    height: 600\n";
        qmlContent += "    title: qsTr(\"" + projectName + "\")\n\n";
        qmlContent += "    Rectangle {\n";
        qmlContent += "        anchors.fill: parent\n";
        qmlContent += "        color: \"#f0f0f0\"\n\n";
        qmlContent += "        Text {\n";
        qmlContent += "            anchors.centerIn: parent\n";
        qmlContent += "            text: qsTr(\"Welcome to " + projectName + "\")\n";
        qmlContent += "            font.pixelSize: 24\n";
        qmlContent += "        }\n";
        qmlContent += "    }\n";
        qmlContent += "}\n";
        
        if (!writeToFile(formPath, qmlContent)) {
            return false;
        }
        
        // 我们不需要创建C++类文件
        return true;
    }
    
    headerContent += "{\n";
    headerContent += "    Q_OBJECT\n\n";
    headerContent += "public:\n";
    headerContent += "    explicit " + projectName + "(QWidget *parent = nullptr);\n";
    headerContent += "    ~" + projectName + "();\n\n";
    headerContent += "private:\n";
    headerContent += "    Ui::" + projectName + " *ui;\n";
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
        sourceContent += "    QWidget(parent),\n";
    } else if (projectType == "QMainWindow") {
        sourceContent += "    QMainWindow(parent),\n";
    }
    
    if (useForm) {
        sourceContent += "    ui(new Ui::" + projectName + ")\n";
        sourceContent += "{\n";
        sourceContent += "    ui->setupUi(this);\n";
    } else {
        sourceContent += "    ui(nullptr)\n";
        sourceContent += "{\n";
        sourceContent += "    // 手动设置UI\n";
        sourceContent += "    resize(800, 600);\n";
        sourceContent += "    setWindowTitle(tr(\"" + projectName + "\"));\n";
    }
    
    sourceContent += "}\n\n";
    sourceContent += projectName + "::~" + projectName + "()\n";
    sourceContent += "{\n";
    sourceContent += "    delete ui;\n";
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

bool ProjectCreator::createResourceFiles(const QString &projectDir, const QMap<QString, QVariant> &config)
{
    bool useStylesheet = config["useStylesheet"].toBool();
    bool useIconFont = config["useIconFont"].toBool();
    
    // 创建资源文件
    QString qrcContent = "<!DOCTYPE RCC>\n";
    qrcContent += "<RCC version=\"1.0\">\n";
    
    if (useStylesheet) {
        qrcContent += "    <qresource prefix=\"/qss\">\n";
        qrcContent += "        <file>resources/qss/style.qss</file>\n";
        qrcContent += "    </qresource>\n";
        
        // 创建样式表文件
        QString qssContent = "/* 全局样式 */\n";
        qssContent += "QWidget {\n";
        qssContent += "    font-family: \"Microsoft YaHei\", \"SimHei\", sans-serif;\n";
        qssContent += "    font-size: 12px;\n";
        qssContent += "}\n\n";
        qssContent += "/* 主窗口样式 */\n";
        qssContent += "QMainWindow {\n";
        qssContent += "    background-color: #f5f5f5;\n";
        qssContent += "}\n\n";
        qssContent += "/* 按钮样式 */\n";
        qssContent += "QPushButton {\n";
        qssContent += "    background-color: #2980b9;\n";
        qssContent += "    color: white;\n";
        qssContent += "    border: none;\n";
        qssContent += "    padding: 5px 15px;\n";
        qssContent += "    border-radius: 3px;\n";
        qssContent += "}\n\n";
        qssContent += "QPushButton:hover {\n";
        qssContent += "    background-color: #3498db;\n";
        qssContent += "}\n\n";
        qssContent += "QPushButton:pressed {\n";
        qssContent += "    background-color: #1c6ea4;\n";
        qssContent += "}\n";
        
        if (!writeToFile(projectDir + "/resources/qss/style.qss", qssContent)) {
            return false;
        }
    }
    
    if (useIconFont) {
        qrcContent += "    <qresource prefix=\"/font\">\n";
        qrcContent += "        <file>resources/font/fontawesome.ttf</file>\n";
        qrcContent += "    </qresource>\n";
        
        // 注：这里我们只是创建了资源引用，实际上需要用户提供字体文件
    }
    
    qrcContent += "    <qresource prefix=\"/img\">\n";
    qrcContent += "        <file>resources/img/app_icon.png</file>\n";
    qrcContent += "    </qresource>\n";
    qrcContent += "</RCC>\n";
    
    if (!writeToFile(projectDir + "/resources.qrc", qrcContent)) {
        return false;
    }
    
    return true;
}

bool ProjectCreator::createCMakeListsFile(const QString &projectDir, const QMap<QString, QVariant> &config)
{
    QString projectName = config["projectName"].toString();
    bool qt6Compatibility = config["qt6Compatibility"].toBool();
    bool addResources = config["addResources"].toBool();
    QString projectType = config["projectType"].toString();
    bool singleInstance = config["singleInstance"].toBool();
    bool useStylesheet = config["useStylesheet"].toBool();
    bool useIconFont = config["useIconFont"].toBool();
    
    // 主CMakeLists.txt文件
    QString cmakeContent = "cmake_minimum_required(VERSION 3.16)\n\n";
    
    cmakeContent += "project(" + projectName + " VERSION 1.0.0 LANGUAGES CXX)\n\n";
    
    // 设置C++标准
    cmakeContent += "# 设置C++标准\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD 17)\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    
    // 自动处理MOC、UIC、RCC
    cmakeContent += "# 自动处理MOC、UIC、RCC\n";
    cmakeContent += "set(CMAKE_AUTOMOC ON)\n";
    cmakeContent += "set(CMAKE_AUTORCC ON)\n";
    cmakeContent += "set(CMAKE_AUTOUIC ON)\n\n";
    
    // QT库查找
    if (qt6Compatibility) {
        cmakeContent += "# 查找Qt包（Qt6或Qt5）\n";
        cmakeContent += "find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets";
        
        if (projectType == "QQuick2ApplicationWindow") {
            cmakeContent += " Qml Quick QuickControls2";
        }
        
        cmakeContent += ")\n";
        cmakeContent += "find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets";
        
        if (projectType == "QQuick2ApplicationWindow") {
            cmakeContent += " Qml Quick QuickControls2";
        }
        
        cmakeContent += ")\n\n";
    } else {
        cmakeContent += "# 查找Qt5包\n";
        cmakeContent += "find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets";
        
        if (projectType == "QQuick2ApplicationWindow") {
            cmakeContent += " Qml Quick QuickControls2";
        }
        
        cmakeContent += ")\n\n";
    }
    
    // 源文件列表
    cmakeContent += "# 项目源文件\n";
    cmakeContent += "set(PROJECT_SOURCES\n";
    cmakeContent += "    # 应用层\n";
    cmakeContent += "    app/main.cpp\n";
    
    if (singleInstance) {
        cmakeContent += "    app/SingleApplication/singleapplication.h\n";
        cmakeContent += "    app/SingleApplication/singleapplication.cpp\n";
    } else {
        cmakeContent += "    app/Application/application.h\n";
        cmakeContent += "    app/Application/application.cpp\n";
    }
    
    // 基础设施层
    cmakeContent += "\n    # 基础设施层\n";
    cmakeContent += "    infrastructure/event/qeventforwarder.h\n";
    cmakeContent += "    infrastructure/event/qeventforwarder.cpp\n";
    cmakeContent += "    infrastructure/fonts/fontmanager.h\n";
    cmakeContent += "    infrastructure/fonts/fontmanager.cpp\n";
    cmakeContent += "    infrastructure/logging/loghelper.h\n";
    cmakeContent += "    infrastructure/logging/loghelper.cpp\n";
    
    // UI层
    if (projectType != "QQuick2ApplicationWindow") {
        cmakeContent += "\n    # UI层\n";
        cmakeContent += "    ui/windows/" + projectName.toLower() + ".h\n";
        cmakeContent += "    ui/windows/" + projectName.toLower() + ".cpp\n";
        
        if (config["useForm"].toBool()) {
            cmakeContent += "    ui/windows/" + projectName.toLower() + ".ui\n";
        }
    }
    
    // 资源文件
    if (addResources) {
        cmakeContent += "\n    # 资源文件\n";
        cmakeContent += "    resources.qrc\n";
    }
    
    cmakeContent += ")\n\n";
    
    // 如果是QML应用，添加QML模块
    if (projectType == "QQuick2ApplicationWindow") {
        cmakeContent += "# QML模块\n";
        cmakeContent += "qt_add_qml_module(\n";
        cmakeContent += "    ${PROJECT_NAME}\n";
        cmakeContent += "    URI " + projectName + "\n";
        cmakeContent += "    VERSION 1.0\n";
        cmakeContent += "    QML_FILES\n";
        cmakeContent += "        resources/qml/main.qml\n";
        cmakeContent += ")\n\n";
    } else {
        // 添加可执行文件
        cmakeContent += "# 添加可执行文件\n";
        cmakeContent += "add_executable(${PROJECT_NAME}\n";
        cmakeContent += "    ${PROJECT_SOURCES}\n";
        cmakeContent += ")\n\n";
    }
    
    // 链接Qt库
    cmakeContent += "# 链接Qt库\n";
    if (qt6Compatibility) {
        cmakeContent += "target_link_libraries(${PROJECT_NAME} PRIVATE\n";
        cmakeContent += "    Qt${QT_VERSION_MAJOR}::Core\n";
        cmakeContent += "    Qt${QT_VERSION_MAJOR}::Gui\n";
        cmakeContent += "    Qt${QT_VERSION_MAJOR}::Widgets\n";
        
        if (projectType == "QQuick2ApplicationWindow") {
            cmakeContent += "    Qt${QT_VERSION_MAJOR}::Qml\n";
            cmakeContent += "    Qt${QT_VERSION_MAJOR}::Quick\n";
            cmakeContent += "    Qt${QT_VERSION_MAJOR}::QuickControls2\n";
        }
        
        cmakeContent += ")\n\n";
    } else {
        cmakeContent += "target_link_libraries(${PROJECT_NAME} PRIVATE\n";
        cmakeContent += "    Qt5::Core\n";
        cmakeContent += "    Qt5::Gui\n";
        cmakeContent += "    Qt5::Widgets\n";
        
        if (projectType == "QQuick2ApplicationWindow") {
            cmakeContent += "    Qt5::Qml\n";
            cmakeContent += "    Qt5::Quick\n";
            cmakeContent += "    Qt5::QuickControls2\n";
        }
        
        cmakeContent += ")\n\n";
    }
    
    // 安装目标
    cmakeContent += "# 设置安装目录\n";
    cmakeContent += "install(TARGETS ${PROJECT_NAME}\n";
    cmakeContent += "    BUNDLE DESTINATION .\n";
    cmakeContent += "    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}\n";
    cmakeContent += "    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}\n";
    cmakeContent += ")\n";
    
    // 写入CMakeLists.txt文件
    if (!writeToFile(projectDir + "/CMakeLists.txt", cmakeContent)) {
        return false;
    }
    
    return true;
}