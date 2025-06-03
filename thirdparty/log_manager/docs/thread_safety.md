# 分类日志系统线程安全指南

本文档详细介绍分类日志系统的线程安全机制设计，以及在多线程环境中使用日志系统的最佳实践。

## 目录

- [线程安全设计](#线程安全设计)
- [关键线程安全机制](#关键线程安全机制)
- [多线程使用场景](#多线程使用场景)
- [性能优化策略](#性能优化策略)
- [最佳实践](#最佳实践)
- [常见问题及解决方案](#常见问题及解决方案)

---

## 线程安全设计

分类日志系统采用了精心设计的线程安全机制，确保在多线程环境中可靠工作。系统的线程安全性基于以下几个核心原则：

1. **互斥锁保护共享资源**：使用 `QMutex` 保护所有共享数据和文件操作
2. **最小锁范围**：仅在必要的操作上使用锁，减少线程阻塞
3. **饿汉式单例模式**：确保单例实例在程序启动时就被初始化，避免多线程初始化问题
4. **线程本地缓冲**：减少线程间的竞争，提高并发性能

## 关键线程安全机制

### 1. 饿汉式单例实现

```cpp
// SimpleCategoryLogger 的饿汉式单例实现
SimpleCategoryLogger& SimpleCategoryLogger::instance() {
    // 静态局部变量保证线程安全的初始化
    static SimpleCategoryLogger instance;
    return instance;
}
```

这种实现方式确保单例实例在程序启动时被创建，避免了多线程环境下懒汉式单例可能出现的问题。

### 2. 互斥锁保护

```cpp
bool SimpleCategoryLogger::log(const QString &category, const QString &message, LogLevel level) {
    QMutexLocker locker(&m_mutex);  // 自动锁定和解锁
    
    // 执行日志写入操作...
    
    return true;  // 锁会在函数返回时自动释放
}
```

使用 `QMutexLocker` 自动管理锁的获取和释放，确保即使出现异常情况锁也能被正确释放。

### 3. 文件操作保护

所有涉及文件创建、写入、轮换等操作都受到互斥锁保护，确保多线程环境下文件操作的一致性。

```cpp
void SimpleCategoryLogger::rotateLogFile(const QString &category) {
    QMutexLocker locker(&m_mutex);
    
    // 执行文件轮换操作...
}
```

### 4. 批量操作优化

批量日志记录方法使用单次锁定完成多条日志的写入，减少锁的获取和释放次数：

```cpp
bool SimpleCategoryLogger::batchLog(const QString &category, const QStringList &messages, LogLevel level) {
    QMutexLocker locker(&m_mutex);
    
    // 一次性写入多条日志...
    
    return true;
}
```

## 多线程使用场景

### 场景1：UI线程和工作线程同时记录日志

在这种场景下，UI线程处理界面交互并记录相关日志，同时后台工作线程执行任务并记录操作日志。系统的线程安全机制确保两个线程的日志不会相互干扰。

```cpp
// UI线程代码
void MainWindow::onButtonClicked() {
    SIMPLE_LOG_INFO("UI", "用户点击了按钮");
    
    // 启动工作线程
    WorkerThread* worker = new WorkerThread();
    connect(worker, &WorkerThread::finished, worker, &WorkerThread::deleteLater);
    worker->start();
}

// 工作线程代码
void WorkerThread::run() {
    SIMPLE_LOG_INFO("Worker", "工作线程开始执行");
    
    // 执行耗时操作
    performTask();
    
    SIMPLE_LOG_INFO("Worker", "工作线程执行完成");
}
```

### 场景2：多工作线程并发记录日志

多个工作线程同时执行任务并记录日志时，互斥锁机制确保日志记录的原子性和一致性。

```cpp
// 创建多个工作线程
for (int i = 0; i < 10; i++) {
    WorkerThread* worker = new WorkerThread(i);
    worker->start();
}

// 工作线程实现
void WorkerThread::run() {
    SIMPLE_LOG_INFO("Worker", QString("线程 %1 开始执行").arg(m_id));
    
    // 执行任务
    
    SIMPLE_LOG_INFO("Worker", QString("线程 %1 完成执行").arg(m_id));
}
```

## 性能优化策略

### 1. 减少互斥锁竞争

- **使用线程本地缓冲**：每个线程先将日志写入本地缓冲区，定期批量提交
- **批量处理日志**：减少锁获取和释放的次数
- **降低锁粒度**：为不同的日志类别使用不同的锁

### 2. 异步日志记录

在性能关键场景中，可以考虑使用异步日志记录方式：

```cpp
// 简单的异步日志队列实现
class AsyncLogQueue : public QObject {
    Q_OBJECT
public:
    static void enqueueLog(const QString& category, const QString& message, LogLevel level) {
        emit instance().newLogSignal(category, message, level);
    }
    
    static AsyncLogQueue& instance() {
        static AsyncLogQueue instance;
        return instance;
    }
    
signals:
    void newLogSignal(const QString& category, const QString& message, LogLevel level);
    
private:
    AsyncLogQueue() {
        connect(this, &AsyncLogQueue::newLogSignal, this, &AsyncLogQueue::processLog);
        moveToThread(&m_logThread);
        m_logThread.start();
    }
    
    ~AsyncLogQueue() {
        m_logThread.quit();
        m_logThread.wait();
    }
    
    QThread m_logThread;
    
private slots:
    void processLog(const QString& category, const QString& message, LogLevel level) {
        // 在专用线程中写入日志
        SIMPLE_LOG_INFO(category, message);
    }
};

// 使用方式
#define ASYNC_LOG_INFO(category, message) \
    AsyncLogQueue::enqueueLog(category, message, LogLevel::INFO)
```

## 最佳实践

### 1. 初始化顺序

确保在使用日志系统前进行初始化，最好在应用启动的早期阶段：

```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 在创建窗口前初始化日志系统
    SIMPLE_LOG_INIT("System", "");
    SIMPLE_LOG_INIT("UI", "");
    SIMPLE_LOG_INIT("Worker", "");
    
    // 继续应用初始化
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

### 2. 避免日志爆发

在循环或高频调用的函数中，避免在每次迭代都记录日志，这可能导致线程间的高度竞争：

```cpp
// 不推荐
for (int i = 0; i < 10000; i++) {
    SIMPLE_LOG_DEBUG("Loop", QString("迭代 %1").arg(i));  // 会导致严重的锁竞争
    process(i);
}

// 推荐
SIMPLE_LOG_INFO("Loop", "循环开始");
for (int i = 0; i < 10000; i++) {
    if (i % 1000 == 0) {  // 只记录关键节点
        SIMPLE_LOG_DEBUG("Loop", QString("进度: %1%").arg(i / 100.0));
    }
    process(i);
}
SIMPLE_LOG_INFO("Loop", "循环完成");
```

### 3. 线程安全的类别初始化

确保所有日志类别在应用启动时一次性初始化，避免在多线程环境中动态创建类别：

```cpp
void initializeLogging() {
    // 一次性初始化所有可能用到的日志类别
    QStringList categories = {"System", "Network", "Database", "UI", "Worker"};
    QString logRootDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    
    for (const QString& category : categories) {
        SIMPLE_LOG_INIT(category, logRootDir);
    }
    
    SIMPLE_LOG_INFO("System", "日志系统初始化完成");
}
```

### 4. 合理使用批量日志

当需要记录多条相关日志时，优先使用批量记录方法：

```cpp
// 不推荐
for (const auto& result : results) {
    SIMPLE_LOG_INFO("Database", QString("处理结果: %1").arg(result));  // 多次获取锁
}

// 推荐
QStringList logMessages;
for (const auto& result : results) {
    logMessages << QString("处理结果: %1").arg(result);
}
SimpleCategoryLogger::instance().batchLog("Database", logMessages, LogLevel::INFO);  // 只获取一次锁
```

## 常见问题及解决方案

### 问题1：日志丢失或不完整

**可能原因**：在多线程环境中，如果不正确管理互斥锁，可能导致日志写入中断或冲突。

**解决方案**：
- 确保使用 `QMutexLocker` 保护所有文件操作
- 检查是否有代码绕过了互斥锁保护
- 考虑使用批量日志或异步日志减少锁竞争

### 问题2：死锁

**可能原因**：在持有日志锁的同时尝试获取其他锁，而其他线程则以相反的顺序获取锁。

**解决方案**：
- 避免在日志操作内部调用可能获取其他锁的代码
- 保持锁的获取顺序一致
- 缩短持有锁的时间

### 问题3：性能瓶颈

**可能原因**：日志操作成为线程竞争的瓶颈，特别是在高并发环境中。

**解决方案**：
- 减少日志频率，只记录必要信息
- 使用异步日志机制
- 考虑按线程或类别分离日志文件，减少竞争
