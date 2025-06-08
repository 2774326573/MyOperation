/**
 * @file optimization_example.cpp
 * @brief Comprehensive example demonstrating libmodbus performance optimizations
 * 
 * This example shows how to use all the optimization features:
 * - Connection pooling
 * - Smart caching
 * - Asynchronous operations
 * - Batch operations
 * - Performance monitoring
 * - Intelligent reconnection
 */

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "optimized_modbus_manager.h"

class ModbusOptimizationDemo : public QObject
{
    Q_OBJECT

public:
    explicit ModbusOptimizationDemo(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new OptimizedModbusManager(this))
    {
        setupConfiguration();
        setupConnections();
    }

private slots:
    void runDemo()
    {
        qDebug() << "=== Modbus Optimization Demo ===";
        
        // Demo 1: Basic connection and configuration
        demonstrateBasicSetup();
        
        // Demo 2: Synchronous operations with caching
        demonstrateSynchronousOperations();
        
        // Demo 3: Asynchronous operations
        demonstrateAsynchronousOperations();
        
        // Demo 4: Batch operations
        demonstrateBatchOperations();
        
        // Demo 5: Performance monitoring
        demonstratePerformanceMonitoring();
        
        // Demo 6: Advanced features
        demonstrateAdvancedFeatures();
    }

private:
    void setupConfiguration()
    {
        // Create optimization configuration
        QJsonObject config;
        
        // Connection pool settings
        QJsonObject poolConfig;
        poolConfig["maxConnections"] = 5;
        poolConfig["connectionTimeout"] = 5000;
        poolConfig["keepAliveInterval"] = 30000;
        config["connectionPool"] = poolConfig;
        
        // Cache settings
        QJsonObject cacheConfig;
        cacheConfig["maxSize"] = 1000;
        cacheConfig["defaultTtl"] = 5000;  // 5 seconds
        cacheConfig["cleanupInterval"] = 10000;
        config["cache"] = cacheConfig;
        
        // Async settings
        QJsonObject asyncConfig;
        asyncConfig["workerThreads"] = 2;
        asyncConfig["maxQueueSize"] = 100;
        asyncConfig["operationTimeout"] = 10000;
        config["async"] = asyncConfig;
        
        // Reconnection settings
        QJsonObject reconnectConfig;
        reconnectConfig["maxRetries"] = 5;
        reconnectConfig["baseDelay"] = 1000;
        reconnectConfig["maxDelay"] = 30000;
        reconnectConfig["jitterEnabled"] = true;
        config["reconnection"] = reconnectConfig;
        
        // Performance monitoring
        QJsonObject monitorConfig;
        monitorConfig["enabled"] = true;
        monitorConfig["reportInterval"] = 10000;
        monitorConfig["alertThresholds"] = QJsonObject{
            {"responseTime", 1000},
            {"errorRate", 0.1},
            {"cacheHitRate", 0.8}
        };
        config["monitoring"] = monitorConfig;
        
        m_manager->setConfiguration(config);
    }
    
    void setupConnections()
    {
        // Connect performance alerts
        connect(m_manager, &OptimizedModbusManager::performanceAlert,
                this, [](const QString &message) {
            qWarning() << "Performance Alert:" << message;
        });
        
        // Connect async operation results
        connect(m_manager, &OptimizedModbusManager::asyncOperationCompleted,
                this, [](const QString &operationId, bool success, const QVariant &result) {
            qDebug() << "Async operation" << operationId << "completed:"
                     << (success ? "SUCCESS" : "FAILED") << result;
        });
    }
    
    void demonstrateBasicSetup()
    {
        qDebug() << "\n--- Demo 1: Basic Setup ---";
        
        // Configure serial connection
        ModbusConnectionConfig config;
        config.type = ModbusConnectionType::Serial;
        config.serialPort = "COM3";  // Adjust as needed
        config.baudRate = 9600;
        config.parity = QSerialPort::NoParity;
        config.dataBits = QSerialPort::Data8;
        config.stopBits = QSerialPort::OneStop;
        config.slaveId = 1;
        
        bool connected = m_manager->connectDevice(config);
        qDebug() << "Connection result:" << (connected ? "SUCCESS" : "FAILED");
        
        if (connected) {
            qDebug() << "Device connected successfully";
            qDebug() << "System health:" << m_manager->getSystemHealth();
        }
    }
    
    void demonstrateSynchronousOperations()
    {
        qDebug() << "\n--- Demo 2: Synchronous Operations with Caching ---";
        
        if (!m_manager->isConnected()) {
            qDebug() << "Device not connected, skipping sync demo";
            return;
        }
        
        // Read holding registers multiple times to demonstrate caching
        QVector<quint16> registers;
        bool success;
        
        qDebug() << "First read (will be cached):";
        auto start = QDateTime::currentMSecsSinceEpoch();
        success = m_manager->readHoldingRegisters(0, 10, registers);
        auto elapsed = QDateTime::currentMSecsSinceEpoch() - start;
        qDebug() << "Read result:" << success << "Time:" << elapsed << "ms";
        
        qDebug() << "Second read (from cache):";
        start = QDateTime::currentMSecsSinceEpoch();
        success = m_manager->readHoldingRegisters(0, 10, registers);
        elapsed = QDateTime::currentMSecsSinceEpoch() - start;
        qDebug() << "Read result:" << success << "Time:" << elapsed << "ms (should be faster)";
        
        // Write operation (will invalidate cache)
        QVector<quint16> writeData = {100, 200, 300};
        qDebug() << "Writing data (will invalidate cache):";
        success = m_manager->writeMultipleRegisters(0, writeData);
        qDebug() << "Write result:" << success;
        
        // Read again (cache miss, will read from device)
        qDebug() << "Third read (cache invalidated):";
        start = QDateTime::currentMSecsSinceEpoch();
        success = m_manager->readHoldingRegisters(0, 10, registers);
        elapsed = QDateTime::currentMSecsSinceEpoch() - start;
        qDebug() << "Read result:" << success << "Time:" << elapsed << "ms";
    }
    
    void demonstrateAsynchronousOperations()
    {
        qDebug() << "\n--- Demo 3: Asynchronous Operations ---";
        
        if (!m_manager->isConnected()) {
            qDebug() << "Device not connected, skipping async demo";
            return;
        }
        
        // Queue multiple async operations
        QString opId1 = m_manager->readHoldingRegistersAsync(10, 5);
        QString opId2 = m_manager->readInputRegistersAsync(20, 3);
        QString opId3 = m_manager->readCoilsAsync(0, 16);
        
        qDebug() << "Queued async operations:" << opId1 << opId2 << opId3;
        qDebug() << "Async queue status:" << m_manager->getAsyncQueueStatus();
    }
    
    void demonstrateBatchOperations()
    {
        qDebug() << "\n--- Demo 4: Batch Operations ---";
        
        if (!m_manager->isConnected()) {
            qDebug() << "Device not connected, skipping batch demo";
            return;
        }
        
        // Create batch read operations
        QVector<BatchReadOperation> batchReads;
        
        BatchReadOperation op1;
        op1.operationId = "read_batch_1";
        op1.functionCode = 3;  // Read holding registers
        op1.startAddress = 0;
        op1.quantity = 10;
        batchReads.append(op1);
        
        BatchReadOperation op2;
        op2.operationId = "read_batch_2";
        op2.functionCode = 4;  // Read input registers
        op2.startAddress = 100;
        op2.quantity = 5;
        batchReads.append(op2);
        
        BatchReadOperation op3;
        op3.operationId = "read_batch_3";
        op3.functionCode = 1;  // Read coils
        op3.startAddress = 0;
        op3.quantity = 16;
        batchReads.append(op3);
        
        qDebug() << "Executing batch read operations...";
        auto start = QDateTime::currentMSecsSinceEpoch();
        QString batchId = m_manager->executeBatchRead(batchReads);
        auto elapsed = QDateTime::currentMSecsSinceEpoch() - start;
        
        qDebug() << "Batch operation ID:" << batchId << "Time:" << elapsed << "ms";
    }
    
    void demonstratePerformanceMonitoring()
    {
        qDebug() << "\n--- Demo 5: Performance Monitoring ---";
        
        // Get current performance statistics
        auto stats = m_manager->getPerformanceStatistics();
        qDebug() << "Performance Statistics:";
        qDebug() << "  Total Operations:" << stats.totalOperations;
        qDebug() << "  Successful Operations:" << stats.successfulOperations;
        qDebug() << "  Failed Operations:" << stats.failedOperations;
        qDebug() << "  Average Response Time:" << stats.averageResponseTime << "ms";
        qDebug() << "  Cache Hit Rate:" << QString::number(stats.cacheHitRate * 100, 'f', 1) << "%";
        qDebug() << "  Connection Pool Utilization:" << QString::number(stats.connectionPoolUtilization * 100, 'f', 1) << "%";
        
        // Get system health
        auto health = m_manager->getSystemHealth();
        qDebug() << "System Health:";
        for (auto it = health.begin(); it != health.end(); ++it) {
            qDebug() << " " << it.key() << ":" << it.value().toString();
        }
    }
    
    void demonstrateAdvancedFeatures()
    {
        qDebug() << "\n--- Demo 6: Advanced Features ---";
        
        // Cache management
        qDebug() << "Cache statistics before cleanup:";
        auto cacheStats = m_manager->getCacheStatistics();
        for (auto it = cacheStats.begin(); it != cacheStats.end(); ++it) {
            qDebug() << " " << it.key() << ":" << it.value();
        }
        
        // Force cache cleanup
        m_manager->clearCache();
        qDebug() << "Cache cleared";
        
        // Connection pool management
        qDebug() << "Warming up connection pool...";
        m_manager->warmupConnectionPool(3);  // Warm up 3 connections
        
        // Configuration export/import
        qDebug() << "Current configuration:";
        auto config = m_manager->getConfiguration();
        QJsonDocument doc(config);
        qDebug() << doc.toJson(QJsonDocument::Compact);
        
        // Reset statistics
        m_manager->resetStatistics();
        qDebug() << "Statistics reset";
    }

private:
    OptimizedModbusManager *m_manager;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    ModbusOptimizationDemo demo;
    
    // Start demo after event loop starts
    QTimer::singleShot(100, &demo, &ModbusOptimizationDemo::runDemo);
    
    // Exit after demo completes
    QTimer::singleShot(10000, &app, &QCoreApplication::quit);
    
    return app.exec();
}

#include "optimization_example.moc"
