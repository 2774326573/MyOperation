# 📝 日志管理器文档 / Log Manager Documentation

<div align="center">

![Logging](https://img.shields.io/badge/Logging-System-green?style=for-the-badge&logo=data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMjQiIGhlaWdodD0iMjQiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTEyIDJMMTMuMDkgOC4yNkwyMCA5TDEzLjA5IDE1Ljc0TDEyIDIyTDEwLjkxIDE1Ljc0TDQgOUwxMC45MSA4LjI2TDEyIDJaIiBmaWxsPSJ3aGl0ZSIvPgo8L3N2Zz4K)
![Qt](https://img.shields.io/badge/Qt-6.0+-brightgreen?style=for-the-badge&logo=qt)
![Thread Safe](https://img.shields.io/badge/Thread-Safe-blue?style=for-the-badge)

**🚀 高性能分类日志系统 | High-Performance Category Logging System**

*轻量级、线程安全的分类日志解决方案*  
*Lightweight, thread-safe category-based logging solution*

</div>

---

## 📖 概述 / Overview

**SimpleCategoryLogger** 是一个轻量级的分类日志记录器，支持多类别日志管理、文件轮换、大小限制和历史记录管理。集成了调试功能，支持定期清理过期文件。

**SimpleCategoryLogger** is a lightweight category-based logger supporting multi-category log management, file rotation, size limits, and history management. Integrates debug functionality with periodic cleanup of expired files.

### 🌟 核心特性 / Key Features

<table>
<tr>
<td width="50%">

#### 📂 分类管理 / Category Management
- 多类别日志分离
- 独立文件存储
- 动态类别创建

#### 🔄 文件管理 / File Management
- 自动文件轮换
- 大小限制控制
- 历史文件清理

</td>
<td width="50%">

#### 🛡️ 安全性 / Security
- 线程安全操作
- 内存安全管理
- 异常处理机制

#### 🔧 调试支持 / Debug Support
- 调试断言功能
- 终端输出控制
- 配置化调试模式

</td>
</tr>
</table>

---

## ⚡ 快速开始 / Quick Start

### 📦 基本使用 / Basic Usage

```cpp
#include "simplecategorylogger.h"

int main()
{
    auto& logger = SimpleCategoryLogger::instance();
    
    // 初始化类别 (Initialize category)
    logger.initCategory("network");
    logger.initCategory("database");
    
    // 写入日志 (Write logs)
    logger.log("network", tr("连接服务器成功 Connected to server successfully"), LogLevel::INFO);
    logger.log("database", tr("查询执行完成 Query execution completed"), LogLevel::DEBUG);
    
    return 0;
}
```

### 🔧 高级配置 / Advanced Configuration

```cpp
class LoggerSetup
{
public:
    static void configure()
    {
        auto& logger = SimpleCategoryLogger::instance();
        
        // 设置文件大小限制 (Set file size limit)
        logger.setDefaultMaxLogFileSize(10 * 1024 * 1024); // 10MB
        
        // 设置历史文件数量 (Set history file count)
        logger.setMaxHistoryFileCount(5);
        
        // 配置调试选项 (Configure debug options)
        logger.setDebugConfig(true, true, false);
        
        // 设置定期清理 (Set periodic cleanup)
        logger.setPeriodicCleanup(true, 7, 30);
    }
};
```

---

## 🛠️ API 参考 / API Reference

### 📋 核心方法 / Core Methods

<table>
<tr>
<th>🔧 方法 / Method</th>
<th>📝 描述 / Description</th>
<th>💡 示例 / Example</th>
</tr>
<tr>
<td><code>initCategory()</code></td>
<td>初始化日志类别<br/>Initialize log category</td>
<td><code>logger.initCategory("app");</code></td>
</tr>
<tr>
<td><code>log()</code></td>
<td>写入日志消息<br/>Write log message</td>
<td><code>logger.log("app", "message", LogLevel::INFO);</code></td>
</tr>
<tr>
<td><code>batchLog()</code></td>
<td>批量写入日志<br/>Batch write logs</td>
<td><code>logger.batchLog("app", messages, LogLevel::DEBUG);</code></td>
</tr>
<tr>
<td><code>debugAssert()</code></td>
<td>调试断言<br/>Debug assertion</td>
<td><code>logger.debugAssert(condition, "error");</code></td>
</tr>
</table>

### 🎛️ 配置方法 / Configuration Methods

```cpp
// 文件大小管理 (File size management)
logger.setMaxLogFileSize("network", 5 * 1024 * 1024); // 5MB for specific category
logger.setDefaultMaxLogFileSize(10 * 1024 * 1024);    // 10MB for all categories

// 历史文件管理 (History file management)
logger.setMaxHistoryFileCount(10); // Keep 10 history files

// 调试配置 (Debug configuration)
logger.setDebugConfig(
    true,  // showInTerminal - 终端显示
    true,  // writeToFile - 写入文件
    false  // debugMode - 调试模式
);

// 定期清理配置 (Periodic cleanup configuration)
logger.setPeriodicCleanup(
    true, // enabled - 启用清理
    7,    // intervalDays - 清理间隔(天)
    30    // maxAgeDays - 最大保留天数
);
```

---

## 💡 使用示例 / Usage Examples

### 1️⃣ **网络模块日志 / Network Module Logging**

```cpp
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    NetworkManager()
    {
        // 初始化网络日志类别 (Initialize network log category)
        auto& logger = SimpleCategoryLogger::instance();
        logger.initCategory("network", "./logs/network");
        logger.setMaxLogFileSize("network", 5 * 1024 * 1024); // 5MB
    }

    void connectToServer(const QString& host, int port)
    {
        auto& logger = SimpleCategoryLogger::instance();
        
        logger.log("network", 
                  tr("尝试连接服务器 Attempting to connect to server: %1:%2")
                  .arg(host).arg(port), 
                  LogLevel::INFO);
        
        // 连接逻辑 (Connection logic)
        if (performConnection(host, port)) {
            logger.log("network", 
                      tr("服务器连接成功 Server connection successful"), 
                      LogLevel::INFO);
        } else {
            logger.log("network", 
                      tr("服务器连接失败 Server connection failed"), 
                      LogLevel::ERR);
        }
    }

private:
    bool performConnection(const QString& host, int port)
    {
        // 实际连接实现 (Actual connection implementation)
        return true; // 示例返回 (Example return)
    }
};
```

### 2️⃣ **数据库操作日志 / Database Operation Logging**

```cpp
class DatabaseManager
{
public:
    DatabaseManager()
    {
        auto& logger = SimpleCategoryLogger::instance();
        logger.initCategory("database");
        
        // 设置批量日志缓冲 (Set batch log buffer)
        m_logBuffer.reserve(100);
    }

    void executeQuery(const QString& sql)
    {
        auto& logger = SimpleCategoryLogger::instance();
        
        QElapsedTimer timer;
        timer.start();
        
        // 执行查询 (Execute query)
        bool success = performQuery(sql);
        qint64 elapsed = timer.elapsed();
        
        // 记录执行结果 (Log execution result)
        QString message = tr("SQL执行 SQL execution: %1 | 耗时 Time: %2ms | 结果 Result: %3")
                         .arg(sql)
                         .arg(elapsed)
                         .arg(success ? tr("成功 Success") : tr("失败 Failed"));
        
        LogLevel level = success ? LogLevel::INFO : LogLevel::ERR;
        
        // 添加到批量缓冲 (Add to batch buffer)
        m_logBuffer.append(message);
        
        // 达到批量大小时写入 (Write when batch size reached)
        if (m_logBuffer.size() >= 10) {
            logger.batchLog("database", m_logBuffer, level);
            m_logBuffer.clear();
        }
    }

    ~DatabaseManager()
    {
        // 写入剩余日志 (Write remaining logs)
        if (!m_logBuffer.isEmpty()) {
            auto& logger = SimpleCategoryLogger::instance();
            logger.batchLog("database", m_logBuffer, LogLevel::INFO);
        }
    }

private:
    bool performQuery(const QString& sql)
    {
        // 实际查询实现 (Actual query implementation)
        return true; // 示例返回 (Example return)
    }

    QStringList m_logBuffer;
};
```

### 3️⃣ **调试和断言 / Debug and Assertions**

```cpp
class DebugHelper
{
public:
    static void setupDebugLogging()
    {
        auto& logger = SimpleCategoryLogger::instance();
        
        // 初始化调试类别 (Initialize debug category)
        logger.initCategory("debug");
        
        // 配置调试模式 (Configure debug mode)
        logger.setDebugConfig(
            true,  // 终端显示 (Terminal display)
            true,  // 文件写入 (File writing)
            true   // 调试模式 (Debug mode)
        );
    }

    static void validateData(const QVariant& data, const QString& context)
    {
        auto& logger = SimpleCategoryLogger::instance();
        
        // 数据有效性断言 (Data validity assertion)
        logger.debugAssert(data.isValid(), 
                          tr("数据无效 Invalid data in context: %1").arg(context), 
                          "debug");
        
        // 类型检查断言 (Type check assertion)
        if (data.type() == QVariant::String) {
            logger.debugAssert(!data.toString().isEmpty(),
                              tr("字符串数据为空 String data is empty in context: %1").arg(context),
                              "debug");
        }
    }

    static void logPerformanceMetrics(const QString& operation, qint64 duration)
    {
        auto& logger = SimpleCategoryLogger::instance();
        
        LogLevel level = LogLevel::INFO;
        if (duration > 1000) {
            level = LogLevel::WARNING; // 超过1秒警告 (Warning if over 1 second)
        }
        if (duration > 5000) {
            level = LogLevel::ERR; // 超过5秒错误 (Error if over 5 seconds)
        }
        
        logger.log("debug", 
                  tr("性能指标 Performance metric - 操作 Operation: %1, 耗时 Duration: %2ms")
                  .arg(operation).arg(duration),
                  level);
    }
};
```

---

## ⚠️ 注意事项 / Precautions

### 🚨 大项目注意事项 / Large Project Considerations

#### 1️⃣ **性能优化 / Performance Optimization**

```cpp
// ❌ 错误：频繁的小量日志写入 (Wrong: Frequent small log writes)
void badLoggingPractice()
{
    auto& logger = SimpleCategoryLogger::instance();
    for (int i = 0; i < 1000; ++i) {
        logger.log("app", QString("Message %1").arg(i), LogLevel::INFO);
    }
}

// ✅ 正确：批量日志写入 (Correct: Batch log writing)
void goodLoggingPractice()
{
    auto& logger = SimpleCategoryLogger::instance();
    QStringList messages;
    for (int i = 0; i < 1000; ++i) {
        messages.append(QString("Message %1").arg(i));
    }
    logger.batchLog("app", messages, LogLevel::INFO);
}
```

#### 2️⃣ **内存管理 / Memory Management**

```cpp
// 推荐：日志缓冲区管理 (Recommended: Log buffer management)
class LogBuffer
{
private:
    QStringList m_buffer;
    const int MAX_BUFFER_SIZE = 100;
    QTimer* m_flushTimer;

public:
    LogBuffer()
    {
        m_flushTimer = new QTimer(this);
        m_flushTimer->setSingleShot(false);
        m_flushTimer->setInterval(5000); // 5秒刷新 (5-second flush)
        connect(m_flushTimer, &QTimer::timeout, this, &LogBuffer::flush);
        m_flushTimer->start();
    }

    void addMessage(const QString& message)
    {
        m_buffer.append(message);
        if (m_buffer.size() >= MAX_BUFFER_SIZE) {
            flush();
        }
    }

private slots:
    void flush()
    {
        if (!m_buffer.isEmpty()) {
            auto& logger = SimpleCategoryLogger::instance();
            logger.batchLog("app", m_buffer, LogLevel::INFO);
            m_buffer.clear();
        }
    }
};
```

#### 3️⃣ **线程安全使用 / Thread-Safe Usage**

```cpp
// 推荐：多线程环境下的日志使用 (Recommended: Multi-threaded logging)
class ThreadSafeLogger
{
public:
    static void logFromWorkerThread(const QString& category, const QString& message)
    {
        // 使用队列连接确保线程安全 (Use queued connection for thread safety)
        QMetaObject::invokeMethod(&SimpleCategoryLogger::instance(),
                                "log",
                                Qt::QueuedConnection,
                                Q_ARG(QString, category),
                                Q_ARG(QString, message),
                                Q_ARG(LogLevel, LogLevel::INFO));
    }
};
```

---

## 🔍 故障排除 / Troubleshooting

### 📋 常见问题 / Common Issues

<table>
<tr>
<th>⚠️ 问题 / Issue</th>
<th>🔍 原因 / Cause</th>
<th>✅ 解决方案 / Solution</th>
</tr>
<tr>
<td>日志文件创建失败</td>
<td>目录权限不足或路径不存在</td>
<td>检查目录权限，确保路径存在</td>
</tr>
<tr>
<td>日志轮换不工作</td>
<td>文件大小限制设置错误</td>
<td>正确设置文件大小限制</td>
</tr>
<tr>
<td>性能问题</td>
<td>频繁的单条日志写入</td>
<td>使用批量日志写入</td>
</tr>
<tr>
<td>内存占用过高</td>
<td>日志缓冲区过大</td>
<td>定期刷新缓冲区，控制缓冲区大小</td>
</tr>
</table>

---

<div align="center">

## 🎉 总结 / Summary

**SimpleCategoryLogger 提供了强大而灵活的分类日志管理能力，适合各种规模的 Qt 应用程序。通过合理配置和使用，可以构建高效、可靠的日志系统。**

**SimpleCategoryLogger provides powerful and flexible category-based log management capabilities suitable for Qt applications of all sizes. With proper configuration and usage, you can build efficient and reliable logging systems.**

---

<img src="https://img.shields.io/badge/Status-Document%20Complete-success?style=for-the-badge" alt="Complete">
<img src="https://img.shields.io/badge/Quality-Professional-blue?style=for-the-badge" alt="Quality">
<img src="https://img.shields.io/badge/Language-中英双语-orange?style=for-the-badge" alt="Bilingual">

**📝 文档版本 / Document Version:** v1.0  
**📅 最后更新 / Last Updated:** 2024  
**👨‍💻 维护者 / Maintainer:** TestModbus Team

</div> 