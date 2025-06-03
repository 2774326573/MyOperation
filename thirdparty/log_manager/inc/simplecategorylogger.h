#ifndef SIMPLE_CATEGORY_LOGGER_H
#define SIMPLE_CATEGORY_LOGGER_H

#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>
#include <QDebug>
#include <QObject>
#include "loglevel.h"

/**
 * @brief 简单的分类日志记录器 | Simple category-based logger
 *
 * 轻量级的分类日志解决方案，直接将不同类别的日志写入不同的文件，
 * 不依赖于LogManager。支持日志轮换、大小限制和历史记录管理。
 * 集成了DebugHelper功能，支持调试断言和终端日志输出。
 * 支持定期清理过期文件功能。
 *
 * A lightweight category-based logging solution that writes logs for different categories
 * directly to different files without depending on LogManager. Supports log rotation,
 * size limits, and history management.
 * Integrates DebugHelper functionality, supporting debug assertions and terminal log output.
 * Supports periodic cleanup of expired files.
 */

class QTimer;

class SimpleCategoryLogger : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 获取单例实例 | Get singleton instance
     * @return SimpleCategoryLogger 单例 | SimpleCategoryLogger singleton
     */
    static SimpleCategoryLogger& instance();

    /**
     * @brief 初始化指定类别的日志 | Initialize log for a specific category
     * @param category 类别名称 | Category name
     * @param rootDir 根目录（可选）| Root directory (optional)
     * @return 是否成功初始化 | Whether initialization was successful
     */
    bool initCategory(const QString &category, const QString &rootDir = QString());

    /**
     * @brief 为指定类别设置最大日志文件大小 | Set maximum log file size for a specific category
     * @param category 类别名称 | Category name
     * @param maxSizeBytes 最大文件大小（字节）| Maximum file size (bytes)
     */
    void setMaxLogFileSize(const QString &category, qint64 maxSizeBytes);

    /**
     * @brief 为所有类别设置默认最大日志文件大小 | Set default maximum log file size for all categories
     * @param maxSizeBytes 最大文件大小（字节）| Maximum file size (bytes)
     */
    void setDefaultMaxLogFileSize(qint64 maxSizeBytes);

    /**
     * @brief 设置最大保留的历史日志文件数 | Set maximum number of history log files to retain
     * @param count 最大文件数 | Maximum file count
     */
    void setMaxHistoryFileCount(int count);

    /**
     * @brief 获取所有已初始化的类别 | Get all initialized categories
     * @return 类别列表 | Category list
     */
    QStringList getCategories() const;

    /**
     * @brief 写入日志 | Write log
     * @param category 类别名称 | Category name
     * @param message 日志消息 | Log message
     * @param level 日志级别 | Log level
     * @return 是否成功写入 | Whether writing was successful
     */
    bool log(const QString &category, const QString &message, LogLevel level = LogLevel::INFO);

    /**
     * @brief 批量写入日志 | Batch write logs
     * @param category 类别名称 | Category name
     * @param messages 消息列表 | Message list
     * @param level 日志级别 | Log level
     * @return 是否成功写入 | Whether writing was successful
     */
    bool batchLog(const QString &category, const QStringList &messages, LogLevel level = LogLevel::INFO);

    /**
     * @brief 获取日志文件路径 | Get log file path
     * @param category 类别名称 | Category name
     * @return 日志文件路径 | Log file path
     */
    QString getLogFilePath(const QString &category) const;

    /**
     * @brief 调试配置结构体 | Debug configuration structure
     */
    struct DebugConfig {
        bool showInTerminal = true;  ///< 是否在终端显示 | Whether to display in terminal
        bool writeToFile = true;     ///< 是否写入文件 | Whether to write to file
        bool debugMode = false;      ///< 是否启用调试模式 | Whether debug mode is enabled
    };

    /**
     * @brief 设置调试配置 | Set debug configuration
     * @param showInTerminal 是否在终端显示 | Whether to display in terminal
     * @param writeToFile 是否写入文件 | Whether to write to file
     * @param debugMode 是否启用调试模式 | Whether to enable debug mode
     */
    void setDebugConfig(bool showInTerminal, bool writeToFile, bool debugMode);

    /**
     * @brief 获取当前调试配置 | Get current debug configuration
     * @return 当前调试配置 | Current debug configuration
     */
    const DebugConfig& getDebugConfig() const;

    /**
     * @brief 带配置的日志 | Logging with configuration
     * @param category 类别名称 | Category name
     * @param message 日志消息 | Log message 
     * @param level 日志级别 | Log level
     * @param showInTerminal 是否在终端显示 | Whether to display in terminal
     * @param writeToFile 是否写入文件 | Whether to write to file
     * @return 是否成功写入 | Whether writing was successful
     */
    bool logWithConfig(const QString &category, const QString &message, LogLevel level, 
                      bool showInTerminal, bool writeToFile);

    /**
     * @brief 断言函数 | Assertion function
     * @param condition 断言条件 | Assertion condition
     * @param message 断言失败时的消息 | Message when assertion fails
     * @param category 类别名称(可选) | Category name (optional)
     */
    void debugAssert(bool condition, const QString& message, const QString &category = QString());

    /**
     * @brief 设置定期清理配置 | Set periodic cleanup configuration
     * @param enabled 是否启用定期清理 | Whether to enable periodic cleanup
     * @param intervalDays 清理间隔（天） | Cleanup interval (days)
     * @param maxAgeDays 文件最大保留天数 | Maximum age of files (days)
     * @param baseDir 基础目录（可选，默认为日志目录） | Base directory (optional, defaults to log directory)
     */
    void setPeriodicCleanup(bool enabled, int intervalDays = 7, int maxAgeDays = 30, const QString &baseDir = QString());

    /**
     * @brief 设置文件清理过滤器 | Set file cleanup filter
     * @param nameFilters 文件名过滤器列表 | List of filename filters (e.g. *.log, *.txt)
     */
    void setCleanupFilters(const QStringList &nameFilters);

    /**
     * @brief 立即执行文件清理 | Execute file cleanup immediately
     * @param baseDir 基础目录（可选，默认为已配置目录） | Base directory (optional, defaults to configured directory)
     * @param maxAgeDays 文件最大保留天数（可选，默认为已配置天数） | Maximum age of files (optional, defaults to configured days)
     * @return 被删除的文件数量 | Number of deleted files
     */
    int cleanupFiles(const QString &baseDir = QString(), int maxAgeDays = -1);

    /**
     * @brief 析构函数，关闭所有文件 | Destructor, close all files
     */
    ~SimpleCategoryLogger();
private:
    /**
     * @brief 构造函数 | Constructor
     */
    SimpleCategoryLogger();
    // 禁止拷贝和赋值 | Prevent copy and assignment
    SimpleCategoryLogger(const SimpleCategoryLogger&) = delete;
    SimpleCategoryLogger& operator=(const SimpleCategoryLogger&) = delete;

    /**
     * @brief 确定日志目录 | Determine log directory
     * @param category 类别名称 | Category name
     * @param rootDir 根目录 | Root directory
     * @return 日志目录路径 | Log directory path
     */
    static QString determineLogDirectory(const QString &category, const QString &rootDir);

    /**
     * @brief 创建日志目录 | Create log directory
     * @param logDir 日志目录路径 | Log directory path
     * @return 是否成功创建 | Whether creation was successful
     */
    static bool createLogDirectory(const QString &logDir);

    /**
     * @brief 创建日志文件 | Create log file
     * @param category 类别名称 | Category name
     * @param logDir 日志目录路径 | Log directory path
     * @return 是否成功创建 | Whether creation was successful
     */
    bool createLogFile(const QString &category, const QString &logDir);

    /**
     * @brief 检查并在必要时轮换日志文件 | Check and rotate log file if necessary
     * @param category 类别名称 | Category name
     */
    void checkAndRotateLogFile(const QString &category);

    /**
     * @brief 轮换指定类别的日志文件 | Rotate log file for a specific category
     * @param category 类别名称 | Category name
     */
    void rotateLogFile(const QString &category);

    /**
     * @brief 获取当前时间戳字符串 | Get current timestamp string
     * @return 格式化的时间戳 | Formatted timestamp
     */
    static QString getTimestamp();
    
    /**
     * @brief 执行定期清理的槽函数 | Slot function for periodic cleanup
     */
    void periodicCleanupSlot();

    /**
     * @brief 递归删除指定目录下的过期文件 | Recursively delete expired files in the specified directory
     * @param dir 目录 | Directory
     * @param maxAgeDays 文件最大保留天数 | Maximum age of files (days)
     * @param nameFilters 文件名过滤器列表 | List of filename filters
     * @param recursive 是否递归处理子目录 | Whether to recursively process subdirectories
     * @return 被删除的文件数量 | Number of deleted files
     */
    int cleanupFilesInDir(const QDir &dir, int maxAgeDays, const QStringList &nameFilters, bool recursive = true);

    QMap<QString, QFile*> m_categoryFiles;        ///< 类别对应的文件对象 | Category to file object mapping
    QMap<QString, QTextStream*> m_categoryStreams; ///< 类别对应的文本流 | Category to text stream mapping
    QMap<QString, QString> m_categoryPaths;       ///< 类别对应的日志文件路径 | Category to log file path mapping
    QMap<QString, qint64> m_categoryMaxSizes;     ///< 类别对应的最大文件大小限制 | Category to maximum file size limit mapping
    qint64 m_defaultMaxFileSize;                  ///< 默认文件大小上限 | Default file size limit
    int m_maxHistoryFiles;                        ///< 最大历史文件保留数 | Maximum history file count
    mutable QMutex m_mutex;                       ///< 互斥锁 | Mutex lock
    DebugConfig m_debugConfig;                    ///< 调试配置 | Debug configuration
    
    // 清理相关成员变量 | Cleanup related member variables
    QTimer* m_cleanupTimer;                       ///< 清理定时器 | Cleanup timer
    bool m_cleanupEnabled;                        ///< 是否启用清理 | Whether cleanup is enabled
    int m_cleanupIntervalDays;                    ///< 清理间隔（天） | Cleanup interval (days)
    int m_maxAgeDays;                             ///< 文件最大保留天数 | Maximum age of files (days)
    QString m_cleanupBaseDir;                     ///< 清理基础目录 | Cleanup base directory
    QStringList m_nameFilters;                    ///< 文件名过滤器列表 | List of filename filters
};

// 单例宏 | Singleton macro
#define SIMPLE_LOGGER SimpleCategoryLogger::instance()
// 便捷宏 | Convenience macros
#define SIMPLE_LOG_INIT(category, rootDir) SIMPLE_LOGGER.initCategory(category, rootDir)
#define SIMPLE_LOG_INFO(category, message) SIMPLE_LOGGER.log(category, message, LogLevel::INFO)
#define SIMPLE_LOG_WARNING(category, message) SIMPLE_LOGGER.log(category, message, LogLevel::WARNING)
#define SIMPLE_LOG_ERROR(category, message) SIMPLE_LOGGER.log(category, message, LogLevel::ERR)
#define SIMPLE_LOG_DEBUG(category, message) SIMPLE_LOGGER.log(category, message, LogLevel::DEBUG)
#define SIMPLE_LOG_TRACE(category, message) SIMPLE_LOGGER.log(category, message, LogLevel::TRACE)

// Debug模式宏 | Debug mode macros
#define SIMPLE_DEBUG_LOG_INFO(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::INFO, true, true)
#define SIMPLE_DEBUG_LOG_WARNING(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::WARNING, true, true)
#define SIMPLE_DEBUG_LOG_ERROR(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::ERR, true, true)
#define SIMPLE_DEBUG_LOG_DEBUG(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::DEBUG, true, true)

// 带配置的日志宏 | Log macros with configuration
#define SIMPLE_LOG_INFO_CONFIG(category, message, showTerminal, writeFile) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::INFO, showTerminal, writeFile)
#define SIMPLE_LOG_WARNING_CONFIG(category, message, showTerminal, writeFile) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::WARNING, showTerminal, writeFile)
#define SIMPLE_LOG_ERROR_CONFIG(category, message, showTerminal, writeFile) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::ERR, showTerminal, writeFile)
#define SIMPLE_LOG_DEBUG_CONFIG(category, message, showTerminal, writeFile) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::DEBUG, showTerminal, writeFile)

// 断言宏 | Assertion macro
#define SIMPLE_ASSERT(condition, message) SIMPLE_LOGGER.debugAssert(condition, message)
#define SIMPLE_ASSERT_CATEGORY(condition, message, category) SIMPLE_LOGGER.debugAssert(condition, message, category)

// 文件清理宏 | File cleanup macros
#define SIMPLE_ENABLE_CLEANUP(intervalDays, maxAgeDays) SIMPLE_LOGGER.setPeriodicCleanup(true, intervalDays, maxAgeDays)
#define SIMPLE_DISABLE_CLEANUP() SIMPLE_LOGGER.setPeriodicCleanup(false)
#define SIMPLE_CLEANUP_NOW() SIMPLE_LOGGER.cleanupFiles()

#endif
