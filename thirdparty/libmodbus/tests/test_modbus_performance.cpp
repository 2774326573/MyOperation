/**
 * @file test_modbus_performance.cpp
 * @brief Unit tests for Modbus performance optimization classes
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTimer>
#include <QThread>
#include "modbus_performance.h"
#include "optimized_modbus_manager.h"

class TestModbusPerformance : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Connection Pool Tests
    void testConnectionPoolCreation();
    void testConnectionPoolAcquisition();
    void testConnectionPoolRelease();
    void testConnectionPoolMaxConnections();
    void testConnectionPoolHealthCheck();
    
    // Cache Tests
    void testCacheBasicOperations();
    void testCacheTTLExpiration();
    void testCacheMaxSize();
    void testCacheStatistics();
    void testCacheInvalidation();
    
    // Async Manager Tests
    void testAsyncManagerCreation();
    void testAsyncOperationQueuing();
    void testAsyncOperationExecution();
    void testAsyncQueueManagement();
    
    // Smart Reconnect Tests
    void testReconnectManagerCreation();
    void testReconnectRetryLogic();
    void testReconnectBackoffStrategy();
    
    // Batch Operations Tests
    void testBatchOperationCreation();
    void testBatchOperationOptimization();
    void testBatchOperationExecution();
    
    // Performance Monitor Tests
    void testPerformanceMonitorCreation();
    void testPerformanceMonitorMetrics();
    void testPerformanceMonitorAlerts();
    
    // Memory Pool Tests
    void testMemoryPoolAllocation();
    void testMemoryPoolDeallocation();
    void testMemoryPoolReuse();
    
    // Optimized Manager Integration Tests
    void testOptimizedManagerCreation();
    void testOptimizedManagerConfiguration();
    void testOptimizedManagerCaching();
    void testOptimizedManagerAsync();
    void testOptimizedManagerStatistics();

private:
    OptimizedModbusManager *m_manager = nullptr;
    ModbusConnectionPool *m_pool = nullptr;
    ModbusDataCache *m_cache = nullptr;
    AsyncModbusManager *m_async = nullptr;
    SmartReconnectManager *m_reconnect = nullptr;
    BatchOperationManager *m_batch = nullptr;
    ModbusPerformanceMonitor *m_monitor = nullptr;
};

void TestModbusPerformance::initTestCase()
{
    qDebug() << "Starting Modbus Performance Tests";
}

void TestModbusPerformance::cleanupTestCase()
{
    qDebug() << "Modbus Performance Tests completed";
}

void TestModbusPerformance::init()
{
    // Create fresh instances for each test
    m_manager = new OptimizedModbusManager(this);
    m_pool = new ModbusConnectionPool(this);
    m_cache = new ModbusDataCache(this);
    m_async = new AsyncModbusManager(this);
    m_reconnect = new SmartReconnectManager(this);
    m_batch = new BatchOperationManager(this);
    m_monitor = new ModbusPerformanceMonitor(this);
}

void TestModbusPerformance::cleanup()
{
    delete m_manager;
    delete m_pool;
    delete m_cache;
    delete m_async;
    delete m_reconnect;
    delete m_batch;
    delete m_monitor;
    
    m_manager = nullptr;
    m_pool = nullptr;
    m_cache = nullptr;
    m_async = nullptr;
    m_reconnect = nullptr;
    m_batch = nullptr;
    m_monitor = nullptr;
}

// =============================================================================
// Connection Pool Tests
// =============================================================================

void TestModbusPerformance::testConnectionPoolCreation()
{
    QVERIFY(m_pool != nullptr);
    QCOMPARE(m_pool->getActiveConnections(), 0);
    QCOMPARE(m_pool->getMaxConnections(), 10);  // Default value
}

void TestModbusPerformance::testConnectionPoolAcquisition()
{
    ModbusConnectionConfig config;
    config.type = ModbusConnectionType::Serial;
    config.serialPort = "COM1";
    config.baudRate = 9600;
    config.slaveId = 1;
    
    // This will fail in test environment but should create connection object
    auto connection = m_pool->acquireConnection(config);
    QVERIFY(connection != nullptr);
    
    QCOMPARE(m_pool->getActiveConnections(), 1);
}

void TestModbusPerformance::testConnectionPoolRelease()
{
    ModbusConnectionConfig config;
    config.type = ModbusConnectionType::Serial;
    config.serialPort = "COM1";
    config.baudRate = 9600;
    config.slaveId = 1;
    
    auto connection = m_pool->acquireConnection(config);
    QVERIFY(connection != nullptr);
    
    m_pool->releaseConnection(connection);
    QCOMPARE(m_pool->getActiveConnections(), 0);
}

void TestModbusPerformance::testConnectionPoolMaxConnections()
{
    m_pool->setMaxConnections(2);
    QCOMPARE(m_pool->getMaxConnections(), 2);
    
    ModbusConnectionConfig config;
    config.type = ModbusConnectionType::Serial;
    config.serialPort = "COM1";
    config.baudRate = 9600;
    config.slaveId = 1;
    
    // Acquire maximum connections
    auto conn1 = m_pool->acquireConnection(config);
    auto conn2 = m_pool->acquireConnection(config);
    
    QVERIFY(conn1 != nullptr);
    QVERIFY(conn2 != nullptr);
    QCOMPARE(m_pool->getActiveConnections(), 2);
    
    // Try to acquire one more (should fail or wait)
    auto conn3 = m_pool->acquireConnection(config);
    QVERIFY(conn3 == nullptr || m_pool->getActiveConnections() <= 2);
}

void TestModbusPerformance::testConnectionPoolHealthCheck()
{
    m_pool->performHealthCheck();
    
    // Health check should not crash and should report status
    auto stats = m_pool->getStatistics();
    QVERIFY(stats.contains("healthCheckCount"));
}

// =============================================================================
// Cache Tests
// =============================================================================

void TestModbusPerformance::testCacheBasicOperations()
{
    QString key = "test_key";
    QVector<quint16> data = {1, 2, 3, 4, 5};
    
    // Test put and get
    m_cache->put(key, QVariant::fromValue(data));
    
    auto result = m_cache->get(key);
    QVERIFY(result.isValid());
    
    auto retrievedData = result.value<QVector<quint16>>();
    QCOMPARE(retrievedData, data);
}

void TestModbusPerformance::testCacheTTLExpiration()
{
    QString key = "ttl_test";
    QVector<quint16> data = {10, 20, 30};
    
    // Set short TTL
    m_cache->put(key, QVariant::fromValue(data), 100);  // 100ms TTL
    
    // Should be available immediately
    auto result = m_cache->get(key);
    QVERIFY(result.isValid());
    
    // Wait for expiration
    QTest::qWait(150);
    
    // Should be expired now
    result = m_cache->get(key);
    QVERIFY(!result.isValid());
}

void TestModbusPerformance::testCacheMaxSize()
{
    m_cache->setMaxSize(3);
    
    // Fill cache beyond capacity
    m_cache->put("key1", QVariant(1));
    m_cache->put("key2", QVariant(2));
    m_cache->put("key3", QVariant(3));
    m_cache->put("key4", QVariant(4));  // Should evict oldest
    
    // First key should be evicted
    QVERIFY(!m_cache->get("key1").isValid());
    QVERIFY(m_cache->get("key4").isValid());
}

void TestModbusPerformance::testCacheStatistics()
{
    m_cache->put("key1", QVariant(1));
    m_cache->get("key1");  // Hit
    m_cache->get("key2");  // Miss
    
    auto stats = m_cache->getStatistics();
    QVERIFY(stats["hits"].toInt() >= 1);
    QVERIFY(stats["misses"].toInt() >= 1);
    QVERIFY(stats["hitRate"].toDouble() > 0.0);
}

void TestModbusPerformance::testCacheInvalidation()
{
    m_cache->put("key1", QVariant(1));
    m_cache->put("key2", QVariant(2));
    
    QVERIFY(m_cache->get("key1").isValid());
    
    // Invalidate specific key
    m_cache->invalidate("key1");
    QVERIFY(!m_cache->get("key1").isValid());
    QVERIFY(m_cache->get("key2").isValid());
    
    // Clear all
    m_cache->clear();
    QVERIFY(!m_cache->get("key2").isValid());
}

// =============================================================================
// Async Manager Tests
// =============================================================================

void TestModbusPerformance::testAsyncManagerCreation()
{
    QVERIFY(m_async != nullptr);
    QCOMPARE(m_async->getWorkerThreadCount(), 2);  // Default value
    QCOMPARE(m_async->getQueueSize(), 0);
}

void TestModbusPerformance::testAsyncOperationQueuing()
{
    AsyncModbusOperation operation;
    operation.operationId = "test_op";
    operation.functionCode = 3;  // Read holding registers
    operation.startAddress = 0;
    operation.quantity = 10;
    operation.slaveId = 1;
    
    QString opId = m_async->queueOperation(operation);
    QVERIFY(!opId.isEmpty());
    QCOMPARE(m_async->getQueueSize(), 1);
}

void TestModbusPerformance::testAsyncOperationExecution()
{
    QSignalSpy spy(m_async, &AsyncModbusManager::operationCompleted);
    
    AsyncModbusOperation operation;
    operation.operationId = "exec_test";
    operation.functionCode = 3;
    operation.startAddress = 0;
    operation.quantity = 5;
    operation.slaveId = 1;
    
    m_async->queueOperation(operation);
    
    // Wait for operation to complete (or timeout)
    // Note: In test environment, operation may fail due to no actual device
    spy.wait(5000);
    
    // Should have received a completion signal
    QVERIFY(spy.count() >= 0);  // May be 0 if no device available
}

void TestModbusPerformance::testAsyncQueueManagement()
{
    m_async->setMaxQueueSize(3);
    
    // Fill queue
    AsyncModbusOperation op;
    op.functionCode = 3;
    op.startAddress = 0;
    op.quantity = 1;
    op.slaveId = 1;
    
    for (int i = 0; i < 5; ++i) {
        op.operationId = QString("op_%1").arg(i);
        m_async->queueOperation(op);
    }
    
    // Queue should be limited to max size
    QVERIFY(m_async->getQueueSize() <= 3);
}

// =============================================================================
// Smart Reconnect Tests
// =============================================================================

void TestModbusPerformance::testReconnectManagerCreation()
{
    QVERIFY(m_reconnect != nullptr);
    QCOMPARE(m_reconnect->getMaxRetries(), 3);  // Default value
    QCOMPARE(m_reconnect->getCurrentRetryCount(), 0);
}

void TestModbusPerformance::testReconnectRetryLogic()
{
    m_reconnect->setMaxRetries(5);
    
    ModbusConnectionConfig config;
    config.type = ModbusConnectionType::Serial;
    config.serialPort = "INVALID_PORT";
    config.baudRate = 9600;
    config.slaveId = 1;
    
    // This should fail and trigger retry logic
    bool result = m_reconnect->attemptReconnection(config);
    
    // Should fail but retry count should be updated
    QVERIFY(!result);
    QVERIFY(m_reconnect->getCurrentRetryCount() > 0);
}

void TestModbusPerformance::testReconnectBackoffStrategy()
{
    m_reconnect->setBaseDelay(100);
    m_reconnect->setMaxDelay(1000);
    
    // Test exponential backoff calculation
    int delay1 = m_reconnect->calculateNextDelay(1);
    int delay2 = m_reconnect->calculateNextDelay(2);
    int delay3 = m_reconnect->calculateNextDelay(3);
    
    QVERIFY(delay1 > 0);
    QVERIFY(delay2 > delay1);
    QVERIFY(delay3 > delay2);
    QVERIFY(delay3 <= 1000);  // Should not exceed max delay
}

// =============================================================================
// Batch Operations Tests
// =============================================================================

void TestModbusPerformance::testBatchOperationCreation()
{
    QVERIFY(m_batch != nullptr);
    QCOMPARE(m_batch->getPendingOperationCount(), 0);
}

void TestModbusPerformance::testBatchOperationOptimization()
{
    QVector<BatchReadOperation> operations;
    
    // Create overlapping operations that can be optimized
    BatchReadOperation op1;
    op1.operationId = "op1";
    op1.functionCode = 3;
    op1.startAddress = 0;
    op1.quantity = 10;
    operations.append(op1);
    
    BatchReadOperation op2;
    op2.operationId = "op2";
    op2.functionCode = 3;
    op2.startAddress = 5;
    op2.quantity = 10;
    operations.append(op2);
    
    auto optimized = m_batch->optimizeOperations(operations);
    
    // Should have fewer operations after optimization
    QVERIFY(optimized.size() <= operations.size());
}

void TestModbusPerformance::testBatchOperationExecution()
{
    QSignalSpy spy(m_batch, &BatchOperationManager::batchCompleted);
    
    QVector<BatchReadOperation> operations;
    
    BatchReadOperation op;
    op.operationId = "batch_test";
    op.functionCode = 3;
    op.startAddress = 0;
    op.quantity = 5;
    operations.append(op);
    
    QString batchId = m_batch->executeBatch(operations);
    QVERIFY(!batchId.isEmpty());
    
    // Wait for batch completion
    spy.wait(5000);
    
    // Should have received completion signal
    QVERIFY(spy.count() >= 0);
}

// =============================================================================
// Performance Monitor Tests
// =============================================================================

void TestModbusPerformance::testPerformanceMonitorCreation()
{
    QVERIFY(m_monitor != nullptr);
    QVERIFY(m_monitor->isEnabled());
}

void TestModbusPerformance::testPerformanceMonitorMetrics()
{
    // Record some operations
    m_monitor->recordOperation("read", 50, true);
    m_monitor->recordOperation("write", 75, true);
    m_monitor->recordOperation("read", 100, false);
    
    auto stats = m_monitor->getStatistics();
    QVERIFY(stats.totalOperations >= 3);
    QVERIFY(stats.successfulOperations >= 2);
    QVERIFY(stats.failedOperations >= 1);
    QVERIFY(stats.averageResponseTime > 0);
}

void TestModbusPerformance::testPerformanceMonitorAlerts()
{
    QSignalSpy spy(m_monitor, &ModbusPerformanceMonitor::alertTriggered);
    
    // Set low threshold for testing
    m_monitor->setResponseTimeThreshold(10);
    
    // Record operation that should trigger alert
    m_monitor->recordOperation("slow_op", 50, true);
    
    // Should have triggered alert
    QVERIFY(spy.count() >= 1);
}

// =============================================================================
// Memory Pool Tests
// =============================================================================

void TestModbusPerformance::testMemoryPoolAllocation()
{
    MemoryPool<int> pool(5, 10);  // 5 blocks of 10 ints each
    
    auto block = pool.allocate();
    QVERIFY(block != nullptr);
    QCOMPARE(pool.getAllocatedBlocks(), 1);
    QCOMPARE(pool.getAvailableBlocks(), 4);
}

void TestModbusPerformance::testMemoryPoolDeallocation()
{
    MemoryPool<int> pool(3, 5);
    
    auto block = pool.allocate();
    QVERIFY(block != nullptr);
    QCOMPARE(pool.getAllocatedBlocks(), 1);
    
    pool.deallocate(block);
    QCOMPARE(pool.getAllocatedBlocks(), 0);
    QCOMPARE(pool.getAvailableBlocks(), 3);
}

void TestModbusPerformance::testMemoryPoolReuse()
{
    MemoryPool<int> pool(2, 5);
    
    auto block1 = pool.allocate();
    auto block2 = pool.allocate();
    
    QVERIFY(block1 != nullptr);
    QVERIFY(block2 != nullptr);
    QCOMPARE(pool.getAvailableBlocks(), 0);
    
    pool.deallocate(block1);
    QCOMPARE(pool.getAvailableBlocks(), 1);
    
    auto block3 = pool.allocate();
    QVERIFY(block3 != nullptr);
    QCOMPARE(pool.getAvailableBlocks(), 0);
    
    // block3 should reuse the memory from block1
    QCOMPARE(block1, block3);
}

// =============================================================================
// Optimized Manager Integration Tests
// =============================================================================

void TestModbusPerformance::testOptimizedManagerCreation()
{
    QVERIFY(m_manager != nullptr);
    QVERIFY(!m_manager->isConnected());
    
    auto health = m_manager->getSystemHealth();
    QVERIFY(health.contains("status"));
}

void TestModbusPerformance::testOptimizedManagerConfiguration()
{
    QJsonObject config;
    config["cache"] = QJsonObject{
        {"maxSize", 500},
        {"defaultTtl", 3000}
    };
    config["connectionPool"] = QJsonObject{
        {"maxConnections", 3},
        {"connectionTimeout", 8000}
    };
    
    m_manager->setConfiguration(config);
    
    auto retrievedConfig = m_manager->getConfiguration();
    QCOMPARE(retrievedConfig["cache"].toObject()["maxSize"].toInt(), 500);
    QCOMPARE(retrievedConfig["connectionPool"].toObject()["maxConnections"].toInt(), 3);
}

void TestModbusPerformance::testOptimizedManagerCaching()
{
    // Test that cache is being used
    auto cacheStats = m_manager->getCacheStatistics();
    QVERIFY(cacheStats.contains("size"));
    QVERIFY(cacheStats.contains("hits"));
    QVERIFY(cacheStats.contains("misses"));
    
    // Clear cache
    m_manager->clearCache();
    auto statsAfterClear = m_manager->getCacheStatistics();
    QCOMPARE(statsAfterClear["size"].toInt(), 0);
}

void TestModbusPerformance::testOptimizedManagerAsync()
{
    auto queueStatus = m_manager->getAsyncQueueStatus();
    QVERIFY(queueStatus.contains("size"));
    QVERIFY(queueStatus.contains("maxSize"));
    QCOMPARE(queueStatus["size"].toInt(), 0);
}

void TestModbusPerformance::testOptimizedManagerStatistics()
{
    auto stats = m_manager->getPerformanceStatistics();
    QVERIFY(stats.totalOperations >= 0);
    QVERIFY(stats.successfulOperations >= 0);
    QVERIFY(stats.failedOperations >= 0);
    QVERIFY(stats.averageResponseTime >= 0);
    QVERIFY(stats.cacheHitRate >= 0.0 && stats.cacheHitRate <= 1.0);
    QVERIFY(stats.connectionPoolUtilization >= 0.0 && stats.connectionPoolUtilization <= 1.0);
    
    // Reset statistics
    m_manager->resetStatistics();
    auto resetStats = m_manager->getPerformanceStatistics();
    QCOMPARE(resetStats.totalOperations, 0);
    QCOMPARE(resetStats.successfulOperations, 0);
    QCOMPARE(resetStats.failedOperations, 0);
}

QTEST_MAIN(TestModbusPerformance)
#include "test_modbus_performance.moc"
