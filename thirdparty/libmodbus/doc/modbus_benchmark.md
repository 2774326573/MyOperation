# Modbus 性能基准测试文档

## 概述

`modbus_benchmark.h` 提供了 Modbus 通信性能基准测试工具，用于测量和比较不同 Modbus 实现的性能表现。

## 文件信息

- **文件路径**: `inc/modbus/modbus_benchmark.h`
- **基类**: `QObject`
- **Qt 模块**: Core, Network

## 主要特性

### 基准测试类型
- 读操作性能测试
- 写操作性能测试
- 混合操作性能测试
- 缓存效果测试
- 连接池测试
- 异步操作测试

### 性能指标
- 操作总数和成功率
- 平均响应时间
- 每秒操作数（OPS）
- 自定义性能指标

### 比较测试
- 不同管理器性能对比
- 优化前后性能对比
- 多线程并发测试

## 数据结构

### 基准测试结果
```cpp
struct BenchmarkResult {
    QString testName;              // 测试名称
    int totalOperations = 0;       // 总操作数
    int successfulOperations = 0;  // 成功操作数
    int failedOperations = 0;      // 失败操作数
    qint64 totalTimeMs = 0;        // 总耗时（毫秒）
    double averageTimeMs = 0.0;    // 平均耗时（毫秒）
    double operationsPerSecond = 0.0; // 每秒操作数
    double successRate = 0.0;      // 成功率
    QVariantMap additionalMetrics; // 附加指标
    
    // JSON 序列化支持
    QJsonObject toJson() const;
    static BenchmarkResult fromJson(const QJsonObject &json);
};
```

### 基准测试配置
```cpp
struct BenchmarkConfig {
    int iterations = 100;              // 迭代次数
    int concurrentThreads = 1;         // 并发线程数
    bool enableCaching = true;         // 启用缓存
    bool enableConnectionPooling = true; // 启用连接池
    int warmupIterations = 10;         // 预热迭代次数
    int registerStartAddress = 0;      // 寄存器起始地址
    int registerCount = 10;            // 寄存器数量
    int delayBetweenOperationsMs = 0;  // 操作间延迟（毫秒）
    bool randomizeAddresses = false;   // 随机化地址
    
    // JSON 序列化支持
    QJsonObject toJson() const;
    static BenchmarkConfig fromJson(const QJsonObject &json);
};
```

## 核心类：ModbusBenchmark

### 构造和配置
```cpp
// 创建基准测试对象
ModbusBenchmark *benchmark = new ModbusBenchmark(this);

// 设置配置
BenchmarkConfig config;
config.iterations = 1000;
config.concurrentThreads = 4;
config.enableCaching = true;
benchmark->setConfig(config);

// 设置管理器
benchmark->setOriginalManager(originalManager);
benchmark->setOptimizedManager(optimizedManager);
```

### 基准测试方法
```cpp
// 读操作基准测试
BenchmarkResult benchmarkReadOperations(const QString &testName = "Read Operations");

// 写操作基准测试
BenchmarkResult benchmarkWriteOperations(const QString &testName = "Write Operations");

// 混合操作基准测试
BenchmarkResult benchmarkMixedOperations(const QString &testName = "Mixed Operations");

// 缓存效果测试
BenchmarkResult benchmarkCacheEffectiveness(const QString &testName = "Cache Effectiveness");

// 连接池测试
BenchmarkResult benchmarkConnectionPooling(const QString &testName = "Connection Pooling");

// 异步操作测试
BenchmarkResult benchmarkAsyncOperations(const QString &testName = "Async Operations");

// 管理器对比测试
BenchmarkResult compareManagers(const QString &operation, const QString &testName = "Manager Comparison");

// 完整测试套件
QVector<BenchmarkResult> runFullBenchmarkSuite();
```

### 报告生成
```cpp
// 生成报告
QString generateReport(const QVector<BenchmarkResult> &results, const QString &format = "text");

// 保存结果
void saveResults(const QVector<BenchmarkResult> &results, const QString &filename);

// 加载结果
QVector<BenchmarkResult> loadResults(const QString &filename);
```

## 使用示例

### 基本性能测试
```cpp
#include "modbus_benchmark.h"
#include "modbusmanager.h"
#include "optimized_modbus_manager.h"

// 创建基准测试
ModbusBenchmark *benchmark = new ModbusBenchmark(this);

// 创建管理器
ModbusManager *originalManager = new ModbusManager(this);
OptimizedModbusManager *optimizedManager = new OptimizedModbusManager(this);

// 连接到设备
originalManager->connectRTU("COM1", 9600, 8, 'N', 1);
optimizedManager->connectRTU("COM1", 9600, 8, 'N', 1);

// 设置管理器
benchmark->setOriginalManager(originalManager);
benchmark->setOptimizedManager(optimizedManager);

// 配置测试参数
BenchmarkConfig config;
config.iterations = 500;
config.concurrentThreads = 2;
config.registerCount = 20;
benchmark->setConfig(config);

// 执行读操作测试
BenchmarkResult readResult = benchmark->benchmarkReadOperations("RTU Read Test");
qDebug() << QString("读操作性能: %1 OPS, 成功率: %2%")
            .arg(readResult.operationsPerSecond, 0, 'f', 2)
            .arg(readResult.successRate, 0, 'f', 1);
```

### 完整测试套件
```cpp
// 监听测试进度
connect(benchmark, &ModbusBenchmark::benchmarkProgress, this,
        [](int current, int total, const QString &currentTest) {
    qDebug() << QString("进度: %1/%2, 当前测试: %3")
                .arg(current).arg(total).arg(currentTest);
});

connect(benchmark, &ModbusBenchmark::benchmarkCompleted, this,
        [](const BenchmarkResult &result) {
    qDebug() << QString("测试完成: %1, OPS: %2")
                .arg(result.testName).arg(result.operationsPerSecond);
});

// 运行完整测试套件
QVector<BenchmarkResult> results = benchmark->runFullBenchmarkSuite();

// 生成文本报告
QString textReport = benchmark->generateReport(results, "text");
qDebug() << "性能报告:\n" << textReport;

// 生成 HTML 报告
QString htmlReport = benchmark->generateReport(results, "html");

// 保存结果到文件
benchmark->saveResults(results, "benchmark_results.json");
```

### 管理器对比测试
```cpp
// 对比读操作性能
BenchmarkResult comparison = benchmark->compareManagers("read", "Read Performance Comparison");

qDebug() << QString("对比结果 - %1:").arg(comparison.testName);
qDebug() << QString("  总操作数: %1").arg(comparison.totalOperations);
qDebug() << QString("  成功率: %1%").arg(comparison.successRate, 0, 'f', 1);
qDebug() << QString("  平均耗时: %1ms").arg(comparison.averageTimeMs, 0, 'f', 2);
qDebug() << QString("  OPS: %1").arg(comparison.operationsPerSecond, 0, 'f', 2);

// 检查附加指标
if (comparison.additionalMetrics.contains("performance_improvement")) {
    double improvement = comparison.additionalMetrics["performance_improvement"].toDouble();
    qDebug() << QString("  性能提升: %1%").arg(improvement, 0, 'f', 1);
}
```

### 缓存效果测试
```cpp
// 测试缓存对性能的影响
BenchmarkConfig cacheConfig;
cacheConfig.iterations = 200;
cacheConfig.enableCaching = true;
cacheConfig.randomizeAddresses = false; // 使用相同地址以测试缓存效果
benchmark->setConfig(cacheConfig);

BenchmarkResult cacheResult = benchmark->benchmarkCacheEffectiveness("Cache Test");

qDebug() << QString("缓存测试结果:");
qDebug() << QString("  缓存命中率: %1%")
            .arg(cacheResult.additionalMetrics["cache_hit_rate"].toDouble(), 0, 'f', 1);
qDebug() << QString("  性能提升: %1%")
            .arg(cacheResult.additionalMetrics["cache_performance_gain"].toDouble(), 0, 'f', 1);
```

### 并发性能测试
```cpp
// 测试不同并发级别下的性能
QVector<int> threadCounts = {1, 2, 4, 8, 16};
QVector<BenchmarkResult> concurrencyResults;

for (int threads : threadCounts) {
    BenchmarkConfig concurrencyConfig;
    concurrencyConfig.iterations = 100;
    concurrencyConfig.concurrentThreads = threads;
    benchmark->setConfig(concurrencyConfig);
    
    BenchmarkResult result = benchmark->benchmarkReadOperations(
        QString("Concurrency Test - %1 threads").arg(threads));
    
    concurrencyResults.append(result);
    
    qDebug() << QString("%1 线程: %2 OPS")
                .arg(threads).arg(result.operationsPerSecond, 0, 'f', 2);
}

// 保存并发测试结果
benchmark->saveResults(concurrencyResults, "concurrency_test.json");
```

### 自定义测试配置
```cpp
// 创建自定义配置
BenchmarkConfig customConfig;
customConfig.iterations = 1000;
customConfig.concurrentThreads = 1;
customConfig.enableCaching = false;
customConfig.enableConnectionPooling = false;
customConfig.warmupIterations = 50;
customConfig.registerStartAddress = 100;
customConfig.registerCount = 50;
customConfig.delayBetweenOperationsMs = 10;
customConfig.randomizeAddresses = true;

benchmark->setConfig(customConfig);

// 执行自定义测试
BenchmarkResult customResult = benchmark->benchmarkMixedOperations("Custom Mixed Test");
```

## 信号（Signals）

```cpp
// 测试进度信号
void benchmarkProgress(int current, int total, const QString &currentTest);

// 测试完成信号
void benchmarkCompleted(const BenchmarkResult &result);
```

## 报告格式

### 文本报告示例
```
=== Modbus Performance Benchmark Report ===
Generated: 2025-06-07 10:30:45

Test: Read Operations
- Total Operations: 1000
- Successful Operations: 995
- Success Rate: 99.5%
- Total Time: 2543ms
- Average Time: 2.55ms
- Operations Per Second: 391.26
- Additional Metrics:
  * Cache Hit Rate: 85.2%
  * Memory Usage: 12.3MB
```

### JSON 报告示例
```json
{
  "testName": "Read Operations",
  "totalOperations": 1000,
  "successfulOperations": 995,
  "failedOperations": 5,
  "totalTimeMs": 2543,
  "averageTimeMs": 2.55,
  "operationsPerSecond": 391.26,
  "successRate": 99.5,
  "additionalMetrics": {
    "cache_hit_rate": 85.2,
    "memory_usage_mb": 12.3,
    "cpu_usage_percent": 15.7
  }
}
```

## 性能优化建议

### 基于测试结果的优化
```cpp
// 分析测试结果并给出建议
void analyzeResults(const BenchmarkResult &result) {
    if (result.successRate < 95.0) {
        qDebug() << "建议: 检查网络连接稳定性";
    }
    
    if (result.averageTimeMs > 100.0) {
        qDebug() << "建议: 考虑启用连接池或缓存";
    }
    
    if (result.additionalMetrics.contains("cache_hit_rate")) {
        double hitRate = result.additionalMetrics["cache_hit_rate"].toDouble();
        if (hitRate < 50.0) {
            qDebug() << "建议: 优化缓存策略";
        }
    }
}
```

## 最佳实践

### 1. 测试环境准备
- 确保网络环境稳定
- 关闭不必要的后台程序
- 使用专用测试设备

### 2. 配置选择
- 预热测试避免冷启动影响
- 适当的迭代次数确保结果可靠性
- 根据实际场景调整并发数

### 3. 结果分析
- 关注平均值和标准差
- 对比不同配置下的性能差异
- 结合实际业务需求评估结果

## 相关文档

- [Modbus 管理器文档](modbusmanager.md)
- [优化 Modbus 管理器文档](optimized_modbus_manager.md)
- [Modbus 性能文档](modbus_performance.md)
- [Modbus 核心文档](modbus.md)
