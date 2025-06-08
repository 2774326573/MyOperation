#include "../../inc/modbus/optimized_modbus_manager.h"
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QElapsedTimer>
#include <QCoreApplication>

OptimizedModbusManager::OptimizedModbusManager(QObject* parent)
    : QObject(parent), m_debugMode(false)
{
    initializeComponents();
    connectSignals();
}

OptimizedModbusManager::~OptimizedModbusManager()
{
    // 组件会在父对象析构时自动删除
}

void OptimizedModbusManager::setOptimizationConfig(const OptimizationConfig& config)
{
    m_config = config;
    
    // 应用配置到各个组件
    if (m_connectionPool) {
        // 连接池配置在运行时不能更改，需要重新创建
    }
    
    if (m_dataCache && !config.cacheEnabled) {
        m_dataCache->clear();
    }
    
    if (m_reconnectManager) {
        SmartReconnectManager::ReconnectStrategy strategy;
        strategy.initialDelayMs = config.initialReconnectDelayMs;
        strategy.maxDelayMs = config.maxReconnectDelayMs;
        strategy.backoffMultiplier = config.reconnectBackoffMultiplier;
        strategy.maxRetries = config.maxReconnectRetries;
        strategy.jitterEnabled = true;
        m_reconnectManager->setReconnectStrategy(strategy);
    }
    
    if (m_batchManager) {
        m_batchManager->setBatchSizeLimit(config.batchSizeLimit);
        m_batchManager->setRequestOptimizationEnabled(config.batchOptimizationEnabled);
    }
}

OptimizedModbusManager::OptimizationConfig OptimizedModbusManager::getOptimizationConfig() const
{
    return m_config;
}

void OptimizedModbusManager::saveConfigToFile(const QString& filePath)
{
    QJsonObject configJson;
    
    // 连接池配置
    QJsonObject poolConfig;
    poolConfig["maxConnections"] = m_config.maxConnections;
    poolConfig["connectionTimeoutMinutes"] = m_config.connectionTimeoutMinutes;
    configJson["connectionPool"] = poolConfig;
    
    // 缓存配置
    QJsonObject cacheConfig;
    cacheConfig["enabled"] = m_config.cacheEnabled;
    cacheConfig["defaultTtlMs"] = m_config.defaultCacheTtlMs;
    cacheConfig["cleanupIntervalMs"] = m_config.cacheCleanupIntervalMs;
    configJson["cache"] = cacheConfig;
    
    // 异步配置
    QJsonObject asyncConfig;
    asyncConfig["enabled"] = m_config.asyncEnabled;
    asyncConfig["maxOperations"] = m_config.maxAsyncOperations;
    configJson["async"] = asyncConfig;
    
    // 重连配置
    QJsonObject reconnectConfig;
    reconnectConfig["enabled"] = m_config.autoReconnectEnabled;
    reconnectConfig["initialDelayMs"] = m_config.initialReconnectDelayMs;
    reconnectConfig["maxDelayMs"] = m_config.maxReconnectDelayMs;
    reconnectConfig["backoffMultiplier"] = m_config.reconnectBackoffMultiplier;
    reconnectConfig["maxRetries"] = m_config.maxReconnectRetries;
    configJson["reconnect"] = reconnectConfig;
    
    // 批量操作配置
    QJsonObject batchConfig;
    batchConfig["optimizationEnabled"] = m_config.batchOptimizationEnabled;
    batchConfig["sizeLimit"] = m_config.batchSizeLimit;
    configJson["batch"] = batchConfig;
    
    // 性能监控配置
    QJsonObject monitorConfig;
    monitorConfig["enabled"] = m_config.performanceMonitoringEnabled;
    configJson["monitoring"] = monitorConfig;
    
    QJsonDocument doc(configJson);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        qDebug() << "配置已保存到:" << filePath;
    } else {
        qWarning() << "无法保存配置文件:" << filePath;
    }
}

void OptimizedModbusManager::loadConfigFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        qWarning() << "配置文件格式错误:" << filePath;
        return;
    }
    
    QJsonObject configJson = doc.object();
    OptimizationConfig config;
    
    // 解析连接池配置
    if (configJson.contains("connectionPool")) {
        QJsonObject poolConfig = configJson["connectionPool"].toObject();
        config.maxConnections = poolConfig["maxConnections"].toInt(10);
        config.connectionTimeoutMinutes = poolConfig["connectionTimeoutMinutes"].toInt(30);
    }
    
    // 解析缓存配置
    if (configJson.contains("cache")) {
        QJsonObject cacheConfig = configJson["cache"].toObject();
        config.cacheEnabled = cacheConfig["enabled"].toBool(true);
        config.defaultCacheTtlMs = cacheConfig["defaultTtlMs"].toInt(5000);
        config.cacheCleanupIntervalMs = cacheConfig["cleanupIntervalMs"].toInt(60000);
    }
    
    // 解析异步配置
    if (configJson.contains("async")) {
        QJsonObject asyncConfig = configJson["async"].toObject();
        config.asyncEnabled = asyncConfig["enabled"].toBool(true);
        config.maxAsyncOperations = asyncConfig["maxOperations"].toInt(100);
    }
    
    // 解析重连配置
    if (configJson.contains("reconnect")) {
        QJsonObject reconnectConfig = configJson["reconnect"].toObject();
        config.autoReconnectEnabled = reconnectConfig["enabled"].toBool(true);
        config.initialReconnectDelayMs = reconnectConfig["initialDelayMs"].toInt(1000);
        config.maxReconnectDelayMs = reconnectConfig["maxDelayMs"].toInt(30000);
        config.reconnectBackoffMultiplier = reconnectConfig["backoffMultiplier"].toDouble(2.0);
        config.maxReconnectRetries = reconnectConfig["maxRetries"].toInt(10);
    }
    
    // 解析批量操作配置
    if (configJson.contains("batch")) {
        QJsonObject batchConfig = configJson["batch"].toObject();
        config.batchOptimizationEnabled = batchConfig["optimizationEnabled"].toBool(true);
        config.batchSizeLimit = batchConfig["sizeLimit"].toInt(100);
    }
    
    // 解析性能监控配置
    if (configJson.contains("monitoring")) {
        QJsonObject monitorConfig = configJson["monitoring"].toObject();
        config.performanceMonitoringEnabled = monitorConfig["enabled"].toBool(true);
    }
    
    setOptimizationConfig(config);
    qDebug() << "配置已从文件加载:" << filePath;
}

bool OptimizedModbusManager::connectDevice(const QString& deviceId, const QString& connectionString, int slaveId)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("CONNECT", deviceId, 0, 0, false, timer.elapsed());
        return false;
    }
    
    manager->setSlaveID(slaveId);
    
    // 保存设备连接信息
    m_deviceConnections[deviceId] = connectionString;
    m_deviceSlaveIds[deviceId] = slaveId;
    
    // 如果启用了智能重连，注册设备
    if (m_config.autoReconnectEnabled) {
        m_reconnectManager->registerConnection(deviceId, manager, connectionString);
        m_reconnectManager->startMonitoring(deviceId);
    }
    
    // 释放连接回池中
    m_connectionPool->releaseConnection(manager);
    
    logOperation("CONNECT", deviceId, 0, 0, true, timer.elapsed());
    emit connectionEstablished(deviceId);
    
    if (m_debugMode) {
        qDebug() << "设备连接成功:" << deviceId << "连接字符串:" << connectionString;
    }
    
    return true;
}

void OptimizedModbusManager::disconnectDevice(const QString& deviceId)
{
    if (m_config.autoReconnectEnabled) {
        m_reconnectManager->stopMonitoring(deviceId);
    }
    
    m_deviceConnections.remove(deviceId);
    m_deviceSlaveIds.remove(deviceId);
    
    emit connectionLost(deviceId);
    
    if (m_debugMode) {
        qDebug() << "设备已断开连接:" << deviceId;
    }
}

bool OptimizedModbusManager::readHoldingRegisters(const QString& deviceId, int address, int count, 
                                                 QVector<quint16>& values, int cacheTtlMs)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("read_holding_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // 检查缓存
    if (m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "holding", address, count);
        bool found = false;
        QVariant cachedValue = m_dataCache->getValue(cacheKey, &found);
        
        if (found) {
            values = cachedValue.value<QVector<quint16>>();
            emit cacheHit(cacheKey);
            
            if (m_config.performanceMonitoringEnabled) {
                m_performanceMonitor->recordOperationEnd(operationId, true);
            }
            
            logOperation("READ_HOLDING_CACHED", deviceId, address, count, true, timer.elapsed());
            return true;
        } else {
            emit cacheMiss(cacheKey);
        }
    }
    
    // 从设备读取
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("READ_HOLDING", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("READ_HOLDING", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->readHoldingRegisters(address, count, values);
    
    m_connectionPool->releaseConnection(manager);
    
    // 更新缓存
    if (success && m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "holding", address, count);
        int ttl = (cacheTtlMs > 0) ? cacheTtlMs : m_config.defaultCacheTtlMs;
        m_dataCache->setValue(cacheKey, QVariant::fromValue(values), ttl);
    }
    
    logOperation("READ_HOLDING", deviceId, address, count, success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

bool OptimizedModbusManager::readInputRegisters(const QString& deviceId, int address, int count, 
                                               QVector<quint16>& values, int cacheTtlMs)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("read_input_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // 检查缓存
    if (m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "input", address, count);
        bool found = false;
        QVariant cachedValue = m_dataCache->getValue(cacheKey, &found);
        
        if (found) {
            values = cachedValue.value<QVector<quint16>>();
            emit cacheHit(cacheKey);
            
            if (m_config.performanceMonitoringEnabled) {
                m_performanceMonitor->recordOperationEnd(operationId, true);
            }
            
            logOperation("READ_INPUT_CACHED", deviceId, address, count, true, timer.elapsed());
            return true;
        } else {
            emit cacheMiss(cacheKey);
        }
    }
    
    // 从设备读取
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("READ_INPUT", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("READ_INPUT", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->readInputRegisters(address, count, values);
    
    m_connectionPool->releaseConnection(manager);
    
    // 更新缓存
    if (success && m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "input", address, count);
        int ttl = (cacheTtlMs > 0) ? cacheTtlMs : m_config.defaultCacheTtlMs;
        m_dataCache->setValue(cacheKey, QVariant::fromValue(values), ttl);
    }
    
    logOperation("READ_INPUT", deviceId, address, count, success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

bool OptimizedModbusManager::readCoils(const QString& deviceId, int address, int count, 
                                      QVector<bool>& values, int cacheTtlMs)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("read_coils_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // 检查缓存
    if (m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "coils", address, count);
        bool found = false;
        QVariant cachedValue = m_dataCache->getValue(cacheKey, &found);
        
        if (found) {
            values = cachedValue.value<QVector<bool>>();
            emit cacheHit(cacheKey);
            
            if (m_config.performanceMonitoringEnabled) {
                m_performanceMonitor->recordOperationEnd(operationId, true);
            }
            
            logOperation("READ_COILS_CACHED", deviceId, address, count, true, timer.elapsed());
            return true;
        } else {
            emit cacheMiss(cacheKey);
        }
    }
    
    // 从设备读取
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("READ_COILS", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("READ_COILS", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->readCoils(address, count, values);
    
    m_connectionPool->releaseConnection(manager);
    
    // 更新缓存
    if (success && m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "coils", address, count);
        int ttl = (cacheTtlMs > 0) ? cacheTtlMs : m_config.defaultCacheTtlMs;
        m_dataCache->setValue(cacheKey, QVariant::fromValue(values), ttl);
    }
    
    logOperation("READ_COILS", deviceId, address, count, success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

bool OptimizedModbusManager::readDiscreteInputs(const QString& deviceId, int address, int count, 
                                               QVector<bool>& values, int cacheTtlMs)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("read_discrete_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // 检查缓存
    if (m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "discrete", address, count);
        bool found = false;
        QVariant cachedValue = m_dataCache->getValue(cacheKey, &found);
        
        if (found) {
            values = cachedValue.value<QVector<bool>>();
            emit cacheHit(cacheKey);
            
            if (m_config.performanceMonitoringEnabled) {
                m_performanceMonitor->recordOperationEnd(operationId, true);
            }
            
            logOperation("READ_DISCRETE_CACHED", deviceId, address, count, true, timer.elapsed());
            return true;
        } else {
            emit cacheMiss(cacheKey);
        }
    }
    
    // 从设备读取
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("READ_DISCRETE", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("READ_DISCRETE", deviceId, address, count, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->readDiscreteInputs(address, count, values);
    
    m_connectionPool->releaseConnection(manager);
    
    // 更新缓存
    if (success && m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "discrete", address, count);
        int ttl = (cacheTtlMs > 0) ? cacheTtlMs : m_config.defaultCacheTtlMs;
        m_dataCache->setValue(cacheKey, QVariant::fromValue(values), ttl);
    }
    
    logOperation("READ_DISCRETE", deviceId, address, count, success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

bool OptimizedModbusManager::writeSingleRegister(const QString& deviceId, int address, quint16 value)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("write_single_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("WRITE_SINGLE", deviceId, address, 1, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("WRITE_SINGLE", deviceId, address, 1, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->writeSingleRegister(address, value);
    
    m_connectionPool->releaseConnection(manager);
    
    // 写入成功后清除相关缓存
    if (success && m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "holding", address, 1);
        // 简单的缓存失效策略：清除可能受影响的缓存
        m_dataCache->setValue(cacheKey, QVariant(), 0); // TTL为0，立即过期
    }
    
    logOperation("WRITE_SINGLE", deviceId, address, 1, success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

bool OptimizedModbusManager::writeMultipleRegisters(const QString& deviceId, int address, const QVector<quint16>& values)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("write_multiple_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("WRITE_MULTIPLE", deviceId, address, values.size(), false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("WRITE_MULTIPLE", deviceId, address, values.size(), false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->writeMultipleRegisters(address, values);
    
    m_connectionPool->releaseConnection(manager);
    
    // 写入成功后清除相关缓存
    if (success && m_config.cacheEnabled) {
        // 清除可能受影响的缓存项
        for (int i = 0; i < values.size(); ++i) {
            QString cacheKey = generateCacheKey(deviceId, "holding", address + i, 1);
            m_dataCache->setValue(cacheKey, QVariant(), 0); // TTL为0，立即过期
        }
    }
    
    logOperation("WRITE_MULTIPLE", deviceId, address, values.size(), success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

bool OptimizedModbusManager::writeSingleCoil(const QString& deviceId, int address, bool value)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("write_coil_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("WRITE_COIL", deviceId, address, 1, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("WRITE_COIL", deviceId, address, 1, false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->writeSingleCoil(address, value);
    
    m_connectionPool->releaseConnection(manager);
    
    // 写入成功后清除相关缓存
    if (success && m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "coils", address, 1);
        m_dataCache->setValue(cacheKey, QVariant(), 0); // TTL为0，立即过期
    }
    
    logOperation("WRITE_COIL", deviceId, address, 1, success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

bool OptimizedModbusManager::writeMultipleCoils(const QString& deviceId, int address, const QVector<bool>& values)
{
    if (!validateDeviceId(deviceId)) {
        return false;
    }
    
    QString operationId = QString("write_multiple_coils_%1").arg(QDateTime::currentMSecsSinceEpoch());
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationStart(operationId);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        logOperation("WRITE_MULTIPLE_COILS", deviceId, address, values.size(), false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
    if (!manager) {
        logOperation("WRITE_MULTIPLE_COILS", deviceId, address, values.size(), false, timer.elapsed());
        if (m_config.performanceMonitoringEnabled) {
            m_performanceMonitor->recordOperationEnd(operationId, false);
        }
        return false;
    }
    
    int slaveId = m_deviceSlaveIds.value(deviceId, 1);
    manager->setSlaveID(slaveId);
    
    bool success = manager->writeMultipleCoils(address, values);
    
    m_connectionPool->releaseConnection(manager);
    
    // 写入成功后清除相关缓存
    if (success && m_config.cacheEnabled) {
        // 清除可能受影响的缓存项
        for (int i = 0; i < values.size(); ++i) {
            QString cacheKey = generateCacheKey(deviceId, "coils", address + i, 1);
            m_dataCache->setValue(cacheKey, QVariant(), 0); // TTL为0，立即过期
        }
    }
    
    logOperation("WRITE_MULTIPLE_COILS", deviceId, address, values.size(), success, timer.elapsed());
    
    if (m_config.performanceMonitoringEnabled) {
        m_performanceMonitor->recordOperationEnd(operationId, success);
    }
    
    return success;
}

// =============================================================================
// 异步API实现
// =============================================================================

QString OptimizedModbusManager::readHoldingRegistersAsync(const QString& deviceId, int address, int count,
                                                         std::function<void(bool, const QVector<quint16>&)> callback,
                                                         int cacheTtlMs)
{
    if (!m_config.asyncEnabled || !m_asyncManager) {
        // 异步功能未启用，回退到同步调用
        QVector<quint16> values;
        bool success = readHoldingRegisters(deviceId, address, count, values, cacheTtlMs);
        callback(success, values);
        return QString();
    }
    
    if (!validateDeviceId(deviceId)) {
        callback(false, QVector<quint16>());
        return QString();
    }
    
    // 检查缓存
    if (m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "holding", address, count);
        bool found = false;
        QVariant cachedValue = m_dataCache->getValue(cacheKey, &found);
        
        if (found) {
            QVector<quint16> values = cachedValue.value<QVector<quint16>>();
            emit cacheHit(cacheKey);
            
            // 异步调用回调
            QTimer::singleShot(0, [callback, values]() {
                callback(true, values);
            });
            
            return QString("cached");
        } else {
            emit cacheMiss(cacheKey);
        }
    }
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        callback(false, QVector<quint16>());
        return QString();
    }
    
    // 创建包装回调，处理缓存更新
    auto wrappedCallback = [this, deviceId, address, count, cacheTtlMs, callback](bool success, const QVector<quint16>& values) {
        if (success && m_config.cacheEnabled) {
            QString cacheKey = generateCacheKey(deviceId, "holding", address, count);
            int ttl = (cacheTtlMs > 0) ? cacheTtlMs : m_config.defaultCacheTtlMs;
            m_dataCache->setValue(cacheKey, QVariant::fromValue(values), ttl);
        }
        callback(success, values);
    };
    
    return m_asyncManager->readHoldingRegistersAsync(deviceId, connectionString, address, count, wrappedCallback);
}

QString OptimizedModbusManager::readInputRegistersAsync(const QString& deviceId, int address, int count,
                                                       std::function<void(bool, const QVector<quint16>&)> callback,
                                                       int cacheTtlMs)
{
    if (!m_config.asyncEnabled || !m_asyncManager) {
        // 异步功能未启用，回退到同步调用
        QVector<quint16> values;
        bool success = readInputRegisters(deviceId, address, count, values, cacheTtlMs);
        callback(success, values);
        return QString();
    }
    
    if (!validateDeviceId(deviceId)) {
        callback(false, QVector<quint16>());
        return QString();
    }
    
    // 检查缓存
    if (m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "input", address, count);
        bool found = false;
        QVariant cachedValue = m_dataCache->getValue(cacheKey, &found);
        
        if (found) {
            QVector<quint16> values = cachedValue.value<QVector<quint16>>();
            emit cacheHit(cacheKey);
            
            // 异步调用回调
            QTimer::singleShot(0, [callback, values]() {
                callback(true, values);
            });
            
            return QString("cached");
        } else {
            emit cacheMiss(cacheKey);
        }
    }
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        callback(false, QVector<quint16>());
        return QString();
    }
    
    // 由于AsyncModbusManager没有readInputRegistersAsync方法，我们需要创建一个通用的异步调用
    QString operationId = QString("async_read_input_%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    // 在工作线程中执行操作
    QtConcurrent::run([this, deviceId, address, count, cacheTtlMs, callback, connectionString]() {
        QVector<quint16> values;
        bool success = false;
        
        ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
        if (manager) {
            int slaveId = m_deviceSlaveIds.value(deviceId, 1);
            manager->setSlaveID(slaveId);
            success = manager->readInputRegisters(address, count, values);
            m_connectionPool->releaseConnection(manager);
        }
        
        // 更新缓存
        if (success && m_config.cacheEnabled) {
            QString cacheKey = generateCacheKey(deviceId, "input", address, count);
            int ttl = (cacheTtlMs > 0) ? cacheTtlMs : m_config.defaultCacheTtlMs;
            m_dataCache->setValue(cacheKey, QVariant::fromValue(values), ttl);
        }
        
        // 在主线程中调用回调
        QMetaObject::invokeMethod(this, [callback, success, values]() {
            callback(success, values);
        }, Qt::QueuedConnection);
    });
    
    return operationId;
}

QString OptimizedModbusManager::readCoilsAsync(const QString& deviceId, int address, int count,
                                              std::function<void(bool, const QVector<bool>&)> callback,
                                              int cacheTtlMs)
{
    if (!m_config.asyncEnabled || !m_asyncManager) {
        // 异步功能未启用，回退到同步调用
        QVector<bool> values;
        bool success = readCoils(deviceId, address, count, values, cacheTtlMs);
        callback(success, values);
        return QString();
    }
    
    if (!validateDeviceId(deviceId)) {
        callback(false, QVector<bool>());
        return QString();
    }
    
    // 检查缓存
    if (m_config.cacheEnabled) {
        QString cacheKey = generateCacheKey(deviceId, "coils", address, count);
        bool found = false;
        QVariant cachedValue = m_dataCache->getValue(cacheKey, &found);
        
        if (found) {
            QVector<bool> values = cachedValue.value<QVector<bool>>();
            emit cacheHit(cacheKey);
            
            // 异步调用回调
            QTimer::singleShot(0, [callback, values]() {
                callback(true, values);
            });
            
            return QString("cached");
        } else {
            emit cacheMiss(cacheKey);
        }
    }
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        callback(false, QVector<bool>());
        return QString();
    }
    
    // 创建包装回调，处理缓存更新
    auto wrappedCallback = [this, deviceId, address, count, cacheTtlMs, callback](bool success, const QVector<bool>& values) {
        if (success && m_config.cacheEnabled) {
            QString cacheKey = generateCacheKey(deviceId, "coils", address, count);
            int ttl = (cacheTtlMs > 0) ? cacheTtlMs : m_config.defaultCacheTtlMs;
            m_dataCache->setValue(cacheKey, QVariant::fromValue(values), ttl);
        }
        callback(success, values);
    };
    
    return m_asyncManager->readCoilsAsync(deviceId, connectionString, address, count, wrappedCallback);
}

QString OptimizedModbusManager::writeMultipleRegistersAsync(const QString& deviceId, int address, 
                                                           const QVector<quint16>& values,
                                                           std::function<void(bool)> callback)
{
    if (!m_config.asyncEnabled || !m_asyncManager) {
        // 异步功能未启用，回退到同步调用
        bool success = writeMultipleRegisters(deviceId, address, values);
        callback(success);
        return QString();
    }
    
    if (!validateDeviceId(deviceId)) {
        callback(false);
        return QString();
    }
    
    QString connectionString = m_deviceConnections.value(deviceId);
    if (connectionString.isEmpty()) {
        callback(false);
        return QString();
    }
    
    // 创建包装回调，处理缓存清理
    auto wrappedCallback = [this, deviceId, address, values, callback](bool success) {
        if (success && m_config.cacheEnabled) {
            // 清除可能受影响的缓存项
            for (int i = 0; i < values.size(); ++i) {
                QString cacheKey = generateCacheKey(deviceId, "holding", address + i, 1);
                m_dataCache->setValue(cacheKey, QVariant(), 0); // TTL为0，立即过期
            }
        }
        callback(success);
    };
    
    return m_asyncManager->writeMultipleRegistersAsync(deviceId, connectionString, address, values, wrappedCallback);
}

void OptimizedModbusManager::cancelAsyncOperation(const QString& operationId)
{
    if (m_asyncManager) {
        m_asyncManager->cancelOperation(operationId);
    }
}

// =============================================================================
// 批量操作API实现
// =============================================================================

void OptimizedModbusManager::addBatchReadRequest(const QString& deviceId, int address, int count, 
                                                ModbusManager::DataType dataType)
{
    if (!m_batchManager) {
        return;
    }
    
    BatchOperationManager::BatchRequest request;
    request.deviceId = deviceId;
    request.connectionString = m_deviceConnections.value(deviceId);
    request.startAddress = address;
    request.count = count;
    request.dataType = dataType;
    request.timestamp = QDateTime::currentDateTime();
    
    m_batchManager->addReadRequest(request);
}

QMap<QString, QVariant> OptimizedModbusManager::executeBatchOperations()
{
    if (!m_batchManager) {
        return QMap<QString, QVariant>();
    }
    
    auto responses = m_batchManager->executeBatch();
    
    QMap<QString, QVariant> results;
    for (auto it = responses.begin(); it != responses.end(); ++it) {
        QVariantMap responseMap;
        responseMap["success"] = it.value().success;
        responseMap["values"] = QVariant::fromValue(it.value().values);
        responseMap["errorMessage"] = it.value().errorMessage;
        responseMap["processingTimeMs"] = it.value().processingTimeMs;
        
        results[it.key()] = responseMap;
    }
    
    return results;
}

void OptimizedModbusManager::clearBatchRequests()
{
    // BatchOperationManager没有提供清空方法，我们执行一次空批量操作来清空
    if (m_batchManager) {
        m_batchManager->executeBatch();
    }
}

// =============================================================================
// 监控和统计API实现
// =============================================================================

QMap<QString, QVariant> OptimizedModbusManager::getConnectionPoolStats() const
{
    if (m_connectionPool) {
        return m_connectionPool->getPoolStatistics();
    }
    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> OptimizedModbusManager::getCacheStats() const
{
    if (m_dataCache) {
        return m_dataCache->getCacheStatistics();
    }
    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> OptimizedModbusManager::getCacheStatistics() const
{
    return getCacheStats();
}

QMap<QString, QVariant> OptimizedModbusManager::getPerformanceStats() const
{
    if (m_performanceMonitor) {
        return m_performanceMonitor->getPerformanceStatistics();
    }
    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> OptimizedModbusManager::getPerformanceStatistics() const
{
    return getPerformanceStats();
}

QMap<QString, QVariant> OptimizedModbusManager::getAsyncQueueStatus() const
{
    if (m_asyncManager) {
        return m_asyncManager->getQueueStatus();
    }
    return QMap<QString, QVariant>();
}

void OptimizedModbusManager::clearCache()
{
    if (m_dataCache) {
        m_dataCache->clear();
        if (m_debugMode) {
            qDebug() << "缓存已清空";
        }
    }
}

void OptimizedModbusManager::setConfiguration(const OptimizedModbusManager::OptimizationConfig& config)
{
    setOptimizationConfig(config);
}

// =============================================================================
// 槽函数实现
// =============================================================================

void OptimizedModbusManager::onConnectionRestored(const QString& connectionId)
{
    emit connectionRestored(connectionId);
    
    if (m_debugMode) {
        qDebug() << "连接已恢复:" << connectionId;
    }
}

void OptimizedModbusManager::onConnectionFailed(const QString& connectionId, int retryCount)
{
    if (m_debugMode) {
        qDebug() << "连接失败:" << connectionId << "重试次数:" << retryCount;
    }
    
    if (retryCount > 3) {
        emit performanceAlert(QString("设备 %1 连接失败，重试次数: %2").arg(connectionId).arg(retryCount));
    }
}

void OptimizedModbusManager::onMaxRetriesReached(const QString& connectionId)
{
    emit performanceAlert(QString("设备 %1 达到最大重试次数，请检查连接").arg(connectionId));
    
    if (m_debugMode) {
        qDebug() << "设备达到最大重试次数:" << connectionId;
    }
}

// =============================================================================
// 私有方法实现
// =============================================================================

QString OptimizedModbusManager::generateCacheKey(const QString& deviceId, const QString& operation, 
                                                int address, int count) const
{
    return QString("%1_%2_%3_%4").arg(deviceId, operation).arg(address).arg(count);
}

void OptimizedModbusManager::initializeComponents()
{
    // 初始化连接池
    m_connectionPool = new ModbusConnectionPool(m_config.maxConnections, this);
    
    // 初始化数据缓存
    m_dataCache = new ModbusDataCache(this);
    
    // 初始化异步管理器
    m_asyncManager = new AsyncModbusManager(m_connectionPool, this);
    
    // 初始化智能重连管理器
    m_reconnectManager = new SmartReconnectManager(this);
    
    // 初始化批量操作管理器
    m_batchManager = new BatchOperationManager(m_connectionPool, this);
    
    // 初始化性能监控器
    m_performanceMonitor = new ModbusPerformanceMonitor(this);
}

void OptimizedModbusManager::connectSignals()
{
    // 连接智能重连管理器信号
    connect(m_reconnectManager, &SmartReconnectManager::connectionRestored,
            this, &OptimizedModbusManager::onConnectionRestored);
    connect(m_reconnectManager, &SmartReconnectManager::connectionFailed,
            this, &OptimizedModbusManager::onConnectionFailed);
    connect(m_reconnectManager, &SmartReconnectManager::maxRetriesReached,
            this, &OptimizedModbusManager::onMaxRetriesReached);
}

bool OptimizedModbusManager::validateDeviceId(const QString& deviceId) const
{
    if (deviceId.isEmpty()) {
        qWarning() << "设备ID不能为空";
        return false;
    }
    
    if (!m_deviceConnections.contains(deviceId)) {
        qWarning() << "设备未连接:" << deviceId;
        return false;
    }
    
    return true;
}

void OptimizedModbusManager::logOperation(const QString& operation, const QString& deviceId, 
                                        int address, int count, bool success, qint64 durationMs)
{
    if (m_debugMode) {
        qDebug() << QString("[%1] %2 设备:%3 地址:%4 数量:%5 结果:%6 耗时:%7ms")
                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                    .arg(operation, deviceId)
                    .arg(address)
                    .arg(count)
                    .arg(success ? "成功" : "失败")
                    .arg(durationMs);
    }
    
    // 性能警报
    if (durationMs > 1000) {
        emit performanceAlert(QString("操作 %1 响应时间过长: %2ms").arg(operation).arg(durationMs));
    }
}

void OptimizedModbusManager::warmupConnectionPool(const QStringList& deviceIds)
{
    if (m_debugMode) {
        qDebug() << "预热连接池，设备列表:" << deviceIds;
    }
    
    for (const QString& deviceId : deviceIds) {
        if (m_deviceConnections.contains(deviceId)) {
            // 为每个设备创建一个预热连接
            QString connectionString = m_deviceConnections.value(deviceId);
            int slaveId = m_deviceSlaveIds.value(deviceId, 1);
              if (m_connectionPool) {
                // 预热连接池中的连接
                auto connection = m_connectionPool->acquireConnection(deviceId, connectionString);
                if (connection) {
                    // 释放连接回池中
                    m_connectionPool->releaseConnection(connection);
                    
                    if (m_debugMode) {
                        qDebug() << "设备" << deviceId << "连接预热完成";
                    }
                } else {
                    qWarning() << "设备" << deviceId << "连接预热失败";
                }
            }
        } else {
            qWarning() << "设备" << deviceId << "未在设备列表中，跳过预热";
        }
    }
    
    if (m_debugMode) {
        qDebug() << "连接池预热完成";
    }
}

void OptimizedModbusManager::resetStatistics()
{
    if (m_debugMode) {
        qDebug() << "重置所有统计数据";
    }
    
    // 重置连接池统计
    if (m_connectionPool) {
        m_connectionPool->resetStatistics();
    }
    
    // 重置缓存统计
    if (m_dataCache) {
        m_dataCache->resetStatistics();
    }
    
    // 重置异步管理器统计
    if (m_asyncManager) {
        m_asyncManager->resetStatistics();
    }
    
    // 重置重连管理器统计
    if (m_reconnectManager) {
        m_reconnectManager->resetStatistics();
    }
    
    // 重置批量操作管理器统计
    if (m_batchManager) {
        m_batchManager->resetStatistics();
    }
    
    // 重置性能监控器统计
    if (m_performanceMonitor) {
        m_performanceMonitor->reset();
    }
    
    if (m_debugMode) {
        qDebug() << "统计数据重置完成";
    }
}
