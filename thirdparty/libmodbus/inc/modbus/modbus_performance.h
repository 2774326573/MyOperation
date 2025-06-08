#pragma once

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QMap>
#include <QDateTime>
#include <QVector>
#include <QWaitCondition>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QSharedPointer>
#include <QAtomicInt>
#include <QElapsedTimer>
#include <functional>
#include <memory>

#include "modbusmanager.h"
#include "modbus_rw_manager.h"

/**
 * @brief 高性能Modbus连接池管理器
 * 
 * 提供连接池功能，避免频繁创建和销毁连接，提升性能
 */
class ModbusConnectionPool : public QObject
{
    Q_OBJECT

public:
    struct ConnectionInfo {
        QString deviceId;           ///< 设备标识
        ModbusManager* manager;     ///< Modbus管理器
        bool inUse;                ///< 是否正在使用
        QDateTime lastUsed;         ///< 最后使用时间
        int useCount;              ///< 使用次数
        QString connectionString;   ///< 连接字符串
    };

    explicit ModbusConnectionPool(int maxConnections = 10, QObject* parent = nullptr);
    ~ModbusConnectionPool();

    /**
     * @brief 获取连接
     * @param deviceId 设备ID
     * @param connectionString 连接字符串 (格式: "RTU:COM1:9600:8:N:1" 或 "TCP:192.168.1.100:502")
     * @return Modbus管理器指针
     */
    ModbusManager* acquireConnection(const QString& deviceId, const QString& connectionString);

    /**
     * @brief 释放连接
     * @param manager Modbus管理器指针
     */
    void releaseConnection(ModbusManager* manager);

    /**
     * @brief 获取连接池统计信息
     */
    QMap<QString, QVariant> getPoolStatistics() const;    
    /**
     * @brief 清理过期连接
     * @param timeoutMinutes 超时时间（分钟）
     */
    void cleanupExpiredConnections(int timeoutMinutes = 30);

    /**
     * @brief 重置连接池统计数据
     */
    void resetStatistics();

private slots:
    void performPeriodicCleanup();

private:
    QList<ConnectionInfo> m_connections;
    mutable QMutex m_poolMutex;
    int m_maxConnections;
    QTimer* m_cleanupTimer;
    
    ModbusManager* createConnection(const QString& connectionString);
    bool isConnectionValid(ModbusManager* manager);
};

/**
 * @brief 智能数据缓存管理器
 * 
 * 提供TTL缓存机制，减少重复的网络请求
 */
class ModbusDataCache : public QObject
{
    Q_OBJECT

public:
    struct CacheEntry {
        QVariant value;             ///< 缓存值
        QDateTime timestamp;        ///< 缓存时间
        int ttl;                   ///< 生存时间（毫秒）
        int hitCount;              ///< 命中次数
        QString key;               ///< 缓存键
    };

    explicit ModbusDataCache(QObject* parent = nullptr);
    ~ModbusDataCache();

    /**
     * @brief 设置缓存值
     * @param key 缓存键
     * @param value 缓存值
     * @param ttlMsec 生存时间（毫秒）
     */
    void setValue(const QString& key, const QVariant& value, int ttlMsec = 5000);

    /**
     * @brief 获取缓存值
     * @param key 缓存键
     * @param found 是否找到（可选）
     * @return 缓存值
     */
    QVariant getValue(const QString& key, bool* found = nullptr);

    /**
     * @brief 检查缓存是否存在且有效
     * @param key 缓存键
     * @return 是否存在且有效
     */
    bool isValid(const QString& key) const;

    /**
     * @brief 清理过期缓存
     */
    void cleanupExpiredEntries();

    /**
     * @brief 获取缓存统计信息
     */
    QMap<QString, QVariant> getCacheStatistics() const;    /**
     * @brief 清空所有缓存
     */
    void clear();

    /**
     * @brief 重置缓存统计数据
     */
    void resetStatistics();

private slots:
    void performPeriodicCleanup();

private:
    QMap<QString, CacheEntry> m_cache;
    mutable QMutex m_cacheMutex;
    QTimer* m_cleanupTimer;
    QAtomicInt m_totalHits;
    QAtomicInt m_totalMisses;
};

/**
 * @brief 异步Modbus操作管理器
 * 
 * 提供非阻塞的异步操作，提升UI响应性
 */
class AsyncModbusManager : public QObject
{
    Q_OBJECT

public:    struct AsyncOperation {
        std::function<QVariant()> operation;
        std::function<void(bool, const QVariant&)> callback;
        QString operationId;
        QDateTime timestamp;
    };

    explicit AsyncModbusManager(ModbusConnectionPool* pool, QObject* parent = nullptr);
    ~AsyncModbusManager();

    /**
     * @brief 异步读取保持寄存器
     */
    QString readHoldingRegistersAsync(const QString& deviceId, const QString& connectionString,
                                    int address, int count,
                                    std::function<void(bool, const QVector<quint16>&)> callback);

    /**
     * @brief 异步写入多个寄存器
     */
    QString writeMultipleRegistersAsync(const QString& deviceId, const QString& connectionString,
                                      int address, const QVector<quint16>& values,
                                      std::function<void(bool)> callback);

    /**
     * @brief 异步读取线圈
     */
    QString readCoilsAsync(const QString& deviceId, const QString& connectionString,
                          int address, int count,
                          std::function<void(bool, const QVector<bool>&)> callback);

    /**
     * @brief 取消异步操作
     * @param operationId 操作ID
     */
    void cancelOperation(const QString& operationId);

    /**
     * @brief 获取待处理操作数量
     */
    int getPendingOperationsCount() const;    /**
     * @brief 获取队列状态
     */
    QMap<QString, QVariant> getQueueStatus() const;

    /**
     * @brief 重置异步操作统计数据
     */
    void resetStatistics();

private slots:
    void processOperations();

private:    ModbusConnectionPool* m_connectionPool;
    QQueue<AsyncOperation> m_operationQueue;
    mutable QMutex m_queueMutex;
    QWaitCondition m_queueCondition;
    QThread* m_workerThread;
    bool m_running;
    QAtomicInt m_operationIdCounter;
};

/**
 * @brief 智能重连管理器
 * 
 * 提供指数退避重连策略，智能处理连接失败
 */
class SmartReconnectManager : public QObject
{
    Q_OBJECT

public:
    struct ReconnectStrategy {
        int initialDelayMs;         ///< 初始延迟
        int maxDelayMs;            ///< 最大延迟
        double backoffMultiplier;   ///< 退避倍数
        int maxRetries;            ///< 最大重试次数
        bool jitterEnabled;        ///< 是否启用抖动
    };

    explicit SmartReconnectManager(QObject* parent = nullptr);
    ~SmartReconnectManager();

    /**
     * @brief 设置重连策略
     */
    void setReconnectStrategy(const ReconnectStrategy& strategy);

    /**
     * @brief 注册连接管理器
     */
    void registerConnection(const QString& connectionId, ModbusManager* manager,
                          const QString& connectionString);

    /**
     * @brief 开始监控连接
     */
    void startMonitoring(const QString& connectionId);

    /**
     * @brief 停止监控连接
     */
    void stopMonitoring(const QString& connectionId);

    /**
     * @brief 重置重连管理器统计数据
     */
    void resetStatistics();

signals:
    void connectionRestored(const QString& connectionId);
    void connectionFailed(const QString& connectionId, int retryCount);
    void maxRetriesReached(const QString& connectionId);

private slots:
    void checkConnections();
    void performReconnect();

private:
    struct ConnectionState {
        QString connectionId;
        ModbusManager* manager;
        QString connectionString;
        bool isConnected;
        int currentRetryCount;
        int currentDelayMs;
        QDateTime lastReconnectAttempt;
        QTimer* reconnectTimer;
    };

    QMap<QString, ConnectionState> m_connections;
    QTimer* m_monitorTimer;
    ReconnectStrategy m_strategy;
    QMutex m_connectionsMutex;
};

/**
 * @brief 高性能批量操作管理器
 * 
 * 优化批量读写操作，自动合并连续地址的请求
 */
class BatchOperationManager : public QObject
{
    Q_OBJECT

public:
    struct BatchRequest {
        QString deviceId;
        QString connectionString;
        int startAddress;
        int count;
        ModbusManager::DataType dataType;
        QDateTime timestamp;
    };

    struct BatchResponse {
        bool success;
        QVector<QVariant> values;
        QString errorMessage;
        qint64 processingTimeMs;
    };

    explicit BatchOperationManager(ModbusConnectionPool* pool, QObject* parent = nullptr);
    ~BatchOperationManager();

    /**
     * @brief 添加批量读取请求
     */
    void addReadRequest(const BatchRequest& request);

    /**
     * @brief 执行批量操作
     */
    QMap<QString, BatchResponse> executeBatch();

    /**
     * @brief 设置批量大小限制
     */
    void setBatchSizeLimit(int limit);    /**
     * @brief 启用请求优化（合并连续地址）
     */
    void setRequestOptimizationEnabled(bool enabled);

    /**
     * @brief 重置批量操作统计数据
     */
    void resetStatistics();

private:
    ModbusConnectionPool* m_connectionPool;
    QList<BatchRequest> m_pendingRequests;
    QMutex m_requestsMutex;
    int m_batchSizeLimit;
    bool m_optimizationEnabled;

    QList<BatchRequest> optimizeRequests(const QList<BatchRequest>& requests);
    BatchResponse executeRequest(const BatchRequest& request);
};

/**
 * @brief 性能监控器
 * 
 * 监控Modbus操作性能，提供性能统计和报告
 */
class ModbusPerformanceMonitor : public QObject
{
    Q_OBJECT

public:
    struct PerformanceMetrics {
        int totalOperations;        ///< 总操作数
        int successfulOperations;   ///< 成功操作数
        int failedOperations;      ///< 失败操作数
        double averageResponseTime; ///< 平均响应时间
        double maxResponseTime;     ///< 最大响应时间
        double minResponseTime;     ///< 最小响应时间
        double operationsPerSecond; ///< 每秒操作数
        QDateTime startTime;        ///< 开始时间
        QDateTime lastOperationTime; ///< 最后操作时间
    };

    explicit ModbusPerformanceMonitor(QObject* parent = nullptr);
    ~ModbusPerformanceMonitor();

    /**
     * @brief 记录操作开始
     */
    void recordOperationStart(const QString& operationId);

    /**
     * @brief 记录操作结束
     */
    void recordOperationEnd(const QString& operationId, bool success);

    /**
     * @brief 获取性能指标
     */
    PerformanceMetrics getMetrics() const;

    /**
     * @brief 获取性能统计信息 (返回QMap格式)
     */
    QMap<QString, QVariant> getPerformanceStatistics() const;

    /**
     * @brief 重置统计数据
     */
    void reset();

    /**
     * @brief 生成性能报告
     */
    QString generateReport() const;

private:
    struct OperationRecord {
        QDateTime startTime;
        QDateTime endTime;
        bool success;
        qint64 durationMs;
    };

    QMap<QString, QElapsedTimer> m_activeOperations;
    QList<OperationRecord> m_completedOperations;
    mutable QMutex m_metricsMutex;
    QDateTime m_monitorStartTime;
};

/**
 * @brief 内存池管理器
 * 
 * 预分配内存池，减少内存分配开销
 */
template<typename T>
class MemoryPool
{
public:
    explicit MemoryPool(int poolSize = 100, int objectSize = 128)
        : m_poolSize(poolSize), m_objectSize(objectSize) {
        
        for (int i = 0; i < poolSize; ++i) {
            m_availableObjects.enqueue(new QVector<T>(objectSize));
        }
    }

    ~MemoryPool() {
        while (!m_availableObjects.isEmpty()) {
            delete m_availableObjects.dequeue();
        }
        for (auto obj : m_usedObjects) {
            delete obj;
        }
    }

    QVector<T>* acquire() {
        QMutexLocker locker(&m_poolMutex);
        
        if (!m_availableObjects.isEmpty()) {
            QVector<T>* obj = m_availableObjects.dequeue();
            m_usedObjects.insert(obj);
            return obj;
        }
        
        // 池中没有可用对象，创建新对象
        QVector<T>* newObj = new QVector<T>(m_objectSize);
        m_usedObjects.insert(newObj);
        return newObj;
    }

    void release(QVector<T>* obj) {
        QMutexLocker locker(&m_poolMutex);
        
        if (m_usedObjects.remove(obj)) {
            obj->fill(T{});  // 重置对象
            
            if (m_availableObjects.size() < m_poolSize) {
                m_availableObjects.enqueue(obj);
            } else {
                delete obj;  // 池已满，删除对象
            }
        }
    }

private:
    QQueue<QVector<T>*> m_availableObjects;
    QSet<QVector<T>*> m_usedObjects;
    QMutex m_poolMutex;
    int m_poolSize;
    int m_objectSize;
};

// 全局内存池实例
extern MemoryPool<quint16> g_registerPool;
extern MemoryPool<bool> g_coilPool;
