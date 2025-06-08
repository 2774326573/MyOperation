# Modbus 性能优化文档

## 概述

`modbus_performance.h` 提供了 Modbus 通信的高性能优化组件，包括连接池管理、智能缓存、批量操作和性能监控等功能。

## 文件信息

- **文件路径**: `inc/modbus/modbus_performance.h`
- **基类**: `QObject`
- **Qt 模块**: Core, Concurrent

## 主要组件

### 1. ModbusConnectionPool - 连接池管理器
### 2. ModbusDataCache - 智能数据缓存
### 3. ModbusBatchProcessor - 批量操作处理器
### 4. ModbusPerformanceMonitor - 性能监控器

## ModbusConnectionPool - 连接池管理器

### 概述
提供连接池功能，避免频繁创建和销毁连接，大幅提升性能。

### 连接信息结构
```cpp
struct ConnectionInfo {
    QString deviceId;           // 设备标识
    ModbusManager* manager;     // Modbus管理器
    bool inUse;                // 是否正在使用
    QDateTime lastUsed;         // 最后使用时间
    int useCount;              // 使用次数
    QString connectionString;   // 连接字符串
};
```

### 核心方法
```cpp
class ModbusConnectionPool : public QObject {
public:
    explicit ModbusConnectionPool(int maxConnections = 10, QObject* parent = nullptr);
    
    // 获取连接
    ModbusManager* acquireConnection(const QString& deviceId, const QString& connectionString);
    
    // 释放连接
    void releaseConnection(ModbusManager* manager);
    
    // 获取统计信息
    QMap<QString, QVariant> getPoolStatistics() const;
    
    // 清理过期连接
    void cleanupExpiredConnections(int timeoutMinutes = 30);
    
    // 重置统计数据
    void resetStatistics();
};
```

### 使用示例
```cpp
// 创建连接池
ModbusConnectionPool *pool = new ModbusConnectionPool(20, this);

// 获取RTU连接
QString rtuConnection = "RTU:COM1:9600:8:N:1";
ModbusManager *rtuManager = pool->acquireConnection("device1", rtuConnection);

if (rtuManager && rtuManager->isConnected()) {
    // 执行Modbus操作
    QVector<quint16> values;
    rtuManager->readHoldingRegisters(0, 10, values);
    
    // 处理数据...
    
    // 释放连接回池
    pool->releaseConnection(rtuManager);
}

// 获取TCP连接
QString tcpConnection = "TCP:192.168.1.100:502";
ModbusManager *tcpManager = pool->acquireConnection("device2", tcpConnection);

// 使用完毕后释放
pool->releaseConnection(tcpManager);

// 查看连接池统计
QMap<QString, QVariant> stats = pool->getPoolStatistics();
qDebug() << "活跃连接数:" << stats["active_connections"].toInt();
qDebug() << "总连接数:" << stats["total_connections"].toInt();
qDebug() << "缓存命中率:" << stats["hit_rate"].toDouble();
```

## ModbusDataCache - 智能数据缓存

### 概述
提供TTL（生存时间）缓存机制，减少重复的网络请求，提升读取性能。

### 缓存条目结构
```cpp
struct CacheEntry {
    QVariant value;             // 缓存值
    QDateTime timestamp;        // 缓存时间
    int ttl;                   // 生存时间（毫秒）
    int hitCount;              // 命中次数
    QString key;               // 缓存键
};
```

### 核心方法
```cpp
class ModbusDataCache : public QObject {
public:
    explicit ModbusDataCache(QObject* parent = nullptr);
    
    // 设置缓存值
    void setValue(const QString& key, const QVariant& value, int ttlMsec = 5000);
    
    // 获取缓存值
    QVariant getValue(const QString& key, bool* found = nullptr);
    
    // 检查缓存有效性
    bool isValid(const QString& key) const;
    
    // 清理过期缓存
    void cleanupExpiredEntries();
    
    // 获取缓存统计
    QMap<QString, QVariant> getCacheStatistics() const;
    
    // 清空缓存
    void clear();
    
    // 重置统计
    void resetStatistics();
};
```

### 使用示例
```cpp
// 创建缓存管理器
ModbusDataCache *cache = new ModbusDataCache(this);

// 生成缓存键
QString cacheKey = QString("device1:holding:%1:%2").arg(startAddr).arg(count);

// 尝试从缓存获取数据
bool found;
QVariant cachedValue = cache->getValue(cacheKey, &found);

if (found) {
    // 使用缓存数据
    QVector<quint16> values = cachedValue.value<QVector<quint16>>();
    qDebug() << "从缓存获取数据:" << values;
} else {
    // 从设备读取数据
    QVector<quint16> values;
    if (manager->readHoldingRegisters(startAddr, count, values)) {
        // 将数据存入缓存（TTL: 3秒）
        cache->setValue(cacheKey, QVariant::fromValue(values), 3000);
        qDebug() << "从设备读取并缓存数据:" << values;
    }
}

// 查看缓存统计
QMap<QString, QVariant> cacheStats = cache->getCacheStatistics();
qDebug() << "缓存条目数:" << cacheStats["entry_count"].toInt();
qDebug() << "命中率:" << cacheStats["hit_rate"].toDouble();
qDebug() << "总命中次数:" << cacheStats["total_hits"].toInt();
```

## 性能优化策略

### 1. 连接池配置
```cpp
// 根据并发需求配置连接池大小
int maxConnections = qMax(10, QThread::idealThreadCount() * 2);
ModbusConnectionPool *pool = new ModbusConnectionPool(maxConnections, this);

// 设置清理间隔
pool->cleanupExpiredConnections(15); // 每15分钟清理一次过期连接
```

### 2. 缓存策略
```cpp
// 为不同类型的数据设置不同的TTL
ModbusDataCache *cache = new ModbusDataCache(this);

// 配置数据 - 长TTL（不经常变化）
QString configKey = "device1:config:param1";
cache->setValue(configKey, configValue, 60000); // 60秒

// 状态数据 - 中等TTL
QString statusKey = "device1:status:temperature";
cache->setValue(statusKey, statusValue, 5000); // 5秒

// 实时数据 - 短TTL
QString realtimeKey = "device1:realtime:current";
cache->setValue(realtimeKey, realtimeValue, 1000); // 1秒
```

### 3. 批量操作优化
```cpp
// 使用批量读取代替多次单独读取
QVector<quint16> allValues;
if (manager->readHoldingRegisters(0, 100, allValues)) { // 一次读取100个寄存器
    // 分别处理不同用途的数据
    QVector<quint16> configData = allValues.mid(0, 20);
    QVector<quint16> statusData = allValues.mid(20, 30);
    QVector<quint16> measureData = allValues.mid(50, 50);
    
    // 分别缓存不同类型的数据
    cache->setValue("device1:config", QVariant::fromValue(configData), 60000);
    cache->setValue("device1:status", QVariant::fromValue(statusData), 5000);
    cache->setValue("device1:measure", QVariant::fromValue(measureData), 1000);
}
```

### 4. 异步操作
```cpp
// 使用 QtConcurrent 进行异步操作
QFuture<QVector<quint16>> future = QtConcurrent::run([=]() {
    QVector<quint16> values;
    ModbusManager *manager = pool->acquireConnection("device1", connectionString);
    
    if (manager && manager->readHoldingRegisters(0, 50, values)) {
        pool->releaseConnection(manager);
        return values;
    }
    
    pool->releaseConnection(manager);
    return QVector<quint16>();
});

// 监听完成信号
QFutureWatcher<QVector<quint16>> *watcher = new QFutureWatcher<QVector<quint16>>(this);
connect(watcher, &QFutureWatcher<QVector<quint16>>::finished, this, [=]() {
    QVector<quint16> result = watcher->result();
    // 处理结果...
    watcher->deleteLater();
});
watcher->setFuture(future);
```

## 性能监控

### 连接池监控
```cpp
// 定期监控连接池状态
QTimer *monitorTimer = new QTimer(this);
connect(monitorTimer, &QTimer::timeout, this, [=]() {
    QMap<QString, QVariant> stats = pool->getPoolStatistics();
    
    int activeConnections = stats["active_connections"].toInt();
    int totalConnections = stats["total_connections"].toInt();
    double hitRate = stats["hit_rate"].toDouble();
    
    qDebug() << QString("连接池状态 - 活跃: %1, 总数: %2, 命中率: %3%")
                .arg(activeConnections).arg(totalConnections).arg(hitRate, 0, 'f', 1);
    
    // 如果命中率过低，考虑增加连接池大小
    if (hitRate < 80.0 && totalConnections < 50) {
        qDebug() << "建议增加连接池大小";
    }
});
monitorTimer->start(30000); // 30秒监控一次
```

### 缓存监控
```cpp
// 监控缓存效果
QTimer *cacheMonitorTimer = new QTimer(this);
connect(cacheMonitorTimer, &QTimer::timeout, this, [=]() {
    QMap<QString, QVariant> cacheStats = cache->getCacheStatistics();
    
    int entryCount = cacheStats["entry_count"].toInt();
    double hitRate = cacheStats["hit_rate"].toDouble();
    int totalHits = cacheStats["total_hits"].toInt();
    int totalMisses = cacheStats["total_misses"].toInt();
    
    qDebug() << QString("缓存状态 - 条目: %1, 命中率: %2%, 命中: %3, 未命中: %4")
                .arg(entryCount).arg(hitRate, 0, 'f', 1).arg(totalHits).arg(totalMisses);
    
    // 自动调整TTL
    if (hitRate > 90.0) {
        qDebug() << "缓存效果良好，可以考虑延长TTL";
    } else if (hitRate < 50.0) {
        qDebug() << "缓存效果不佳，检查TTL设置或数据访问模式";
    }
});
cacheMonitorTimer->start(60000); // 60秒监控一次
```

## 综合优化示例

### 高性能 Modbus 客户端
```cpp
class HighPerformanceModbusClient : public QObject {
    Q_OBJECT
    
public:
    explicit HighPerformanceModbusClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_pool(new ModbusConnectionPool(20, this))
        , m_cache(new ModbusDataCache(this))
    {
        // 设置定期清理
        m_cleanupTimer = new QTimer(this);
        connect(m_cleanupTimer, &QTimer::timeout, this, &HighPerformanceModbusClient::performCleanup);
        m_cleanupTimer->start(300000); // 5分钟清理一次
    }
    
    // 高性能读取方法
    QFuture<QVector<quint16>> readHoldingRegistersAsync(const QString &deviceId, 
                                                       const QString &connectionString,
                                                       int startAddr, int count) {
        return QtConcurrent::run([=]() -> QVector<quint16> {
            // 检查缓存
            QString cacheKey = QString("%1:holding:%2:%3").arg(deviceId).arg(startAddr).arg(count);
            bool found;
            QVariant cachedValue = m_cache->getValue(cacheKey, &found);
            
            if (found) {
                return cachedValue.value<QVector<quint16>>();
            }
            
            // 从连接池获取连接
            ModbusManager *manager = m_pool->acquireConnection(deviceId, connectionString);
            if (!manager) {
                return QVector<quint16>();
            }
            
            QVector<quint16> values;
            if (manager->readHoldingRegisters(startAddr, count, values)) {
                // 缓存结果
                m_cache->setValue(cacheKey, QVariant::fromValue(values), 5000);
            }
            
            m_pool->releaseConnection(manager);
            return values;
        });
    }
    
    // 批量写入方法
    QFuture<bool> writeMultipleRegistersAsync(const QString &deviceId,
                                            const QString &connectionString,
                                            int startAddr, const QVector<quint16> &values) {
        return QtConcurrent::run([=]() -> bool {
            ModbusManager *manager = m_pool->acquireConnection(deviceId, connectionString);
            if (!manager) {
                return false;
            }
            
            bool success = manager->writeMultipleRegisters(startAddr, values);
            
            if (success) {
                // 更新缓存
                QString cacheKey = QString("%1:holding:%2:%3").arg(deviceId).arg(startAddr).arg(values.size());
                m_cache->setValue(cacheKey, QVariant::fromValue(values), 5000);
            }
            
            m_pool->releaseConnection(manager);
            return success;
        });
    }
    
    // 获取性能统计
    QVariantMap getPerformanceStats() const {
        QVariantMap stats;
        stats["pool"] = m_pool->getPoolStatistics();
        stats["cache"] = m_cache->getCacheStatistics();
        return stats;
    }

private slots:
    void performCleanup() {
        m_pool->cleanupExpiredConnections(30);
        m_cache->cleanupExpiredEntries();
    }

private:
    ModbusConnectionPool *m_pool;
    ModbusDataCache *m_cache;
    QTimer *m_cleanupTimer;
};
```

## 性能调优建议

### 1. 连接池大小
- **小型系统**: 5-10个连接
- **中型系统**: 10-20个连接  
- **大型系统**: 20-50个连接
- **规则**: 不超过 CPU核心数 × 4

### 2. 缓存TTL设置
- **配置数据**: 60-300秒
- **状态数据**: 3-10秒
- **实时数据**: 0.5-2秒
- **历史数据**: 10-60秒

### 3. 批量操作
- **读取**: 一次读取10-100个寄存器
- **写入**: 批量写入相邻寄存器
- **避免**: 频繁的单个寄存器操作

### 4. 网络优化
- 使用TCP时启用Nagle算法禁用
- 设置合适的socket缓冲区大小
- 考虑TCP连接复用

## 相关文档

- [Modbus 管理器文档](modbusmanager.md)
- [Modbus 基准测试文档](modbus_benchmark.md)
- [Modbus 读写管理器文档](modbus_rw_manager.md)
- [优化 Modbus 管理器文档](optimized_modbus_manager.md)
