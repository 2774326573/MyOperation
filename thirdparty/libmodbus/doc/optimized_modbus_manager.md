# 优化的 Modbus 管理器文档

## 概述

`OptimizedModbusManager` 是一个高性能的 Modbus 通信管理器，整合了连接池、缓存、异步操作、智能重连、批量操作和性能监控等多种优化功能，为需要高效 Modbus 通信的应用提供统一的高级 API。

## 文件信息

- **头文件**: `optimized_modbus_manager.h`
- **依赖**: `modbus_performance.h`, Qt5 核心库
- **继承**: `QObject`

## 主要特性

- ✅ **连接池管理** - 自动管理和复用 Modbus 连接
- ✅ **智能缓存** - 自动缓存读取结果，减少重复请求
- ✅ **异步操作** - 支持非阻塞的异步 Modbus 操作
- ✅ **智能重连** - 自动检测连接断开并智能重连
- ✅ **批量优化** - 合并多个请求以提高效率
- ✅ **性能监控** - 实时监控性能指标和健康状态
- ✅ **配置持久化** - 支持配置的保存和加载
- ✅ **调试支持** - 内置调试模式和详细日志

## 核心结构

### OptimizationConfig 配置结构

```cpp
struct OptimizationConfig {
    // 连接池配置
    int maxConnections = 10;                    // 最大连接数
    int connectionTimeoutMinutes = 30;          // 连接超时时间（分钟）
    
    // 缓存配置  
    bool cacheEnabled = true;                   // 是否启用缓存
    int defaultCacheTtlMs = 5000;              // 默认缓存TTL（毫秒）
    int cacheCleanupIntervalMs = 60000;        // 缓存清理间隔
    
    // 异步配置
    bool asyncEnabled = true;                   // 是否启用异步操作
    int maxAsyncOperations = 100;              // 最大异步操作数
    
    // 重连配置
    bool autoReconnectEnabled = true;           // 是否启用自动重连
    int initialReconnectDelayMs = 1000;        // 初始重连延迟
    int maxReconnectDelayMs = 30000;           // 最大重连延迟
    double reconnectBackoffMultiplier = 2.0;   // 重连延迟倍增因子
    int maxReconnectRetries = 10;              // 最大重连次数
    
    // 批量操作配置
    bool batchOptimizationEnabled = true;       // 是否启用批量优化
    int batchSizeLimit = 100;                  // 批量操作大小限制
    
    // 性能监控配置
    bool performanceMonitoringEnabled = true;   // 是否启用性能监控
};
```

## 核心方法

### 构造函数和配置

```cpp
// 构造函数
explicit OptimizedModbusManager(QObject* parent = nullptr);

// 设置优化配置
void setOptimizationConfig(const OptimizationConfig& config);

// 获取优化配置
OptimizationConfig getOptimizationConfig() const;

// 保存配置到文件
void saveConfigToFile(const QString& filePath);

// 从文件加载配置
void loadConfigFromFile(const QString& filePath);
```

### 连接管理

```cpp
// 连接设备（自动选择最优连接方式）
bool connectDevice(const QString& deviceId, const QString& connectionString, int slaveId = 1);

// 断开设备连接
void disconnectDevice(const QString& deviceId);
```

### 同步读写操作

#### 读取操作

```cpp
// 智能读取保持寄存器（自动缓存）
bool readHoldingRegisters(const QString& deviceId, int address, int count, 
                        QVector<quint16>& values, int cacheTtlMs = -1);

// 智能读取输入寄存器（自动缓存）
bool readInputRegisters(const QString& deviceId, int address, int count, 
                      QVector<quint16>& values, int cacheTtlMs = -1);

// 智能读取线圈（自动缓存）
bool readCoils(const QString& deviceId, int address, int count, 
              QVector<bool>& values, int cacheTtlMs = -1);

// 智能读取离散输入（自动缓存）
bool readDiscreteInputs(const QString& deviceId, int address, int count, 
                      QVector<bool>& values, int cacheTtlMs = -1);
```

#### 写入操作

```cpp
// 写入单个寄存器
bool writeSingleRegister(const QString& deviceId, int address, quint16 value);

// 写入多个寄存器
bool writeMultipleRegisters(const QString& deviceId, int address, const QVector<quint16>& values);

// 写入单个线圈
bool writeSingleCoil(const QString& deviceId, int address, bool value);

// 写入多个线圈
bool writeMultipleCoils(const QString& deviceId, int address, const QVector<bool>& values);
```

### 异步操作

```cpp
// 异步读取保持寄存器
QString readHoldingRegistersAsync(const QString& deviceId, int address, int count,
                                std::function<void(bool, const QVector<quint16>&)> callback,
                                int cacheTtlMs = -1);

// 异步读取输入寄存器
QString readInputRegistersAsync(const QString& deviceId, int address, int count,
                              std::function<void(bool, const QVector<quint16>&)> callback,
                              int cacheTtlMs = -1);

// 异步读取线圈
QString readCoilsAsync(const QString& deviceId, int address, int count,
                     std::function<void(bool, const QVector<bool>&)> callback,
                     int cacheTtlMs = -1);

// 异步写入多个寄存器
QString writeMultipleRegistersAsync(const QString& deviceId, int address, 
                                  const QVector<quint16>& values,
                                  std::function<void(bool)> callback);

// 取消异步操作
void cancelAsyncOperation(const QString& operationId);
```

### 批量操作

```cpp
// 添加批量读取请求
void addBatchReadRequest(const QString& deviceId, int address, int count, 
                       ModbusManager::DataType dataType);

// 执行批量操作
QMap<QString, QVariant> executeBatchOperations();

// 清空批量请求队列
void clearBatchRequests();
```

### 监控和统计

```cpp
// 获取连接池统计信息
QMap<QString, QVariant> getConnectionPoolStats() const;

// 获取缓存统计信息
QMap<QString, QVariant> getCacheStats() const;

// 获取性能统计信息
QMap<QString, QVariant> getPerformanceStats() const;

// 获取异步队列状态
QMap<QString, QVariant> getAsyncQueueStatus() const;

// 生成综合性能报告
QString generatePerformanceReport() const;

// 获取系统健康状态
QMap<QString, QVariant> getSystemHealth() const;
```

### 高级功能

```cpp
// 启用智能重连
void enableSmartReconnect(const QString& deviceId);

// 禁用智能重连
void disableSmartReconnect(const QString& deviceId);

// 预热连接池（预创建连接）
void warmupConnectionPool(const QStringList& deviceIds);

// 清理资源（清理过期连接和缓存）
void cleanup();

// 重置所有统计数据
void resetStatistics();

// 设置调试模式
void setDebugMode(bool enabled);

// 清除缓存
void clearCache();
```

## 信号（Signals）

```cpp
// 连接建立
void connectionEstablished(const QString& deviceId);

// 连接丢失
void connectionLost(const QString& deviceId);

// 连接恢复
void connectionRestored(const QString& deviceId);

// 操作完成
void operationCompleted(const QString& operationId, bool success);

// 异步操作完成
void asyncOperationCompleted(const QString& operationId, bool success, const QVariant& result);

// 性能警报
void performanceAlert(const QString& message);

// 缓存命中
void cacheHit(const QString& key);

// 缓存未命中
void cacheMiss(const QString& key);
```

## 使用示例

### 基本使用

```cpp
#include "optimized_modbus_manager.h"

// 创建优化的管理器
OptimizedModbusManager* manager = new OptimizedModbusManager(this);

// 配置优化参数
OptimizedModbusManager::OptimizationConfig config;
config.maxConnections = 20;
config.cacheEnabled = true;
config.defaultCacheTtlMs = 3000;
config.asyncEnabled = true;
config.autoReconnectEnabled = true;

manager->setOptimizationConfig(config);

// 连接设备
bool success = manager->connectDevice("device1", "192.168.1.100:502", 1);
if (success) {
    qDebug() << "设备连接成功";
}
```

### 同步读取操作

```cpp
// 读取保持寄存器（自动缓存）
QVector<quint16> values;
bool success = manager->readHoldingRegisters("device1", 0, 10, values);
if (success) {
    qDebug() << "读取成功，值：" << values;
}

// 指定缓存TTL
success = manager->readHoldingRegisters("device1", 100, 5, values, 10000); // 10秒缓存
```

### 异步读取操作

```cpp
// 异步读取保持寄存器
QString operationId = manager->readHoldingRegistersAsync("device1", 0, 10,
    [](bool success, const QVector<quint16>& values) {
        if (success) {
            qDebug() << "异步读取成功，值：" << values;
        } else {
            qDebug() << "异步读取失败";
        }
    });

qDebug() << "异步操作ID：" << operationId;
```

### 批量操作

```cpp
// 添加批量读取请求
manager->addBatchReadRequest("device1", 0, 10, ModbusManager::DataType::HoldingRegisters);
manager->addBatchReadRequest("device1", 50, 5, ModbusManager::DataType::InputRegisters);
manager->addBatchReadRequest("device2", 100, 20, ModbusManager::DataType::Coils);

// 执行批量操作
QMap<QString, QVariant> results = manager->executeBatchOperations();
for (auto it = results.begin(); it != results.end(); ++it) {
    qDebug() << "批量结果：" << it.key() << "=" << it.value();
}
```

### 性能监控

```cpp
// 获取性能统计
QMap<QString, QVariant> stats = manager->getPerformanceStats();
qDebug() << "总请求数：" << stats["totalRequests"];
qDebug() << "平均响应时间：" << stats["averageResponseTime"] << "ms";
qDebug() << "缓存命中率：" << stats["cacheHitRate"] << "%";

// 生成性能报告
QString report = manager->generatePerformanceReport();
qDebug() << "性能报告：\n" << report;

// 获取系统健康状态
QMap<QString, QVariant> health = manager->getSystemHealth();
qDebug() << "系统健康评分：" << health["healthScore"];
```

### 配置持久化

```cpp
// 保存配置
manager->saveConfigToFile("modbus_config.json");

// 加载配置
manager->loadConfigFromFile("modbus_config.json");
```

### 信号连接

```cpp
// 连接信号
connect(manager, &OptimizedModbusManager::connectionEstablished,
        this, [](const QString& deviceId) {
    qDebug() << "设备连接建立：" << deviceId;
});

connect(manager, &OptimizedModbusManager::connectionLost,
        this, [](const QString& deviceId) {
    qDebug() << "设备连接丢失：" << deviceId;
});

connect(manager, &OptimizedModbusManager::performanceAlert,
        this, [](const QString& message) {
    qWarning() << "性能警报：" << message;
});

connect(manager, &OptimizedModbusManager::cacheHit,
        this, [](const QString& key) {
    qDebug() << "缓存命中：" << key;
});
```

## 最佳实践

### 配置优化

```cpp
// 根据应用场景调整配置
OptimizedModbusManager::OptimizationConfig config;

// 高频读取场景
if (isHighFrequencyReadingScenario) {
    config.cacheEnabled = true;
    config.defaultCacheTtlMs = 1000;  // 短缓存时间
    config.maxConnections = 20;       // 更多连接
    config.batchOptimizationEnabled = true;
}

// 低延迟场景
if (isLowLatencyScenario) {
    config.asyncEnabled = true;
    config.maxAsyncOperations = 200;
    config.cacheEnabled = false;      // 禁用缓存确保实时性
}

// 稳定性优先场景
if (isStabilityFirstScenario) {
    config.autoReconnectEnabled = true;
    config.maxReconnectRetries = 20;
    config.maxReconnectDelayMs = 60000;
}
```

### 错误处理

```cpp
class ModbusErrorHandler {
public:
    static void setupErrorHandling(OptimizedModbusManager* manager) {
        // 连接丢失处理
        QObject::connect(manager, &OptimizedModbusManager::connectionLost,
                        [manager](const QString& deviceId) {
            qWarning() << "设备连接丢失：" << deviceId;
            // 启用智能重连
            manager->enableSmartReconnect(deviceId);
        });

        // 性能警报处理
        QObject::connect(manager, &OptimizedModbusManager::performanceAlert,
                        [](const QString& message) {
            qCritical() << "性能问题：" << message;
            // 可以在这里触发清理或优化操作
        });
    }
};
```

### 资源管理

```cpp
// 定期清理资源
QTimer* cleanupTimer = new QTimer(this);
connect(cleanupTimer, &QTimer::timeout, manager, &OptimizedModbusManager::cleanup);
cleanupTimer->start(300000); // 每5分钟清理一次

// 应用退出前保存配置
connect(qApp, &QApplication::aboutToQuit, [manager]() {
    manager->saveConfigToFile("modbus_config.json");
    manager->resetStatistics();
});
```

## 性能优化指南

### 缓存策略

1. **合理设置缓存TTL**：根据数据更新频率设置合适的缓存时间
2. **选择性缓存**：只对读取频繁的数据启用缓存
3. **定期清理**：定期清理过期缓存避免内存泄露

### 连接池优化

1. **预热连接**：在应用启动时预创建常用连接
2. **连接复用**：尽量复用现有连接而不是频繁创建新连接
3. **合理设置池大小**：根据并发需求设置连接池大小

### 批量操作

1. **合并请求**：将相邻的读取请求合并为批量操作
2. **控制批量大小**：避免单次批量操作过大影响响应时间
3. **异步批量**：对于大量数据的批量操作使用异步方式

## 故障排除

### 常见问题

1. **连接超时**
   - 检查网络连接
   - 调整连接超时参数
   - 启用智能重连

2. **性能下降**
   - 检查缓存命中率
   - 监控连接池使用情况
   - 查看性能报告

3. **内存使用过高**
   - 定期调用 `cleanup()`
   - 减少缓存TTL时间
   - 限制连接池大小

### 调试模式

```cpp
// 启用调试模式获取详细日志
manager->setDebugMode(true);

// 监控性能指标
QTimer* monitorTimer = new QTimer(this);
connect(monitorTimer, &QTimer::timeout, [manager]() {
    QMap<QString, QVariant> stats = manager->getPerformanceStats();
    qDebug() << "当前性能状态：" << stats;
});
monitorTimer->start(10000); // 每10秒监控一次
```

## 线程安全

`OptimizedModbusManager` 内部使用了线程安全的组件，但建议：

1. 在主线程中创建和配置管理器
2. 异步操作的回调会在主线程中执行
3. 避免在多个线程中同时修改配置

## 相关文档

- [Modbus 管理器文档](modbus.md) - 基础 Modbus 功能
- [Modbus 性能优化文档](modbus_performance.md) - 性能优化组件详情
- [Modbus 基准测试文档](modbus_benchmark.md) - 性能测试工具
- [Modbus 读写管理器文档](modbus_rw_manager.md) - 高级读写功能
