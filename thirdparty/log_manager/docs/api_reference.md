# 分类日志系统API参考

本文档提供了分类日志系统的完整API参考，包含所有类、方法和宏的详细说明。

## 目录

- [核心类](#核心类)
  - [SimpleCategoryLogger](#simplecategorylogger)
- [日志级别](#日志级别)
- [便捷宏](#便捷宏)
  - [基本日志宏](#基本日志宏)
  - [调试日志宏](#调试日志宏)
  - [配置日志宏](#配置日志宏)
  - [断言与清理宏](#断言与清理宏)
- [高级用法](#高级用法)
  - [批量日志处理](#批量日志处理)
  - [文件清理功能](#文件清理功能)
  - [调试配置](#调试配置)
  - [日志输出目标控制](#日志输出目标控制)
- [最佳实践](#最佳实践)

---

## 核心类

### SimpleCategoryLogger

`SimpleCategoryLogger` 是系统的核心类，提供所有日志记录和管理功能。采用单例模式实现。

#### 获取实例

```cpp
static SimpleCategoryLogger& instance();
```

返回 `SimpleCategoryLogger` 的单例实例。也可以使用 `SIMPLE_LOGGER` 宏代替。

#### 初始化

```cpp
bool initCategory(const QString &category, const QString &rootDir = QString());
```

初始化指定类别的日志，第二个参数为可选的根目录。如果不提供根目录，则使用应用程序数据目录下的 `logs` 子目录。

也可以使用 `SIMPLE_LOG_INIT(category, rootDir)` 宏进行初始化。

#### 文件管理

```cpp
void setMaxLogFileSize(const QString &category, qint64 maxSizeBytes);
```

设置指定类别的日志文件最大大小（以字节为单位）。当文件达到此大小时，会自动进行轮换。

```cpp
void setDefaultMaxLogFileSize(qint64 maxSizeBytes);
```

为所有类别设置默认的最大文件大小。

```cpp
void setMaxHistoryFileCount(int count);
```

设置保留的历史日志文件数量。超过此数量的旧文件将被删除。

```cpp
QString getLogFilePath(const QString &category) const;
```

获取指定类别的当前日志文件路径。

#### 日志记录

```cpp
bool log(const QString &category, const QString &message, LogLevel level = LogLevel::INFO);
```

基本的日志记录方法，记录一条指定级别的日志到指定类别。

```cpp
bool batchLog(const QString &category, const QStringList &messages, LogLevel level = LogLevel::INFO);
```

批量记录日志，一次性写入多条消息，提高性能。

```cpp
bool logWithConfig(const QString &category, const QString &message, LogLevel level, 
                  bool showInTerminal, bool writeToFile);
```

带配置的日志记录方法，可以控制是否在终端显示和是否写入文件。

#### 调试配置

```cpp
void setDebugConfig(bool showInTerminal, bool writeToFile, bool debugMode);
```

设置全局调试配置，控制日志的默认输出行为。

```cpp
const DebugConfig& getDebugConfig() const;
```

获取当前的调试配置。

#### 断言功能

```cpp
void debugAssert(bool condition, const QString& message, const QString &category = QString());
```

断言函数，当条件为假时记录错误日志。可以使用 `SIMPLE_ASSERT(condition, message)` 或 `SIMPLE_ASSERT_CATEGORY(condition, message, category)` 宏。

#### 文件清理

```cpp
void setPeriodicCleanup(bool enabled, int intervalDays = 7, int maxAgeDays = 30, const QString &baseDir = QString());
```

配置定期清理过期日志文件的功能。

```cpp
void setCleanupFilters(const QStringList &nameFilters);
```

设置文件清理过滤器，指定清理哪些类型的文件。

```cpp
int cleanupFiles(const QString &baseDir = QString(), int maxAgeDays = -1);
```

立即执行文件清理，返回已删除的文件数量。

## 日志级别

系统支持以下日志级别，定义在 `LogLevel` 枚举中：

- `LogLevel::TRACE` - 最详细的跟踪信息，用于开发和调试
- `LogLevel::DEBUG` - 调试信息，仅在开发环境中有用
- `LogLevel::INFO` - 一般信息，记录正常操作
- `LogLevel::WARNING` - 警告信息，潜在问题
- `LogLevel::ERR` - 错误信息，操作失败
- `LogLevel::FATAL` - 致命错误，可能导致应用崩溃

## 便捷宏

系统提供了一系列便捷宏，简化日志记录操作。

### 基本日志宏

```cpp
SIMPLE_LOG_INIT(category, rootDir)  // 初始化日志类别
SIMPLE_LOG_INFO(category, message)  // 记录信息级别日志
SIMPLE_LOG_WARNING(category, message)  // 记录警告级别日志
SIMPLE_LOG_ERROR(category, message)  // 记录错误级别日志
SIMPLE_LOG_DEBUG(category, message)  // 记录调试级别日志
SIMPLE_LOG_TRACE(category, message)  // 记录跟踪级别日志
```

### 调试日志宏

```cpp
SIMPLE_DEBUG_LOG_INFO(category, message)  // 同时输出到终端和文件的信息日志
SIMPLE_DEBUG_LOG_WARNING(category, message)  // 同时输出到终端和文件的警告日志
SIMPLE_DEBUG_LOG_ERROR(category, message)  // 同时输出到终端和文件的错误日志
SIMPLE_DEBUG_LOG_DEBUG(category, message)  // 同时输出到终端和文件的调试日志
```

### 配置日志宏

```cpp
SIMPLE_LOG_INFO_CONFIG(category, message, showTerminal, writeFile)  // 可配置信息日志
SIMPLE_LOG_WARNING_CONFIG(category, message, showTerminal, writeFile)  // 可配置警告日志
SIMPLE_LOG_ERROR_CONFIG(category, message, showTerminal, writeFile)  // 可配置错误日志
SIMPLE_LOG_DEBUG_CONFIG(category, message, showTerminal, writeFile)  // 可配置调试日志
```

### 断言与清理宏

```cpp
SIMPLE_ASSERT(condition, message)  // 基本断言
SIMPLE_ASSERT_CATEGORY(condition, message, category)  // 带类别的断言
SIMPLE_ENABLE_CLEANUP(intervalDays, maxAgeDays)  // 启用自动清理
SIMPLE_DISABLE_CLEANUP()  // 禁用自动清理
SIMPLE_CLEANUP_NOW()  // 立即执行清理
```

## 高级用法

### 批量日志处理

对于需要记录大量日志的场景，可以使用批量处理提高性能：

```cpp
QStringList messages;
messages << "消息1" << "消息2" << "消息3";
SimpleCategoryLogger::instance().batchLog("类别", messages, LogLevel::INFO);
```

### 文件清理功能

设置自动清理过期日志文件，避免日志占用过多磁盘空间：

```cpp
// 设置文件清理过滤器
QStringList filters;
filters << "*.log" << "*.txt";
SimpleCategoryLogger::instance().setCleanupFilters(filters);

// 启用自动清理，每7天执行一次，保留30天内的文件
SimpleCategoryLogger::instance().setPeriodicCleanup(true, 7, 30);

// 可以随时手动执行清理
int deletedCount = SimpleCategoryLogger::instance().cleanupFiles();
```

### 调试配置

调试配置允许控制日志的默认输出行为：

```cpp
// 设置全局配置：在终端显示，写入文件，启用调试模式
SimpleCategoryLogger::instance().setDebugConfig(true, true, true);

// 获取当前配置
const SimpleCategoryLogger::DebugConfig& config = 
    SimpleCategoryLogger::instance().getDebugConfig();
```

### 日志输出目标控制

可以灵活控制日志的输出目标，优化不同场景下的性能：

```cpp
// 仅输出到控制台，不写入文件
SIMPLE_LOG_INFO_CONFIG("类别", "消息", true, false);

// 仅写入文件，不输出到控制台
SIMPLE_LOG_INFO_CONFIG("类别", "消息", false, true);

// 完全禁用此条日志（既不显示也不写入）
SIMPLE_LOG_INFO_CONFIG("类别", "消息", false, false);
```

## 最佳实践

1. **合理设置日志级别**：在开发环境使用 DEBUG/TRACE 级别，生产环境使用 INFO 或更高级别

2. **优化I/O操作**：
   - 批量记录日志减少I/O操作
   - 避免在性能敏感的循环中过度记录日志
   - 对于高频操作，考虑只记录关键节点或异常情况

3. **循环中的日志优化**：
   ```cpp
   for (int i = 0; i < 1000; i++) {
       // 仅记录开始、结束和关键节点
       if (i == 0) {
           LOG_INFO("类别", "循环开始");
       } else if (i % 100 == 0) {
           LOG_DEBUG("类别", QString("进度: %1%").arg(i / 10));
       }
       
       // 只记录异常情况
       if (someErrorCondition) {
           LOG_ERROR("类别", "发生错误");
       }
   }
   ```

4. **合理分类**：
   - 按功能模块分类（如"网络"、"数据库"、"用户界面"等）
   - 为每个主要组件创建专用类别
   - 避免创建过多细分类别，通常5-10个核心类别就足够了

5. **配置清理策略**：
   - 为重要日志设置较长的保留期
   - 为调试日志设置较短的保留期
   - 根据磁盘空间调整文件大小限制
