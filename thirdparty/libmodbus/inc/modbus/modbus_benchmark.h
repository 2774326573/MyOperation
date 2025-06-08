/**
 * @file modbus_benchmark.h
 * @brief Benchmark utilities for measuring Modbus performance improvements
 */

#ifndef MODBUS_BENCHMARK_H
#define MODBUS_BENCHMARK_H

#include <QObject>
#include <QElapsedTimer>
#include <QVector>
#include <QVariantMap>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <memory>

class ModbusManagerInterface;
class OptimizedModbusManager;

/**
 * @brief Benchmark result data structure
 */
struct BenchmarkResult
{
    QString testName;
    int totalOperations = 0;
    int successfulOperations = 0;
    int failedOperations = 0;
    qint64 totalTimeMs = 0;
    double averageTimeMs = 0.0;
    double operationsPerSecond = 0.0;
    double successRate = 0.0;
    QVariantMap additionalMetrics;
    
    QJsonObject toJson() const;
    static BenchmarkResult fromJson(const QJsonObject &json);
};

/**
 * @brief Test configuration for benchmarks
 */
struct BenchmarkConfig
{
    int iterations = 100;
    int concurrentThreads = 1;
    bool enableCaching = true;
    bool enableConnectionPooling = true;
    int warmupIterations = 10;
    int registerStartAddress = 0;
    int registerCount = 10;
    int delayBetweenOperationsMs = 0;
    bool randomizeAddresses = false;
    
    QJsonObject toJson() const;
    static BenchmarkConfig fromJson(const QJsonObject &json);
};

/**
 * @brief Comprehensive benchmarking suite for Modbus operations
 */
class ModbusBenchmark : public QObject
{
    Q_OBJECT

public:
    explicit ModbusBenchmark(QObject *parent = nullptr);
    ~ModbusBenchmark();

    // Configuration
    void setConfig(const BenchmarkConfig &config);
    BenchmarkConfig getConfig() const { return m_config; }

    // Manager setup
    void setOriginalManager(ModbusManagerInterface *manager);
    void setOptimizedManager(OptimizedModbusManager *manager);

    // Benchmark execution
    BenchmarkResult benchmarkReadOperations(const QString &testName = "Read Operations");
    BenchmarkResult benchmarkWriteOperations(const QString &testName = "Write Operations");
    BenchmarkResult benchmarkMixedOperations(const QString &testName = "Mixed Operations");
    BenchmarkResult benchmarkCacheEffectiveness(const QString &testName = "Cache Effectiveness");
    BenchmarkResult benchmarkConnectionPooling(const QString &testName = "Connection Pooling");
    BenchmarkResult benchmarkAsyncOperations(const QString &testName = "Async Operations");

    // Comparison benchmarks
    BenchmarkResult compareManagers(const QString &operation, const QString &testName = "Manager Comparison");

    // Batch benchmarks
    QVector<BenchmarkResult> runFullBenchmarkSuite();

    // Reporting
    QString generateReport(const QVector<BenchmarkResult> &results, const QString &format = "text");
    void saveResults(const QVector<BenchmarkResult> &results, const QString &filename);
    QVector<BenchmarkResult> loadResults(const QString &filename);

signals:
    void benchmarkProgress(int current, int total, const QString &currentTest);
    void benchmarkCompleted(const BenchmarkResult &result);

private slots:
    void onAsyncOperationCompleted(const QString &operationId, bool success, const QVariant &result);

private:
    // Helper methods
    void setupBenchmarkEnvironment();
    void cleanupBenchmarkEnvironment();
    QVector<int> generateRandomAddresses(int count, int maxAddress = 1000);
    BenchmarkResult createEmptyResult(const QString &testName);
    void updateResultMetrics(BenchmarkResult &result);
    
    // Specific benchmark implementations
    BenchmarkResult runReadBenchmark();
    BenchmarkResult runWriteBenchmark();
    BenchmarkResult runMixedBenchmark();
    BenchmarkResult runCacheBenchmark();
    BenchmarkResult runPoolingBenchmark();
    BenchmarkResult runAsyncBenchmark();

private:
    BenchmarkConfig m_config;
    ModbusManagerInterface *m_originalManager = nullptr;
    OptimizedModbusManager *m_optimizedManager = nullptr;
    
    // Async operation tracking
    QHash<QString, QPair<qint64, bool>> m_pendingAsyncOps;
    int m_completedAsyncOps = 0;
    int m_totalAsyncOps = 0;
    mutable QMutex m_asyncMutex;
};

/**
 * @brief Automated performance regression testing
 */
class ModbusPerformanceTest : public QObject
{
    Q_OBJECT

public:
    explicit ModbusPerformanceTest(QObject *parent = nullptr);

    // Test configuration
    void addBenchmarkConfig(const QString &name, const BenchmarkConfig &config);
    void setBaselineResults(const QVector<BenchmarkResult> &baseline);

    // Test execution
    void runRegressionTests();
    bool checkPerformanceRegression(const BenchmarkResult &current, const BenchmarkResult &baseline, double threshold = 0.1);

    // Reporting
    QString generateRegressionReport();

signals:
    void regressionDetected(const QString &testName, double performanceDrop);
    void testCompleted(const QString &testName, bool passed);

private:
    QHash<QString, BenchmarkConfig> m_configs;
    QVector<BenchmarkResult> m_baselineResults;
    QVector<BenchmarkResult> m_currentResults;
    QStringList m_regressionIssues;
};

/**
 * @brief Memory and resource usage monitoring during benchmarks
 */
class ModbusResourceMonitor : public QObject
{
    Q_OBJECT

public:
    explicit ModbusResourceMonitor(QObject *parent = nullptr);

    void startMonitoring();
    void stopMonitoring();
    QVariantMap getResourceUsage();
    void resetCounters();

private slots:
    void updateResourceUsage();

private:
    QTimer *m_monitorTimer = nullptr;
    qint64 m_startTime = 0;
    qint64 m_peakMemoryUsage = 0;
    qint64 m_currentMemoryUsage = 0;
    int m_threadCount = 0;
    QVariantMap m_resourceData;
};

#endif // MODBUS_BENCHMARK_H
