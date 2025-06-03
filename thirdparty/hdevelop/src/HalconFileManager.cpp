//
// Created by 开发团队 on 2025-01-31.
// Halcon文件管理工具类 | Halcon File Manager Utility Class
//

#include "../include/HalconFileManager.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>  // Qt 5.12兼容：使用QRegularExpression替代QRegExp

HalconFileManager::HalconFileManager(QObject *parent) : QObject(parent)
{
    initializeExtensionMap();
    
    // 🏗️ 初始化标准目录结构
    m_standardDirectories << "models" << "images" << "regions" << "params" 
                         << "results" << "calibration" << "temp" << "backup";
}

void HalconFileManager::initializeExtensionMap()
{
    // 🎯 初始化Halcon专用文件扩展名映射表
    // 优先推荐使用.hobj作为通用格式
    m_extensionMap = {
        // 🌟 通用对象文件 - 推荐优先使用 | Universal Object Files - Recommended Priority
        {"object", ".hobj"},           // 通用Halcon对象文件 - 可存储图像、区域、XLD等
        {"universal", ".hobj"},        // 万能格式别名
        {"general", ".hobj"},          // 通用格式别名
        
        // 基础对象文件 | Basic Object Files (保留用于特殊需求)
        {"region", ".hobj"},           // 区域对象 - 建议使用.hobj
        {"image", ".hobj"},            // 图像对象 - 建议使用.hobj  
        {"xld", ".hobj"},              // XLD轮廓对象 - 建议使用.hobj
        
        // 特殊格式 - 仅在需要时使用 | Special Formats - Use when necessary
        {"region_special", ".hreg"},   // 特殊区域文件格式
        {"image_special", ".himage"},  // 特殊图像文件格式
        {"xld_special", ".hxld"},      // 特殊XLD文件格式
        
        // 模型文件 | Model Files
        {"shape_model", ".shm"},       // 形状模型文件
        {"ncc_model", ".ncm"},         // NCC模型文件
        {"surface_model", ".som"},     // 表面模型文件
        {"3d_model", ".h3d"},          // 3D模型文件
        
        // 参数和数据文件 | Parameter and Data Files
        {"tuple", ".tup"},             // Tuple参数文件
        {"dict", ".hdict"},            // Halcon字典文件
        {"calib_data", ".calb"},       // 相机标定数据
        {"matrix", ".hmat"},           // 矩阵文件
        
        // 测量相关 | Measurement Related
        {"measure_model", ".hmea"},    // 测量模板文件
        {"measure_data", ".mdat"},     // 测量数据文件
        
        // 图像处理 | Image Processing
        {"filter", ".hfil"},           // 滤波器文件
        {"lut", ".hlut"},              // 查找表文件
        
        // 检测相关 | Detection Related
        {"template", ".htmp"},         // 通用模板文件
        {"roi", ".hroi"},              // 感兴趣区域文件
        
        // 序列化文件 | Serialized Files
        {"serialized", ".hser"}        // 序列化对象文件
    };
}

QString HalconFileManager::getFileExtension(const QString& fileType) const
{
    return m_extensionMap.value(fileType, ".hobj"); // 默认返回.hobj
}

QString HalconFileManager::generateFileName(const QString& baseName, const QString& fileType) const
{
    QString extension = getFileExtension(fileType);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
    return QString("%1_%2%3").arg(baseName, timestamp, extension);
}

QString HalconFileManager::generateUniqueFileName(const QString& basePath, const QString& baseName, const QString& fileType) const
{
    QString extension = getFileExtension(fileType);
    QString baseFileName = QString("%1%2").arg(baseName, extension);
    QString fullPath = QDir(basePath).absoluteFilePath(baseFileName);
    
    // 🔄 如果文件已存在，添加数字后缀
    int counter = 1;
    while (QFile::exists(fullPath)) {
        QString numberedFileName = QString("%1_%2%3").arg(baseName).arg(counter).arg(extension);
        fullPath = QDir(basePath).absoluteFilePath(numberedFileName);
        counter++;
    }
    
    return QFileInfo(fullPath).fileName();
}

QString HalconFileManager::generateHobjFileName(const QString& baseName, const QString& objectType) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
    
    if (objectType.isEmpty()) {
        return QString("%1_%2.hobj").arg(baseName, timestamp);
    } else {
        return QString("%1_%2_%3.hobj").arg(baseName, objectType, timestamp);
    }
}

QString HalconFileManager::generateUniqueHobjFileName(const QString& basePath, const QString& baseName, const QString& objectType) const
{
    QString baseFileName;
    if (objectType.isEmpty()) {
        baseFileName = QString("%1.hobj").arg(baseName);
    } else {
        baseFileName = QString("%1_%2.hobj").arg(baseName, objectType);
    }
    
    QString fullPath = QDir(basePath).absoluteFilePath(baseFileName);
    
    // 🔄 如果文件已存在，添加数字后缀
    int counter = 1;
    while (QFile::exists(fullPath)) {
        QString numberedFileName;
        if (objectType.isEmpty()) {
            numberedFileName = QString("%1_%2.hobj").arg(baseName).arg(counter);
        } else {
            numberedFileName = QString("%1_%2_%3.hobj").arg(baseName, objectType).arg(counter);
        }
        fullPath = QDir(basePath).absoluteFilePath(numberedFileName);
        counter++;
    }
    
    return QFileInfo(fullPath).fileName();
}

bool HalconFileManager::isHobjFile(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower() == "hobj" && fileInfo.exists();
}

QString HalconFileManager::getHobjFileDescription(const QString& objectType) const
{
    // 🎯 返回HOBJ文件类型的详细描述
    QMap<QString, QString> hobjDescriptions = {
        {"region", "HOBJ区域对象文件 | HOBJ Region Object File - 存储ROI、掩膜等区域数据"},
        {"image", "HOBJ图像对象文件 | HOBJ Image Object File - 存储图像数据，支持多通道"},
        {"xld", "HOBJ轮廓对象文件 | HOBJ XLD Contour File - 存储边缘、线条等轮廓数据"},
        {"contour", "HOBJ轮廓对象文件 | HOBJ Contour Object File - 存储检测到的轮廓"},
        {"template", "HOBJ模板对象文件 | HOBJ Template Object File - 存储匹配模板"},
        {"measurement", "HOBJ测量对象文件 | HOBJ Measurement Object File - 存储测量相关对象"},
        {"detection", "HOBJ检测对象文件 | HOBJ Detection Object File - 存储检测结果对象"},
        {"qr_region", "HOBJ二维码区域文件 | HOBJ QR Code Region File - 存储二维码识别区域"},
        {"measure_region", "HOBJ测量区域文件 | HOBJ Measurement Region File - 存储测量区域"},
        {"check_region", "HOBJ检测区域文件 | HOBJ Check Region File - 存储检测区域"},
        {"", "HOBJ通用对象文件 | HOBJ Universal Object File - 可存储任意Halcon对象"}
    };
    
    return hobjDescriptions.value(objectType, 
        "HOBJ通用对象文件 | HOBJ Universal Object File - Halcon通用二进制格式，支持多种对象类型");
}

// 🈚 中文文件名支持方法实现 | Chinese Filename Support Methods Implementation

QString HalconFileManager::sanitizeChineseFileName(const QString& fileName) const
{
    QString sanitized = fileName;
    
    // 🚫 移除或替换Windows文件名中的非法字符
    QStringList invalidChars = {"<", ">", ":", "\"", "/", "\\", "|", "?", "*"};
    for (const QString& invalidChar : invalidChars) {
        sanitized = sanitized.replace(invalidChar, "_");
    }
    
    // 🔧 处理特殊情况
    sanitized = sanitized.trimmed();
    
    // 🚫 避免保留的文件名
    QStringList reservedNames = {"CON", "PRN", "AUX", "NUL", 
                                "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
                                "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};
    
    QString baseName = QFileInfo(sanitized).baseName().toUpper();
    if (reservedNames.contains(baseName)) {
        sanitized = "_" + sanitized;
    }
    
    // 📏 限制文件名长度（Windows限制为255字符）
    if (sanitized.length() > 200) {  // 留出扩展名空间
        sanitized = sanitized.left(200);
    }
    
    return sanitized;
}

bool HalconFileManager::testChineseFileNameSupport(const QString& testPath) const
{
    QString testDir = testPath.isEmpty() ? QDir::currentPath() + "/temp_test" : testPath;
    
    // 🧪 创建测试目录
    if (!QDir().mkpath(testDir)) {
        qDebug() << "⚠️ 无法创建测试目录:" << testDir;
        return false;
    }
    
    // 🈚 测试中文文件名
    QString chineseTestFile = QDir(testDir).absoluteFilePath("测试中文文件名_test.hobj");
    
    try {
        // 📝 尝试创建测试文件
        QFile testFile(chineseTestFile);
        if (!testFile.open(QIODevice::WriteOnly)) {
            qDebug() << "❌ 无法创建中文文件名测试文件:" << chineseTestFile;
            return false;
        }
        
        // ✍️ 写入测试内容
        testFile.write("测试中文文件名支持 | Test Chinese filename support");
        testFile.close();
        
        // 📖 尝试读取测试文件
        if (!testFile.open(QIODevice::ReadOnly)) {
            qDebug() << "❌ 无法读取中文文件名测试文件:" << chineseTestFile;
            return false;
        }
        
        QByteArray content = testFile.readAll();
        testFile.close();
        
        // 🗑️ 清理测试文件
        QFile::remove(chineseTestFile);
        QDir().rmdir(testDir);
        
        qDebug() << "✅ 中文文件名支持测试成功";
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "❌ 中文文件名测试异常:" << QString::fromLocal8Bit(e.what());
        return false;
    } catch (...) {
        qDebug() << "❌ 中文文件名测试发生未知异常";
        return false;
    }
}

QString HalconFileManager::convertToSafeFileName(const QString& originalName) const
{
    QString safeName = originalName;
    
    // 🔄 首先进行基本的清理
    safeName = sanitizeChineseFileName(safeName);
    
    // 🈚 如果包含中文，使用UTF-8编码确保兼容性
    QRegularExpression chineseRegex("[\\x4e00-\\x9fa5]");
    if (safeName.contains(chineseRegex)) {
        // 中文字符范围检测
        qDebug() << "检测到中文字符，使用UTF-8编码处理:" << safeName;
        
        // 🔧 对于HALCON，建议添加英文前缀以增强兼容性
        QRegularExpression englishStartRegex("^[a-zA-Z]");
        if (!englishStartRegex.match(safeName).hasMatch()) {
            safeName = "zh_" + safeName;
        }
    }
    
    // 📅 添加时间戳以避免重复
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QFileInfo fileInfo(safeName);
    
    if (fileInfo.suffix().isEmpty()) {
        safeName = QString("%1_%2").arg(fileInfo.baseName(), timestamp);
    } else {
        safeName = QString("%1_%2.%3").arg(fileInfo.baseName(), timestamp, fileInfo.suffix());
    }
    
    return safeName;
}

bool HalconFileManager::validateChineseFilePath(const QString& filePath, QString& errorMessage) const
{
    QFileInfo fileInfo(filePath);
    
    // 📂 检查目录是否存在或可创建
    QDir parentDir = fileInfo.dir();
    if (!parentDir.exists()) {
        if (!parentDir.mkpath(".")) {
            errorMessage = tr("无法创建目录: %1").arg(parentDir.absolutePath());
            return false;
        }
    }
    
    // 📝 检查文件名合法性
    QString fileName = fileInfo.fileName();
    if (fileName.isEmpty()) {
        errorMessage = tr("文件名不能为空");
        return false;
    }
    
    // 🈚 检查中文字符编码
    QRegularExpression chineseRegex("[\\x4e00-\\x9fa5]");
    if (fileName.contains(chineseRegex)) {
        // 🧪 执行中文文件名支持测试
        QString testDir = parentDir.absolutePath() + "/.test_chinese";
        if (!testChineseFileNameSupport(testDir)) {
            errorMessage = tr("当前系统或位置不支持中文文件名，建议使用英文文件名");
            return false;
        }
    }
    
    // 📏 检查路径长度（Windows路径限制）
    QString absolutePath = fileInfo.absoluteFilePath();
    if (absolutePath.length() > 260) {
        errorMessage = tr("文件路径过长（超过260字符），请缩短文件名或目录名");
        return false;
    }
    
    // ✅ 检查写入权限
    QString testFileName = parentDir.absoluteFilePath(".write_test_" + QDateTime::currentDateTime().toString("hhmmss"));
    QFile testFile(testFileName);
    if (!testFile.open(QIODevice::WriteOnly)) {
        errorMessage = tr("目录没有写入权限: %1").arg(parentDir.absolutePath());
        return false;
    }
    testFile.close();
    QFile::remove(testFileName);
    
    errorMessage = tr("文件路径验证通过");
    return true;
}

bool HalconFileManager::createDirectoryStructure(const QString& basePath) const
{
    QDir baseDir(basePath);
    if (!baseDir.exists()) {
        if (!baseDir.mkpath(".")) {
            qDebug() << "❌ 无法创建基础目录:" << basePath;
            return false;
        }
    }
    
    // 🏗️ 创建标准子目录
    bool allSuccess = true;
    for (const QString& subDir : m_standardDirectories) {
        QString fullPath = QDir(basePath).absoluteFilePath(subDir);
        if (!QDir().mkpath(fullPath)) {
            qDebug() << "❌ 无法创建子目录:" << fullPath;
            allSuccess = false;
        } else {
            qDebug() << "✅ 创建目录:" << fullPath;
        }
    }
    
    return allSuccess;
}

bool HalconFileManager::createProjectDirectory(const QString& projectName, const QString& basePath) const
{
    QString projectPath = QDir(basePath).absoluteFilePath(projectName);
    return createDirectoryStructure(projectPath);
}

FileStats HalconFileManager::analyzeDirectory(const QString& directoryPath) const
{
    FileStats stats;
    QDir dir(directoryPath);
    
    if (!dir.exists()) {
        qDebug() << "⚠️ 目录不存在:" << directoryPath;
        return stats;
    }
    
    // 🔍 递归扫描所有文件
    QDirIterator it(directoryPath, QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        
        // 📊 更新统计信息
        stats.totalFiles++;
        stats.totalSize += fileInfo.size();
        
        // 🔍 检查是否是Halcon文件
        QString fileType = getFileTypeFromExtension(filePath);
        if (!fileType.isEmpty()) {
            stats.fileTypeCount[fileType]++;
        }
        
        // 📈 跟踪最大文件
        if (fileInfo.size() > stats.largestFileSize) {
            stats.largestFileSize = fileInfo.size();
            stats.largestFileName = fileInfo.fileName();
        }
        
        // 📅 跟踪最新修改文件
        if (fileInfo.lastModified() > stats.lastModified) {
            stats.lastModified = fileInfo.lastModified();
            stats.lastModifiedFile = fileInfo.fileName();
        }
    }
    
    return stats;
}

QStringList HalconFileManager::findFilesByType(const QString& directoryPath, const QString& fileType) const
{
    QStringList foundFiles;
    QString extension = getFileExtension(fileType);
    QString pattern = QString("*%1").arg(extension);
    
    QDir dir(directoryPath);
    QStringList filters;
    filters << pattern;
    
    // 🔍 递归搜索
    QDirIterator it(directoryPath, filters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        foundFiles << it.next();
    }
    
    return foundFiles;
}

QStringList HalconFileManager::getAllHalconFiles(const QString& directoryPath) const
{
    QStringList allFiles;
    
    // 🔍 搜索所有支持的Halcon文件类型
    for (auto it = m_extensionMap.begin(); it != m_extensionMap.end(); ++it) {
        QStringList typeFiles = findFilesByType(directoryPath, it.key());
        allFiles.append(typeFiles);
    }
    
    allFiles.removeDuplicates();
    allFiles.sort();
    
    return allFiles;
}

bool HalconFileManager::cleanupOldFiles(const QString& directoryPath, int daysOld) const
{
    QDir dir(directoryPath);
    if (!dir.exists()) {
        return false;
    }
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-daysOld);
    bool hasDeleted = false;
    
    // 🗑️ 删除旧文件
    QDirIterator it(directoryPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        
        if (fileInfo.lastModified() < cutoffDate && isValidHalconFile(filePath)) {
            if (QFile::remove(filePath)) {
                qDebug() << "🗑️ 已删除旧文件:" << fileInfo.fileName();
                hasDeleted = true;
            }
        }
    }
    
    return hasDeleted;
}

bool HalconFileManager::backupFile(const QString& filePath, const QString& backupDirectory) const
{
    QFileInfo sourceInfo(filePath);
    if (!sourceInfo.exists()) {
        return false;
    }
    
    // 🎯 确定备份目录
    QString backupDir = backupDirectory.isEmpty() ? 
                       sourceInfo.dir().absoluteFilePath("backup") : backupDirectory;
    
    QDir().mkpath(backupDir);
    
    // 📁 生成备份文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString backupFileName = QString("%1_%2.%3")
                            .arg(sourceInfo.baseName())
                            .arg(timestamp)
                            .arg(sourceInfo.completeSuffix());
    
    QString backupFilePath = QDir(backupDir).absoluteFilePath(backupFileName);
    
    // 💾 执行备份
    if (QFile::copy(filePath, backupFilePath)) {
        qDebug() << "💾 备份成功:" << backupFilePath;
        return true;
    }
    
    return false;
}

bool HalconFileManager::isValidHalconFile(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }
    
    // ✅ 检查扩展名是否在支持列表中
    QString extension = "." + fileInfo.completeSuffix();
    return m_extensionMap.values().contains(extension);
}

QString HalconFileManager::getFileTypeFromExtension(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString extension = "." + fileInfo.completeSuffix();
    
    // 🔍 查找对应的文件类型
    for (auto it = m_extensionMap.begin(); it != m_extensionMap.end(); ++it) {
        if (it.value() == extension) {
            return it.key();
        }
    }
    
    return QString(); // 未找到对应类型
}

QStringList HalconFileManager::getSupportedFileTypes() const
{
    return m_extensionMap.keys();
}

QMap<QString, QString> HalconFileManager::getFileTypeDescriptions() const
{
    // 🎯 返回文件类型的中英文描述，优先推荐.hobj格式
    QMap<QString, QString> descriptions = {
        // 🌟 优先推荐的通用格式
        {"object", "🌟 HOBJ通用对象文件 | HOBJ Universal Object File - 推荐使用，可存储图像、区域、XLD等所有Halcon对象"},
        {"universal", "🌟 HOBJ万能格式 | HOBJ Universal Format - 最灵活的Halcon文件格式"},
        {"general", "🌟 HOBJ通用格式 | HOBJ General Format - 适用于所有Halcon对象类型"},
        
        // 基础对象类型（现在都使用.hobj）
        {"region", "HOBJ区域对象 | HOBJ Region Object - 使用.hobj格式存储区域数据"},
        {"image", "HOBJ图像对象 | HOBJ Image Object - 使用.hobj格式存储图像数据"},
        {"xld", "HOBJ轮廓对象 | HOBJ XLD Contour - 使用.hobj格式存储轮廓数据"},
        
        // 特殊格式（仅在特殊需求时使用）
        {"region_special", "特殊区域文件 | Special Region File - 仅在需要特定格式时使用.hreg"},
        {"image_special", "特殊图像文件 | Special Image File - 仅在需要特定格式时使用.himage"},
        {"xld_special", "特殊XLD文件 | Special XLD File - 仅在需要特定格式时使用.hxld"},
        
        // 模型文件（保持特定格式）
        {"shape_model", "形状模型文件 | Shape Model File - .shm格式"},
        {"ncc_model", "NCC模型文件 | NCC Model File - .ncm格式"},
        {"surface_model", "表面模型文件 | Surface Model File - .som格式"},
        {"3d_model", "3D模型文件 | 3D Model File - .h3d格式"},
        
        // 参数和数据文件
        {"tuple", "Tuple参数文件 | Tuple Parameter File - .tup格式"},
        {"dict", "Halcon字典文件 | Halcon Dictionary File - .hdict格式"},
        {"calib_data", "相机标定数据 | Camera Calibration Data - .calb格式"},
        {"matrix", "矩阵文件 | Matrix File - .hmat格式"},
        
        // 测量相关
        {"measure_model", "测量模板文件 | Measurement Model File - .hmea格式"},
        {"measure_data", "测量数据文件 | Measurement Data File - .mdat格式"},
        
        // 图像处理
        {"filter", "滤波器文件 | Filter File - .hfil格式"},
        {"lut", "查找表文件 | Look-up Table File - .hlut格式"},
        
        // 检测相关
        {"template", "通用模板文件 | Generic Template File - .htmp格式"},
        {"roi", "感兴趣区域文件 | Region of Interest File - .hroi格式"},
        
        // 序列化文件
        {"serialized", "序列化对象文件 | Serialized Object File - .hser格式"}
    };
    
    return descriptions;
}

QString HalconFileManager::formatFileSize(qint64 size) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (size >= GB) {
        return QString::number(size / GB, 'f', 2) + " GB";
    } else if (size >= MB) {
        return QString::number(size / MB, 'f', 2) + " MB";
    } else if (size >= KB) {
        return QString::number(size / KB, 'f', 2) + " KB";
    } else {
        return QString::number(size) + " bytes";
    }
} 