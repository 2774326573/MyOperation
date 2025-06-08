#pragma once

#include "modbus_performance.h"
#include <QObject>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief 高性能Modbus管理器
 * 
 * 整合所有性能优化功能，提供统一的高级API
 */
class OptimizedModbusManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 优化配置结构
     */
    struct OptimizationConfig {
        // 连接池配置
        int maxConnections = 10;
        int connectionTimeoutMinutes = 30;
        
        // 缓存配置
        bool cacheEnabled = true;
        int defaultCacheTtlMs = 5000;
        int cacheCleanupIntervalMs = 60000;
        
        // 异步配置
        bool asyncEnabled = true;
        int maxAsyncOperations = 100;
        
        // 重连配置
        bool autoReconnectEnabled = true;
        int initialReconnectDelayMs = 1000;
        int maxReconnectDelayMs = 30000;
        double reconnectBackoffMultiplier = 2.0;
        int maxReconnectRetries = 10;
        
        // 批量操作配置
        bool batchOptimizationEnabled = true;
        int batchSizeLimit = 100;
        
        // 性能监控配置
        bool performanceMonitoringEnabled = true;
        
        OptimizationConfig() = default;
    };

    explicit OptimizedModbusManager(QObject* parent = nullptr);
    ~OptimizedModbusManager();

    /**
     * @brief 设置优化配置
     */
    void setOptimizationConfig(const OptimizationConfig& config);

    /**
     * @brief 获取优化配置
     */
    OptimizationConfig getOptimizationConfig() const;

    /**
     * @brief 保存配置到文件
     */
    void saveConfigToFile(const QString& filePath);

    /**
     * @brief 从文件加载配置
     */
    void loadConfigFromFile(const QString& filePath);

    // =============================================================================
    // 同步API (Synchronous API)
    // =============================================================================

    /**
     * @brief 连接设备（自动选择最优连接方式）
     * @param deviceId 设备ID
     * @param connectionString 连接字符串
     * @param slaveId 从站ID
     * @return 是否连接成功
     */
    bool connectDevice(const QString& deviceId, const QString& connectionString, int slaveId = 1);

    /**
     * @brief 断开设备连接
     */
    void disconnectDevice(const QString& deviceId);

    /**
     * @brief 智能读取保持寄存器（自动缓存）
     */
    bool readHoldingRegisters(const QString& deviceId, int address, int count, 
                            QVector<quint16>& values, int cacheTtlMs = -1);

    /**
     * @brief 智能读取输入寄存器（自动缓存）
     */
    bool readInputRegisters(const QString& deviceId, int address, int count, 
                          QVector<quint16>& values, int cacheTtlMs = -1);

    /**
     * @brief 智能读取线圈（自动缓存）
     */
    bool readCoils(const QString& deviceId, int address, int count, 
                  QVector<bool>& values, int cacheTtlMs = -1);

    /**
     * @brief 智能读取离散输入（自动缓存）
     */
    bool readDiscreteInputs(const QString& deviceId, int address, int count, 
                          QVector<bool>& values, int cacheTtlMs = -1);

    /**
     * @brief 写入单个寄存器
     */
    bool writeSingleRegister(const QString& deviceId, int address, quint16 value);

    /**
     * @brief 写入多个寄存器
     */
    bool writeMultipleRegisters(const QString& deviceId, int address, const QVector<quint16>& values);

    /**
     * @brief 写入单个线圈
     */
    bool writeSingleCoil(const QString& deviceId, int address, bool value);

    /**
     * @brief 写入多个线圈
     */
    bool writeMultipleCoils(const QString& deviceId, int address, const QVector<bool>& values);

    // =============================================================================
    // 异步API (Asynchronous API)
    // =============================================================================

    /**
     * @brief 异步读取保持寄存器
     */
    QString readHoldingRegistersAsync(const QString& deviceId, int address, int count,
                                    std::function<void(bool, const QVector<quint16>&)> callback,
                                    int cacheTtlMs = -1);

    /**
     * @brief 异步读取输入寄存器
     */
    QString readInputRegistersAsync(const QString& deviceId, int address, int count,
                                  std::function<void(bool, const QVector<quint16>&)> callback,
                                  int cacheTtlMs = -1);

    /**
     * @brief 异步读取线圈
     */
    QString readCoilsAsync(const QString& deviceId, int address, int count,
                         std::function<void(bool, const QVector<bool>&)> callback,
                         int cacheTtlMs = -1);

    /**
     * @brief 异步写入多个寄存器
     */
    QString writeMultipleRegistersAsync(const QString& deviceId, int address, 
                                      const QVector<quint16>& values,
                                      std::function<void(bool)> callback);

    /**
     * @brief 取消异步操作
     */
    void cancelAsyncOperation(const QString& operationId);

    // =============================================================================
    // 批量操作API (Batch Operations API)
    // =============================================================================

    /**
     * @brief 添加批量读取请求
     */
    void addBatchReadRequest(const QString& deviceId, int address, int count, 
                           ModbusManager::DataType dataType);

    /**
     * @brief 执行批量操作
     */
    QMap<QString, QVariant> executeBatchOperations();

    /**
     * @brief 清空批量请求队列
     */
    void clearBatchRequests();

    // =============================================================================
    // 监控和统计API (Monitoring and Statistics API)
    // =============================================================================

    /**
     * @brief 获取连接池统计信息
     */
    QMap<QString, QVariant> getConnectionPoolStats() const;

    /**
     * @brief 获取缓存统计信息
     */
    QMap<QString, QVariant> getCacheStats() const;

    /**
     * @brief 获取缓存统计信息 (别名)
     */
    QMap<QString, QVariant> getCacheStatistics() const;

    /**
     * @brief 获取性能统计信息
     */
    QMap<QString, QVariant> getPerformanceStats() const;

    /**
     * @brief 获取性能统计信息 (别名)
     */
    QMap<QString, QVariant> getPerformanceStatistics() const;

    /**
     * @brief 获取异步队列状态
     */
    QMap<QString, QVariant> getAsyncQueueStatus() const;

    /**
     * @brief 清除缓存
     */
    void clearCache();

    /**
     * @brief 设置配置 (别名)
     */
    void setConfiguration(const OptimizationConfig& config);

    /**
     * @brief 生成综合性能报告
     */
    QString generatePerformanceReport() const;

    /**
     * @brief 获取系统健康状态
     */
    QMap<QString, QVariant> getSystemHealth() const;

    // =============================================================================
    // 高级功能API (Advanced Features API)
    // =============================================================================

    /**
     * @brief 启用智能重连
     */
    void enableSmartReconnect(const QString& deviceId);

    /**
     * @brief 禁用智能重连
     */
    void disableSmartReconnect(const QString& deviceId);

    /**
     * @brief 预热连接池（预创建连接）
     */
    void warmupConnectionPool(const QStringList& deviceIds);

    /**
     * @brief 清理资源（清理过期连接和缓存）
     */
    void cleanup();

    /**
     * @brief 重置所有统计数据
     */
    void resetStatistics();

    /**
     * @brief 设置调试模式
     */
    void setDebugMode(bool enabled);

signals:
    void connectionEstablished(const QString& deviceId);
    void connectionLost(const QString& deviceId);
    void connectionRestored(const QString& deviceId);
    void operationCompleted(const QString& operationId, bool success);
    void asyncOperationCompleted(const QString& operationId, bool success, const QVariant& result);
    void performanceAlert(const QString& message);
    void cacheHit(const QString& key);
    void cacheMiss(const QString& key);

public slots:
    void onConnectionRestored(const QString& connectionId);
    void onConnectionFailed(const QString& connectionId, int retryCount);
    void onMaxRetriesReached(const QString& connectionId);

private:
    OptimizationConfig m_config;
    
    // 核心组件
    ModbusConnectionPool* m_connectionPool;
    ModbusDataCache* m_dataCache;
    AsyncModbusManager* m_asyncManager;
    SmartReconnectManager* m_reconnectManager;
    BatchOperationManager* m_batchManager;
    ModbusPerformanceMonitor* m_performanceMonitor;
    
    // 设备连接映射
    QMap<QString, QString> m_deviceConnections; // deviceId -> connectionString
    QMap<QString, int> m_deviceSlaveIds;        // deviceId -> slaveId
    
    // 内部方法
    QString generateCacheKey(const QString& deviceId, const QString& operation, 
                           int address, int count) const;
    void initializeComponents();
    void connectSignals();
    bool validateDeviceId(const QString& deviceId) const;
    void logOperation(const QString& operation, const QString& deviceId, 
                     int address, int count, bool success, qint64 durationMs);

    // 调试模式
    bool m_debugMode;
};

Q_DECLARE_METATYPE(OptimizedModbusManager::OptimizationConfig)
