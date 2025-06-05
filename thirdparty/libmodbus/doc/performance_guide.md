# 🚀 libmodbus 性能优化指南

> 提升 Modbus 通信性能的最佳实践

![Performance](https://img.shields.io/badge/Level-Advanced-red.svg) ![Optimization](https://img.shields.io/badge/Type-Optimization-green.svg)

---

## 📋 目录

1. [性能基准测试](#-性能基准测试)
2. [连接优化](#-连接优化)
3. [数据传输优化](#-数据传输优化)
4. [内存管理](#-内存管理)
5. [多线程优化](#-多线程优化)
6. [网络优化](#-网络优化)
7. [监控与分析](#-监控与分析)

---

## 📊 性能基准测试

### 测试框架

```cpp
class ModbusPerformanceTester {
private:
    modbus_t* ctx;
    QElapsedTimer timer;
    
public:
    struct TestResult {
        int totalOperations;
        qint64 totalTime;
        double averageTime;
        double operationsPerSecond;
        int errorCount;
    };
    
    TestResult testReadPerformance(int address, int count, int iterations) {
        TestResult result = {};
        uint16_t* registers = new uint16_t[count];
        
        timer.start();
        
        for (int i = 0; i < iterations; i++) {
            if (modbus_read_registers(ctx, address, count, registers) == -1) {
                result.errorCount++;
            }
            result.totalOperations++;
        }
        
        result.totalTime = timer.elapsed();
        result.averageTime = static_cast<double>(result.totalTime) / iterations;
        result.operationsPerSecond = 1000.0 * iterations / result.totalTime;
        
        delete[] registers;
        return result;
    }
    
    void printResults(const TestResult& result) {
        qDebug() << "=== 性能测试结果 ===";
        qDebug() << "总操作数:" << result.totalOperations;
        qDebug() << "总耗时:" << result.totalTime << "ms";
        qDebug() << "平均耗时:" << result.averageTime << "ms";
        qDebug() << "每秒操作数:" << result.operationsPerSecond;
        qDebug() << "错误率:" << (100.0 * result.errorCount / result.totalOperations) << "%";
    }
};
```

### 基准测试示例

```cpp
void runPerformanceTests() {
    ModbusPerformanceTester tester;
    
    // 测试不同数据块大小的性能
    QVector<int> blockSizes = {1, 10, 50, 100, 125};
    
    for (int blockSize : blockSizes) {
        auto result = tester.testReadPerformance(0, blockSize, 1000);
        qDebug() << QString("块大小 %1:").arg(blockSize);
        tester.printResults(result);
    }
}
```

---

## 🔗 连接优化

### 连接池管理

```cpp
class ModbusConnectionPool {
private:
    QQueue<modbus_t*> availableConnections;
    QSet<modbus_t*> busyConnections;
    QMutex poolMutex;
    QString host;
    int port;
    int maxConnections;
    
public:
    ModbusConnectionPool(const QString& host, int port, int maxConn = 10)
        : host(host), port(port), maxConnections(maxConn) {
        // 预创建连接
        for (int i = 0; i < maxConnections / 2; i++) {
            modbus_t* ctx = createConnection();
            if (ctx) {
                availableConnections.enqueue(ctx);
            }
        }
    }
    
    modbus_t* acquireConnection() {
        QMutexLocker locker(&poolMutex);
        
        if (!availableConnections.isEmpty()) {
            modbus_t* ctx = availableConnections.dequeue();
            busyConnections.insert(ctx);
            return ctx;
        }
        
        // 如果没有可用连接且未达到最大限制，创建新连接
        if (busyConnections.size() < maxConnections) {
            modbus_t* ctx = createConnection();
            if (ctx) {
                busyConnections.insert(ctx);
                return ctx;
            }
        }
        
        return nullptr;  // 连接池已满
    }
    
    void releaseConnection(modbus_t* ctx) {
        QMutexLocker locker(&poolMutex);
        
        if (busyConnections.remove(ctx)) {
            // 检查连接是否仍然有效
            if (isConnectionValid(ctx)) {
                availableConnections.enqueue(ctx);
            } else {
                // 连接已断开，创建新连接替换
                modbus_close(ctx);
                modbus_free(ctx);
                
                modbus_t* newCtx = createConnection();
                if (newCtx) {
                    availableConnections.enqueue(newCtx);
                }
            }
        }
    }
    
private:
    modbus_t* createConnection() {
        modbus_t* ctx = modbus_new_tcp(host.toLocal8Bit().data(), port);
        if (!ctx) return nullptr;
        
        // 优化设置
        modbus_set_response_timeout(ctx, 1, 0);  // 1秒超时
        modbus_set_byte_timeout(ctx, 0, 500000); // 500ms字节超时
        
        if (modbus_connect(ctx) == -1) {
            modbus_free(ctx);
            return nullptr;
        }
        
        return ctx;
    }
    
    bool isConnectionValid(modbus_t* ctx) {
        uint16_t test_reg;
        return modbus_read_registers(ctx, 0, 1, &test_reg) != -1;
    }
};
```

### 连接保活机制

```cpp
class ConnectionKeepAlive : public QObject {
    Q_OBJECT
    
private:
    modbus_t* ctx;
    QTimer* keepAliveTimer;
    int keepAliveInterval;  // 秒
    
public:
    ConnectionKeepAlive(modbus_t* context, int interval = 30)
        : ctx(context), keepAliveInterval(interval) {
        
        keepAliveTimer = new QTimer(this);
        connect(keepAliveTimer, &QTimer::timeout, this, &ConnectionKeepAlive::sendKeepAlive);
        keepAliveTimer->start(keepAliveInterval * 1000);
    }
    
private slots:
    void sendKeepAlive() {
        uint16_t dummy;
        if (modbus_read_registers(ctx, 0, 1, &dummy) == -1) {
            emit connectionLost();
        }
    }
    
signals:
    void connectionLost();
};
```

---

## 📈 数据传输优化

### 批量操作

```cpp
class BatchOperations {
public:
    // 批量读取优化
    struct ReadRequest {
        int address;
        int count;
        uint16_t* buffer;
    };
    
    bool batchRead(modbus_t* ctx, const QVector<ReadRequest>& requests) {
        // 合并连续地址的读取请求
        auto optimizedRequests = optimizeReadRequests(requests);
        
        for (const auto& req : optimizedRequests) {
            if (modbus_read_registers(ctx, req.address, req.count, req.buffer) == -1) {
                return false;
            }
        }
        return true;
    }
    
private:
    QVector<ReadRequest> optimizeReadRequests(const QVector<ReadRequest>& requests) {
        QVector<ReadRequest> optimized;
        
        // 按地址排序
        QVector<ReadRequest> sorted = requests;
        std::sort(sorted.begin(), sorted.end(), 
                 [](const ReadRequest& a, const ReadRequest& b) {
                     return a.address < b.address;
                 });
        
        // 合并连续地址
        for (const auto& req : sorted) {
            if (optimized.isEmpty()) {
                optimized.append(req);
                continue;
            }
            
            auto& last = optimized.last();
            if (last.address + last.count == req.address) {
                // 连续地址，合并
                last.count += req.count;
            } else {
                optimized.append(req);
            }
        }
        
        return optimized;
    }
};
```

### 数据压缩

```cpp
class DataCompressor {
public:
    // 位数据压缩
    QByteArray compressBitData(const QVector<bool>& bits) {
        QByteArray compressed;
        uint8_t byte = 0;
        int bitPos = 0;
        
        for (bool bit : bits) {
            if (bit) {
                byte |= (1 << bitPos);
            }
            
            if (++bitPos == 8) {
                compressed.append(byte);
                byte = 0;
                bitPos = 0;
            }
        }
        
        if (bitPos > 0) {
            compressed.append(byte);
        }
        
        return compressed;
    }
    
    // 重复数据压缩 (RLE)
    QByteArray compressRegisters(const QVector<uint16_t>& registers) {
        QByteArray compressed;
        QDataStream stream(&compressed, QIODevice::WriteOnly);
        
        if (registers.isEmpty()) return compressed;
        
        uint16_t current = registers[0];
        int count = 1;
        
        for (int i = 1; i < registers.size(); i++) {
            if (registers[i] == current && count < 255) {
                count++;
            } else {
                stream << current << static_cast<uint8_t>(count);
                current = registers[i];
                count = 1;
            }
        }
        
        stream << current << static_cast<uint8_t>(count);
        return compressed;
    }
};
```

---

## 💾 内存管理

### 缓冲池

```cpp
template<typename T>
class BufferPool {
private:
    QQueue<QVector<T>*> availableBuffers;
    QMutex poolMutex;
    int bufferSize;
    int maxBuffers;
    
public:
    BufferPool(int size, int maxCount = 100) 
        : bufferSize(size), maxBuffers(maxCount) {}
    
    QVector<T>* acquire() {
        QMutexLocker locker(&poolMutex);
        
        if (!availableBuffers.isEmpty()) {
            return availableBuffers.dequeue();
        }
        
        return new QVector<T>(bufferSize);
    }
    
    void release(QVector<T>* buffer) {
        QMutexLocker locker(&poolMutex);
        
        if (availableBuffers.size() < maxBuffers) {
            buffer->fill(T{});  // 清零
            availableBuffers.enqueue(buffer);
        } else {
            delete buffer;
        }
    }
};

// 全局缓冲池
BufferPool<uint16_t> g_registerBufferPool(125);  // Modbus最大寄存器数
BufferPool<uint8_t> g_bitBufferPool(2000);       // 位缓冲池
```

### 内存预分配

```cpp
class PreallocatedModbusClient {
private:
    modbus_t* ctx;
    uint16_t* registerBuffer;
    uint8_t* bitBuffer;
    int maxRegisters;
    int maxBits;
    
public:
    PreallocatedModbusClient(int maxRegs = 125, int maxBits = 2000) 
        : maxRegisters(maxRegs), maxBits(maxBits) {
        
        registerBuffer = new uint16_t[maxRegisters];
        bitBuffer = new uint8_t[maxBits];
    }
    
    ~PreallocatedModbusClient() {
        delete[] registerBuffer;
        delete[] bitBuffer;
    }
    
    bool readRegisters(int address, int count, uint16_t*& result) {
        if (count > maxRegisters) return false;
        
        if (modbus_read_registers(ctx, address, count, registerBuffer) == -1) {
            return false;
        }
        
        result = registerBuffer;
        return true;
    }
};
```

---

## ⚡ 多线程优化

### 工作线程池

```cpp
class ModbusWorkerPool : public QObject {
    Q_OBJECT
    
private:
    QThreadPool* threadPool;
    ModbusConnectionPool* connectionPool;
    
public:
    ModbusWorkerPool(ModbusConnectionPool* pool) 
        : connectionPool(pool) {
        threadPool = new QThreadPool(this);
        threadPool->setMaxThreadCount(QThread::idealThreadCount());
    }
    
    QFuture<uint16_t*> readRegistersAsync(int address, int count) {
        return QtConcurrent::run(threadPool, [=]() -> uint16_t* {
            modbus_t* ctx = connectionPool->acquireConnection();
            if (!ctx) return nullptr;
            
            uint16_t* result = new uint16_t[count];
            bool success = (modbus_read_registers(ctx, address, count, result) != -1);
            
            connectionPool->releaseConnection(ctx);
            
            return success ? result : nullptr;
        });
    }
    
    QFuture<bool> writeRegistersAsync(int address, const QVector<uint16_t>& values) {
        return QtConcurrent::run(threadPool, [=]() -> bool {
            modbus_t* ctx = connectionPool->acquireConnection();
            if (!ctx) return false;
            
            bool success = (modbus_write_registers(ctx, address, values.size(), 
                                                 values.constData()) != -1);
            
            connectionPool->releaseConnection(ctx);
            return success;
        });
    }
};
```

### 无锁队列

```cpp
template<typename T>
class LockFreeQueue {
private:
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};
    std::vector<std::atomic<T*>> buffer;
    size_t mask;
    
public:
    LockFreeQueue(size_t capacity) {
        // 确保容量是2的幂
        size_t size = 1;
        while (size < capacity) size <<= 1;
        
        buffer.resize(size);
        mask = size - 1;
        
        for (auto& slot : buffer) {
            slot.store(nullptr);
        }
    }
    
    bool enqueue(T* item) {
        size_t current_tail = tail.load();
        size_t next_tail = (current_tail + 1) & mask;
        
        if (next_tail == head.load()) {
            return false;  // 队列满
        }
        
        buffer[current_tail].store(item);
        tail.store(next_tail);
        return true;
    }
    
    T* dequeue() {
        size_t current_head = head.load();
        
        if (current_head == tail.load()) {
            return nullptr;  // 队列空
        }
        
        T* item = buffer[current_head].load();
        buffer[current_head].store(nullptr);
        head.store((current_head + 1) & mask);
        
        return item;
    }
};
```

---

## 🌐 网络优化

### TCP 优化

```cpp
void optimizeTcpSocket(modbus_t* ctx) {
    int socket_fd = modbus_get_socket(ctx);
    
    // 禁用 Nagle 算法
    int flag = 1;
    setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, 
               reinterpret_cast<char*>(&flag), sizeof(flag));
    
    // 设置接收缓冲区
    int recv_buffer = 64 * 1024;  // 64KB
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF,
               reinterpret_cast<char*>(&recv_buffer), sizeof(recv_buffer));
    
    // 设置发送缓冲区
    int send_buffer = 64 * 1024;  // 64KB
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF,
               reinterpret_cast<char*>(&send_buffer), sizeof(send_buffer));
    
    // 启用 Keep-Alive
    int keepalive = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE,
               reinterpret_cast<char*>(&keepalive), sizeof(keepalive));
}
```

### 数据预取

```cpp
class DataPrefetcher : public QObject {
    Q_OBJECT
    
private:
    modbus_t* ctx;
    QTimer* prefetchTimer;
    QSet<int> hotAddresses;  // 热点地址
    QMap<int, uint16_t> cache;  // 缓存
    
public:
    DataPrefetcher(modbus_t* context) : ctx(context) {
        prefetchTimer = new QTimer(this);
        connect(prefetchTimer, &QTimer::timeout, this, &DataPrefetcher::prefetchData);
        prefetchTimer->start(100);  // 100ms间隔预取
    }
    
    void addHotAddress(int address) {
        hotAddresses.insert(address);
    }
    
    bool getCachedValue(int address, uint16_t& value) {
        if (cache.contains(address)) {
            value = cache[address];
            return true;
        }
        return false;
    }
    
private slots:
    void prefetchData() {
        for (int address : hotAddresses) {
            uint16_t value;
            if (modbus_read_registers(ctx, address, 1, &value) != -1) {
                cache[address] = value;
            }
        }
    }
};
```

---

## 📊 监控与分析

### 性能监控器

```cpp
class PerformanceMonitor : public QObject {
    Q_OBJECT
    
private:
    struct Metrics {
        int totalRequests = 0;
        int successfulRequests = 0;
        qint64 totalResponseTime = 0;
        qint64 minResponseTime = LLONG_MAX;
        qint64 maxResponseTime = 0;
        QMap<int, int> errorCounts;
    };
    
    Metrics currentMetrics;
    QTimer* reportTimer;
    
public:
    PerformanceMonitor() {
        reportTimer = new QTimer(this);
        connect(reportTimer, &QTimer::timeout, this, &PerformanceMonitor::generateReport);
        reportTimer->start(60000);  // 每分钟生成报告
    }
    
    void recordRequest(qint64 responseTime, bool success, int errorCode = 0) {
        currentMetrics.totalRequests++;
        
        if (success) {
            currentMetrics.successfulRequests++;
            currentMetrics.totalResponseTime += responseTime;
            currentMetrics.minResponseTime = qMin(currentMetrics.minResponseTime, responseTime);
            currentMetrics.maxResponseTime = qMax(currentMetrics.maxResponseTime, responseTime);
        } else {
            currentMetrics.errorCounts[errorCode]++;
        }
    }
    
private slots:
    void generateReport() {
        qDebug() << "=== 性能报告 ===";
        qDebug() << "总请求数:" << currentMetrics.totalRequests;
        qDebug() << "成功率:" << (100.0 * currentMetrics.successfulRequests / currentMetrics.totalRequests) << "%";
        
        if (currentMetrics.successfulRequests > 0) {
            qDebug() << "平均响应时间:" << (currentMetrics.totalResponseTime / currentMetrics.successfulRequests) << "ms";
            qDebug() << "最小响应时间:" << currentMetrics.minResponseTime << "ms";
            qDebug() << "最大响应时间:" << currentMetrics.maxResponseTime << "ms";
        }
        
        // 重置指标
        currentMetrics = Metrics{};
    }
};
```

### 自适应优化

```cpp
class AdaptiveOptimizer {
private:
    modbus_t* ctx;
    int currentTimeout = 1000;  // 当前超时时间(ms)
    double successRate = 1.0;
    int sampleCount = 0;
    
public:
    void adjustTimeout() {
        if (sampleCount < 100) return;  // 样本不足
        
        if (successRate > 0.95) {
            // 成功率高，可以减少超时时间
            currentTimeout = qMax(100, static_cast<int>(currentTimeout * 0.9));
        } else if (successRate < 0.8) {
            // 成功率低，增加超时时间
            currentTimeout = qMin(5000, static_cast<int>(currentTimeout * 1.2));
        }
        
        modbus_set_response_timeout(ctx, currentTimeout / 1000, 
                                   (currentTimeout % 1000) * 1000);
        
        qDebug() << "调整超时时间为:" << currentTimeout << "ms";
    }
    
    void recordResult(bool success) {
        sampleCount++;
        successRate = (successRate * (sampleCount - 1) + (success ? 1.0 : 0.0)) / sampleCount;
        
        if (sampleCount % 50 == 0) {
            adjustTimeout();
        }
    }
};
```

---

## 📈 性能基准

### 理论性能限制

| 协议 | 理论最大吞吐量 | 实际典型性能 |
|------|----------------|--------------|
| Modbus TCP (100Mbps) | ~12,500 包/秒 | ~5,000 包/秒 |
| Modbus RTU (115200) | ~200 包/秒 | ~100 包/秒 |
| Modbus RTU (9600) | ~20 包/秒 | ~15 包/秒 |

### 优化前后对比

```cpp
void performanceComparison() {
    qDebug() << "=== 优化效果对比 ===";
    
    // 优化前: 单连接，逐个读取
    auto before = testSingleConnection();
    
    // 优化后: 连接池，批量读取
    auto after = testOptimizedConnection();
    
    qDebug() << "优化前性能:" << before.operationsPerSecond << " ops/sec";
    qDebug() << "优化后性能:" << after.operationsPerSecond << " ops/sec";
    qDebug() << "性能提升:" << (after.operationsPerSecond / before.operationsPerSecond) << "倍";
}
```

---

## 💡 最佳实践总结

1. **连接管理**: 使用连接池减少连接开销
2. **批量操作**: 合并连续地址的读写请求
3. **异步处理**: 使用多线程避免阻塞
4. **缓存策略**: 缓存频繁访问的数据
5. **自适应调优**: 根据网络状况动态调整参数
6. **监控分析**: 持续监控性能指标

---

📝 **文档版本**: v1.0  
📅 **最后更新**: 2025年6月5日  
👨‍💻 **维护者**: MyOperation 开发团队
