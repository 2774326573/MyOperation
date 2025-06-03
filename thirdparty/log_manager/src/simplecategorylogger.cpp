//
// Created by jinxi on 25-5-15.
//

#include "../inc/simplecategorylogger.h"
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMutexLocker>

// 静态成员函数实现
SimpleCategoryLogger& SimpleCategoryLogger::instance() {
    static SimpleCategoryLogger instance;
    return instance;
}

bool SimpleCategoryLogger::initCategory(const QString &category, const QString &rootDir) {
    QMutexLocker locker(&m_mutex);
    if (m_categoryFiles.contains(category)) {
        return true;
    }
    QString logDir = determineLogDirectory(category, rootDir);
    if (!createLogDirectory(logDir)) {
        return false;
    }
    if (!createLogFile(category, logDir)) {
        return false;
    }
    return true;
}

void SimpleCategoryLogger::setMaxLogFileSize(const QString &category, qint64 maxSizeBytes) {
    QMutexLocker locker(&m_mutex);
    m_categoryMaxSizes[category] = maxSizeBytes;
}

void SimpleCategoryLogger::setDefaultMaxLogFileSize(qint64 maxSizeBytes) {
    QMutexLocker locker(&m_mutex);
    m_defaultMaxFileSize = maxSizeBytes;
}

void SimpleCategoryLogger::setMaxHistoryFileCount(int count) {
    QMutexLocker locker(&m_mutex);
    m_maxHistoryFiles = count;
}

QStringList SimpleCategoryLogger::getCategories() const {
    QMutexLocker locker(&m_mutex);
    return m_categoryFiles.keys();
}

bool SimpleCategoryLogger::log(const QString &category, const QString &message, LogLevel level) {
    QMutexLocker locker(&m_mutex);
    if (!m_categoryFiles.contains(category) || !m_categoryFiles[category] || !m_categoryStreams[category]) {
        qWarning() << "类别未初始化:" << category;
        return false;
    }
    checkAndRotateLogFile(category);
    QString timestamp = getTimestamp();
    *m_categoryStreams[category] << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message << "\n";
    m_categoryStreams[category]->flush();
    
    // 在调试模式下同时输出到终端
    if (m_debugConfig.debugMode && m_debugConfig.showInTerminal) {
        switch (level) {
            case LogLevel::WARNING:
                qWarning().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
            case LogLevel::ERR:
                qCritical().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
            case LogLevel::DEBUG:
                qDebug().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
            case LogLevel::INFO:
            default:
                qInfo().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
        }
    }
    
    return true;
}

bool SimpleCategoryLogger::batchLog(const QString &category, const QStringList &messages, LogLevel level) {
    QMutexLocker locker(&m_mutex);
    if (!m_categoryFiles.contains(category) || !m_categoryFiles[category] || !m_categoryStreams[category]) {
        qWarning() << "类别未初始化:" << category;
        return false;
    }
    checkAndRotateLogFile(category);
    QString timestamp = getTimestamp();
    QString levelStr = logLevelToString(level);
    for (const QString &message : messages) {
        *m_categoryStreams[category] << "[" << timestamp << "] [" << levelStr << "] " << message << "\n";
        
        // 在调试模式下同时输出到终端
        if (m_debugConfig.debugMode && m_debugConfig.showInTerminal) {
            switch (level) {
                case LogLevel::WARNING:
                    qWarning().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << levelStr << "] " << message;
                    break;
                case LogLevel::ERR:
                    qCritical().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << levelStr << "] " << message;
                    break;
                case LogLevel::DEBUG:
                    qDebug().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << levelStr << "] " << message;
                    break;
                case LogLevel::INFO:
                default:
                    qInfo().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << levelStr << "] " << message;
                    break;
            }
        }
    }
    m_categoryStreams[category]->flush();
    return true;
}

bool SimpleCategoryLogger::logWithConfig(const QString &category, const QString &message, LogLevel level,
                                        bool showInTerminal, bool writeToFile) {
    QMutexLocker locker(&m_mutex);
    bool result = true;
    
    // 写入文件
    if (writeToFile) {
        if (!m_categoryFiles.contains(category) || !m_categoryFiles[category] || !m_categoryStreams[category]) {
            qWarning() << "类别未初始化:" << category;
            return false;
        }
        checkAndRotateLogFile(category);
        QString timestamp = getTimestamp();
        *m_categoryStreams[category] << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message << "\n";
        m_categoryStreams[category]->flush();
    }
    
    // 输出到终端
    if (showInTerminal) {
        QString timestamp = getTimestamp();
        switch (level) {
            case LogLevel::WARNING:
                qWarning().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
            case LogLevel::ERR:
                qCritical().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
            case LogLevel::DEBUG:
                qDebug().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
            case LogLevel::INFO:
            default:
                qInfo().noquote() << "[" << category << "] " << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message;
                break;
        }
    }
    
    return result;
}

void SimpleCategoryLogger::setDebugConfig(bool showInTerminal, bool writeToFile, bool debugMode) {
    QMutexLocker locker(&m_mutex);
    m_debugConfig.showInTerminal = showInTerminal;
    m_debugConfig.writeToFile = writeToFile;
    m_debugConfig.debugMode = debugMode;
}

const SimpleCategoryLogger::DebugConfig& SimpleCategoryLogger::getDebugConfig() const {
    QMutexLocker locker(&m_mutex);
    return m_debugConfig;
}

void SimpleCategoryLogger::debugAssert(bool condition, const QString& message, const QString &category) {
    if (!condition) {
        QString assertMessage = "ASSERT FAILED: " + message;
        
        if (!category.isEmpty()) {
            // 如果提供了类别，则写入到对应的日志文件
            log(category, assertMessage, LogLevel::ERR);
        } else {
            // 否则只输出到控制台
            qCritical().noquote() << "[" << getTimestamp() << "] " << assertMessage;
        }
        
        // 在调试模式下触发Qt断言
        if (m_debugConfig.debugMode) {
            Q_ASSERT_X(condition, "SimpleCategoryLogger", message.toLocal8Bit().constData());
        }
    }
}

void SimpleCategoryLogger::setPeriodicCleanup(bool enabled, int intervalDays, int maxAgeDays, const QString &baseDir) {
    QMutexLocker locker(&m_mutex);
    
    m_cleanupEnabled = enabled;
    
    if (enabled) {
        // 设置清理参数
        m_cleanupIntervalDays = qMax(1, intervalDays); // 最少1天
        m_maxAgeDays = qMax(1, maxAgeDays); // 最少1天
        
        // 设置清理目录
        if (baseDir.isEmpty()) {
            // 默认使用日志主目录
            m_cleanupBaseDir = QDir::currentPath() + "/logs";
        } else {
            m_cleanupBaseDir = baseDir;
        }
        
        // 如果未设置文件过滤器，添加默认的日志文件过滤器
        if (m_nameFilters.isEmpty()) {
            m_nameFilters << "*.log" << "*.txt";
        }
        
        // 配置定时器
        if (!m_cleanupTimer) {
            m_cleanupTimer = new QTimer(nullptr);
            connect(m_cleanupTimer, &QTimer::timeout, this, &SimpleCategoryLogger::periodicCleanupSlot);
        }
        
        // 转换天数为毫秒
        int intervalMs = m_cleanupIntervalDays * 24 * 60 * 60 * 1000;
        m_cleanupTimer->start(intervalMs);
        
        // 输出日志
        qInfo() << "已启用定期文件清理，间隔:" << m_cleanupIntervalDays << "天，最大保留期:" << m_maxAgeDays
                << "天，目录:" << m_cleanupBaseDir;
    } else if (m_cleanupTimer) {
        // 停止定时器
        m_cleanupTimer->stop();
        qInfo() << "已禁用定期文件清理";
    }
}

void SimpleCategoryLogger::setCleanupFilters(const QStringList &nameFilters) {
    QMutexLocker locker(&m_mutex);
    m_nameFilters = nameFilters;
    
    if (m_nameFilters.isEmpty()) {
        // 添加默认过滤器
        m_nameFilters << "*.log" << "*.txt";
    }
}

int SimpleCategoryLogger::cleanupFiles(const QString &baseDir, int maxAgeDays) {
    QMutexLocker locker(&m_mutex);
    
    // 使用提供的参数或默认值
    QString cleanupDir = baseDir.isEmpty() ? m_cleanupBaseDir : baseDir;
    int maxAge = maxAgeDays > 0 ? maxAgeDays : m_maxAgeDays;
    
    // 检查参数
    if (cleanupDir.isEmpty()) {
        qWarning() << "清理目录未设置，使用当前日志目录";
        cleanupDir = QDir::currentPath() + "/logs";
    }
    
    if (maxAge <= 0) {
        qWarning() << "无效的最大保留天数，使用默认值30天";
        maxAge = 30;
    }
    
    QDir dir(cleanupDir);
    if (!dir.exists()) {
        qWarning() << "清理目录不存在:" << cleanupDir;
        return 0;
    }
    
    // 执行清理
    int deletedCount = cleanupFilesInDir(dir, maxAge, m_nameFilters);
    qInfo() << "文件清理完成，共删除" << deletedCount << "个文件";
    
    return deletedCount;
}

int SimpleCategoryLogger::cleanupFilesInDir(const QDir &dir, int maxAgeDays, const QStringList &nameFilters, bool recursive) {
    int deletedCount = 0;
    
    // 获取当前时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    
    // 过滤文件
    QFileInfoList fileList;
    if (!nameFilters.isEmpty()) {
        fileList = dir.entryInfoList(nameFilters, QDir::Files);
    } else {
        fileList = dir.entryInfoList(QDir::Files);
    }
    
    // 检查每个文件
    for (const QFileInfo &fileInfo : fileList) {
        QDateTime lastModified = fileInfo.lastModified();
        int daysDiff = lastModified.daysTo(currentDateTime);
        
        // 如果文件超过最大保留天数，删除它
        if (daysDiff > maxAgeDays) {
            QString filePath = fileInfo.absoluteFilePath();
            QFile file(filePath);
            
            // 尝试删除文件
            if (file.remove()) {
                deletedCount++;
                qDebug() << "已删除过期文件:" << filePath << "，距今" << daysDiff << "天";
            } else {
                qWarning() << "无法删除文件:" << filePath << "，错误:" << file.errorString();
            }
        }
    }
    
    // 递归处理子目录
    if (recursive) {
        QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &dirInfo : dirList) {
            QDir subDir(dirInfo.absoluteFilePath());
            deletedCount += cleanupFilesInDir(subDir, maxAgeDays, nameFilters, true);
        }
    }
    
    return deletedCount;
}

void SimpleCategoryLogger::periodicCleanupSlot() {
    // 执行定期清理
    if (m_cleanupEnabled) {
        qDebug() << "执行定期文件清理...";
        int deletedCount = cleanupFiles();
        qDebug() << "定期清理完成，删除了" << deletedCount << "个过期文件";
        
        // 记录到系统日志
        if (m_categoryFiles.contains("System")) {
            log("System", QString("执行定期文件清理，删除了 %1 个过期文件").arg(deletedCount), LogLevel::INFO);
        }
    }
}

QString SimpleCategoryLogger::getLogFilePath(const QString &category) const {
    QMutexLocker locker(&m_mutex);
    if (m_categoryFiles.contains(category) && m_categoryFiles[category]) {
        return m_categoryFiles[category]->fileName();
    }
    return QString();
}

SimpleCategoryLogger::~SimpleCategoryLogger() {
    QMutexLocker locker(&m_mutex);
    // 停止清理定时器
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
        delete m_cleanupTimer;
    }
    
    for (auto it = m_categoryStreams.begin(); it != m_categoryStreams.end(); ++it) {
        delete it.value();
    }
    for (auto it = m_categoryFiles.begin(); it != m_categoryFiles.end(); ++it) {
        if (it.value()->isOpen()) {
            it.value()->close();
        }
        delete it.value();
    }
}

SimpleCategoryLogger::SimpleCategoryLogger()
    : m_defaultMaxFileSize(5 * 1024 * 1024)
    , m_maxHistoryFiles(5)
    , m_cleanupTimer(nullptr)
    , m_cleanupEnabled(false)
    , m_cleanupIntervalDays(7)
    , m_maxAgeDays(30)
{
    // 初始化调试配置
    m_debugConfig.showInTerminal = true;
    m_debugConfig.writeToFile = true;
    m_debugConfig.debugMode = false;
    
    // 初始化清理过滤器
    m_nameFilters << "*.log" << "*.txt";
}

QString SimpleCategoryLogger::determineLogDirectory(const QString &category, const QString &rootDir) {
    QString currentTime = QDateTime::currentDateTime().toString("yyyy/MM/dd");
    if (rootDir.isEmpty()) {
        // 使用当前应用程序目录下的logs目录，避免权限问题
        return QDir::currentPath() + "/logs/" + category + "/" + currentTime;
    } else {
        return rootDir + "/" + category + "/" + currentTime;
    }
}

bool SimpleCategoryLogger::createLogDirectory(const QString &logDir) {
    QDir dir(logDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "无法创建目录:" << logDir;
            return false;
        }
    }
    return true;
}

bool SimpleCategoryLogger::createLogFile(const QString &category, const QString &logDir) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString filePath = logDir + "/" + category + "_" + timestamp + ".log";
    
    // 确保目录存在
    QFileInfo fileInfo(filePath);
    QString dirPath = fileInfo.absolutePath();
    QDir dir(dirPath);
    if (!dir.exists()) {
        qDebug() << "创建日志文件目录:" << dirPath;
        if (!dir.mkpath(".")) {
            qWarning() << "无法创建日志文件目录:" << dirPath;
            return false;
        }
    }
    
    // 尝试创建一个测试文件以验证写入权限
    QString testFilePath = dirPath + "/.write_test";
    QFile testFile(testFilePath);
    if (!testFile.open(QIODevice::WriteOnly)) {
        qWarning() << "无法在目录中创建测试文件，权限不足:" << dirPath;
        return false;
    }
    testFile.close();
    testFile.remove();  // 删除测试文件
    
    // 创建实际的日志文件
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qWarning() << "无法打开日志文件:" << filePath << "错误:" << file->errorString();
        delete file;
        return false;
    }
    
    m_categoryFiles[category] = file;
    m_categoryStreams[category] = new QTextStream(file);        // 创建文本流
    // m_categoryStreams[category]->setCodec("UTF-8");             // 设置编码
    m_categoryMaxSizes[category] = m_defaultMaxFileSize;        // 设置默认大小
    m_categoryPaths[category] = filePath;
    
    qDebug() << "已初始化类别" << category << "的日志，文件路径:" << filePath;
    *m_categoryStreams[category] << "=== " << category << " 日志开始 - " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " ===\n";
    m_categoryStreams[category]->flush();
    
    // 验证文件已创建
    QFileInfo checkFile(filePath);
    if (!checkFile.exists()) {
        qWarning() << "文件创建失败，尽管没有错误:" << filePath;
        return false;
    }
    
    qDebug() << "日志文件成功创建和初始化:" << filePath;
    return true;
}

void SimpleCategoryLogger::checkAndRotateLogFile(const QString &category) {
    if (!m_categoryFiles.contains(category) || !m_categoryFiles[category]) {
        return;
    }
    m_categoryStreams[category]->flush();
    QFileInfo fileInfo(m_categoryPaths[category]);
    qint64 maxSize = m_categoryMaxSizes.value(category, m_defaultMaxFileSize);
    if (fileInfo.size() >= maxSize) {
        rotateLogFile(category);
    }
}

void SimpleCategoryLogger::rotateLogFile(const QString &category) {
    if (!m_categoryFiles.contains(category) || !m_categoryFiles[category]) {
        return;
    }
    m_categoryStreams[category]->flush();
    m_categoryFiles[category]->close();
    QString currentPath = m_categoryPaths[category];
    QFileInfo fileInfo(currentPath);
    QString dirPath = fileInfo.absolutePath();
    QString baseName = fileInfo.completeBaseName();
    QDir dir(dirPath);
    QStringList filters;
    filters << baseName + "_*.log";
    QStringList logFiles = dir.entryList(filters, QDir::Files, QDir::Time);
    while (logFiles.size() >= m_maxHistoryFiles) {
        QString oldestFile = dirPath + "/" + logFiles.last();
        QFile::remove(oldestFile);
        logFiles.removeLast();
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString newPath = dirPath + "/" + baseName + "_" + timestamp + ".log";
    QFile::rename(currentPath, newPath);
    QFile* newFile = new QFile(currentPath);
    if (!newFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qWarning() << "无法创建新的日志文件:" << currentPath;
        delete newFile;
        return;
    }
    delete m_categoryStreams[category];
    delete m_categoryFiles[category];
    m_categoryFiles[category] = newFile;
    m_categoryStreams[category] = new QTextStream(newFile);
    m_categoryStreams[category]->setCodec("UTF-8");
    *m_categoryStreams[category] << "=== " << category << " 日志开始 (轮换于 "
                                 << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                 << ") ===\n";
    m_categoryStreams[category]->flush();
    qDebug() << "已轮换类别" << category << "的日志文件，旧文件:" << newPath;
}

QString SimpleCategoryLogger::getTimestamp() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}




