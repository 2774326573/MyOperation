# Modbus 管理器文档

## 概述

`ModbusManager` 是一个基于 Qt 的 Modbus 通信管理类，提供了对 Modbus RTU 和 TCP 协议的完整支持，包括连接管理、数据读写、错误处理和串口诊断功能。

## 文件信息

- **文件路径**: `inc/modbus/modbusmanager.h`
- **基类**: `QObject`
- **Qt 模块**: Core, Network, SerialPort

## 主要特性

### 连接管理
- Modbus RTU（串口通信）
- Modbus TCP（网络通信）
- 自动重连机制
- 连接状态监控

### 数据操作
- 线圈（Coils）读写
- 离散输入（Discrete Inputs）读取
- 保持寄存器（Holding Registers）读写
- 输入寄存器（Input Registers）读取

### 高级功能
- 串口诊断和故障排除
- 连接超时管理
- 调试模式
- 异步操作支持

## 枚举和结构体

### 连接类型
```cpp
enum class ConnectionType {
    RTU,    // 串口连接
    TCP,    // 网络连接
};
```

### 数据类型
```cpp
enum DataType {
    Coils,              // 线圈
    DiscreteInputs,     // 离散输入
    HoldingRegisters,   // 保持寄存器
    InputRegisters,     // 输入寄存器
};
```

### 串口信息结构体
```cpp
struct PortInfo {
    QString portName;           // 端口名称
    QString description;        // 端口描述
    QString manufacturer;       // 制造商
    QString serialNumber;       // 序列号
    QString systemLocation;     // 系统位置
    quint16 vendorId;          // 厂商ID
    quint16 productId;         // 产品ID
    bool exists;               // 端口是否存在
    bool accessible;           // 端口是否可访问
    bool inUse;                // 端口是否被占用
    bool isAvailable;          // 端口是否可用
    bool isBusy;               // 端口是否忙碌
    QString errorString;       // 错误信息
    int errorCode;             // 错误代码
};
```

## 核心方法

### 连接管理
```cpp
// RTU 连接
bool connectRTU(const QString& port, int baudRate = 9600, 
                int dataBits = 8, char parity = 'N', int stopBits = 1);

// RTU 连接（整数校验位版本）
bool connectRTU(const QString& port, int baudRate, int dataBits, 
                int parity, int stopBits);

// TCP 连接
bool connectTCP(const QString& ip, int port = 502);

// 断开连接
void disconnect();

// 检查连接状态
bool isConnected() const;
```

### 从站管理
```cpp
// 设置从站地址
void setSlaveID(int id);

// 获取从站地址
int getSlaveID() const;
```

### 超时设置
```cpp
// 设置通用超时时间
void setTimeout(int timeoutMsec);

// 设置响应超时时间
void setResponseTimeout(int timeoutMsec);
```

### 数据读取
```cpp
// 读取线圈
bool readCoils(int address, int count, QVector<bool>& values);

// 读取离散输入
bool readDiscreteInputs(int address, int count, QVector<bool>& values);

// 读取保持寄存器
bool readHoldingRegisters(int address, int count, QVector<quint16>& values);

// 读取输入寄存器
bool readInputRegisters(int address, int count, QVector<quint16>& values);
```

### 数据写入
```cpp
// 写入单个线圈
bool writeSingleCoil(int address, bool value);

// 写入单个寄存器
bool writeSingleRegister(int address, quint16 value);

// 写入多个线圈
bool writeMultipleCoils(int address, const QVector<bool>& values);

// 写入多个寄存器
bool writeMultipleRegisters(int address, const QVector<quint16>& values);
```

### 高级操作
```cpp
// 读写寄存器（原子操作）
bool readWriteRegisters(int readAddress, int readCount, 
                       QVector<quint16>& readValues,
                       int writeAddress, const QVector<quint16>& writeValues);

// 掩码写入寄存器
bool maskWriteRegister(int address, quint16 andMask, quint16 orMask);
```

## 串口诊断功能

### 静态诊断方法
```cpp
// 获取可用串口列表
static QStringList getAvailablePorts();

// 获取串口详细信息
static PortInfo getPortInfo(const QString &portName);

// 测试串口连接
static bool testPortConnection(const QString &portName, int baudRate, 
                              int dataBits, char parity, int stopBits);

// 生成诊断报告
static QString generateDiagnosticReport();

// 诊断指定串口
static QString diagnoseSerialPort(const QString &portName);

// 获取建议
static QStringList getRecommendations(const QString &portName);

// 获取快速解决方案
static QStringList getQuickFixes();

// 生成系统信息
static QString generateSystemInfo();

// 检查端口可用性
static bool isPortAvailable(const QString &portName);
```

### 实例诊断方法
```cpp
// 检查端口忙碌状态
bool isPortBusy(const QString &portName);

// 测试端口连通性
bool testPortConnectivity(const QString &portName);

// 获取端口诊断信息
QVariantMap getPortDiagnostics(const QString &portName);
```

## 信号（Signals）

```cpp
// 连接成功
void connected();

// 断开连接
void disconnected();

// 错误发生
void errorOccurred(const QString& error);

// 数据接收
void dataReceived(DataType type, int address, const QVariant& data);

// 数据发送
void dataSent(DataType type, int address, const QVariant& data);

// 信息日志
void infoLog(const QString& message);
```

## 使用示例

### RTU 连接示例
```cpp
#include "modbusmanager.h"

ModbusManager *manager = new ModbusManager(this);

// 连接信号
connect(manager, &ModbusManager::connected, this, [=](){
    qDebug() << "RTU 连接成功";
});

connect(manager, &ModbusManager::errorOccurred, this, [=](const QString& error){
    qDebug() << "错误:" << error;
});

// 连接 RTU
if (manager->connectRTU("COM1", 9600, 8, 'N', 1)) {
    manager->setSlaveID(1);
    
    // 读取保持寄存器
    QVector<quint16> values;
    if (manager->readHoldingRegisters(0, 10, values)) {
        for (int i = 0; i < values.size(); ++i) {
            qDebug() << QString("寄存器 %1: %2").arg(i).arg(values[i]);
        }
    }
}
```

### TCP 连接示例
```cpp
ModbusManager *manager = new ModbusManager(this);

// 连接 TCP
if (manager->connectTCP("192.168.1.100", 502)) {
    manager->setSlaveID(1);
    manager->setTimeout(5000);  // 5秒超时
    
    // 写入单个寄存器
    if (manager->writeSingleRegister(100, 1234)) {
        qDebug() << "写入成功";
    }
    
    // 读取输入寄存器
    QVector<quint16> inputs;
    if (manager->readInputRegisters(0, 5, inputs)) {
        qDebug() << "读取成功:" << inputs;
    }
}
```

### 串口诊断示例
```cpp
// 获取可用串口
QStringList ports = ModbusManager::getAvailablePorts();
qDebug() << "可用串口:" << ports;

// 诊断特定串口
for (const QString& port : ports) {
    ModbusManager::PortInfo info = ModbusManager::getPortInfo(port);
    qDebug() << QString("端口: %1, 描述: %2, 可用: %3")
                .arg(info.portName)
                .arg(info.description)
                .arg(info.isAvailable ? "是" : "否");
}

// 生成诊断报告
QString report = ModbusManager::generateDiagnosticReport();
qDebug() << "诊断报告:\n" << report;

// 测试串口连接
if (ModbusManager::testPortConnection("COM1", 9600, 8, 'N', 1)) {
    qDebug() << "串口测试成功";
} else {
    qDebug() << "串口测试失败";
}
```

### 数据监控示例
```cpp
ModbusManager *manager = new ModbusManager(this);

// 监控数据变化
connect(manager, &ModbusManager::dataReceived, this, 
        [=](DataType type, int address, const QVariant& data){
    switch (type) {
    case DataType::HoldingRegisters:
        qDebug() << QString("保持寄存器 %1: %2").arg(address).arg(data.toString());
        break;
    case DataType::Coils:
        qDebug() << QString("线圈 %1: %2").arg(address).arg(data.toBool() ? "ON" : "OFF");
        break;
    // ... 其他类型
    }
});

// 启用调试模式
manager->setDebugMode(true);
```

## 错误处理

### 错误信息获取
```cpp
// 获取最后错误
QString error = manager->getLastError();
int errorCode = manager->getLastErrorCode();

if (!error.isEmpty()) {
    qDebug() << QString("错误 %1: %2").arg(errorCode).arg(error);
}
```

### 连接信息
```cpp
// 获取连接信息
QString info = manager->getConnectionInfo();
qDebug() << "连接信息:" << info;
```

## 配置建议

### RTU 参数配置
```cpp
// 标准配置
manager->connectRTU("COM1", 9600, 8, 'N', 1);   // 9600,8,N,1

// 高速配置
manager->connectRTU("COM1", 115200, 8, 'N', 1); // 115200,8,N,1

// 工业标准配置
manager->connectRTU("COM1", 19200, 8, 'E', 1);  // 19200,8,E,1
```

### 超时配置
```cpp
// 响应超时（毫秒）
manager->setResponseTimeout(1000);  // 1秒

// 通用超时
manager->setTimeout(5000);          // 5秒
```

## 性能优化

### 批量操作
```cpp
// 批量读取寄存器
QVector<quint16> values;
manager->readHoldingRegisters(0, 100, values);  // 一次读取100个寄存器

// 批量写入寄存器
QVector<quint16> writeValues(50, 1234);
manager->writeMultipleRegisters(100, writeValues);
```

### 连接复用
```cpp
// 保持连接，避免频繁连接/断开
if (!manager->isConnected()) {
    manager->connectRTU("COM1", 9600, 8, 'N', 1);
}
```

## 故障排除

### 常见问题
1. **串口占用**
   ```cpp
   if (!ModbusManager::isPortAvailable("COM1")) {
       qDebug() << "串口被占用";
   }
   ```

2. **连接超时**
   ```cpp
   manager->setTimeout(10000);  // 增加超时时间
   ```

3. **参数错误**
   ```cpp
   // 检查串口参数
   if (!ModbusManager::testPortConnection("COM1", 9600, 8, 'N', 1)) {
       qDebug() << "串口参数配置错误";
   }
   ```

## 线程安全

`ModbusManager` 使用 `QMutex` 确保线程安全：
```cpp
// 可以在不同线程中安全使用
QThread *workerThread = new QThread;
manager->moveToThread(workerThread);
```

## 相关文档

- [Modbus 核心文档](modbus.md)
- [Modbus RTU 文档](modbus-rtu.md)
- [Modbus TCP 文档](modbus-tcp.md)
- [Modbus 版本信息](modbus-version.md)
