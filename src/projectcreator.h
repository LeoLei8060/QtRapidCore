#ifndef PROJECTCREATOR_H
#define PROJECTCREATOR_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QDir>
#include <QFile>
#include <QTextStream>

class ProjectCreator
{
public:
    ProjectCreator();
    ~ProjectCreator();
    
    // 创建项目
    bool createProject(const QMap<QString, QVariant> &config);
    
    // 获取最后一个错误
    QString lastError() const;
    
private:
    QString m_lastError;
    
    // 创建目录结构
    bool createDirectoryStructure(const QString &projectDir);
    
    // 创建应用层文件
    bool createAppFiles(const QString &projectDir, const QMap<QString, QVariant> &config);
    
    // 创建基础设施层文件
    bool createInfrastructureFiles(const QString &projectDir);
    
    // 创建UI层文件
    bool createUIFiles(const QString &projectDir, const QMap<QString, QVariant> &config);
    
    // 创建资源文件
    bool createResourceFiles(const QString &projectDir, const QMap<QString, QVariant> &config);
    
    // 创建CMakeLists.txt文件
    bool createCMakeListsFile(const QString &projectDir, const QMap<QString, QVariant> &config);
    
    // 辅助函数：创建文件并写入内容
    bool writeToFile(const QString &filePath, const QString &content);
    
    // 辅助函数：确保目录存在
    bool ensureDirectoryExists(const QString &dirPath);
    
    // 辅助函数：复制目录内容（包含子目录）
    bool copyDirectory(const QString &sourceDir, const QString &destDir, bool recursive = true);
    
    // 辅助函数：复制单个文件
    bool copyFile(const QString &sourceFilePath, const QString &targetFilePath);
};

#endif // PROJECTCREATOR_H
