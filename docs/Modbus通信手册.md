# MyOperation Modbus通信手册

## 📋 目录

- [概述](#概述)
- [Modbus协议基础](#modbus协议基础)
- [系统架构](#系统架构)
- [核心API参考](#核心api参考)
- [数据类型处理](#数据类型处理)
- [配置管理](#配置管理)
- [串口诊断功能](#串口诊断功能) ⭐ **新增功能**
- [错误处理和诊断](#错误处理和诊断)
- [性能优化](#性能优化)
- [应用示例](#应用示例)
- [最佳实践](#最佳实践)
- [故障排除](#故障排除)

---

## 概述

本手册详细介绍了MyOperation系统中Modbus通信功能的实现、配置和使用方法。系统基于libmodbus 3.1.1库实现，支持Modbus TCP和Modbus RTU两种通信协议，可与各种工业设备进行数据交换。

**🔄 最新更新 (2025-06-06):**
- ✅ 整合串口诊断功能到ModbusManager
- ✅ 废弃独立的SerialDiagnostic和ModbusTroubleshooter类
- ✅ 提供统一的诊断API接口
- ✅ 增强错误处理和故障排除能力

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
- **ModbusManager**: Modbus通信管理器（**包含串口诊断功能**）
- **ModbusRwManager**: 读写操作管理器（**提供诊断接口**）
- **ModbusTcp**: TCP通信实现
- **ModbusRtu**: RTU通信实现
- **ModbusDevice**: 设备抽象类

### 🔄 架构变更说明
从2025年6月6日起，原有的独立诊断组件已整合：

**整合前：**
```
ModbusManager (通信管理)
ModbusRwManager (读写管理)
SerialDiagnostic (串口诊断) ❌ 已废弃
ModbusTroubleshooter (故障排除) ❌ 已废弃
```

**整合后：**
```
ModbusManager (通信管理 + 串口诊断) ✅
ModbusRwManager (读写管理 + 诊断接口) ✅
```

### 迁移指南
如果您的代码中使用了旧的诊断类，请按以下方式更新：

```cpp
// 旧代码 (不再推荐)
#include "serial_diagnostic.h"
#include "modbus_troubleshooter.h"

SerialDiagnostic::PortInfo info = SerialDiagnostic::getPortInfo(portName);
QString report = ModbusTroubleshooter::diagnoseSerialPort(portName);

// 新代码 (推荐使用 ModbusManager)
#include "modbusmanager.h"

ModbusManager::PortInfo info = ModbusManager::getPortInfo(portName);
QString report = ModbusManager::getPortDiagnostics(portName);
QString report = ModbusManager::getPortDiagnostics(portName);

// 或者使用 ModbusRwManager 接口
#include "modbusrwmanager.h"

ModbusManager::PortInfo info = ModbusRwManager::getPortInfo(portName);
QString report = ModbusRwManager::getPortDiagnostics(portName);
```

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

##### 串口诊断功能 ⭐ **新增**
```cpp
// 串口信息结构
struct PortInfo {
    QString portName;        // 串口名称
    QString description;     // 设备描述
    QString manufacturer;    // 制造商信息
    QString systemLocation;  // 系统位置
    bool exists;            // 串口是否存在
    bool accessible;        // 是否可访问
    bool inUse;            // 是否被占用
    QString errorString;    // 错误信息
};

// 获取系统中所有可用串口列表
static QStringList getAvailablePorts();

// 获取指定串口的详细信息
static PortInfo getPortInfo(const QString& portName);

// 测试串口连接参数
static bool testPortConnection(const QString& portName, 
                             int baudRate, int dataBits, 
                             char parity, int stopBits);

// 生成完整的串口诊断报告
static QString generateDiagnosticReport();

// 对特定串口进行详细诊断
static QString diagnoseSerialPort(const QString& portName);

// 获取针对特定串口的建议
static QStringList getRecommendations(const QString& portName);

// 获取通用的快速解决方案
static QStringList getQuickFixes();

// 生成系统信息报告
static QString generateSystemInfo();
```

**诊断功能使用示例：**
```cpp
// 检查可用串口
QStringList ports = ModbusManager::getAvailablePorts();
qDebug() << "可用串口:" << ports;

// 获取串口详细信息
ModbusManager::PortInfo info = ModbusManager::getPortInfo("COM1");
if (info.accessible) {
    qDebug() << "串口可用，描述:" << info.description;
} else {
    qWarning() << "串口不可用:" << info.errorString;
}

// 生成诊断报告
QString report = ModbusManager::generateDiagnosticReport();
qDebug() << report;

// 获取建议
QStringList recommendations = ModbusManager::getRecommendations("COM1");
for (const QString& rec : recommendations) {
    qDebug() << "建议:" << rec;
}
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

#### 串口诊断接口

ModbusRwManager 提供了完整的串口诊断接口，方便在应用层直接调用诊断功能：

##### 诊断方法列表

```cpp
class ModbusRwManager : public QObject {
public:
    // 串口诊断接口 - 通过内部 ModbusManager 实现
    static QStringList getAvailablePorts();
    static ModbusManager::PortInfo getPortInfo(const QString& portName);
    static bool testPortConnection(const QString& portName, int baudRate = 9600, 
                                   int dataBits = 8, char parity = 'N', int stopBits = 1);
    static QStringList scanDevices(const QString& portName, int baudRate = 9600,
                                   int startSlaveId = 1, int endSlaveId = 247);
    static QString getPortDiagnostics(const QString& portName);
    static QString getConnectionDiagnostics(const QString& portName, int baudRate, 
                                            int dataBits, char parity, int stopBits);
    static QString getSerialDiagnosticReport();
    static QStringList getSerialRecommendations(const QString& portName = QString());
    static QStringList getQuickFixes();
};
```

##### 使用示例

```cpp
#include "modbusrwmanager.h"

// 1. 获取所有可用串口
QStringList availablePorts = ModbusRwManager::getAvailablePorts();
qDebug() << "可用串口:" << availablePorts;

// 2. 获取特定串口详细信息
QString targetPort = "COM3";
if (availablePorts.contains(targetPort)) {
    ModbusManager::PortInfo portInfo = ModbusRwManager::getPortInfo(targetPort);
    qDebug() << QString("串口 %1 信息:").arg(targetPort);
    qDebug() << " - 描述:" << portInfo.description;
    qDebug() << " - 制造商:" << portInfo.manufacturer;
    qDebug() << " - 系统位置:" << portInfo.systemLocation;
    qDebug() << " - 是否可访问:" << (portInfo.accessible ? "是" : "否");
}

// 3. 测试串口连接
bool connectionOk = ModbusRwManager::testPortConnection(targetPort, 9600, 8, 'N', 1);
if (connectionOk) {
    qDebug() << "串口连接测试成功";
    
    // 4. 扫描Modbus设备
    QStringList foundDevices = ModbusRwManager::scanDevices(targetPort, 9600, 1, 10);
    qDebug() << "发现的Modbus设备:" << foundDevices;
} else {
    qDebug() << "串口连接测试失败";
    
    // 5. 获取诊断信息
    QString diagnostics = ModbusRwManager::getPortDiagnostics(targetPort);
    qDebug() << "端口诊断信息:" << diagnostics;
    
    // 6. 获取修复建议
    QStringList recommendations = ModbusRwManager::getSerialRecommendations(targetPort);
    qDebug() << "修复建议:";
    for (const QString& rec : recommendations) {
        qDebug() << " -" << rec;
    }
}

// 7. 获取系统级诊断报告 
QString systemReport = ModbusRwManager::getSerialDiagnosticReport();
qDebug() << "系统诊断报告:" << systemReport;
```

##### 在项目中集成诊断功能

```cpp
class MyModbusApplication : public QWidget {
    Q_OBJECT
private:
    ModbusRwManager* rwManager;
    QComboBox* portComboBox;
    QTextEdit* diagnosticTextEdit;
    
private slots:
    void refreshPorts() {
        portComboBox->clear();
        QStringList ports = ModbusRwManager::getAvailablePorts();
        portComboBox->addItems(ports);
    }
    
    void runDiagnostics() {
        QString selectedPort = portComboBox->currentText();
        if (selectedPort.isEmpty()) return;
        
        // 运行完整诊断流程
        QString report = QString("=== 串口诊断报告 ===\n\n");
        
        // 基本信息
        ModbusManager::PortInfo info = ModbusRwManager::getPortInfo(selectedPort);
        report += QString("端口: %1\n").arg(info.portName);
        report += QString("描述: %1\n").arg(info.description);
        report += QString("制造商: %1\n").arg(info.manufacturer);
        report += QString("可访问: %1\n\n").arg(info.accessible ? "是" : "否");
        
        // 连接测试
        bool connected = ModbusRwManager::testPortConnection(selectedPort);
        report += QString("连接测试: %1\n\n").arg(connected ? "成功" : "失败");
        
        if (connected) {
            // 设备扫描
            QStringList devices = ModbusRwManager::scanDevices(selectedPort);
            report += QString("发现设备: %1个\n").arg(devices.size());
            for (const QString& device : devices) {
                report += QString(" - %1\n").arg(device);
            }
        } else {
            // 故障诊断
            QString portDiag = ModbusRwManager::getPortDiagnostics(selectedPort);
            report += QString("故障诊断:\n%1\n\n").arg(portDiag);
            
            QStringList fixes = ModbusRwManager::getSerialRecommendations(selectedPort);
            report += "建议解决方案:\n";
            for (const QString& fix : fixes) {
                report += QString(" - %1\n").arg(fix);
            }
        }
        
        diagnosticTextEdit->setText(report);
    }
};
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
        
        return true;    }
};
```

## 串口诊断功能

### 1. 功能概述

MyOperation系统集成了强大的串口诊断功能，所有诊断工具已统一整合到`ModbusManager`类中，提供完整的串口问题检测、分析和解决方案。这些功能可以帮助开发者快速定位和解决串口通信问题。

**主要功能：**
- 🔍 串口设备自动检测
- 📊 详细的串口信息获取
- 🧪 串口连接测试
- 📝 诊断报告生成
- 💡 智能建议和快速修复
- 🖥️ 系统信息收集

### 2. 核心诊断API

#### 2.1 串口信息结构

```cpp
// ModbusManager 中定义的串口信息结构
struct PortInfo {
    QString portName;        // 串口名称 (如 COM1, /dev/ttyS0)
    QString description;     // 设备描述
    QString manufacturer;    // 制造商信息
    QString systemLocation;  // 系统位置
    bool exists;            // 串口是否存在
    bool accessible;        // 是否可访问
    bool inUse;            // 是否被占用
    QString errorString;    // 错误信息
};
```

#### 2.2 基础诊断方法

```cpp
class ModbusManager {
public:
    // 获取系统中所有可用串口列表
    static QStringList getAvailablePorts();
    
    // 获取指定串口的详细信息
    static PortInfo getPortInfo(const QString& portName);
    
    // 测试串口连接参数
    static bool testPortConnection(const QString& portName, 
                                 int baudRate, int dataBits, 
                                 char parity, int stopBits);
    
    // 生成完整的串口诊断报告
    static QString generateDiagnosticReport();
    
    // 对特定串口进行详细诊断
    static QString getPortDiagnostics(const QString& portName);
    
    // 获取针对特定串口的建议
    static QStringList getRecommendations(const QString& portName);
    
    // 获取通用的快速解决方案
    static QStringList getQuickFixes();
    
    // 生成系统信息报告
    static QString generateSystemInfo();
};
```

#### 2.3 ModbusRwManager 诊断接口

ModbusRwManager 提供完整的串口诊断接口，是应用层访问诊断功能的主要入口：

```cpp
class ModbusRwManager : public QObject {
public:
    // 串口诊断接口 - 通过内部 ModbusManager 实现
    static QStringList getAvailablePorts();
    static ModbusManager::PortInfo getPortInfo(const QString& portName);
    static bool testPortConnection(const QString& portName, int baudRate = 9600, 
                                   int dataBits = 8, char parity = 'N', int stopBits = 1);
    static QStringList scanDevices(const QString& portName, int baudRate = 9600,
                                   int startSlaveId = 1, int endSlaveId = 247);
    static QString getPortDiagnostics(const QString& portName);
    static QString getConnectionDiagnostics(const QString& portName, int baudRate, 
                                            int dataBits, char parity, int stopBits);
    static QString getSerialDiagnosticReport();
    static QStringList getSerialRecommendations(const QString& portName = QString());
    static QStringList getQuickFixes();
};
```

**接口特点：**
- ✅ **静态方法设计** - 无需实例化即可使用
- ✅ **与ModbusManager集成** - 内部调用ModbusManager的诊断功能
- ✅ **应用层友好** - 提供高级抽象接口
- ✅ **完整功能覆盖** - 涵盖所有串口诊断功能

**推荐使用场景：**
- 应用启动时的串口检测
- 用户界面中的诊断工具
- 自动化测试和验证
- 故障排除和技术支持

### 3. 诊断功能详解

#### 3.1 串口检测和信息获取

```cpp
// 示例：获取所有可用串口
QStringList ports = ModbusManager::getAvailablePorts();
for (const QString& port : ports) {
    qDebug() << "发现串口:" << port;
}

// 示例：获取串口详细信息
ModbusManager::PortInfo info = ModbusManager::getPortInfo("COM1");
qDebug() << "串口名称:" << info.portName;
qDebug() << "设备描述:" << info.description;
qDebug() << "制造商:" << info.manufacturer;
qDebug() << "是否可用:" << (info.accessible ? "是" : "否");
```

#### 3.2 连接测试

```cpp
// 示例：测试串口连接
QString portName = "COM1";
bool canConnect = ModbusManager::testPortConnection(portName, 9600, 8, 'N', 1);

if (canConnect) {
    qDebug() << "串口连接测试成功";
} else {
    qDebug() << "串口连接测试失败";
    
    // 获取诊断建议
    QStringList recommendations = ModbusManager::getRecommendations(portName);
    for (const QString& rec : recommendations) {
        qDebug() << "建议:" << rec;
    }
}
```

#### 3.3 诊断报告生成

```cpp
// 示例：生成完整诊断报告
QString fullReport = ModbusManager::generateDiagnosticReport();
qDebug() << "=== 串口诊断报告 ===";
qDebug() << fullReport;

// 示例：针对特定串口的诊断
QString specificReport = ModbusManager::getPortDiagnostics("COM1");
qDebug() << "=== COM1 详细诊断 ===";
qDebug() << specificReport;
```

### 4. 实际应用示例

#### 4.1 连接前的诊断检查

```cpp
bool connectWithDiagnostics(const QString& portName) {
    // 1. 检查串口是否存在
    ModbusManager::PortInfo portInfo = ModbusManager::getPortInfo(portName);
    if (!portInfo.exists) {
        qWarning() << "串口不存在:" << portName;
        return false;
    }
    
    // 2. 检查串口是否可访问
    if (!portInfo.accessible) {
        qWarning() << "串口不可访问:" << portInfo.errorString;
        
        // 显示建议
        QStringList recommendations = ModbusManager::getRecommendations(portName);
        for (const QString& rec : recommendations) {
            qInfo() << "建议:" << rec;
        }
        return false;
    }
    
    // 3. 测试连接参数
    if (!ModbusManager::testPortConnection(portName, 9600, 8, 'N', 1)) {
        qWarning() << "串口连接测试失败";
        
        // 生成详细诊断报告
        QString report = ModbusManager::diagnoseSerialPort(portName);
        qDebug() << report;
        return false;
    }
    
    // 4. 尝试实际连接
    ModbusManager modbus;
    return modbus.connectRTU(portName, 9600, 8, 'N', 1);
}
```

#### 4.2 故障排除助手

```cpp
void troubleshootSerialIssues() {
    qDebug() << "=== 串口故障排除助手 ===";
    
    // 1. 获取系统信息
    QString systemInfo = ModbusManager::generateSystemInfo();
    qDebug() << systemInfo;
    
    // 2. 检查可用串口
    QStringList ports = ModbusManager::getAvailablePorts();
    if (ports.isEmpty()) {
        qWarning() << "未检测到任何串口设备";
        
        // 显示快速解决方案
        QStringList fixes = ModbusManager::getQuickFixes();
        qDebug() << "\n快速解决方案:";
        for (const QString& fix : fixes) {
            qDebug() << fix;
        }
        return;
    }
    
    // 3. 逐个诊断每个串口
    for (const QString& port : ports) {
        qDebug() << "\n正在诊断串口:" << port;
        QString diagnosis = ModbusManager::diagnoseSerialPort(port);
        qDebug() << diagnosis;
    }
}
```

#### 4.3 自动修复建议

```cpp
class SerialPortAssistant {
public:
    static void autoFixSerialIssues(const QString& portName) {
        ModbusManager::PortInfo info = ModbusManager::getPortInfo(portName);
        
        if (!info.exists) {
            qWarning() << "串口不存在，可能的原因：";
            qWarning() << "1. 设备未连接";
            qWarning() << "2. 驱动程序未安装";
            qWarning() << "3. 设备故障";
            return;
        }
        
        if (info.inUse) {
            qWarning() << "串口被占用，建议：";
            qWarning() << "1. 关闭其他使用该串口的程序";
            qWarning() << "2. 检查设备管理器中的串口状态";
            qWarning() << "3. 重启应用程序";
        }
        
        if (!info.accessible) {
            qWarning() << "串口不可访问，建议：";
            qWarning() << "1. 以管理员权限运行程序";
            qWarning() << "2. 检查串口权限设置";
            qWarning() << "3. 重新安装串口驱动";
        }
        
        // 获取系统生成的建议
        QStringList recommendations = ModbusManager::getRecommendations(portName);
        if (!recommendations.isEmpty()) {
            qInfo() << "\n系统建议：";
            for (const QString& rec : recommendations) {
                qInfo() << rec;
            }
        }
    }
};
```

### 5. 与现有代码的集成

#### 5.1 替换旧的诊断代码

**旧代码（已废弃）：**
```cpp
// 不再使用
#include "serial_diagnostic.h"
#include "serial_diagnostic.h"
#include "modbus_troubleshooter.h"

SerialDiagnostic::PortInfo info = SerialDiagnostic::getPortInfo(portName);
QString report = ModbusTroubleshooter::diagnoseSerialPort(portName);
```

**新代码（推荐）：**
```cpp
// 直接使用 ModbusManager
#include "modbusmanager.h"

ModbusManager::PortInfo info = ModbusManager::getPortInfo(portName);
QString report = ModbusManager::getPortDiagnostics(portName);
QString report = ModbusManager::getPortDiagnostics(portName);
```

#### 5.2 在现有应用中添加诊断功能

```cpp
class SerialDialog : public QDialog {
private slots:
    void onDiagnosePort() {
        QString portName = portComboBox->currentText();
          // 生成诊断报告
        QString report = ModbusManager::getPortDiagnostics(portName);
        
        // 显示在对话框中
        QMessageBox::information(this, "串口诊断报告", report);
        
        // 获取建议
        QStringList recommendations = ModbusManager::getRecommendations(portName);
        if (!recommendations.isEmpty()) {
            QString tips = "建议：\n" + recommendations.join("\n");
            QMessageBox::information(this, "诊断建议", tips);
        }
    }
};
```

### 6. 诊断信息格式

诊断报告包含以下信息：

```
=== 串口诊断报告 ===
扫描时间: 2025-06-06 14:30:00

系统信息:
- 操作系统: Windows 10
- Qt版本: 5.15.2
- 串口驱动: 已安装

可用串口: COM1, COM3, COM4

COM1 详细信息:
- 描述: USB Serial Port
- 制造商: FTDI
- 位置: \\.\COM1
- 状态: 可用
- 占用: 否
- 测试结果: 连接成功 (9600,8,N,1)

建议:
- 串口工作正常，可以使用
- 建议波特率: 9600, 19200, 115200
```

### 7. 最佳实践

1. **在连接前总是进行诊断**
   ```cpp
   if (!ModbusManager::getPortInfo(portName).accessible) {
       // 处理不可访问的情况
       return;
   }
   ```

2. **保存诊断日志**
   ```cpp
   QString report = ModbusManager::generateDiagnosticReport();
   // 保存到日志文件
   ```

3. **用户友好的错误提示**
   ```cpp
   QStringList fixes = ModbusManager::getQuickFixes();
   // 显示给用户
   ```

4. **定期检查串口状态**
   ```cpp
   QTimer* timer = new QTimer(this);
   connect(timer, &QTimer::timeout, this, &MyClass::checkSerialPorts);
   timer->start(30000); // 每30秒检查一次
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
    QMutex poolMutex;
    int maxConnections;
    
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

## 📚 快速参考

### 串口诊断功能快速指南

#### 基本诊断方法
```cpp
// 通过 ModbusManager 使用
#include "modbusmanager.h"

// 1. 获取可用串口
QStringList ports = ModbusManager::getAvailablePorts();

// 2. 获取串口信息
ModbusManager::PortInfo info = ModbusManager::getPortInfo("COM1");

// 3. 测试连接
bool ok = ModbusManager::testPortConnection("COM1", 9600, 8, 'N', 1);

// 4. 生成诊断报告
QString report = ModbusManager::getSerialDiagnosticReport();

// 5. 获取建议
QStringList tips = ModbusManager::getSerialRecommendations("COM1");
```

#### 通过 ModbusRwManager 使用
```cpp
// 通过 ModbusRwManager 使用（推荐用于应用层）
#include "modbusrwmanager.h"

// 所有方法都是静态的，无需实例化
QStringList ports = ModbusRwManager::getAvailablePorts();
ModbusManager::PortInfo info = ModbusRwManager::getPortInfo("COM1");
bool ok = ModbusRwManager::testPortConnection("COM1");
QStringList devices = ModbusRwManager::scanDevices("COM1");
QString report = ModbusRwManager::getSerialDiagnosticReport();
```

#### 诊断流程模板
```cpp
void diagnoseAndConnect(const QString& portName) {
    // 1. 检查串口基本信息
    ModbusManager::PortInfo info = ModbusRwManager::getPortInfo(portName);
    if (!info.exists) {
        qWarning() << "串口不存在:" << portName;
        return;
    }
    
    // 2. 测试连接
    if (!ModbusRwManager::testPortConnection(portName)) {
        // 3. 获取诊断信息
        QString diagnostics = ModbusRwManager::getPortDiagnostics(portName);
        qDebug() << "诊断信息:" << diagnostics;
        
        // 4. 获取修复建议
        QStringList recommendations = ModbusRwManager::getSerialRecommendations(portName);
        for (const QString& rec : recommendations) {
            qInfo() << "建议:" << rec;
        }
        return;
    }
    
    // 5. 扫描设备
    QStringList devices = ModbusRwManager::scanDevices(portName);
    qDebug() << "发现设备:" << devices;
    
    // 6. 建立实际连接
    // ... 你的连接代码 ...
}
```

### 常用错误处理模式

#### 错误码处理
```cpp
ModbusErrorCode handleModbusOperation() {
    try {
        // 执行 Modbus 操作
        QVector<quint16> values = modbusManager->readHoldingRegisters("PLC01", 0, 10);
        return MODBUS_SUCCESS;
    } catch (const ModbusException& e) {
        qWarning() << "Modbus error:" << e.what();
        
        if (ModbusErrorHandler::isRetryableError(e.errorCode())) {
            // 可重试错误
            return e.errorCode();
        } else {
            // 致命错误，需要重新连接
            modbusManager->reconnectDevice("PLC01");
            return e.errorCode();
        }
    }
}
```

#### 自动重连模式
```cpp
class AutoReconnectModbus : public QObject {
    Q_OBJECT
private:
    ModbusManager* manager;
    QTimer* reconnectTimer;
    int retryCount;
    
public slots:
    void onDeviceDisconnected(const QString& deviceId) {
        retryCount = 0;
        reconnectTimer->start(1000); // 1秒后开始重连
    }
    
    void attemptReconnect() {
        if (retryCount < 5) { // 最多重试5次
            if (manager->connectDevice("PLC01")) {
                reconnectTimer->stop();
                retryCount = 0;
                qInfo() << "重连成功";
            } else {
                retryCount++;
                reconnectTimer->start(retryCount * 2000); // 递增延迟
            }
        } else {
            reconnectTimer->stop();
            qError() << "重连失败，已达到最大重试次数";
        }
    }
};
```

### 配置文件模板

#### 基本设备配置
```json
{
    "modbus_devices": [
        {
            "id": "MainPLC",
            "type": "tcp",
            "ip_address": "192.168.1.100",
            "port": 502,
            "slave_id": 1,
            "connection_timeout": 5000,
            "response_timeout": 1000,
            "auto_connect": true,
            "retry_count": 3,
            "retry_interval": 2000
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
            "auto_connect": true
        }
    ]
}
```

### 性能优化清单

#### ✅ 连接优化
- [ ] 使用连接池避免频繁创建连接
- [ ] 合理设置超时参数
- [ ] 实现智能重连机制
- [ ] 批量操作减少网络开销

#### ✅ 数据处理优化  
- [ ] 使用数据缓存减少重复读取
- [ ] 实现增量更新策略
- [ ] 合理的轮询间隔设置
- [ ] 异步处理避免界面卡顿

#### ✅ 错误处理优化
- [ ] 分类处理不同错误类型
- [ ] 记录详细的操作日志
- [ ] 实现故障自动恢复
- [ ] 用户友好的错误提示

### 故障排除检查清单

#### 🔧 连接问题
- [ ] 检查网络连通性 (`ping` 目标设备)
- [ ] 验证端口和IP地址配置
- [ ] 确认防火墙和安全软件设置
- [ ] 检查设备是否支持Modbus协议

#### 🔧 串口问题  
- [ ] 确认串口设备已连接
- [ ] 检查串口参数配置（波特率、数据位等）
- [ ] 验证串口权限和占用情况
- [ ] 测试串口硬件功能

#### 🔧 数据问题
- [ ] 验证寄存器地址和范围
- [ ] 检查数据类型转换正确性
- [ ] 确认设备从站ID配置
- [ ] 测试功能码支持情况

---

*本文档最后更新时间: 2025年6月5日*
