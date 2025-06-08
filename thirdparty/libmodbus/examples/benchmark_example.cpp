/**
 * @file benchmark_example.cpp
 * @brief Example application demonstrating Modbus performance benchmarking
 */

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QDir>
#include "optimized_modbus_manager.h"
#include "modbus_benchmark.h"

class BenchmarkRunner : public QObject
{
    Q_OBJECT

public:
    explicit BenchmarkRunner(QObject *parent = nullptr)
        : QObject(parent)
        , m_manager(new OptimizedModbusManager(this))
        , m_benchmark(new ModbusBenchmark(this))
    {
        setupBenchmark();
        setupConnections();
    }

private slots:
    void runBenchmarks()
    {
        qDebug() << "=== Modbus Performance Benchmark Suite ===";
        
        // Setup device connection
        if (!connectToDevice()) {
            qDebug() << "Failed to connect to device, running simulation mode";
            // Continue with benchmark in simulation mode
        }
        
        // Run basic benchmarks
        runBasicBenchmarks();
        
        // Run optimization-specific benchmarks
        runOptimizationBenchmarks();
        
        // Run comprehensive suite
        runComprehensiveSuite();
        
        // Generate and save reports
        generateReports();
        
        qDebug() << "Benchmark suite completed successfully";
        QCoreApplication::quit();
    }

private:
    void setupBenchmark()
    {
        // Configure benchmark settings
        BenchmarkConfig config;
        config.iterations = 200;
        config.concurrentThreads = 2;
        config.enableCaching = true;
        config.enableConnectionPooling = true;
        config.warmupIterations = 20;
        config.registerStartAddress = 0;
        config.registerCount = 10;
        config.delayBetweenOperationsMs = 0;
        config.randomizeAddresses = false;
        
        m_benchmark->setConfig(config);
        m_benchmark->setOptimizedManager(m_manager);
    }
    
    void setupConnections()
    {
        // Connect benchmark progress signals
        connect(m_benchmark, &ModbusBenchmark::benchmarkProgress,
                this, [](int current, int total, const QString &testName) {
            if (current % 50 == 0 || current == total) {
                qDebug() << QString("Progress [%1]: %2/%3").arg(testName).arg(current).arg(total);
            }
        });
        
        connect(m_benchmark, &ModbusBenchmark::benchmarkCompleted,
                this, [](const BenchmarkResult &result) {
            qDebug() << QString("Completed [%1]: %2 ops/sec, %.1f%% success")
                        .arg(result.testName)
                        .arg(result.operationsPerSecond, 0, 'f', 1)
                        .arg(result.successRate * 100, 0, 'f', 1);
        });
    }
    
    bool connectToDevice()
    {
        // Try to connect to a Modbus device
        ModbusConnectionConfig config;
        config.type = ModbusConnectionType::Serial;
        config.serialPort = "COM3";  // Adjust as needed
        config.baudRate = 9600;
        config.parity = QSerialPort::NoParity;
        config.dataBits = QSerialPort::Data8;
        config.stopBits = QSerialPort::OneStop;
        config.slaveId = 1;
        
        qDebug() << "Attempting to connect to Modbus device...";
        bool connected = m_manager->connectDevice(config);
        
        if (connected) {
            qDebug() << "Successfully connected to Modbus device";
            
            // Configure optimizations for benchmarking
            QJsonObject optConfig;
            optConfig["cache"] = QJsonObject{
                {"maxSize", 1000},
                {"defaultTtl", 5000},
                {"cleanupInterval", 10000}
            };
            optConfig["connectionPool"] = QJsonObject{
                {"maxConnections", 4},
                {"connectionTimeout", 5000},
                {"keepAliveInterval", 30000}
            };
            optConfig["async"] = QJsonObject{
                {"workerThreads", 3},
                {"maxQueueSize", 200},
                {"operationTimeout", 10000}
            };
            optConfig["monitoring"] = QJsonObject{
                {"enabled", true},
                {"reportInterval", 5000}
            };
            
            m_manager->setConfiguration(optConfig);
            return true;
        }
        
        qWarning() << "Failed to connect to Modbus device";
        return false;
    }
    
    void runBasicBenchmarks()
    {
        qDebug() << "\n--- Running Basic Benchmarks ---";
        
        // Read operations benchmark
        auto readResult = m_benchmark->benchmarkReadOperations("Basic Read Operations");
        m_results.append(readResult);
        
        // Write operations benchmark
        auto writeResult = m_benchmark->benchmarkWriteOperations("Basic Write Operations");
        m_results.append(writeResult);
        
        // Mixed operations benchmark
        auto mixedResult = m_benchmark->benchmarkMixedOperations("Basic Mixed Operations");
        m_results.append(mixedResult);
        
        qDebug() << "Basic benchmarks completed";
    }
    
    void runOptimizationBenchmarks()
    {
        qDebug() << "\n--- Running Optimization Benchmarks ---";
        
        // Cache effectiveness benchmark
        auto cacheResult = m_benchmark->benchmarkCacheEffectiveness("Cache Effectiveness");
        m_results.append(cacheResult);
        
        // Connection pooling benchmark
        auto poolResult = m_benchmark->benchmarkConnectionPooling("Connection Pooling");
        m_results.append(poolResult);
        
        // Async operations benchmark
        auto asyncResult = m_benchmark->benchmarkAsyncOperations("Async Operations");
        m_results.append(asyncResult);
        
        qDebug() << "Optimization benchmarks completed";
    }
    
    void runComprehensiveSuite()
    {
        qDebug() << "\n--- Running Comprehensive Suite ---";
        
        // Run with different configurations
        BenchmarkConfig configs[] = {
            // Low load configuration
            {50, 1, true, true, 5, 0, 5, 0, false},
            // Medium load configuration
            {100, 2, true, true, 10, 0, 10, 0, false},
            // High load configuration
            {200, 4, true, true, 20, 0, 20, 0, false},
            // Random access pattern
            {100, 2, true, true, 10, 0, 10, 0, true}
        };
        
        QString configNames[] = {
            "Low Load", "Medium Load", "High Load", "Random Access"
        };
        
        for (int i = 0; i < 4; ++i) {
            qDebug() << QString("Running %1 configuration...").arg(configNames[i]);
            
            m_benchmark->setConfig(configs[i]);
            
            auto result = m_benchmark->benchmarkMixedOperations(
                QString("Comprehensive - %1").arg(configNames[i]));
            m_results.append(result);
        }
        
        qDebug() << "Comprehensive suite completed";
    }
    
    void generateReports()
    {
        qDebug() << "\n--- Generating Reports ---";
        
        // Create reports directory
        QDir dir;
        dir.mkpath("benchmark_reports");
        
        // Generate text report
        QString textReport = m_benchmark->generateReport(m_results, "text");
        
        QFile textFile("benchmark_reports/performance_report.txt");
        if (textFile.open(QIODevice::WriteOnly)) {
            textFile.write(textReport.toUtf8());
            textFile.close();
            qDebug() << "Text report saved to benchmark_reports/performance_report.txt";
        }
        
        // Generate JSON report
        QString jsonReport = m_benchmark->generateReport(m_results, "json");
        
        QFile jsonFile("benchmark_reports/performance_report.json");
        if (jsonFile.open(QIODevice::WriteOnly)) {
            jsonFile.write(jsonReport.toUtf8());
            jsonFile.close();
            qDebug() << "JSON report saved to benchmark_reports/performance_report.json";
        }
        
        // Print summary to console
        qDebug() << "\n=== Benchmark Summary ===";
        for (const auto &result : m_results) {
            qDebug() << QString("%1: %2 ops/sec (%.1f%% success)")
                        .arg(result.testName, -30)
                        .arg(result.operationsPerSecond, 8, 'f', 1)
                        .arg(result.successRate * 100, 0, 'f', 1);
        }
        
        // Show system health
        auto health = m_manager->getSystemHealth();
        qDebug() << "\n=== System Health ===";
        for (auto it = health.begin(); it != health.end(); ++it) {
            qDebug() << QString("%1: %2").arg(it.key(), -20).arg(it.value().toString());
        }
        
        // Show final statistics
        auto stats = m_manager->getPerformanceStatistics();
        qDebug() << "\n=== Final Performance Statistics ===";
        qDebug() << QString("Total Operations: %1").arg(stats.totalOperations);
        qDebug() << QString("Success Rate: %.1f%%").arg((double)stats.successfulOperations / stats.totalOperations * 100);
        qDebug() << QString("Average Response Time: %.2f ms").arg(stats.averageResponseTime);
        qDebug() << QString("Cache Hit Rate: %.1f%%").arg(stats.cacheHitRate * 100);
        qDebug() << QString("Connection Pool Utilization: %.1f%%").arg(stats.connectionPoolUtilization * 100);
    }

private:
    OptimizedModbusManager *m_manager;
    ModbusBenchmark *m_benchmark;
    QVector<BenchmarkResult> m_results;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "Starting Modbus Performance Benchmark";
    qDebug() << "=====================================";
    
    BenchmarkRunner runner;
    
    // Start benchmarks after event loop starts
    QTimer::singleShot(100, &runner, &BenchmarkRunner::runBenchmarks);
    
    return app.exec();
}

#include "benchmark_example.moc"
