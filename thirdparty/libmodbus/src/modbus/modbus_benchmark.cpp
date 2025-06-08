/**
 * @file modbus_benchmark.cpp
 * @brief Implementation of Modbus benchmark utilities
 */

#include "../../inc/modbus/modbus_benchmark.h"
#include "../../inc/modbus/optimized_modbus_manager.h"
#include "../../inc/modbus/modbusmanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include <QMutexLocker>
#include <QTimer>
#include <QDebug>

// =============================================================================
// BenchmarkResult Implementation
// =============================================================================

QJsonObject BenchmarkResult::toJson() const
{
    QJsonObject json;
    json["testName"] = testName;
    json["totalOperations"] = totalOperations;
    json["successfulOperations"] = successfulOperations;
    json["failedOperations"] = failedOperations;
    json["totalTimeMs"] = totalTimeMs;
    json["averageTimeMs"] = averageTimeMs;
    json["operationsPerSecond"] = operationsPerSecond;
    json["successRate"] = successRate;
    
    QJsonObject metrics;
    for (auto it = additionalMetrics.begin(); it != additionalMetrics.end(); ++it) {
        metrics[it.key()] = QJsonValue::fromVariant(it.value());
    }
    json["additionalMetrics"] = metrics;
    
    return json;
}

BenchmarkResult BenchmarkResult::fromJson(const QJsonObject &json)
{
    BenchmarkResult result;
    result.testName = json["testName"].toString();
    result.totalOperations = json["totalOperations"].toInt();
    result.successfulOperations = json["successfulOperations"].toInt();
    result.failedOperations = json["failedOperations"].toInt();
    result.totalTimeMs = json["totalTimeMs"].toInt();
    result.averageTimeMs = json["averageTimeMs"].toDouble();
    result.operationsPerSecond = json["operationsPerSecond"].toDouble();
    result.successRate = json["successRate"].toDouble();
    
    QJsonObject metrics = json["additionalMetrics"].toObject();
    for (auto it = metrics.begin(); it != metrics.end(); ++it) {
        result.additionalMetrics[it.key()] = it.value().toVariant();
    }
    
    return result;
}

// =============================================================================
// BenchmarkConfig Implementation
// =============================================================================

QJsonObject BenchmarkConfig::toJson() const
{
    QJsonObject json;
    json["iterations"] = iterations;
    json["concurrentThreads"] = concurrentThreads;
    json["enableCaching"] = enableCaching;
    json["enableConnectionPooling"] = enableConnectionPooling;
    json["warmupIterations"] = warmupIterations;
    json["registerStartAddress"] = registerStartAddress;
    json["registerCount"] = registerCount;
    json["delayBetweenOperationsMs"] = delayBetweenOperationsMs;
    json["randomizeAddresses"] = randomizeAddresses;
    return json;
}

BenchmarkConfig BenchmarkConfig::fromJson(const QJsonObject &json)
{
    BenchmarkConfig config;
    config.iterations = json["iterations"].toInt(100);
    config.concurrentThreads = json["concurrentThreads"].toInt(1);
    config.enableCaching = json["enableCaching"].toBool(true);
    config.enableConnectionPooling = json["enableConnectionPooling"].toBool(true);
    config.warmupIterations = json["warmupIterations"].toInt(10);
    config.registerStartAddress = json["registerStartAddress"].toInt(0);
    config.registerCount = json["registerCount"].toInt(10);
    config.delayBetweenOperationsMs = json["delayBetweenOperationsMs"].toInt(0);
    config.randomizeAddresses = json["randomizeAddresses"].toBool(false);
    return config;
}

// =============================================================================
// ModbusBenchmark Implementation
// =============================================================================

ModbusBenchmark::ModbusBenchmark(QObject *parent)
    : QObject(parent)
{
    // Default configuration
    m_config.iterations = 100;
    m_config.concurrentThreads = 1;
    m_config.enableCaching = true;
    m_config.enableConnectionPooling = true;
    m_config.warmupIterations = 10;
}

ModbusBenchmark::~ModbusBenchmark() = default;

void ModbusBenchmark::setConfig(const BenchmarkConfig &config)
{
    m_config = config;
}

void ModbusBenchmark::setOriginalManager(ModbusManagerInterface *manager)
{
    m_originalManager = manager;
}

void ModbusBenchmark::setOptimizedManager(OptimizedModbusManager *manager)
{
    m_optimizedManager = manager;
    
    // Connect async signals
    connect(manager, &OptimizedModbusManager::asyncOperationCompleted,
            this, &ModbusBenchmark::onAsyncOperationCompleted);
}

BenchmarkResult ModbusBenchmark::benchmarkReadOperations(const QString &testName)
{
    setupBenchmarkEnvironment();
    auto result = runReadBenchmark();
    result.testName = testName;
    cleanupBenchmarkEnvironment();
    
    emit benchmarkCompleted(result);
    return result;
}

BenchmarkResult ModbusBenchmark::benchmarkWriteOperations(const QString &testName)
{
    setupBenchmarkEnvironment();
    auto result = runWriteBenchmark();
    result.testName = testName;
    cleanupBenchmarkEnvironment();
    
    emit benchmarkCompleted(result);
    return result;
}

BenchmarkResult ModbusBenchmark::benchmarkMixedOperations(const QString &testName)
{
    setupBenchmarkEnvironment();
    auto result = runMixedBenchmark();
    result.testName = testName;
    cleanupBenchmarkEnvironment();
    
    emit benchmarkCompleted(result);
    return result;
}

BenchmarkResult ModbusBenchmark::benchmarkCacheEffectiveness(const QString &testName)
{
    setupBenchmarkEnvironment();
    auto result = runCacheBenchmark();
    result.testName = testName;
    cleanupBenchmarkEnvironment();
    
    emit benchmarkCompleted(result);
    return result;
}

BenchmarkResult ModbusBenchmark::benchmarkConnectionPooling(const QString &testName)
{
    setupBenchmarkEnvironment();
    auto result = runPoolingBenchmark();
    result.testName = testName;
    cleanupBenchmarkEnvironment();
    
    emit benchmarkCompleted(result);
    return result;
}

BenchmarkResult ModbusBenchmark::benchmarkAsyncOperations(const QString &testName)
{
    setupBenchmarkEnvironment();
    auto result = runAsyncBenchmark();
    result.testName = testName;
    cleanupBenchmarkEnvironment();
    
    emit benchmarkCompleted(result);
    return result;
}

QVector<BenchmarkResult> ModbusBenchmark::runFullBenchmarkSuite()
{
    QVector<BenchmarkResult> results;
    
    qDebug() << "Starting full benchmark suite...";
    
    // Basic operation benchmarks
    results.append(benchmarkReadOperations("Read Operations"));
    results.append(benchmarkWriteOperations("Write Operations"));
    results.append(benchmarkMixedOperations("Mixed Operations"));
    
    // Optimization-specific benchmarks
    if (m_optimizedManager) {
        results.append(benchmarkCacheEffectiveness("Cache Effectiveness"));
        results.append(benchmarkConnectionPooling("Connection Pooling"));
        results.append(benchmarkAsyncOperations("Async Operations"));
    }
    
    qDebug() << "Benchmark suite completed with" << results.size() << "tests";
    return results;
}

QString ModbusBenchmark::generateReport(const QVector<BenchmarkResult> &results, const QString &format)
{
    if (format.toLower() == "json") {
        QJsonArray jsonResults;
        for (const auto &result : results) {
            jsonResults.append(result.toJson());
        }
        QJsonDocument doc(jsonResults);
        return doc.toJson();
    }
    
    // Text format
    QString report;
    report += "=== Modbus Performance Benchmark Report ===\n";
    report += QString("Generated: %1\n").arg(QDateTime::currentDateTime().toString());
    report += QString("Configuration: %1 iterations, %2 threads\n\n")
                .arg(m_config.iterations).arg(m_config.concurrentThreads);
    
    for (const auto &result : results) {
        report += QString("--- %1 ---\n").arg(result.testName);
        report += QString("Total Operations: %1\n").arg(result.totalOperations);
        report += QString("Successful: %1 (%.1f%%)\n")
                    .arg(result.successfulOperations)
                    .arg(result.successRate * 100);
        report += QString("Failed: %1\n").arg(result.failedOperations);
        report += QString("Total Time: %1 ms\n").arg(result.totalTimeMs);
        report += QString("Average Time: %.2f ms\n").arg(result.averageTimeMs);
        report += QString("Operations/Second: %.1f\n").arg(result.operationsPerSecond);
        
        if (!result.additionalMetrics.isEmpty()) {
            report += "Additional Metrics:\n";
            for (auto it = result.additionalMetrics.begin(); it != result.additionalMetrics.end(); ++it) {
                report += QString("  %1: %2\n").arg(it.key()).arg(it.value().toString());
            }
        }
        report += "\n";
    }
    
    return report;
}

void ModbusBenchmark::saveResults(const QVector<BenchmarkResult> &results, const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QString jsonData = generateReport(results, "json");
        file.write(jsonData.toUtf8());
        file.close();
        qDebug() << "Benchmark results saved to" << filename;
    } else {
        qWarning() << "Failed to save benchmark results to" << filename;
    }
}

QVector<BenchmarkResult> ModbusBenchmark::loadResults(const QString &filename)
{
    QVector<BenchmarkResult> results;
    
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray jsonResults = doc.array();
        
        for (const auto &value : jsonResults) {
            results.append(BenchmarkResult::fromJson(value.toObject()));
        }
        
        qDebug() << "Loaded" << results.size() << "benchmark results from" << filename;
    } else {
        qWarning() << "Failed to load benchmark results from" << filename;
    }
    
    return results;
}

// Private implementation methods

void ModbusBenchmark::setupBenchmarkEnvironment()
{
    if (m_optimizedManager) {
        // Configure optimizations based on benchmark config
        QJsonObject config;
        
        if (m_config.enableCaching) {
            config["cache"] = QJsonObject{
                {"maxSize", 1000},
                {"defaultTtl", 5000}
            };        }
        
        // Create OptimizationConfig instead of QJsonObject
        OptimizedModbusManager::OptimizationConfig optimConfig;
        
        if (m_config.enableCaching) {
            optimConfig.cacheEnabled = true;
            optimConfig.defaultCacheTtlMs = 5000;
        }
        
        if (m_config.enableConnectionPooling) {
            optimConfig.maxConnections = qMax(2, m_config.concurrentThreads * 2);
            optimConfig.connectionTimeoutMinutes = 5;
        }
        
        optimConfig.asyncEnabled = true;
        optimConfig.maxAsyncOperations = m_config.iterations * 2;
        
        m_optimizedManager->setConfiguration(optimConfig);
    }
}

void ModbusBenchmark::cleanupBenchmarkEnvironment()
{
    if (m_optimizedManager) {
        m_optimizedManager->clearCache();
        m_optimizedManager->resetStatistics();
    }
}

BenchmarkResult ModbusBenchmark::runReadBenchmark()
{
    auto result = createEmptyResult("Read Benchmark");
    
    if (!m_optimizedManager) {
        qWarning() << "No optimized manager available for benchmark";
        return result;
    }
      // Warmup
    for (int i = 0; i < m_config.warmupIterations; ++i) {
        QVector<quint16> dummy;
        m_optimizedManager->readHoldingRegisters("device1", m_config.registerStartAddress, 
                                                m_config.registerCount, dummy);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < m_config.iterations; ++i) {
        QVector<quint16> registers;
        int address = m_config.registerStartAddress;
        
        if (m_config.randomizeAddresses) {
            address = QRandomGenerator::global()->bounded(1000);
        }
        
        bool success = m_optimizedManager->readHoldingRegisters("device1", address, m_config.registerCount, registers);
        
        if (success) {
            result.successfulOperations++;
        } else {
            result.failedOperations++;
        }
        
        result.totalOperations++;
        
        if (m_config.delayBetweenOperationsMs > 0) {
            QThread::msleep(m_config.delayBetweenOperationsMs);
        }
        
        emit benchmarkProgress(i + 1, m_config.iterations, "Read Operations");
    }
    
    result.totalTimeMs = timer.elapsed();
    updateResultMetrics(result);
    
    // Add cache statistics if available
    if (m_optimizedManager) {
        auto cacheStats = m_optimizedManager->getCacheStatistics();
        result.additionalMetrics["cacheHits"] = cacheStats["hits"];
        result.additionalMetrics["cacheMisses"] = cacheStats["misses"];
        result.additionalMetrics["cacheHitRate"] = cacheStats["hitRate"];
    }
    
    return result;
}

BenchmarkResult ModbusBenchmark::runWriteBenchmark()
{
    auto result = createEmptyResult("Write Benchmark");
    
    if (!m_optimizedManager) {
        return result;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < m_config.iterations; ++i) {
        QVector<quint16> data;
        for (int j = 0; j < m_config.registerCount; ++j) {
            data.append(QRandomGenerator::global()->bounded(65536));
        }
        
        int address = m_config.registerStartAddress;
        if (m_config.randomizeAddresses) {
            address = QRandomGenerator::global()->bounded(1000);
        }
        
        bool success = m_optimizedManager->writeMultipleRegisters("device1", address, data);
        
        if (success) {
            result.successfulOperations++;
        } else {
            result.failedOperations++;
        }
        
        result.totalOperations++;
        
        if (m_config.delayBetweenOperationsMs > 0) {
            QThread::msleep(m_config.delayBetweenOperationsMs);
        }
        
        emit benchmarkProgress(i + 1, m_config.iterations, "Write Operations");
    }
    
    result.totalTimeMs = timer.elapsed();
    updateResultMetrics(result);
    
    return result;
}

BenchmarkResult ModbusBenchmark::runMixedBenchmark()
{
    auto result = createEmptyResult("Mixed Benchmark");
    
    if (!m_optimizedManager) {
        return result;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < m_config.iterations; ++i) {
        bool success = false;
        
        // Alternate between read and write operations
        if (i % 2 == 0) {
            // Read operation
            QVector<quint16> registers;            success = m_optimizedManager->readHoldingRegisters("device1", m_config.registerStartAddress, 
                                                              m_config.registerCount, registers);
        } else {
            // Write operation
            QVector<quint16> data;
            for (int j = 0; j < m_config.registerCount; ++j) {
                data.append(QRandomGenerator::global()->bounded(65536));
            }
            success = m_optimizedManager->writeMultipleRegisters("device1", m_config.registerStartAddress, data);
        }
        
        if (success) {
            result.successfulOperations++;
        } else {
            result.failedOperations++;
        }
        
        result.totalOperations++;
        
        emit benchmarkProgress(i + 1, m_config.iterations, "Mixed Operations");
    }
    
    result.totalTimeMs = timer.elapsed();
    updateResultMetrics(result);
    
    return result;
}

BenchmarkResult ModbusBenchmark::runCacheBenchmark()
{
    auto result = createEmptyResult("Cache Benchmark");
    
    if (!m_optimizedManager) {
        return result;
    }
    
    // First, clear cache and read data (cache miss)
    m_optimizedManager->clearCache();
    
    QElapsedTimer timer;
    timer.start();
      // Phase 1: Initial reads (cache misses)
    for (int i = 0; i < m_config.iterations / 2; ++i) {
        QVector<quint16> registers;
        bool success = m_optimizedManager->readHoldingRegisters("device1", i % 10, m_config.registerCount, registers);
        
        if (success) {
            result.successfulOperations++;
        } else {
            result.failedOperations++;
        }
        result.totalOperations++;
    }
    
    // Phase 2: Repeated reads (cache hits)
    for (int i = 0; i < m_config.iterations / 2; ++i) {
        QVector<quint16> registers;
        bool success = m_optimizedManager->readHoldingRegisters("device1", i % 10, m_config.registerCount, registers);
        
        if (success) {
            result.successfulOperations++;
        } else {
            result.failedOperations++;
        }
        result.totalOperations++;
    }
    
    result.totalTimeMs = timer.elapsed();
    updateResultMetrics(result);
    
    // Add detailed cache statistics
    auto cacheStats = m_optimizedManager->getCacheStatistics();
    result.additionalMetrics["cacheHits"] = cacheStats["hits"];
    result.additionalMetrics["cacheMisses"] = cacheStats["misses"];
    result.additionalMetrics["cacheHitRate"] = cacheStats["hitRate"];
    result.additionalMetrics["cacheSize"] = cacheStats["size"];
    
    return result;
}

BenchmarkResult ModbusBenchmark::runPoolingBenchmark()
{
    auto result = createEmptyResult("Connection Pooling Benchmark");
    
    if (!m_optimizedManager) {
        return result;
    }
      // Warm up connection pool
    QStringList deviceList;
    for (int i = 0; i < m_config.concurrentThreads; ++i) {
        deviceList << QString("device%1").arg(i);
    }
    m_optimizedManager->warmupConnectionPool(deviceList);
    
    QElapsedTimer timer;
    timer.start();
    
    // Simulate concurrent operations
    QVector<QThread*> threads;
    QMutex resultMutex;
    
    for (int t = 0; t < m_config.concurrentThreads; ++t) {
        QThread *thread = QThread::create([this, &result, &resultMutex]() {
            int operationsPerThread = m_config.iterations / m_config.concurrentThreads;
            
            for (int i = 0; i < operationsPerThread; ++i) {
                QVector<quint16> registers;
                bool success = m_optimizedManager->readHoldingRegisters("device1", i % 100, m_config.registerCount, registers);
                
                QMutexLocker locker(&resultMutex);
                if (success) {
                    result.successfulOperations++;
                } else {
                    result.failedOperations++;
                }
                result.totalOperations++;
            }
        });
        
        threads.append(thread);
        thread->start();
    }
    
    // Wait for all threads to complete
    for (auto thread : threads) {
        thread->wait();
        delete thread;
    }
    
    result.totalTimeMs = timer.elapsed();
    updateResultMetrics(result);
      // Add connection pool statistics
    auto perfStats = m_optimizedManager->getPerformanceStatistics();
    result.additionalMetrics["connectionPoolUtilization"] = perfStats.value("connectionPoolUtilization", 0.0);
    
    return result;
}

BenchmarkResult ModbusBenchmark::runAsyncBenchmark()
{
    auto result = createEmptyResult("Async Operations Benchmark");
    
    if (!m_optimizedManager) {
        return result;
    }
    
    // Reset async operation tracking
    QMutexLocker locker(&m_asyncMutex);
    m_pendingAsyncOps.clear();
    m_completedAsyncOps = 0;
    m_totalAsyncOps = m_config.iterations;
    locker.unlock();
    
    QElapsedTimer timer;
    timer.start();
      // Queue all async operations
    for (int i = 0; i < m_config.iterations; ++i) {
        QString opId = m_optimizedManager->readHoldingRegistersAsync("device1", i % 100, m_config.registerCount,
            [this, &result, &timer](bool success, const QVector<quint16>& values) {
                QMutexLocker locker(&m_asyncMutex);
                if (success) {
                    result.successfulOperations++;
                } else {
                    result.failedOperations++;
                }
            });
        
        QMutexLocker opLocker(&m_asyncMutex);
        m_pendingAsyncOps[opId] = qMakePair(timer.elapsed(), false);
        opLocker.unlock();
        
        result.totalOperations++;
    }
    
    // Wait for all operations to complete
    while (m_completedAsyncOps < m_totalAsyncOps) {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    
    result.totalTimeMs = timer.elapsed();
    
    // Count successful operations
    QMutexLocker finalLocker(&m_asyncMutex);
    for (auto it = m_pendingAsyncOps.begin(); it != m_pendingAsyncOps.end(); ++it) {
        if (it.value().second) {
            result.successfulOperations++;
        } else {
            result.failedOperations++;
        }
    }
    finalLocker.unlock();
    
    updateResultMetrics(result);
    
    // Add async-specific metrics
    auto queueStatus = m_optimizedManager->getAsyncQueueStatus();
    result.additionalMetrics["maxQueueSize"] = queueStatus["maxSize"];
    result.additionalMetrics["completedOperations"] = m_completedAsyncOps;
    
    return result;
}

QVector<int> ModbusBenchmark::generateRandomAddresses(int count, int maxAddress)
{
    QVector<int> addresses;
    for (int i = 0; i < count; ++i) {
        addresses.append(QRandomGenerator::global()->bounded(maxAddress));
    }
    return addresses;
}

BenchmarkResult ModbusBenchmark::createEmptyResult(const QString &testName)
{
    BenchmarkResult result;
    result.testName = testName;
    result.totalOperations = 0;
    result.successfulOperations = 0;
    result.failedOperations = 0;
    result.totalTimeMs = 0;
    result.averageTimeMs = 0.0;
    result.operationsPerSecond = 0.0;
    result.successRate = 0.0;
    return result;
}

void ModbusBenchmark::updateResultMetrics(BenchmarkResult &result)
{
    if (result.totalOperations > 0) {
        result.successRate = static_cast<double>(result.successfulOperations) / result.totalOperations;
        result.averageTimeMs = static_cast<double>(result.totalTimeMs) / result.totalOperations;
        
        if (result.totalTimeMs > 0) {
            result.operationsPerSecond = (result.totalOperations * 1000.0) / result.totalTimeMs;
        }
    }
}

void ModbusBenchmark::onAsyncOperationCompleted(const QString &operationId, bool success, const QVariant &result)
{
    QMutexLocker locker(&m_asyncMutex);
    
    if (m_pendingAsyncOps.contains(operationId)) {
        m_pendingAsyncOps[operationId].second = success;
        m_completedAsyncOps++;
    }
}

// =============================================================================
// ModbusPerformanceTest Implementation
// =============================================================================

ModbusPerformanceTest::ModbusPerformanceTest(QObject *parent)
    : QObject(parent)
{
}

void ModbusPerformanceTest::addBenchmarkConfig(const QString &name, const BenchmarkConfig &config)
{
    m_configs[name] = config;
}

void ModbusPerformanceTest::setBaselineResults(const QVector<BenchmarkResult> &baseline)
{
    m_baselineResults = baseline;
}

void ModbusPerformanceTest::runRegressionTests()
{
    m_currentResults.clear();
    m_regressionIssues.clear();
    
    ModbusBenchmark benchmark;
    
    for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
        benchmark.setConfig(it.value());
        auto results = benchmark.runFullBenchmarkSuite();
        m_currentResults.append(results);
        
        // Check for regressions
        for (const auto &result : results) {
            auto baseline = std::find_if(m_baselineResults.begin(), m_baselineResults.end(),
                                       [&result](const BenchmarkResult &base) {
                                           return base.testName == result.testName;
                                       });
            
            if (baseline != m_baselineResults.end()) {
                bool passed = !checkPerformanceRegression(result, *baseline);
                emit testCompleted(result.testName, passed);
            }
        }
    }
}

bool ModbusPerformanceTest::checkPerformanceRegression(const BenchmarkResult &current, 
                                                     const BenchmarkResult &baseline, 
                                                     double threshold)
{
    if (baseline.operationsPerSecond == 0) {
        return false;  // Cannot compare
    }
    
    double performanceRatio = current.operationsPerSecond / baseline.operationsPerSecond;
    double performanceDrop = 1.0 - performanceRatio;
    
    if (performanceDrop > threshold) {
        QString issue = QString("Performance regression in %1: %.1f%% drop")
                         .arg(current.testName)
                         .arg(performanceDrop * 100);
        m_regressionIssues.append(issue);
        emit regressionDetected(current.testName, performanceDrop);
        return true;
    }
    
    return false;
}

QString ModbusPerformanceTest::generateRegressionReport()
{
    QString report;
    report += "=== Performance Regression Test Report ===\n";
    report += QString("Generated: %1\n").arg(QDateTime::currentDateTime().toString());
    
    if (m_regressionIssues.isEmpty()) {
        report += "No performance regressions detected.\n";
    } else {
        report += QString("Found %1 performance regression(s):\n").arg(m_regressionIssues.size());
        for (const auto &issue : m_regressionIssues) {
            report += QString("  - %1\n").arg(issue);
        }
    }
    
    return report;
}

// =============================================================================
// ModbusResourceMonitor Implementation
// =============================================================================

ModbusResourceMonitor::ModbusResourceMonitor(QObject *parent)
    : QObject(parent)
    , m_monitorTimer(new QTimer(this))
{
    connect(m_monitorTimer, &QTimer::timeout, this, &ModbusResourceMonitor::updateResourceUsage);
    m_monitorTimer->setInterval(1000);  // Update every second
}

void ModbusResourceMonitor::startMonitoring()
{
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    resetCounters();
    m_monitorTimer->start();
}

void ModbusResourceMonitor::stopMonitoring()
{
    m_monitorTimer->stop();
}

QVariantMap ModbusResourceMonitor::getResourceUsage()
{
    return m_resourceData;
}

void ModbusResourceMonitor::resetCounters()
{
    m_peakMemoryUsage = 0;
    m_currentMemoryUsage = 0;
    m_threadCount = 0;
    m_resourceData.clear();
}

void ModbusResourceMonitor::updateResourceUsage()
{
    // Note: This is a simplified implementation
    // In a real implementation, you would use platform-specific APIs
    // to get actual memory usage, CPU usage, etc.
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsedTime = currentTime - m_startTime;
    
    // Simulate memory usage (in a real implementation, use actual APIs)
    m_currentMemoryUsage = QCoreApplication::applicationPid();  // Placeholder
    if (m_currentMemoryUsage > m_peakMemoryUsage) {
        m_peakMemoryUsage = m_currentMemoryUsage;
    }
    
    m_threadCount = QThread::idealThreadCount();
    
    m_resourceData["elapsedTimeMs"] = elapsedTime;
    m_resourceData["currentMemoryUsage"] = m_currentMemoryUsage;
    m_resourceData["peakMemoryUsage"] = m_peakMemoryUsage;
    m_resourceData["threadCount"] = m_threadCount;
    m_resourceData["timestamp"] = currentTime;
}
