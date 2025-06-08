#include "../../inc/modbus/modbus_performance.h"
#include <QSerialPortInfo>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QDebug>

#ifdef max
#undef max
#endif
#ifdef min  
#undef min
#endif

// 全局内存池实例
MemoryPool<quint16> g_registerPool(50, 125);  // 50个池，每个125个寄存器
MemoryPool<bool> g_coilPool(50, 2000);        // 50个池，每个2000个线圈

// =============================================================================
// ModbusConnectionPool Implementation
// =============================================================================

ModbusConnectionPool::ModbusConnectionPool(int maxConnections, QObject* parent)
    : QObject(parent), m_maxConnections(maxConnections)
{
    // 启动定期清理计时器
    m_cleanupTimer = new QTimer(this);
    connect(m_cleanupTimer, &QTimer::timeout, this, &ModbusConnectionPool::performPeriodicCleanup);
    m_cleanupTimer->start(300000); // 5分钟清理一次
}

ModbusConnectionPool::~ModbusConnectionPool()
{
    QMutexLocker locker(&m_poolMutex);
    
    for (auto& conn : m_connections) {
        if (conn.manager) {
            conn.manager->disconnect();
            conn.manager->deleteLater();
        }
    }
    m_connections.clear();
}

ModbusManager* ModbusConnectionPool::acquireConnection(const QString& deviceId, const QString& connectionString)
{
    QMutexLocker locker(&m_poolMutex);
    
    // 查找现有的空闲连接
    for (auto& conn : m_connections) {
        if (conn.deviceId == deviceId && !conn.inUse && conn.connectionString == connectionString) {
            if (isConnectionValid(conn.manager)) {
                conn.inUse = true;
                conn.lastUsed = QDateTime::currentDateTime();
                conn.useCount++;
                return conn.manager;
            } else {
                // 连接已失效，重新创建
                conn.manager->deleteLater();
                conn.manager = createConnection(connectionString);
                if (conn.manager) {
                    conn.inUse = true;
                    conn.lastUsed = QDateTime::currentDateTime();
                    conn.useCount++;
                    return conn.manager;
                }
            }
        }
    }
    
    // 如果没有可用连接且未达到最大限制，创建新连接
    if (m_connections.size() < m_maxConnections) {
        ModbusManager* newManager = createConnection(connectionString);
        if (newManager) {
            ConnectionInfo newConn;
            newConn.deviceId = deviceId;
            newConn.manager = newManager;
            newConn.connectionString = connectionString;
            newConn.inUse = true;
            newConn.lastUsed = QDateTime::currentDateTime();
            newConn.useCount = 1;
            
            m_connections.append(newConn);
            return newManager;
        }
    }
    
    return nullptr; // 连接池已满或创建失败
}

void ModbusConnectionPool::releaseConnection(ModbusManager* manager)
{
    QMutexLocker locker(&m_poolMutex);
    
    for (auto& conn : m_connections) {
        if (conn.manager == manager) {
            conn.inUse = false;
            conn.lastUsed = QDateTime::currentDateTime();
            break;
        }
    }
}

QMap<QString, QVariant> ModbusConnectionPool::getPoolStatistics() const
{
    QMutexLocker locker(&m_poolMutex);
    
    QMap<QString, QVariant> stats;
    stats["totalConnections"] = m_connections.size();
    stats["maxConnections"] = m_maxConnections;
    
    int activeConnections = 0;
    int totalUseCount = 0;
    
    for (const auto& conn : m_connections) {
        if (conn.inUse) activeConnections++;
        totalUseCount += conn.useCount;
    }
    
    stats["activeConnections"] = activeConnections;
    stats["idleConnections"] = m_connections.size() - activeConnections;
    stats["totalUseCount"] = totalUseCount;
    stats["averageUseCount"] = m_connections.isEmpty() ? 0 : (double)totalUseCount / m_connections.size();
    
    return stats;
}

void ModbusConnectionPool::cleanupExpiredConnections(int timeoutMinutes)
{
    QMutexLocker locker(&m_poolMutex);
    
    QDateTime now = QDateTime::currentDateTime();
    auto it = m_connections.begin();
    
    while (it != m_connections.end()) {
        if (!it->inUse && it->lastUsed.addSecs(timeoutMinutes * 60) < now) {
            qDebug() << "清理过期连接:" << it->deviceId;
            it->manager->disconnect();
            it->manager->deleteLater();
            it = m_connections.erase(it);
        } else {
            ++it;
        }
    }
}

void ModbusConnectionPool::performPeriodicCleanup()
{
    cleanupExpiredConnections(30); // 清理30分钟未使用的连接
}

ModbusManager* ModbusConnectionPool::createConnection(const QString& connectionString)
{
    ModbusManager* manager = new ModbusManager(this);
    
    // 解析连接字符串
    QStringList parts = connectionString.split(':');
    if (parts.size() < 2) {
        delete manager;
        return nullptr;
    }
    
    QString protocol = parts[0].toUpper();
    
    if (protocol == "RTU" && parts.size() >= 6) {
        // RTU连接: "RTU:COM1:9600:8:N:1"
        QString port = parts[1];
        int baudRate = parts[2].toInt();
        int dataBits = parts[3].toInt();
        char parity = parts[4].at(0).toLatin1();
        int stopBits = parts[5].toInt();
        
        if (manager->connectRTU(port, baudRate, dataBits, parity, stopBits)) {
            return manager;
        }
    } else if (protocol == "TCP" && parts.size() >= 3) {
        // TCP连接: "TCP:192.168.1.100:502"
        QString ip = parts[1];
        int port = parts[2].toInt();
        
        if (manager->connectTCP(ip, port)) {
            return manager;
        }
    }
    
    delete manager;
    return nullptr;
}

bool ModbusConnectionPool::isConnectionValid(ModbusManager* manager)
{
    if (!manager || !manager->isConnected()) {
        return false;
    }
    
    // 尝试读取一个寄存器来测试连接
    QVector<quint16> testData;
    return manager->readHoldingRegisters(0, 1, testData);
}

// =============================================================================
// ModbusDataCache Implementation
// =============================================================================

ModbusDataCache::ModbusDataCache(QObject* parent)
    : QObject(parent), m_totalHits(0), m_totalMisses(0)
{
    // 启动定期清理计时器
    m_cleanupTimer = new QTimer(this);
    connect(m_cleanupTimer, &QTimer::timeout, this, &ModbusDataCache::performPeriodicCleanup);
    m_cleanupTimer->start(60000); // 1分钟清理一次
}

ModbusDataCache::~ModbusDataCache()
{
    clear();
}

void ModbusDataCache::setValue(const QString& key, const QVariant& value, int ttlMsec)
{
    QMutexLocker locker(&m_cacheMutex);
    
    CacheEntry entry;
    entry.value = value;
    entry.timestamp = QDateTime::currentDateTime();
    entry.ttl = ttlMsec;
    entry.hitCount = 0;
    entry.key = key;
    
    m_cache[key] = entry;
}

QVariant ModbusDataCache::getValue(const QString& key, bool* found)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (found) *found = false;
      if (!m_cache.contains(key)) {
        m_totalMisses.fetchAndAddAcquire(1);
        return QVariant();
    }
    
    CacheEntry& entry = m_cache[key];
    
    // 检查是否过期
    if (entry.timestamp.addMSecs(entry.ttl) < QDateTime::currentDateTime()) {        m_cache.remove(key);
        m_totalMisses.fetchAndAddAcquire(1);
        return QVariant();
    }
    
    entry.hitCount++;
    m_totalHits.fetchAndAddAcquire(1);
    
    if (found) *found = true;
    return entry.value;
}

bool ModbusDataCache::isValid(const QString& key) const
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (!m_cache.contains(key)) {
        return false;
    }
    
    const CacheEntry& entry = m_cache[key];
    return entry.timestamp.addMSecs(entry.ttl) >= QDateTime::currentDateTime();
}

void ModbusDataCache::cleanupExpiredEntries()
{
    QMutexLocker locker(&m_cacheMutex);
    
    QDateTime now = QDateTime::currentDateTime();
    auto it = m_cache.begin();
    
    while (it != m_cache.end()) {
        if (it->timestamp.addMSecs(it->ttl) < now) {
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }
}

QMap<QString, QVariant> ModbusDataCache::getCacheStatistics() const
{
    QMutexLocker locker(&m_cacheMutex);
      QMap<QString, QVariant> stats;
    stats["totalEntries"] = m_cache.size();
    stats["totalHits"] = m_totalHits.load();
    stats["totalMisses"] = m_totalMisses.load();
    
    int totalHits = m_totalHits.load();
    int totalMisses = m_totalMisses.load();
    int totalRequests = totalHits + totalMisses;
    
    stats["hitRate"] = totalRequests > 0 ? (double)totalHits / totalRequests : 0.0;
    
    // 计算热点数据
    QList<QPair<QString, int>> hotEntries;
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
        hotEntries.append({it->key, it->hitCount});
    }
    
    std::sort(hotEntries.begin(), hotEntries.end(), 
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });
    
    QStringList topKeys;
    for (int i = 0; i < qMin(5, hotEntries.size()); ++i) {
        topKeys.append(QString("%1(%2)").arg(hotEntries[i].first).arg(hotEntries[i].second));
    }
    stats["topHitKeys"] = topKeys;
    
    return stats;
}

void ModbusDataCache::clear()
{    QMutexLocker locker(&m_cacheMutex);
    m_cache.clear();
    m_totalHits.store(0);
    m_totalMisses.store(0);
}

void ModbusDataCache::performPeriodicCleanup()
{
    cleanupExpiredEntries();
}

// =============================================================================
// AsyncModbusManager Implementation
// =============================================================================

AsyncModbusManager::AsyncModbusManager(ModbusConnectionPool* pool, QObject* parent)
    : QObject(parent), m_connectionPool(pool), m_running(true), m_operationIdCounter(0)
{
    // 创建工作线程
    m_workerThread = new QThread(this);
    this->moveToThread(m_workerThread);
    m_workerThread->start();
    
    // 启动操作处理循环
    QTimer::singleShot(0, this, &AsyncModbusManager::processOperations);
}

AsyncModbusManager::~AsyncModbusManager()
{
    m_running = false;
    m_queueCondition.wakeAll();
    m_workerThread->quit();
    m_workerThread->wait(5000);
}

QString AsyncModbusManager::readHoldingRegistersAsync(const QString& deviceId, const QString& connectionString,
                                                    int address, int count,
                                                    std::function<void(bool, const QVector<quint16>&)> callback)
{
    QString operationId = QString("read_holding_%1").arg(m_operationIdCounter.fetchAndAddAcquire(1));
    
    AsyncOperation operation;
    operation.operationId = operationId;
    operation.timestamp = QDateTime::currentDateTime();
    
    operation.operation = [this, deviceId, connectionString, address, count]() {
        ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
        if (!manager) {
            return QVariant::fromValue(QPair<bool, QVector<quint16>>(false, QVector<quint16>()));
        }
        
        QVector<quint16> values;
        bool success = manager->readHoldingRegisters(address, count, values);
        
        m_connectionPool->releaseConnection(manager);
        return QVariant::fromValue(QPair<bool, QVector<quint16>>(success, values));
    };
    
    operation.callback = [callback](bool success, const QVariant& result) {
        auto pair = result.value<QPair<bool, QVector<quint16>>>();
        callback(pair.first, pair.second);
    };
    
    QMutexLocker locker(&m_queueMutex);
    m_operationQueue.enqueue(operation);
    m_queueCondition.wakeOne();
    
    return operationId;
}

QString AsyncModbusManager::writeMultipleRegistersAsync(const QString& deviceId, const QString& connectionString,
                                                      int address, const QVector<quint16>& values,
                                                      std::function<void(bool)> callback)
{
    QString operationId = QString("write_multiple_%1").arg(m_operationIdCounter.fetchAndAddAcquire(1));
    
    AsyncOperation operation;
    operation.operationId = operationId;
    operation.timestamp = QDateTime::currentDateTime();
    
    operation.operation = [this, deviceId, connectionString, address, values]() {
        ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
        if (!manager) {
            return QVariant(false);
        }
        
        bool success = manager->writeMultipleRegisters(address, values);
        
        m_connectionPool->releaseConnection(manager);
        return QVariant(success);
    };
    
    operation.callback = [callback](bool success, const QVariant& result) {
        callback(result.toBool());
    };
    
    QMutexLocker locker(&m_queueMutex);
    m_operationQueue.enqueue(operation);
    m_queueCondition.wakeOne();
    
    return operationId;
}

QString AsyncModbusManager::readCoilsAsync(const QString& deviceId, const QString& connectionString,
                                         int address, int count,
                                         std::function<void(bool, const QVector<bool>&)> callback)
{
    QString operationId = QString("read_coils_%1").arg(m_operationIdCounter.fetchAndAddAcquire(1));
    
    AsyncOperation operation;
    operation.operationId = operationId;
    operation.timestamp = QDateTime::currentDateTime();
    
    operation.operation = [this, deviceId, connectionString, address, count]() {
        ModbusManager* manager = m_connectionPool->acquireConnection(deviceId, connectionString);
        if (!manager) {
            return QVariant::fromValue(QPair<bool, QVector<bool>>(false, QVector<bool>()));
        }
        
        QVector<bool> values;
        bool success = manager->readCoils(address, count, values);
        
        m_connectionPool->releaseConnection(manager);
        return QVariant::fromValue(QPair<bool, QVector<bool>>(success, values));
    };
    
    operation.callback = [callback](bool success, const QVariant& result) {
        auto pair = result.value<QPair<bool, QVector<bool>>>();
        callback(pair.first, pair.second);
    };
    
    QMutexLocker locker(&m_queueMutex);
    m_operationQueue.enqueue(operation);
    m_queueCondition.wakeOne();
    
    return operationId;
}

void AsyncModbusManager::cancelOperation(const QString& operationId)
{
    QMutexLocker locker(&m_queueMutex);
    
    // 从队列中移除指定的操作
    for (int i = 0; i < m_operationQueue.size(); ++i) {
        if (m_operationQueue.at(i).operationId == operationId) {
            // 由于QQueue没有提供removeAt方法，我们需要重建队列
            QQueue<AsyncOperation> newQueue;
            for (int j = 0; j < m_operationQueue.size(); ++j) {
                if (j != i) {
                    newQueue.enqueue(m_operationQueue.at(j));
                }
            }
            m_operationQueue = newQueue;
            break;
        }
    }
}

int AsyncModbusManager::getPendingOperationsCount() const
{
    QMutexLocker locker(&m_queueMutex);
    return m_operationQueue.size();
}

QMap<QString, QVariant> AsyncModbusManager::getQueueStatus() const
{
    QMutexLocker locker(&m_queueMutex);
    
    QMap<QString, QVariant> status;
    status["pendingOperations"] = m_operationQueue.size();
    status["running"] = m_running;
    status["operationIdCounter"] = m_operationIdCounter.load();
      // 添加队列中最旧的操作信息
    if (!m_operationQueue.isEmpty()) {
        const auto& oldestOp = m_operationQueue.first();
        status["oldestOperationAge"] = oldestOp.timestamp.msecsTo(QDateTime::currentDateTime());
    } else {
        status["oldestOperationAge"] = 0;
    }
    
    return status;
}

void AsyncModbusManager::processOperations()
{
    while (m_running) {
        QMutexLocker locker(&m_queueMutex);
        
        // 等待队列中有操作
        while (m_operationQueue.isEmpty() && m_running) {
            m_queueCondition.wait(&m_queueMutex);
        }
        
        if (!m_running) break;
        
        // 执行操作
        AsyncOperation operation = m_operationQueue.dequeue();
        locker.unlock();
        
        // 执行实际操作
        QVariant result = operation.operation();
        
        // 在主线程中调用回调
        QMetaObject::invokeMethod(this, [operation, result]() {
            operation.callback(true, result);
        }, Qt::QueuedConnection);
    }
}

// =============================================================================
// SmartReconnectManager Implementation
// =============================================================================

SmartReconnectManager::SmartReconnectManager(QObject* parent)
    : QObject(parent)
{
    // 设置默认重连策略
    m_strategy.initialDelayMs = 1000;      // 1秒
    m_strategy.maxDelayMs = 30000;         // 30秒
    m_strategy.backoffMultiplier = 2.0;    // 指数退避
    m_strategy.maxRetries = 10;            // 最大重试10次
    m_strategy.jitterEnabled = true;       // 启用抖动
    
    // 启动监控定时器
    m_monitorTimer = new QTimer(this);
    connect(m_monitorTimer, &QTimer::timeout, this, &SmartReconnectManager::checkConnections);
    m_monitorTimer->start(5000); // 每5秒检查一次
}

SmartReconnectManager::~SmartReconnectManager()
{
    QMutexLocker locker(&m_connectionsMutex);
    
    for (auto& state : m_connections) {
        if (state.reconnectTimer) {
            state.reconnectTimer->stop();
            state.reconnectTimer->deleteLater();
        }
    }
}

void SmartReconnectManager::setReconnectStrategy(const ReconnectStrategy& strategy)
{
    m_strategy = strategy;
}

void SmartReconnectManager::registerConnection(const QString& connectionId, ModbusManager* manager,
                                             const QString& connectionString)
{
    QMutexLocker locker(&m_connectionsMutex);
    
    ConnectionState state;
    state.connectionId = connectionId;
    state.manager = manager;
    state.connectionString = connectionString;
    state.isConnected = manager->isConnected();
    state.currentRetryCount = 0;
    state.currentDelayMs = m_strategy.initialDelayMs;
    state.reconnectTimer = new QTimer(this);
    
    connect(state.reconnectTimer, &QTimer::timeout, this, &SmartReconnectManager::performReconnect);
    
    m_connections[connectionId] = state;
}

void SmartReconnectManager::startMonitoring(const QString& connectionId)
{
    QMutexLocker locker(&m_connectionsMutex);
    
    if (m_connections.contains(connectionId)) {
        m_connections[connectionId].isConnected = true;
        m_connections[connectionId].currentRetryCount = 0;
        m_connections[connectionId].currentDelayMs = m_strategy.initialDelayMs;
    }
}

void SmartReconnectManager::stopMonitoring(const QString& connectionId)
{
    QMutexLocker locker(&m_connectionsMutex);
    
    if (m_connections.contains(connectionId)) {
        ConnectionState& state = m_connections[connectionId];
        if (state.reconnectTimer) {
            state.reconnectTimer->stop();
        }
    }
}

void SmartReconnectManager::checkConnections()
{
    QMutexLocker locker(&m_connectionsMutex);
    
    for (auto& state : m_connections) {
        bool currentlyConnected = state.manager->isConnected();
        
        if (state.isConnected && !currentlyConnected) {
            // 连接丢失，开始重连流程
            qDebug() << "检测到连接丢失:" << state.connectionId;
            state.isConnected = false;
            state.currentRetryCount = 0;
            state.currentDelayMs = m_strategy.initialDelayMs;
            state.lastReconnectAttempt = QDateTime::currentDateTime();
            
            // 启动重连定时器
            state.reconnectTimer->start(state.currentDelayMs);
            
        } else if (!state.isConnected && currentlyConnected) {
            // 连接已恢复
            qDebug() << "连接已恢复:" << state.connectionId;
            state.isConnected = true;
            state.currentRetryCount = 0;
            state.reconnectTimer->stop();
            
            emit connectionRestored(state.connectionId);
        }
    }
}

void SmartReconnectManager::performReconnect()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer) return;
    
    QMutexLocker locker(&m_connectionsMutex);
    
    // 找到对应的连接状态
    ConnectionState* targetState = nullptr;
    for (auto& state : m_connections) {
        if (state.reconnectTimer == timer) {
            targetState = &state;
            break;
        }
    }
    
    if (!targetState) return;
    
    targetState->currentRetryCount++;
    targetState->lastReconnectAttempt = QDateTime::currentDateTime();
    
    qDebug() << "尝试重连" << targetState->connectionId 
             << "第" << targetState->currentRetryCount << "次";
    
    // 尝试重连
    bool reconnected = false;
    
    // 解析连接字符串并重连
    QStringList parts = targetState->connectionString.split(':');
    if (parts.size() >= 2) {
        QString protocol = parts[0].toUpper();
        
        if (protocol == "RTU" && parts.size() >= 6) {
            reconnected = targetState->manager->connectRTU(
                parts[1], parts[2].toInt(), parts[3].toInt(), 
                parts[4].at(0).toLatin1(), parts[5].toInt());
        } else if (protocol == "TCP" && parts.size() >= 3) {
            reconnected = targetState->manager->connectTCP(parts[1], parts[2].toInt());
        }
    }
    
    if (reconnected) {
        // 重连成功
        qDebug() << "重连成功:" << targetState->connectionId;
        targetState->isConnected = true;
        targetState->currentRetryCount = 0;
        targetState->reconnectTimer->stop();
        
        emit connectionRestored(targetState->connectionId);
        
    } else {
        // 重连失败
        emit connectionFailed(targetState->connectionId, targetState->currentRetryCount);
        
        if (targetState->currentRetryCount >= m_strategy.maxRetries) {
            // 达到最大重试次数
            qDebug() << "达到最大重试次数:" << targetState->connectionId;
            targetState->reconnectTimer->stop();
            emit maxRetriesReached(targetState->connectionId);
            
        } else {
            // 计算下次重连延迟（指数退避）
            targetState->currentDelayMs = qMin(
                (int)(targetState->currentDelayMs * m_strategy.backoffMultiplier),
                m_strategy.maxDelayMs);
            
            // 添加抖动
            if (m_strategy.jitterEnabled) {
                int jitter = QRandomGenerator::global()->bounded(targetState->currentDelayMs / 4);
                targetState->currentDelayMs += jitter;
            }
            
            qDebug() << "下次重连延迟:" << targetState->currentDelayMs << "ms";
            targetState->reconnectTimer->start(targetState->currentDelayMs);
        }
    }
}

// =============================================================================
// BatchOperationManager Implementation
// =============================================================================

BatchOperationManager::BatchOperationManager(ModbusConnectionPool* pool, QObject* parent)
    : QObject(parent), m_connectionPool(pool), m_batchSizeLimit(100), m_optimizationEnabled(true)
{
}

BatchOperationManager::~BatchOperationManager()
{
}

void BatchOperationManager::addReadRequest(const BatchRequest& request)
{
    QMutexLocker locker(&m_requestsMutex);
    m_pendingRequests.append(request);
}

QMap<QString, BatchOperationManager::BatchResponse> BatchOperationManager::executeBatch()
{
    QMutexLocker locker(&m_requestsMutex);
    
    QList<BatchRequest> requests = m_pendingRequests;
    m_pendingRequests.clear();
    locker.unlock();
    
    // 优化请求
    if (m_optimizationEnabled) {
        requests = optimizeRequests(requests);
    }
    
    QMap<QString, BatchResponse> responses;
    
    for (const auto& request : requests) {
        QString key = QString("%1_%2_%3_%4")
                         .arg(request.deviceId)
                         .arg(request.startAddress)
                         .arg(request.count)
                         .arg((int)request.dataType);
        
        responses[key] = executeRequest(request);
    }
    
    return responses;
}

void BatchOperationManager::setBatchSizeLimit(int limit)
{
    m_batchSizeLimit = limit;
}

void BatchOperationManager::setRequestOptimizationEnabled(bool enabled)
{
    m_optimizationEnabled = enabled;
}

QList<BatchOperationManager::BatchRequest> BatchOperationManager::optimizeRequests(const QList<BatchRequest>& requests)
{
    // 按设备和数据类型分组
    QMap<QString, QList<BatchRequest>> groupedRequests;
    
    for (const auto& request : requests) {
        QString groupKey = QString("%1_%2_%3")
                              .arg(request.deviceId)
                              .arg(request.connectionString)
                              .arg((int)request.dataType);
        groupedRequests[groupKey].append(request);
    }
    
    QList<BatchRequest> optimizedRequests;
    
    // 对每组请求进行优化
    for (auto it = groupedRequests.begin(); it != groupedRequests.end(); ++it) {
        QList<BatchRequest> group = it.value();
        
        // 按地址排序
        std::sort(group.begin(), group.end(), 
                  [](const BatchRequest& a, const BatchRequest& b) {
                      return a.startAddress < b.startAddress;
                  });
        
        // 合并连续地址的请求
        if (!group.isEmpty()) {
            BatchRequest current = group.first();
            
            for (int i = 1; i < group.size(); ++i) {
                const BatchRequest& next = group[i];
                
                // 检查是否可以合并
                if (next.startAddress <= current.startAddress + current.count + 1) {
                    // 扩展当前请求
                    int newEndAddress = qMax(current.startAddress + current.count - 1,
                                           next.startAddress + next.count - 1);
                    current.count = newEndAddress - current.startAddress + 1;
                } else {
                    // 无法合并，添加当前请求并开始新请求
                    optimizedRequests.append(current);
                    current = next;
                }
            }
            
            // 添加最后一个请求
            optimizedRequests.append(current);
        }
    }
    
    return optimizedRequests;
}

BatchOperationManager::BatchResponse BatchOperationManager::executeRequest(const BatchRequest& request)
{
    BatchResponse response;
    QElapsedTimer timer;
    timer.start();
    
    ModbusManager* manager = m_connectionPool->acquireConnection(request.deviceId, request.connectionString);
    if (!manager) {
        response.success = false;
        response.errorMessage = "无法获取连接";
        response.processingTimeMs = timer.elapsed();
        return response;
    }
    
    bool success = false;
    
    switch (request.dataType) {
        case ModbusManager::HoldingRegisters: {
            QVector<quint16> values;
            success = manager->readHoldingRegisters(request.startAddress, request.count, values);
            if (success) {
                for (quint16 value : values) {
                    response.values.append(QVariant(value));
                }
            }
            break;
        }
        case ModbusManager::InputRegisters: {
            QVector<quint16> values;
            success = manager->readInputRegisters(request.startAddress, request.count, values);
            if (success) {
                for (quint16 value : values) {
                    response.values.append(QVariant(value));
                }
            }
            break;
        }
        case ModbusManager::Coils: {
            QVector<bool> values;
            success = manager->readCoils(request.startAddress, request.count, values);
            if (success) {
                for (bool value : values) {
                    response.values.append(QVariant(value));
                }
            }
            break;
        }
        case ModbusManager::DiscreteInputs: {
            QVector<bool> values;
            success = manager->readDiscreteInputs(request.startAddress, request.count, values);
            if (success) {
                for (bool value : values) {
                    response.values.append(QVariant(value));
                }
            }
            break;
        }
    }
    
    m_connectionPool->releaseConnection(manager);
    
    response.success = success;
    if (!success) {
        response.errorMessage = manager->getLastError();
    }
    response.processingTimeMs = timer.elapsed();
    
    return response;
}

// =============================================================================
// ModbusPerformanceMonitor Implementation
// =============================================================================

ModbusPerformanceMonitor::ModbusPerformanceMonitor(QObject* parent)
    : QObject(parent), m_monitorStartTime(QDateTime::currentDateTime())
{
}

ModbusPerformanceMonitor::~ModbusPerformanceMonitor()
{
}

void ModbusPerformanceMonitor::recordOperationStart(const QString& operationId)
{
    QMutexLocker locker(&m_metricsMutex);
    
    if (!m_activeOperations.contains(operationId)) {
        m_activeOperations[operationId].start();
    }
}

void ModbusPerformanceMonitor::recordOperationEnd(const QString& operationId, bool success)
{
    QMutexLocker locker(&m_metricsMutex);
    
    if (m_activeOperations.contains(operationId)) {
        QElapsedTimer& timer = m_activeOperations[operationId];
        qint64 duration = timer.elapsed();
        
        OperationRecord record;
        record.startTime = QDateTime::currentDateTime().addMSecs(-duration);
        record.endTime = QDateTime::currentDateTime();
        record.success = success;
        record.durationMs = duration;
        
        m_completedOperations.append(record);
        m_activeOperations.remove(operationId);
        
        // 限制记录数量，避免内存过度使用
        if (m_completedOperations.size() > 10000) {
            m_completedOperations.removeFirst();
        }
    }
}

ModbusPerformanceMonitor::PerformanceMetrics ModbusPerformanceMonitor::getMetrics() const
{
    QMutexLocker locker(&m_metricsMutex);
    
    PerformanceMetrics metrics;
    metrics.totalOperations = m_completedOperations.size();
    metrics.successfulOperations = 0;
    metrics.failedOperations = 0;
    metrics.averageResponseTime = 0.0;
    metrics.maxResponseTime = 0.0;
    metrics.minResponseTime = std::numeric_limits<double>::max();
    metrics.startTime = m_monitorStartTime;
    
    if (!m_completedOperations.isEmpty()) {
        metrics.lastOperationTime = m_completedOperations.last().endTime;
        
        qint64 totalDuration = 0;
        
        for (const auto& record : m_completedOperations) {
            if (record.success) {
                metrics.successfulOperations++;
            } else {
                metrics.failedOperations++;
            }
            
            totalDuration += record.durationMs;
            metrics.maxResponseTime = qMax(metrics.maxResponseTime, (double)record.durationMs);
            metrics.minResponseTime = qMin(metrics.minResponseTime, (double)record.durationMs);
        }
        
        metrics.averageResponseTime = (double)totalDuration / m_completedOperations.size();
        
        // 计算每秒操作数
        qint64 totalTimeMs = m_monitorStartTime.msecsTo(QDateTime::currentDateTime());
        if (totalTimeMs > 0) {
            metrics.operationsPerSecond = (double)metrics.totalOperations * 1000.0 / totalTimeMs;
        }
    }
    
    if (metrics.minResponseTime == std::numeric_limits<double>::max()) {
        metrics.minResponseTime = 0.0;
    }
    
    return metrics;
}

QMap<QString, QVariant> ModbusPerformanceMonitor::getPerformanceStatistics() const
{
    PerformanceMetrics metrics = getMetrics();
    
    QMap<QString, QVariant> stats;
    stats["totalOperations"] = metrics.totalOperations;
    stats["successfulOperations"] = metrics.successfulOperations;
    stats["failedOperations"] = metrics.failedOperations;
    stats["averageResponseTime"] = metrics.averageResponseTime;
    stats["maxResponseTime"] = metrics.maxResponseTime;
    stats["minResponseTime"] = metrics.minResponseTime;
    stats["operationsPerSecond"] = metrics.operationsPerSecond;
    stats["startTime"] = metrics.startTime;
    stats["lastOperationTime"] = metrics.lastOperationTime;
    stats["successRate"] = metrics.totalOperations > 0 ? 
                          (double)metrics.successfulOperations / metrics.totalOperations * 100.0 : 0.0;
    
    return stats;
}

void ModbusPerformanceMonitor::reset()
{
    QMutexLocker locker(&m_metricsMutex);
    
    m_activeOperations.clear();
    m_completedOperations.clear();
    m_monitorStartTime = QDateTime::currentDateTime();
}

QString ModbusPerformanceMonitor::generateReport() const
{
    PerformanceMetrics metrics = getMetrics();
    
    QString report;
    QTextStream stream(&report);
    
    stream << "=== Modbus性能报告 ===\n";
    stream << "监控开始时间: " << metrics.startTime.toString("yyyy-MM-dd hh:mm:ss") << "\n";
    stream << "最后操作时间: " << metrics.lastOperationTime.toString("yyyy-MM-dd hh:mm:ss") << "\n";
    stream << "总操作数: " << metrics.totalOperations << "\n";
    stream << "成功操作数: " << metrics.successfulOperations << "\n";
    stream << "失败操作数: " << metrics.failedOperations << "\n";
    stream << "成功率: " << QString::number(metrics.totalOperations > 0 ? 
                                         (double)metrics.successfulOperations / metrics.totalOperations * 100 : 0, 'f', 2) << "%\n";
    stream << "平均响应时间: " << QString::number(metrics.averageResponseTime, 'f', 2) << " ms\n";
    stream << "最大响应时间: " << QString::number(metrics.maxResponseTime, 'f', 2) << " ms\n";
    stream << "最小响应时间: " << QString::number(metrics.minResponseTime, 'f', 2) << " ms\n";
    stream << "每秒操作数: " << QString::number(metrics.operationsPerSecond, 'f', 2) << " ops/sec\n";
    
    return report;
}

// =============================================================================
// Missing resetStatistics method implementations
// =============================================================================

void ModbusConnectionPool::resetStatistics()
{
    QMutexLocker locker(&m_poolMutex);
    
    // 重置连接使用统计
    for (auto& conn : m_connections) {
        conn.useCount = 0;
        conn.lastUsed = QDateTime::currentDateTime();
    }
}

void ModbusDataCache::resetStatistics()
{
    QMutexLocker locker(&m_cacheMutex);
    
    // 重置缓存统计计数器
    m_totalHits.store(0);
    m_totalMisses.store(0);
    
    // 重置缓存条目的命中计数
    for (auto& entry : m_cache) {
        entry.hitCount = 0;
    }
}

void AsyncModbusManager::resetStatistics()
{
    QMutexLocker locker(&m_queueMutex);
    
    // 重置操作计数器
    m_operationIdCounter.store(0);
    
    // 清空待处理操作队列（可选择保留或清空）
    // m_operationQueue.clear(); // 取消注释以清空队列
}

void SmartReconnectManager::resetStatistics()
{
    QMutexLocker locker(&m_connectionsMutex);
    
    // 重置重连统计
    for (auto& state : m_connections) {
        state.currentRetryCount = 0;
        state.currentDelayMs = m_strategy.initialDelayMs;
        state.lastReconnectAttempt = QDateTime();
    }
}

void BatchOperationManager::resetStatistics()
{
    QMutexLocker locker(&m_requestsMutex);
    
    // 清空待处理请求
    m_pendingRequests.clear();
}
