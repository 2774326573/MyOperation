#ifndef HALCONFILEMANAGER_H
#define HALCONFILEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QDirIterator>
#include "halconcpp/HalconCpp.h"

using namespace HalconCpp;

/**
 * @brief 文件统计信息结构体 | File Statistics Structure
 */
struct FileStats {
    int totalFiles = 0;                          // 总文件数 | Total files
    qint64 totalSize = 0;                        // 总大小 | Total size
    QMap<QString, int> fileTypeCount;            // 文件类型计数 | File type count
    QString largestFileName;                     // 最大文件名 | Largest file name
    qint64 largestFileSize = 0;                  // 最大文件大小 | Largest file size
    QDateTime lastModified;                      // 最后修改时间 | Last modified time
    QString lastModifiedFile;                    // 最后修改的文件 | Last modified file
};

/**
 * @brief Halcon文件管理器 | Halcon File Manager
 * 
 * 🎯 专门管理Halcon视觉处理库中各种专用文件格式的工具类
 * A utility class specifically designed to manage various file formats in Halcon vision processing library
 * 
 * @author 开发团队 | Development Team
 * @date 2025-01-31
 * @version 1.0
 */
class HalconFileManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化Halcon文件管理器
     * Constructor, initializes the Halcon File Manager
     *
     * @param parent 父QObject对象，用于Qt内存管理 | Parent QObject for Qt memory management
     */
    explicit HalconFileManager(QObject *parent = nullptr);
    
    // 🎯 文件扩展名管理 | File Extension Management
    /**
     * @brief 获取指定文件类型的文件扩展名
     * Get the file extension for the specified file type
     *
     * @param fileType 文件类型标识符 | File type identifier
     * @return QString 对应的文件扩展名（包括点，如".hobj"）| Corresponding file extension (including dot, e.g. ".hobj")
     */
    QString getFileExtension(const QString& fileType) const;

    /**
     * @brief 根据基础名称和文件类型生成完整文件名
     * Generate a complete filename based on base name and file type
     *
     * @param baseName 文件基础名称 | Base name of the file
     * @param fileType 文件类型标识符 | File type identifier
     * @return QString 生成的完整文件名（带扩展名）| Generated complete filename with extension
     */
    QString generateFileName(const QString& baseName, const QString& fileType) const;

    /**
     * @brief 生成唯一的文件名，确保在指定路径下不会重名
     * Generate a unique filename, ensuring no duplicates in the specified path
     *
     * @param basePath 基础路径 | Base path
     * @param baseName 文件基础名称 | Base name of the file
     * @param fileType 文件类型标识符 | File type identifier
     * @return QString 唯一的完整文件名（带扩展名）| Unique complete filename with extension
     */
    QString generateUniqueFileName(const QString& basePath, const QString& baseName, const QString& fileType) const;
    
    // 🌟 HOBJ通用格式专用方法 | HOBJ Universal Format Specific Methods
    /**
     * @brief 生成Halcon对象文件名
     * Generate a Halcon object filename
     *
     * @param baseName 文件基础名称 | Base name of the file
     * @param objectType Halcon对象类型（如"region"、"xld_contour"等）| Halcon object type (e.g. "region", "xld_contour")
     * @return QString 生成的HOBJ文件名 | Generated HOBJ filename
     */
    QString generateHobjFileName(const QString& baseName, const QString& objectType = "") const;

    /**
     * @brief 生成唯一的Halcon对象文件名
     * Generate a unique Halcon object filename
     *
     * @param basePath 基础路径 | Base path
     * @param baseName 文件基础名称 | Base name of the file
     * @param objectType Halcon对象类型 | Halcon object type
     * @return QString 唯一的HOBJ文件名 | Unique HOBJ filename
     */
    QString generateUniqueHobjFileName(const QString& basePath, const QString& baseName, const QString& objectType = "") const;

    /**
     * @brief 检查文件是否为HOBJ格式
     * Check if a file is in HOBJ format
     *
     * @param filePath 文件路径 | File path
     * @return bool 如果是HOBJ文件返回true，否则false | Returns true if it is an HOBJ file, otherwise false
     */
    bool isHobjFile(const QString& filePath) const;

    /**
     * @brief 获取HOBJ对象类型的描述信息
     * Get description for a Halcon object type
     *
     * @param objectType Halcon对象类型 | Halcon object type
     * @return QString 对象类型的描述 | Description of the object type
     */
    QString getHobjFileDescription(const QString& objectType) const;
    
    // 🈚 中文文件名支持方法 | Chinese Filename Support Methods
    /**
     * @brief 清理中文文件名中的非法字符
     * Sanitize illegal characters in Chinese filenames
     *
     * @param fileName 原始文件名 | Original filename
     * @return QString 清理后的安全文件名 | Sanitized safe filename
     */
    QString sanitizeChineseFileName(const QString& fileName) const;

    /**
     * @brief 测试当前系统对中文文件名的支持情况
     * Test the current system's support for Chinese filenames
     *
     * @param testPath 测试路径，默认使用临时目录 | Test path, uses temp directory by default
     * @return bool 如果支持中文文件名返回true，否则false | Returns true if Chinese filenames are supported, otherwise false
     */
    bool testChineseFileNameSupport(const QString& testPath = "") const;

    /**
     * @brief 将原始名称转换为安全的文件名
     * Convert original name to a safe filename
     *
     * @param originalName 原始名称 | Original name
     * @return QString 转换后的安全文件名 | Converted safe filename
     */
    QString convertToSafeFileName(const QString& originalName) const;

    /**
     * @brief 验证中文文件路径的合法性
     * Validate the legality of a Chinese file path
     *
     * @param filePath 要验证的文件路径 | File path to validate
     * @param errorMessage 输出参数，保存错误信息 | Output parameter for error message
     * @return bool 如果路径合法返回true，否则false | Returns true if path is valid, otherwise false
     */
    bool validateChineseFilePath(const QString& filePath, QString& errorMessage) const;
    
    // 📁 目录管理 | Directory Management
    /**
     * @brief 创建标准的目录结构
     * Create standard directory structure
     *
     * @param basePath 基础路径 | Base path
     * @return bool 如果成功创建所有目录返回true，否则false | Returns true if all directories were created successfully, otherwise false
     */
    bool createDirectoryStructure(const QString& basePath) const;

    /**
     * @brief 创建项目目录及其子目录结构
     * Create a project directory with its subdirectory structure
     *
     * @param projectName 项目名称 | Project name
     * @param basePath 基础路径，默认为当前目录 | Base path, defaults to current directory
     * @return bool 如果成功创建项目目录返回true，否则false | Returns true if project directory was created successfully, otherwise false
     */
    bool createProjectDirectory(const QString& projectName, const QString& basePath = "./") const;
    
    // 📊 文件统计 | File Statistics
    /**
     * @brief 分析目录中的文件并生成统计信息
     * Analyze files in a directory and generate statistics
     *
     * @param directoryPath 要分析的目录路径 | Directory path to analyze
     * @return FileStats 包含各种文件统计信息的结构 | Structure containing various file statistics
     */
    FileStats analyzeDirectory(const QString& directoryPath) const;

    /**
     * @brief 查找指定类型的所有文件
     * Find all files of a specified type
     *
     * @param directoryPath 要搜索的目录路径 | Directory path to search
     * @param fileType 文件类型标识符 | File type identifier
     * @return QStringList 匹配文件的路径列表 | List of matching file paths
     */
    QStringList findFilesByType(const QString& directoryPath, const QString& fileType) const;

    /**
     * @brief 获取目录中所有Halcon相关的文件
     * Get all Halcon-related files in a directory
     *
     * @param directoryPath 要搜索的目录路径 | Directory path to search
     * @return QStringList 所有Halcon文件的路径列表 | List of all Halcon file paths
     */
    QStringList getAllHalconFiles(const QString& directoryPath) const;
    
    // 🧹 文件清理 | File Cleanup
    /**
     * @brief 清理指定天数之前的旧文件
     * Clean up old files created before the specified number of days
     *
     * @param directoryPath 要清理的目录路径 | Directory path to clean
     * @param daysOld 文件的最大保留天数 | Maximum number of days to retain files
     * @return bool 如果清理操作成功返回true，否则false | Returns true if cleanup was successful, otherwise false
     */
    bool cleanupOldFiles(const QString& directoryPath, int daysOld = 30) const;

    /**
     * @brief 备份文件到指定目录
     * Backup a file to a specified directory
     *
     * @param filePath 要备份的文件路径 | File path to backup
     * @param backupDirectory 备份目录，默认为自动创建的备份目录 | Backup directory, defaults to auto-generated backup directory
     * @return bool 如果备份成功返回true，否则false | Returns true if backup was successful, otherwise false
     */
    bool backupFile(const QString& filePath, const QString& backupDirectory = "") const;
    
    // ✅ 文件验证 | File Validation
    /**
     * @brief 验证文件是否为有效的Halcon文件
     * Validate if a file is a valid Halcon file
     *
     * @param filePath 要验证的文件路径 | File path to validate
     * @return bool 如果是有效的Halcon文件返回true，否则false | Returns true if it is a valid Halcon file, otherwise false
     */
    bool isValidHalconFile(const QString& filePath) const;

    /**
     * @brief 根据文件扩展名获取文件类型
     * Get file type from file extension
     *
     * @param filePath 文件路径 | File path
     * @return QString 对应的文件类型标识符，如果未知则返回空字符串 | Corresponding file type identifier, returns empty string if unknown
     */
    QString getFileTypeFromExtension(const QString& filePath) const;
    
    // 📋 支持的文件类型列表 | Supported File Types List
    /**
     * @brief 获取支持的所有文件类型列表
     * Get list of all supported file types
     *
     * @return QStringList 支持的文件类型标识符列表 | List of supported file type identifiers
     */
    QStringList getSupportedFileTypes() const;

    /**
     * @brief 获取文件类型及其描述的映射
     * Get mapping of file types and their descriptions
     *
     * @return QMap<QString, QString> 文件类型到描述的映射 | Mapping of file type to description
     */
    QMap<QString, QString> getFileTypeDescriptions() const;

private:
    /**
     * @brief 初始化文件扩展名映射表
     * Initialize file extension mapping
     */
    void initializeExtensionMap();

    /**
     * @brief 格式化文件大小为可读形式
     * Format file size to a readable form
     *
     * @param size 文件大小（字节）| File size in bytes
     * @return QString 格式化后的文件大小（如"1.2 MB"）| Formatted file size (e.g. "1.2 MB")
     */
    QString formatFileSize(qint64 size) const;
    
    QMap<QString, QString> m_extensionMap;       // 文件扩展名映射表 | File extension mapping
    QStringList m_standardDirectories;          // 标准目录结构 | Standard directory structure
};

#endif // HALCONFILEMANAGER_H

