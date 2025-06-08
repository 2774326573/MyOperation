/**
 * @file simple_optimization_example.cpp
 * @brief Simple example showing how to upgrade from ModbusManager to OptimizedModbusManager
 * 
 * This example demonstrates a drop-in replacement scenario where existing code
 * using ModbusManager can be easily upgraded to use OptimizedModbusManager
 * for immediate performance benefits.
 */

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "optimized_modbus_manager.h"

void demonstrateDropInReplacement()
{
    qDebug() << "=== Simple Drop-in Replacement Demo ===";
    
    // Create optimized manager (replaces ModbusManager)
    OptimizedModbusManager manager;
    
    // Configure connection (same as before)
    ModbusConnectionConfig config;
    config.type = ModbusConnectionType::Serial;
    config.serialPort = "COM3";  // Adjust as needed
    config.baudRate = 9600;
    config.parity = QSerialPort::NoParity;
    config.dataBits = QSerialPort::Data8;
    config.stopBits = QSerialPort::OneStop;
    config.slaveId = 1;
    
    // Connect (same API as ModbusManager)
    if (!manager.connectDevice(config)) {
        qWarning() << "Failed to connect to Modbus device";
        return;
    }
    
    qDebug() << "Connected successfully!";
    
    // Read operations (same API, but now with caching and optimizations)
    QVector<quint16> registers;
    
    qDebug() << "\nReading holding registers (first time - from device):";
    auto start = QDateTime::currentMSecsSinceEpoch();
    bool success = manager.readHoldingRegisters(0, 10, registers);
    auto elapsed = QDateTime::currentMSecsSinceEpoch() - start;
    
    if (success) {
        qDebug() << "Read successful, time:" << elapsed << "ms";
        qDebug() << "Data:" << registers;
    } else {
        qWarning() << "Read failed";
    }
    
    qDebug() << "\nReading same registers (second time - from cache):";
    start = QDateTime::currentMSecsSinceEpoch();
    success = manager.readHoldingRegisters(0, 10, registers);
    elapsed = QDateTime::currentMSecsSinceEpoch() - start;
    
    if (success) {
        qDebug() << "Read successful, time:" << elapsed << "ms (should be much faster!)";
        qDebug() << "Data:" << registers;
    }
    
    // Write operation (same API)
    QVector<quint16> writeData = {100, 200, 300, 400, 500};
    qDebug() << "\nWriting data:";
    success = manager.writeMultipleRegisters(0, writeData);
    qDebug() << "Write result:" << (success ? "SUCCESS" : "FAILED");
    
    // Show performance benefits
    qDebug() << "\n=== Performance Benefits ===";
    auto stats = manager.getPerformanceStatistics();
    qDebug() << "Total operations performed:" << stats.totalOperations;
    qDebug() << "Cache hit rate:" << QString::number(stats.cacheHitRate * 100, 'f', 1) << "%";
    qDebug() << "Average response time:" << stats.averageResponseTime << "ms";
    
    // Show cache statistics
    auto cacheStats = manager.getCacheStatistics();
    qDebug() << "\n=== Cache Statistics ===";
    qDebug() << "Cache size:" << cacheStats["size"].toInt();
    qDebug() << "Cache hits:" << cacheStats["hits"].toInt();
    qDebug() << "Cache misses:" << cacheStats["misses"].toInt();
    
    manager.disconnectDevice();
    qDebug() << "\nDisconnected from device";
}

void demonstrateAsyncUpgrade()
{
    qDebug() << "\n=== Async Operations Demo ===";
    
    OptimizedModbusManager manager;
    
    // Enable async operations (new feature)
    QJsonObject config;
    config["async"] = QJsonObject{
        {"workerThreads", 2},
        {"maxQueueSize", 50}
    };
    manager.setConfiguration(config);
    
    // Connect to device
    ModbusConnectionConfig connConfig;
    connConfig.type = ModbusConnectionType::Serial;
    connConfig.serialPort = "COM3";
    connConfig.baudRate = 9600;
    connConfig.slaveId = 1;
    
    if (!manager.connectDevice(connConfig)) {
        qWarning() << "Failed to connect for async demo";
        return;
    }
    
    // Set up async result handler
    QObject::connect(&manager, &OptimizedModbusManager::asyncOperationCompleted,
                     [](const QString &operationId, bool success, const QVariant &result) {
        qDebug() << "Async operation" << operationId << "completed:"
                 << (success ? "SUCCESS" : "FAILED");
        if (success && result.canConvert<QVector<quint16>>()) {
            auto data = result.value<QVector<quint16>>();
            qDebug() << "Received data:" << data;
        }
    });
    
    // Queue multiple async operations
    qDebug() << "Queuing async operations...";
    QString opId1 = manager.readHoldingRegistersAsync(0, 5);
    QString opId2 = manager.readInputRegistersAsync(10, 3);
    QString opId3 = manager.readCoilsAsync(0, 8);
    
    qDebug() << "Queued operations:" << opId1 << opId2 << opId3;
    
    // Check queue status
    auto queueStatus = manager.getAsyncQueueStatus();
    qDebug() << "Queue status:" << queueStatus;
    
    manager.disconnectDevice();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Starting Simple Optimization Example";
    qDebug() << "====================================";
    
    // Run demonstrations
    QTimer::singleShot(100, []() {
        demonstrateDropInReplacement();
        demonstrateAsyncUpgrade();
    });
    
    // Exit after demos
    QTimer::singleShot(5000, &app, &QCoreApplication::quit);
    
    return app.exec();
}
