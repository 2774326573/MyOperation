# MyOperation Modbus通信手册

## 概述

本手册详细介绍了MyOperation系统中Modbus通信功能的实现、配置和使用方法。系统基于libmodbus 3.1.1库实现，支持Modbus TCP和Modbus RTU两种通信协议，可与各种工业设备进行数据交换。

## Modbus协议基础

### 协议类型
- **Modbus TCP**: 基于以太网的TCP/IP协议
- **Modbus RTU**: 基于串行通信的RTU模式

### 功能码支持
- **0x01**: 读取线圈状态 (Read Coils)
- **0x02**: 读取离散输入 (Read Discrete Inputs)
- **0x03**: 读取保持寄存器 (Read Holding Registers)
- **0x04**: 读取输入寄存器 (Read Input Registers)
- **0x05**: 写单个线圈 (Write Single Coil)
- **0x06**: 写单个寄存器 (Write Single Register)
- **0x0F**: 写多个线圈 (Write Multiple Coils)
- **0x10**: 写多个寄存器 (Write Multiple Registers)

## 系统架构

### 核心组件
- **ModbusManager**: Modbus通信管理器
- **ModbusRwManager**: 读写操作管理器
- **ModbusTcp**: TCP通信实现
- **ModbusRtu**: RTU通信实现
- **ModbusDevice**: 设备抽象类

## 核心API参考

### 1. ModbusManager (通信管理器)

#### 类定义
```cpp
class ModbusManager : public QObject
{
    Q_OBJECT
public:
    explicit ModbusManager(QObject *parent = nullptr);
    ~ModbusManager();
```

#### 主要功能
- 管理多个Modbus连接
- 设备生命周期管理
- 通信状态监控
- 错误处理和重连

#### 关键方法

##### 设备管理
```cpp
// 添加TCP设备
bool addTcpDevice(const QString& deviceId, const QString& ipAddress, int port, int slaveId);

// 添加RTU设备
bool addRtuDevice(const QString& deviceId, const QString& portName, int baudRate, char parity, int dataBits, int stopBits, int slaveId);

// 移除设备
bool removeDevice(const QString& deviceId);

// 获取设备列表
QStringList getDeviceList() const;

// 检查设备状态
bool isDeviceConnected(const QString& deviceId) const;
```

##### 连接控制
```cpp
// 连接设备
bool connectDevice(const QString& deviceId);

// 断开设备
bool disconnectDevice(const QString& deviceId);

// 连接所有设备
bool connectAllDevices();

// 断开所有设备
void disconnectAllDevices();
```

##### 数据操作
```cpp
// 读取数据
QVector<bool> readCoils(const QString& deviceId, int startAddress, int quantity);
QVector<bool> readDiscreteInputs(const QString& deviceId, int startAddress, int quantity);
QVector<quint16> readHoldingRegisters(const QString& deviceId, int startAddress, int quantity);
QVector<quint16> readInputRegisters(const QString& deviceId, int startAddress, int quantity);

// 写入数据
bool writeSingleCoil(const QString& deviceId, int address, bool value);
bool writeSingleRegister(const QString& deviceId, int address, quint16 value);
bool writeMultipleCoils(const QString& deviceId, int startAddress, const QVector<bool>& values);
bool writeMultipleRegisters(const QString& deviceId, int startAddress, const QVector<quint16>& values);
```

#### 信号
```cpp
signals:
    void deviceConnected(const QString& deviceId);
    void deviceDisconnected(const QString& deviceId);
    void dataReceived(const QString& deviceId, int functionCode, int startAddress, const QVariant& data);
    void errorOccurred(const QString& deviceId, const QString& error);
```

#### 使用示例
```cpp
#include "modbusmanager.h"

ModbusManager* modbusManager = new ModbusManager(this);

// 连接信号槽
connect(modbusManager, &ModbusManager::deviceConnected,
        this, &MainWindow::onDeviceConnected);
connect(modbusManager, &ModbusManager::errorOccurred,
        this, &MainWindow::onModbusError);

// 添加TCP设备
modbusManager->addTcpDevice("PLC01", "192.168.1.100", 502, 1);

// 连接设备
if (modbusManager->connectDevice("PLC01")) {
    // 读取保持寄存器
    QVector<quint16> values = modbusManager->readHoldingRegisters("PLC01", 0, 10);
    
    // 写入单个寄存器
    modbusManager->writeSingleRegister("PLC01", 100, 12345);
}
```

### 2. ModbusRwManager (读写管理器)

#### 类定义
```cpp
class ModbusRwManager : public QObject
{
    Q_OBJECT
public:
    explicit ModbusRwManager(QObject *parent = nullptr);
```

#### 主要功能
- 批量数据读写
- 数据映射管理
- 周期性数据采集
- 数据缓存和同步

#### 关键方法

##### 数据映射
```cpp
// 添加数据映射
struct DataMap {
    QString deviceId;
    QString tagName;
    int functionCode;
    int address;
    int quantity;
    QVariant::Type dataType;
};

bool addDataMap(const DataMap& dataMap);
bool removeDataMap(const QString& tagName);
QList<DataMap> getDataMaps() const;
```

##### 批量操作
```cpp
// 批量读取
QMap<QString, QVariant> readAllTags();
QMap<QString, QVariant> readTagsByDevice(const QString& deviceId);

// 批量写入
bool writeTagValue(const QString& tagName, const QVariant& value);
bool writeMultipleTags(const QMap<QString, QVariant>& tagValues);
```

##### 周期性采集
```cpp
// 启动/停止周期采集
void startCyclicRead(int intervalMs = 1000);
void stopCyclicRead();

// 设置采集间隔
void setCyclicInterval(int intervalMs);
int getCyclicInterval() const;
```

#### 信号
```cpp
signals:
    void cyclicDataReady(const QMap<QString, QVariant>& data);
    void tagValueChanged(const QString& tagName, const QVariant& value);
    void readWriteError(const QString& error);
```

#### 使用示例
```cpp
#include "modbusrwmanager.h"

ModbusRwManager* rwManager = new ModbusRwManager(this);

// 添加数据映射
ModbusRwManager::DataMap tempMap;
tempMap.deviceId = "PLC01";
tempMap.tagName = "Temperature";
tempMap.functionCode = 3; // 读保持寄存器
tempMap.address = 100;
tempMap.quantity = 1;
tempMap.dataType = QVariant::Double;
rwManager->addDataMap(tempMap);

// 连接信号槽
connect(rwManager, &ModbusRwManager::cyclicDataReady,
        this, &MainWindow::onCyclicDataReady);

// 启动周期采集
rwManager->startCyclicRead(500); // 500ms间隔
```

### 3. ModbusTcp (TCP通信)

#### 类定义
```cpp
class ModbusTcp : public QObject
{
    Q_OBJECT
public:
    explicit ModbusTcp(QObject *parent = nullptr);
```

#### 主要功能
- TCP/IP连接管理
- 网络参数配置
- 连接状态监控
- 网络异常处理

#### 关键方法

##### 连接管理
```cpp
// 设置连接参数
void setConnectionParameters(const QString& ipAddress, int port, int slaveId);

// 连接控制
bool connectToServer();
void disconnectFromServer();
bool isConnected() const;

// 获取连接信息
QString getIpAddress() const;
int getPort() const;
int getSlaveId() const;
```

##### 超时设置
```cpp
// 设置超时时间
void setConnectionTimeout(int timeoutMs);
void setResponseTimeout(int timeoutMs);

// 获取超时设置
int getConnectionTimeout() const;
int getResponseTimeout() const;
```

#### 使用示例
```cpp
#include "modbustcp.h"

ModbusTcp* tcpClient = new ModbusTcp(this);

// 设置连接参数
tcpClient->setConnectionParameters("192.168.1.100", 502, 1);
tcpClient->setConnectionTimeout(3000);
tcpClient->setResponseTimeout(1000);

// 连接到服务器
if (tcpClient->connectToServer()) {
    qDebug() << "Connected to Modbus TCP server";
} else {
    qWarning() << "Failed to connect to Modbus TCP server";
}
```

### 4. ModbusRtu (RTU通信)

#### 类定义
```cpp
class ModbusRtu : public QObject
{
    Q_OBJECT
public:
    explicit ModbusRtu(QObject *parent = nullptr);
```

#### 主要功能
- 串口通信管理
- 串口参数配置
- RTU协议处理
- 串口异常处理

#### 关键方法

##### 串口配置
```cpp
// 设置串口参数
void setSerialParameters(const QString& portName, int baudRate, char parity, int dataBits, int stopBits, int slaveId);

// 连接控制
bool openSerialPort();
void closeSerialPort();
bool isSerialPortOpen() const;

// 获取串口信息
QString getPortName() const;
int getBaudRate() const;
char getParity() const;
int getDataBits() const;
int getStopBits() const;
```

##### 通信参数
```cpp
// 设置RTU参数
void setRtuTimeout(int timeoutMs);
void setInterFrameDelay(int delayMs);

// 获取RTU参数
int getRtuTimeout() const;
int getInterFrameDelay() const;
```

#### 使用示例
```cpp
#include "modbusrtu.h"

ModbusRtu* rtuClient = new ModbusRtu(this);

// 设置串口参数
rtuClient->setSerialParameters("COM1", 9600, 'N', 8, 1, 1);
rtuClient->setRtuTimeout(1000);

// 打开串口
if (rtuClient->openSerialPort()) {
    qDebug() << "Serial port opened successfully";
} else {
    qWarning() << "Failed to open serial port";
}
```

## 数据类型处理

### 1. 寄存器数据转换

#### 基本数据类型
```cpp
class ModbusDataConverter {
public:
    // 16位数据转换
    static qint16 registersToInt16(quint16 reg) {
        return static_cast<qint16>(reg);
    }
    
    static quint16 int16ToRegisters(qint16 value) {
        return static_cast<quint16>(value);
    }
    
    // 32位数据转换（大端序）
    static qint32 registersToInt32(quint16 highReg, quint16 lowReg) {
        return (static_cast<qint32>(highReg) << 16) | lowReg;
    }
    
    static void int32ToRegisters(qint32 value, quint16& highReg, quint16& lowReg) {
        highReg = static_cast<quint16>((value >> 16) & 0xFFFF);
        lowReg = static_cast<quint16>(value & 0xFFFF);
    }
    
    // 浮点数转换
    static float registersToFloat(quint16 reg1, quint16 reg2) {
        union {
            float f;
            quint32 i;
        } converter;
        converter.i = (static_cast<quint32>(reg1) << 16) | reg2;
        return converter.f;
    }
    
    static void floatToRegisters(float value, quint16& reg1, quint16& reg2) {
        union {
            float f;
            quint32 i;
        } converter;
        converter.f = value;
        reg1 = static_cast<quint16>((converter.i >> 16) & 0xFFFF);
        reg2 = static_cast<quint16>(converter.i & 0xFFFF);
    }
};
```

#### 使用示例
```cpp
// 读取32位整数
QVector<quint16> regs = modbusManager->readHoldingRegisters("PLC01", 100, 2);
if (regs.size() == 2) {
    qint32 value = ModbusDataConverter::registersToInt32(regs[0], regs[1]);
    qDebug() << "32-bit value:" << value;
}

// 写入浮点数
float temperature = 25.6f;
quint16 reg1, reg2;
ModbusDataConverter::floatToRegisters(temperature, reg1, reg2);
QVector<quint16> values = {reg1, reg2};
modbusManager->writeMultipleRegisters("PLC01", 200, values);
```

### 2. 位操作

#### 线圈和离散输入处理
```cpp
class ModbusBitOperations {
public:
    // 设置特定位
    static quint16 setBit(quint16 reg, int bitIndex, bool value) {
        if (bitIndex < 0 || bitIndex > 15) return reg;
        
        if (value) {
            return reg | (1 << bitIndex);
        } else {
            return reg & ~(1 << bitIndex);
        }
    }
    
    // 获取特定位
    static bool getBit(quint16 reg, int bitIndex) {
        if (bitIndex < 0 || bitIndex > 15) return false;
        return (reg & (1 << bitIndex)) != 0;
    }
    
    // 字节转换为位数组
    static QVector<bool> byteToBits(quint8 byte) {
        QVector<bool> bits(8);
        for (int i = 0; i < 8; i++) {
            bits[i] = (byte & (1 << i)) != 0;
        }
        return bits;
    }
    
    // 位数组转换为字节
    static quint8 bitsToByte(const QVector<bool>& bits) {
        quint8 byte = 0;
        int count = qMin(bits.size(), 8);
        for (int i = 0; i < count; i++) {
            if (bits[i]) {
                byte |= (1 << i);
            }
        }
        return byte;
    }
};
```

## 配置管理

### 1. 设备配置

#### 配置文件格式 (JSON)
```json
{
    "modbus_devices": [
        {
            "id": "PLC01",
            "type": "tcp",
            "ip_address": "192.168.1.100",
            "port": 502,
            "slave_id": 1,
            "connection_timeout": 3000,
            "response_timeout": 1000,
            "auto_connect": true,
            "retry_count": 3,
            "retry_interval": 1000
        },
        {
            "id": "Sensor01",
            "type": "rtu",
            "port_name": "COM1",
            "baud_rate": 9600,
            "parity": "N",
            "data_bits": 8,
            "stop_bits": 1,
            "slave_id": 2,
            "rtu_timeout": 1000,
            "inter_frame_delay": 50,
            "auto_connect": true
        }
    ],
    "data_maps": [
        {
            "device_id": "PLC01",
            "tag_name": "Temperature",
            "function_code": 3,
            "address": 100,
            "quantity": 2,
            "data_type": "float",
            "scale_factor": 1.0,
            "offset": 0.0,
            "unit": "°C"
        },
        {
            "device_id": "PLC01",
            "tag_name": "Pressure",
            "function_code": 3,
            "address": 102,
            "quantity": 2,
            "data_type": "float",
            "scale_factor": 0.1,
            "offset": 0.0,
            "unit": "bar"
        }
    ]
}
```

#### 配置管理器
```cpp
class ModbusConfigManager {
private:
    QString configFile;
    QJsonObject configData;
    
public:
    ModbusConfigManager(const QString& configFilePath) : configFile(configFilePath) {}
    
    // 加载配置
    bool loadConfiguration() {
        QFile file(configFile);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open config file:" << configFile;
            return false;
        }
        
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            qWarning() << "Invalid JSON format in config file";
            return false;
        }
        
        configData = doc.object();
        return true;
    }
    
    // 保存配置
    bool saveConfiguration() {
        QFile file(configFile);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot write config file:" << configFile;
            return false;
        }
        
        QJsonDocument doc(configData);
        file.write(doc.toJson());
        return true;
    }
    
    // 获取设备配置
    QList<QJsonObject> getDeviceConfigs() {
        QList<QJsonObject> devices;
        QJsonArray deviceArray = configData["modbus_devices"].toArray();
        
        for (const QJsonValue& value : deviceArray) {
            devices.append(value.toObject());
        }
        return devices;
    }
    
    // 获取数据映射配置
    QList<QJsonObject> getDataMapConfigs() {
        QList<QJsonObject> dataMaps;
        QJsonArray dataMapArray = configData["data_maps"].toArray();
        
        for (const QJsonValue& value : dataMapArray) {
            dataMaps.append(value.toObject());
        }
        return dataMaps;
    }
};
```

### 2. 自动配置应用

#### ModbusManager集成配置
```cpp
class ModbusManager : public QObject {
private:
    ModbusConfigManager* configManager;
    
public:
    // 从配置文件加载设备
    bool loadDevicesFromConfig(const QString& configFile) {
        configManager = new ModbusConfigManager(configFile);
        if (!configManager->loadConfiguration()) {
            return false;
        }
        
        // 加载设备配置
        QList<QJsonObject> deviceConfigs = configManager->getDeviceConfigs();
        for (const QJsonObject& config : deviceConfigs) {
            QString deviceId = config["id"].toString();
            QString type = config["type"].toString();
            
            if (type == "tcp") {
                QString ipAddress = config["ip_address"].toString();
                int port = config["port"].toInt();
                int slaveId = config["slave_id"].toInt();
                addTcpDevice(deviceId, ipAddress, port, slaveId);
                
                // 设置超时参数
                if (config.contains("connection_timeout")) {
                    setConnectionTimeout(deviceId, config["connection_timeout"].toInt());
                }
                if (config.contains("response_timeout")) {
                    setResponseTimeout(deviceId, config["response_timeout"].toInt());
                }
                
            } else if (type == "rtu") {
                QString portName = config["port_name"].toString();
                int baudRate = config["baud_rate"].toInt();
                QString parityStr = config["parity"].toString();
                char parity = parityStr.isEmpty() ? 'N' : parityStr[0].toLatin1();
                int dataBits = config["data_bits"].toInt();
                int stopBits = config["stop_bits"].toInt();
                int slaveId = config["slave_id"].toInt();
                
                addRtuDevice(deviceId, portName, baudRate, parity, dataBits, stopBits, slaveId);
            }
            
            // 自动连接
            if (config["auto_connect"].toBool()) {
                connectDevice(deviceId);
            }
        }
        
        return true;
    }
};
```

## 错误处理和诊断

### 1. 错误代码定义

#### 通用错误码
```cpp
enum ModbusErrorCode {
    MODBUS_SUCCESS = 0,
    
    // 连接错误 (1000-1999)
    MODBUS_CONNECTION_FAILED = 1001,
    MODBUS_CONNECTION_TIMEOUT = 1002,
    MODBUS_DISCONNECTED = 1003,
    MODBUS_INVALID_ADDRESS = 1004,
    
    // 协议错误 (2000-2999)
    MODBUS_INVALID_FUNCTION_CODE = 2001,
    MODBUS_INVALID_DATA_ADDRESS = 2002,
    MODBUS_INVALID_DATA_VALUE = 2003,
    MODBUS_SLAVE_DEVICE_FAILURE = 2004,
    MODBUS_ACKNOWLEDGE = 2005,
    MODBUS_SLAVE_DEVICE_BUSY = 2006,
    MODBUS_NEGATIVE_ACKNOWLEDGE = 2008,
    MODBUS_MEMORY_PARITY_ERROR = 2010,
    MODBUS_GATEWAY_PATH_UNAVAILABLE = 2011,
    MODBUS_GATEWAY_TARGET_DEVICE_FAILED = 2012,
    
    // 通信错误 (3000-3999)
    MODBUS_RESPONSE_TIMEOUT = 3001,
    MODBUS_INVALID_CRC = 3002,
    MODBUS_INVALID_RESPONSE = 3003,
    MODBUS_TOO_MANY_DATA = 3004,
    
    // 系统错误 (4000-4999)
    MODBUS_INVALID_PARAMETER = 4001,
    MODBUS_OUT_OF_MEMORY = 4002,
    MODBUS_SYSTEM_ERROR = 4003
};
```

#### 错误处理类
```cpp
class ModbusErrorHandler {
public:
    static QString getErrorDescription(ModbusErrorCode errorCode) {
        switch (errorCode) {
            case MODBUS_SUCCESS:
                return "Operation successful";
            case MODBUS_CONNECTION_FAILED:
                return "Failed to establish connection";
            case MODBUS_CONNECTION_TIMEOUT:
                return "Connection timeout";
            case MODBUS_DISCONNECTED:
                return "Device disconnected";
            case MODBUS_INVALID_FUNCTION_CODE:
                return "Invalid function code";
            case MODBUS_INVALID_DATA_ADDRESS:
                return "Invalid data address";
            case MODBUS_SLAVE_DEVICE_FAILURE:
                return "Slave device failure";
            case MODBUS_RESPONSE_TIMEOUT:
                return "Response timeout";
            default:
                return "Unknown error";
        }
    }
    
    static bool isRetryableError(ModbusErrorCode errorCode) {
        return (errorCode == MODBUS_CONNECTION_TIMEOUT ||
                errorCode == MODBUS_RESPONSE_TIMEOUT ||
                errorCode == MODBUS_SLAVE_DEVICE_BUSY ||
                errorCode == MODBUS_DISCONNECTED);
    }
    
    static bool isTemporaryError(ModbusErrorCode errorCode) {
        return (errorCode == MODBUS_SLAVE_DEVICE_BUSY ||
                errorCode == MODBUS_ACKNOWLEDGE);
    }
};
```

### 2. 自动重连机制

#### 重连管理器
```cpp
class ModbusReconnectManager : public QObject {
    Q_OBJECT
private:
    struct ReconnectInfo {
        QString deviceId;
        int retryCount;
        int maxRetries;
        int retryInterval;
        QTimer* retryTimer;
    };
    
    QMap<QString, ReconnectInfo> reconnectInfos;
    
public:
    ModbusReconnectManager(QObject* parent = nullptr) : QObject(parent) {}
    
    // 设置重连参数
    void setReconnectParameters(const QString& deviceId, int maxRetries, int retryInterval) {
        ReconnectInfo info;
        info.deviceId = deviceId;
        info.retryCount = 0;
        info.maxRetries = maxRetries;
        info.retryInterval = retryInterval;
        info.retryTimer = new QTimer(this);
        info.retryTimer->setSingleShot(true);
        
        connect(info.retryTimer, &QTimer::timeout,
                this, [this, deviceId]() { this->attemptReconnect(deviceId); });
        
        reconnectInfos[deviceId] = info;
    }
    
    // 开始重连
    void startReconnect(const QString& deviceId) {
        if (!reconnectInfos.contains(deviceId)) return;
        
        ReconnectInfo& info = reconnectInfos[deviceId];
        info.retryCount = 0;
        attemptReconnect(deviceId);
    }
    
    // 停止重连
    void stopReconnect(const QString& deviceId) {
        if (!reconnectInfos.contains(deviceId)) return;
        
        ReconnectInfo& info = reconnectInfos[deviceId];
        info.retryTimer->stop();
        info.retryCount = 0;
    }
    
private slots:
    void attemptReconnect(const QString& deviceId) {
        if (!reconnectInfos.contains(deviceId)) return;
        
        ReconnectInfo& info = reconnectInfos[deviceId];
        
        // 尝试重连
        emit reconnectAttempt(deviceId, info.retryCount + 1);
        
        info.retryCount++;
        if (info.retryCount < info.maxRetries) {
            // 继续重试
            info.retryTimer->start(info.retryInterval);
        } else {
            // 达到最大重试次数
            emit reconnectFailed(deviceId);
        }
    }
    
signals:
    void reconnectAttempt(const QString& deviceId, int attemptNumber);
    void reconnectSuccess(const QString& deviceId);
    void reconnectFailed(const QString& deviceId);
};
```

### 3. 通信诊断

#### 诊断工具
```cpp
class ModbusDiagnostic {
private:
    struct DiagnosticData {
        QString deviceId;
        int totalRequests;
        int successfulRequests;
        int failedRequests;
        QTime lastSuccessTime;
        QTime lastFailureTime;
        QList<ModbusErrorCode> recentErrors;
        double averageResponseTime;
    };
    
    QMap<QString, DiagnosticData> diagnosticData;
    
public:
    // 记录请求
    void recordRequest(const QString& deviceId, bool success, ModbusErrorCode errorCode, int responseTime) {
        DiagnosticData& data = diagnosticData[deviceId];
        data.deviceId = deviceId;
        data.totalRequests++;
        
        if (success) {
            data.successfulRequests++;
            data.lastSuccessTime = QTime::currentTime();
        } else {
            data.failedRequests++;
            data.lastFailureTime = QTime::currentTime();
            data.recentErrors.append(errorCode);
            
            // 保持错误历史记录不超过100条
            if (data.recentErrors.size() > 100) {
                data.recentErrors.removeFirst();
            }
        }
        
        // 更新平均响应时间
        updateAverageResponseTime(deviceId, responseTime);
    }
    
    // 获取成功率
    double getSuccessRate(const QString& deviceId) const {
        if (!diagnosticData.contains(deviceId)) return 0.0;
        
        const DiagnosticData& data = diagnosticData[deviceId];
        if (data.totalRequests == 0) return 0.0;
        
        return (double)data.successfulRequests / data.totalRequests * 100.0;
    }
    
    // 获取诊断报告
    QString getDiagnosticReport(const QString& deviceId) const {
        if (!diagnosticData.contains(deviceId)) {
            return QString("No diagnostic data for device: %1").arg(deviceId);
        }
        
        const DiagnosticData& data = diagnosticData[deviceId];
        
        QString report;
        report += QString("Device: %1\n").arg(deviceId);
        report += QString("Total Requests: %1\n").arg(data.totalRequests);
        report += QString("Successful: %1\n").arg(data.successfulRequests);
        report += QString("Failed: %1\n").arg(data.failedRequests);
        report += QString("Success Rate: %1%\n").arg(getSuccessRate(deviceId), 0, 'f', 2);
        report += QString("Average Response Time: %1ms\n").arg(data.averageResponseTime, 0, 'f', 2);
        
        if (data.lastSuccessTime.isValid()) {
            report += QString("Last Success: %1\n").arg(data.lastSuccessTime.toString());
        }
        if (data.lastFailureTime.isValid()) {
            report += QString("Last Failure: %1\n").arg(data.lastFailureTime.toString());
        }
        
        return report;
    }
    
private:
    void updateAverageResponseTime(const QString& deviceId, int responseTime) {
        DiagnosticData& data = diagnosticData[deviceId];
        
        // 简单的移动平均算法
        if (data.totalRequests == 1) {
            data.averageResponseTime = responseTime;
        } else {
            data.averageResponseTime = (data.averageResponseTime * 0.9) + (responseTime * 0.1);
        }
    }
};
```

## 性能优化

### 1. 连接池管理

#### 连接池实现
```cpp
class ModbusConnectionPool {
private:
    struct ConnectionInfo {
        QString deviceId;
        modbus_t* context;
        bool inUse;
        QDateTime lastUsed;
    };
    
    QList<ConnectionInfo> connections;
    QMutex poolMutex;
    int maxConnections;
    
public:
    ModbusConnectionPool(int maxConns = 10) : maxConnections(maxConns) {}
    
    // 获取连接
    modbus_t* acquireConnection(const QString& deviceId) {
        QMutexLocker locker(&poolMutex);
        
        // 查找空闲连接
        for (auto& conn : connections) {
            if (conn.deviceId == deviceId && !conn.inUse) {
                conn.inUse = true;
                conn.lastUsed = QDateTime::currentDateTime();
                return conn.context;
            }
        }
        
        // 创建新连接
        if (connections.size() < maxConnections) {
            ConnectionInfo newConn;
            newConn.deviceId = deviceId;
            newConn.context = createConnection(deviceId);
            newConn.inUse = true;
            newConn.lastUsed = QDateTime::currentDateTime();
            
            connections.append(newConn);
            return newConn.context;
        }
        
        return nullptr; // 连接池已满
    }
    
    // 释放连接
    void releaseConnection(modbus_t* context) {
        QMutexLocker locker(&poolMutex);
        
        for (auto& conn : connections) {
            if (conn.context == context) {
                conn.inUse = false;
                conn.lastUsed = QDateTime::currentDateTime();
                break;
            }
        }
    }
    
    // 清理过期连接
    void cleanupExpiredConnections(int timeoutMinutes = 30) {
        QMutexLocker locker(&poolMutex);
        
        QDateTime now = QDateTime::currentDateTime();
        auto it = connections.begin();
        
        while (it != connections.end()) {
            if (!it->inUse && it->lastUsed.addSecs(timeoutMinutes * 60) < now) {
                modbus_close(it->context);
                modbus_free(it->context);
                it = connections.erase(it);
            } else {
                ++it;
            }
        }
    }
    
private:
    modbus_t* createConnection(const QString& deviceId) {
        // 根据设备配置创建连接
        // 这里需要根据实际的设备配置信息来创建
        return nullptr;
    }
};
```

### 2. 数据缓存机制

#### 缓存管理器
```cpp
class ModbusDataCache {
private:
    struct CacheEntry {
        QVariant value;
        QDateTime timestamp;
        int ttl; // 生存时间（秒）
    };
    
    QMap<QString, CacheEntry> cache;
    QMutex cacheMutex;
    
public:
    // 设置缓存值
    void setValue(const QString& key, const QVariant& value, int ttlSeconds = 300) {
        QMutexLocker locker(&cacheMutex);
        
        CacheEntry entry;
        entry.value = value;
        entry.timestamp = QDateTime::currentDateTime();
        entry.ttl = ttlSeconds;
        
        cache[key] = entry;
    }
    
    // 获取缓存值
    QVariant getValue(const QString& key, bool* found = nullptr) {
        QMutexLocker locker(&cacheMutex);
        
        if (found) *found = false;
        
        if (!cache.contains(key)) {
            return QVariant();
        }
        
        const CacheEntry& entry = cache[key];
        
        // 检查是否过期
        if (entry.timestamp.addSecs(entry.ttl) < QDateTime::currentDateTime()) {
            cache.remove(key);
            return QVariant();
        }
        
        if (found) *found = true;
        return entry.value;
    }
    
    // 清理过期缓存
    void cleanupExpiredEntries() {
        QMutexLocker locker(&cacheMutex);
        
        QDateTime now = QDateTime::currentDateTime();
        auto it = cache.begin();
        
        while (it != cache.end()) {
            if (it->timestamp.addSecs(it->ttl) < now) {
                it = cache.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // 清空缓存
    void clear() {
        QMutexLocker locker(&cacheMutex);
        cache.clear();
    }
};
```

## 应用示例

### 1. 完整的Modbus客户端示例

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>
#include "modbusmanager.h"
#include "modbusrwmanager.h"

class ModbusClientDemo : public QMainWindow {
    Q_OBJECT
    
private:
    ModbusManager* modbusManager;
    ModbusRwManager* rwManager;
    QTextEdit* logTextEdit;
    QLineEdit* ipEdit;
    QLineEdit* portEdit;
    QPushButton* connectButton;
    QPushButton* disconnectButton;
    QPushButton* readButton;
    QTimer* cyclicTimer;
    
public:
    ModbusClientDemo(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupModbus();
    }
    
private:
    void setupUI() {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        
        // 连接参数
        QHBoxLayout* paramLayout = new QHBoxLayout();
        paramLayout->addWidget(new QLabel("IP:"));
        ipEdit = new QLineEdit("192.168.1.100");
        paramLayout->addWidget(ipEdit);
        
        paramLayout->addWidget(new QLabel("Port:"));
        portEdit = new QLineEdit("502");
        paramLayout->addWidget(portEdit);
        
        mainLayout->addLayout(paramLayout);
        
        // 控制按钮
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        connectButton = new QPushButton("Connect");
        disconnectButton = new QPushButton("Disconnect");
        readButton = new QPushButton("Read Data");
        
        buttonLayout->addWidget(connectButton);
        buttonLayout->addWidget(disconnectButton);
        buttonLayout->addWidget(readButton);
        
        mainLayout->addLayout(buttonLayout);
        
        // 日志显示
        logTextEdit = new QTextEdit();
        mainLayout->addWidget(logTextEdit);
        
        // 连接信号槽
        connect(connectButton, &QPushButton::clicked, this, &ModbusClientDemo::onConnect);
        connect(disconnectButton, &QPushButton::clicked, this, &ModbusClientDemo::onDisconnect);
        connect(readButton, &QPushButton::clicked, this, &ModbusClientDemo::onReadData);
    }
    
    void setupModbus() {
        modbusManager = new ModbusManager(this);
        rwManager = new ModbusRwManager(this);
        
        // 连接信号槽
        connect(modbusManager, &ModbusManager::deviceConnected,
                this, &ModbusClientDemo::onDeviceConnected);
        connect(modbusManager, &ModbusManager::deviceDisconnected,
                this, &ModbusClientDemo::onDeviceDisconnected);
        connect(modbusManager, &ModbusManager::errorOccurred,
                this, &ModbusClientDemo::onError);
        
        // 周期性读取定时器
        cyclicTimer = new QTimer(this);
        connect(cyclicTimer, &QTimer::timeout, this, &ModbusClientDemo::onCyclicRead);
    }
    
private slots:
    void onConnect() {
        QString ip = ipEdit->text();
        int port = portEdit->text().toInt();
        
        if (modbusManager->addTcpDevice("PLC01", ip, port, 1)) {
            if (modbusManager->connectDevice("PLC01")) {
                logTextEdit->append(QString("Connecting to %1:%2...").arg(ip).arg(port));
            } else {
                logTextEdit->append("Failed to connect");
            }
        }
    }
    
    void onDisconnect() {
        cyclicTimer->stop();
        modbusManager->disconnectDevice("PLC01");
        logTextEdit->append("Disconnected");
    }
    
    void onReadData() {
        // 读取保持寄存器
        QVector<quint16> values = modbusManager->readHoldingRegisters("PLC01", 0, 10);
        if (!values.isEmpty()) {
            QString result = "Read registers: ";
            for (int i = 0; i < values.size(); i++) {
                result += QString("%1 ").arg(values[i]);
            }
            logTextEdit->append(result);
        }
    }
    
    void onDeviceConnected(const QString& deviceId) {
        logTextEdit->append(QString("Device %1 connected").arg(deviceId));
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        readButton->setEnabled(true);
        cyclicTimer->start(1000); // 每秒读取一次
    }
    
    void onDeviceDisconnected(const QString& deviceId) {
        logTextEdit->append(QString("Device %1 disconnected").arg(deviceId));
        connectButton->setEnabled(true);
        disconnectButton->setEnabled(false);
        readButton->setEnabled(false);
        cyclicTimer->stop();
    }
    
    void onError(const QString& deviceId, const QString& error) {
        logTextEdit->append(QString("Error from %1: %2").arg(deviceId, error));
    }
    
    void onCyclicRead() {
        // 周期性读取数据
        QVector<quint16> values = modbusManager->readHoldingRegisters("PLC01", 0, 4);
        if (values.size() >= 4) {
            // 假设前两个寄存器是温度（float）
            float temperature = ModbusDataConverter::registersToFloat(values[0], values[1]);
            // 后两个寄存器是压力（float）
            float pressure = ModbusDataConverter::registersToFloat(values[2], values[3]);
            
            QString status = QString("Temperature: %1°C, Pressure: %2bar")
                           .arg(temperature, 0, 'f', 1)
                           .arg(pressure, 0, 'f', 2);
            setWindowTitle(QString("Modbus Client - %1").arg(status));
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    ModbusClientDemo window;
    window.show();
    
    return app.exec();
}

#include "main.moc"
```

## 最佳实践

### 1. 通信设计原则
- 合理设置超时时间
- 实现重连机制
- 使用连接池管理
- 避免频繁连接断开

### 2. 数据处理建议
- 使用适当的数据类型转换
- 实现数据验证和范围检查
- 使用缓存减少网络负载
- 批量读写提高效率

### 3. 错误处理策略
- 分类处理不同类型的错误
- 记录详细的错误日志
- 实现自动恢复机制
- 提供用户友好的错误提示

### 4. 性能优化要点
- 使用异步通信避免阻塞
- 合理配置轮询间隔
- 实现智能的数据更新策略
- 优化网络参数配置

## 故障排除

### 常见问题及解决方案

#### 1. 连接超时
**问题**: 无法连接到Modbus设备
**解决方案**:
- 检查网络连接和设备IP地址
- 确认防火墙设置
- 调整连接超时时间
- 验证设备是否支持Modbus协议

#### 2. 数据读取失败
**问题**: 读取操作返回错误
**解决方案**:
- 确认寄存器地址和数量
- 检查设备从站ID
- 验证功能码支持
- 确认设备权限设置

#### 3. 通信不稳定
**问题**: 间歇性通信失败
**解决方案**:
- 检查网络质量和延迟
- 调整响应超时时间
- 实现重试机制
- 使用诊断工具监控通信状态

---

*本文档最后更新时间: 2025年6月5日*
