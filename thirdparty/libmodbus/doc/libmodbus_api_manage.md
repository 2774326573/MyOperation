# libmodbus API 使用手册

## 目录
1. [简介](#简介)
2. [项目架构](#项目架构)
3. [核心API](#核心api)
   - [ModbusManager](#modbusmanager)
   - [ModbusRwManager](#modbusrwmanager)
   - [SerialDiagnostic](#serialdiagnostic)
   - [ModbusTroubleshooter](#modbustroubleshooter)
4. [使用示例](#使用示例)
5. [错误处理](#错误处理)
6. [编译配置](#编译配置)
7. [常见问题](#常见问题)
8. [附录](#附录)

## 简介

本项目基于 **libmodbus 3.1.1** 构建，提供了完整的Modbus协议C++封装库。支持TCP和RTU两种通信方式，并集成了强大的诊断和故障排除工具。主要特性：

- 🔗 支持Modbus TCP/RTU协议
- 🛠️ 提供完整的Qt C++封装
- 🔍 内置串口诊断工具
- 🐛 智能故障排除系统
- 📊 详细的操作结果反馈
- ⚡ 异步操作支持

## 项目架构

```
libmodbus/
├── inc/modbus/                 # 头文件目录
│   ├── modbusmanager.h        # 核心连接管理器
│   ├── modbus_rw_manager.h    # 读写操作管理器
│   ├── serial_diagnostic.h    # 串口诊断工具
│   └── modbus_troubleshooter.h # 故障排除工具
├── src/                       # 实现文件目录
├── lib/                       # 静态库文件
└── doc/                       # 文档目录
```

## 核心API

### ModbusManager

**ModbusManager** 是核心的Modbus连接管理类，提供了所有基础的Modbus操作功能。

#### 头文件引用
```cpp
#include "modbusmanager.h"
```

#### 主要方法

##### 连接管理
```cpp
// TCP连接
bool connectTCP(const QString &ip, int port = 502);

// RTU连接  
bool connectRTU(const QString &port, int baudRate = 9600, 
                int dataBits = 8, char parity = 'N', int stopBits = 1);

// 断开连接
void disconnect();

// 检查连接状态
bool isConnected() const;
```

##### 基本设置
```cpp
// 设置从站ID
void setSlaveID(int id);

// 获取从站ID
int getSlaveID() const;

// 设置超时时间（毫秒）
void setTimeout(int timeoutMsec);

// 设置响应超时（秒, 微秒）
void setResponseTimeout(int seconds, int microseconds);

// 启用调试模式
void setDebugMode(bool enable);

// 获取最后一次错误信息
QString getLastError() const;

// 获取最后一次错误代码
int getLastErrorCode() const;
```

##### 数据读取
```cpp
// 读取线圈状态 (Function Code 01)
bool readCoils(int address, int count, QVector<bool>& values);

// 读取离散输入 (Function Code 02)
bool readDiscreteInputs(int address, int count, QVector<bool>& values);

// 读取保持寄存器 (Function Code 03)
bool readHoldingRegisters(int address, int count, QVector<quint16>& values);

// 读取输入寄存器 (Function Code 04)
bool readInputRegisters(int address, int count, QVector<quint16>& values);
```

##### 数据写入
```cpp
// 写单个线圈 (Function Code 05)
bool writeSingleCoil(int address, bool value);

// 写单个寄存器 (Function Code 06)
bool writeSingleRegister(int address, quint16 value);

// 写多个线圈 (Function Code 15)
bool writeMultipleCoils(int address, const QVector<bool>& values);

// 写多个寄存器 (Function Code 16)
bool writeMultipleRegisters(int address, const QVector<quint16>& values);
```

##### 高级操作
```cpp
// 掩码写寄存器 (Function Code 22)
bool maskWriteRegister(int address, quint16 andMask, quint16 orMask);

// 读写寄存器 (Function Code 23)
bool readWriteRegisters(int readAddress, int readCount, QVector<quint16>& readValues,
                       int writeAddress, const QVector<quint16>& writeValues);
```

##### 信号
```cpp
// 错误发生信号
void errorOccurred(const QString& error);

// 连接状态改变信号
void connectionStateChanged(bool connected);

// 数据接收信号
void dataReceived(const QByteArray& data);
```

### ModbusRwManager

**ModbusRwManager** 提供高级的读写操作管理，支持结构化的操作参数和结果。

#### 头文件引用
```cpp
#include "modbus_rw_manager.h"
```

#### 枚举定义

##### 功能码枚举
```cpp
enum FunctionCode {
    ReadCoils = 1,              // 01 - 读线圈
    ReadDiscreteInputs = 2,     // 02 - 读离散输入  
    ReadHoldingRegisters = 3,   // 03 - 读保持寄存器
    ReadInputRegisters = 4,     // 04 - 读输入寄存器
    WriteSingleCoil = 5,        // 05 - 写单个线圈
    WriteSingleRegister = 6,    // 06 - 写单个寄存器
    WriteMultipleCoils = 15,    // 15 - 写多个线圈
    WriteMultipleRegisters = 16 // 16 - 写多个寄存器
};
```

#### 结构体定义

##### 操作参数结构
```cpp
struct OperationParams {
    FunctionCode functionCode;  // 功能码
    int slaveId;               // 从站ID
    int startAddress;          // 起始地址
    int quantity;              // 数量
    QString writeValue;        // 写入值（仅写操作使用）
    
    // 构造函数
    OperationParams();
    OperationParams(FunctionCode fc, int slave, int addr, int qty, 
                    const QString& value = QString());
};
```

##### 操作结果结构
```cpp
struct OperationResult {
    bool success;              // 操作是否成功
    QString message;           // 结果消息
    QVariant data;             // 读取的数据（仅读操作）
    QString formattedResult;   // 格式化的结果字符串
    int errorCode;             // 错误代码
    qint64 executionTime;      // 执行时间（毫秒）
    
    // 构造函数
    OperationResult();
    OperationResult(bool success, const QString& message);
};
```

#### 主要方法
```cpp
// 设置Modbus管理器
void setModbusManager(ModbusManager* manager);

// 执行读取操作
OperationResult performRead(const OperationParams& params);

// 执行写入操作
OperationResult performWrite(const OperationParams& params);

// 批量操作
QList<OperationResult> performBatchOperations(const QList<OperationParams>& paramsList);

// 验证参数
bool validateParams(const OperationParams& params, QString& errorMessage);

// 格式化数据
QString formatData(const QVariant& data, FunctionCode functionCode);
```

#### 信号
```cpp
// 操作完成信号
void operationCompleted(const OperationResult& result);

// 批量操作进度信号
void batchProgress(int current, int total);

// 批量操作完成信号
void batchCompleted(const QList<OperationResult>& results);
```

### SerialDiagnostic

**SerialDiagnostic** 提供完整的串口诊断功能。

#### 头文件引用
```cpp
#include "serial_diagnostic.h"
```

#### 结构体定义

##### 串口信息结构
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
    QString errorString;       // 错误信息
    int errorCode;             // 错误代码
    qint64 timestamp;          // 信息获取时间戳
    
    // 构造函数
    PortInfo();
    PortInfo(const QString& name);
    
    // 转换为JSON
    QJsonObject toJson() const;
    
    // 从JSON加载
    static PortInfo fromJson(const QJsonObject& json);
};
```

#### 静态方法
```cpp
// 获取所有可用串口
static QStringList getAvailablePorts();

// 获取串口详细信息
static PortInfo getPortInfo(const QString& portName);

// 测试串口连接
static bool testPortConnection(const QString& portName, int baudRate,
                              int dataBits, char parity, int stopBits);

// 生成诊断报告
static QString generateDiagnosticReport();

// 获取针对特定端口的建议
static QStringList getRecommendations(const QString& portName);

// 检查端口权限
static bool checkPortPermissions(const QString& portName);

// 获取端口使用情况
static QString getPortUsageInfo(const QString& portName);

// 刷新端口列表
static void refreshPortList();
```

### ModbusTroubleshooter

**ModbusTroubleshooter** 专门用于Modbus连接问题诊断和故障排除。

#### 头文件引用
```cpp
#include "modbus_troubleshooter.h"
```

#### 静态方法
```cpp
// 诊断串口
static QString diagnoseSerialPort(const QString& portName);

// 测试串口连接
static bool testSerialConnection(const QString& portName, int baudRate,
                                int dataBits, char parity, int stopBits);

// 测试Modbus RTU连接
static bool testModbusRTU(const QString& portName, int slaveId,
                         int baudRate = 9600, int dataBits = 8,
                         char parity = 'N', int stopBits = 1);

// 测试Modbus TCP连接
static bool testModbusTCP(const QString& ip, int port = 502, int slaveId = 1);

// 获取快速修复方案
static QStringList getQuickFixes();

// 生成系统信息
static QString generateSystemInfo();

// 检查网络连接
static bool checkNetworkConnectivity(const QString& ip, int port);

// 诊断Modbus参数
static QString diagnoseModbusParameters(int slaveId, int baudRate,
                                       char parity, int dataBits, int stopBits);

// 生成完整诊断报告
static QString generateFullDiagnosticReport(const QString& connectionInfo);
```

## 使用示例

### 基本TCP连接示例

```cpp
#include "modbusmanager.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    ModbusManager modbus;
    
    // 启用调试模式
    modbus.setDebugMode(true);
    
    // 设置超时
    modbus.setTimeout(5000);  // 5秒超时
    
    // 连接到TCP服务器
    if (modbus.connectTCP("192.168.1.100", 502)) {
        qDebug() << "TCP连接成功";
        
        // 设置从站ID
        modbus.setSlaveID(1);
        
        // 读取保持寄存器
        QVector<quint16> values;
        if (modbus.readHoldingRegisters(0, 10, values)) {
            qDebug() << "读取成功，数据：";
            for (int i = 0; i < values.size(); ++i) {
                qDebug() << QString("  地址%1: %2").arg(i).arg(values[i]);
            }
        } else {
            qDebug() << "读取失败：" << modbus.getLastError();
        }
        
        // 写入单个寄存器
        if (modbus.writeSingleRegister(0, 1234)) {
            qDebug() << "写入成功";
            
            // 验证写入
            QVector<quint16> verifyValues;
            if (modbus.readHoldingRegisters(0, 1, verifyValues)) {
                qDebug() << "验证读取：" << verifyValues[0];
            }
        } else {
            qDebug() << "写入失败：" << modbus.getLastError();
        }
        
        // 读写寄存器组合操作
        QVector<quint16> writeData = {100, 200, 300};
        QVector<quint16> readData;
        if (modbus.readWriteRegisters(10, 3, readData, 0, writeData)) {
            qDebug() << "读写操作成功，读取数据：" << readData;
        }
        
        modbus.disconnect();
    } else {
        qDebug() << "连接失败：" << modbus.getLastError();
        qDebug() << "错误代码：" << modbus.getLastErrorCode();
    }
    
    return 0;
}
```

### RTU连接示例

```cpp
#include "modbusmanager.h"
#include "serial_diagnostic.h"
#include "modbus_troubleshooter.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    QString portName = "COM1";  // Windows
    // QString portName = "/dev/ttyS0";  // Linux
    // QString portName = "/dev/tty.usbserial-0001";  // macOS
    
    // 首先进行串口诊断
    qDebug() << "开始串口诊断...";
    
    // 检查串口是否存在
    SerialDiagnostic::PortInfo portInfo = SerialDiagnostic::getPortInfo(portName);
    if (!portInfo.exists) {
        qDebug() << "串口不存在：" << portName;
        
        // 显示可用串口
        QStringList availablePorts = SerialDiagnostic::getAvailablePorts();
        qDebug() << "可用串口：" << availablePorts;
        
        if (!availablePorts.isEmpty()) {
            portName = availablePorts.first();
            qDebug() << "使用第一个可用串口：" << portName;
            portInfo = SerialDiagnostic::getPortInfo(portName);
        } else {
            qDebug() << "没有可用的串口";
            return -1;
        }
    }
    
    // 显示串口信息
    qDebug() << "串口信息：";
    qDebug() << "  名称：" << portInfo.portName;
    qDebug() << "  描述：" << portInfo.description;
    qDebug() << "  制造商：" << portInfo.manufacturer;
    qDebug() << "  可访问：" << (portInfo.accessible ? "是" : "否");
    qDebug() << "  使用中：" << (portInfo.inUse ? "是" : "否");
    
    // 测试串口连接
    if (!SerialDiagnostic::testPortConnection(portName, 9600, 8, 'N', 1)) {
        qDebug() << "串口连接测试失败";
        
        // 获取诊断信息
        QString diagnostic = ModbusTroubleshooter::diagnoseSerialPort(portName);
        qDebug() << "诊断信息：" << diagnostic;
        
        // 获取建议
        QStringList recommendations = SerialDiagnostic::getRecommendations(portName);
        qDebug() << "建议：" << recommendations;
        
        return -1;
    }
    
    qDebug() << "串口连接测试通过";
    
    // 创建Modbus连接
    ModbusManager modbus;
    modbus.setDebugMode(true);
    
    // 连接到RTU设备
    if (modbus.connectRTU(portName, 9600, 8, 'N', 1)) {
        qDebug() << "RTU连接成功";
        
        modbus.setSlaveID(1);
        modbus.setTimeout(3000);  // 3秒超时
        
        // 读取输入寄存器
        QVector<quint16> inputValues;
        if (modbus.readInputRegisters(0, 5, inputValues)) {
            qDebug() << "读取输入寄存器成功：";
            for (int i = 0; i < inputValues.size(); ++i) {
                qDebug() << QString("  输入寄存器%1: %2").arg(i).arg(inputValues[i]);
            }
        } else {
            qDebug() << "读取输入寄存器失败：" << modbus.getLastError();
        }
        
        // 读取线圈状态
        QVector<bool> coilValues;
        if (modbus.readCoils(0, 8, coilValues)) {
            qDebug() << "读取线圈成功：";
            for (int i = 0; i < coilValues.size(); ++i) {
                qDebug() << QString("  线圈%1: %2").arg(i).arg(coilValues[i] ? "ON" : "OFF");
            }
        } else {
            qDebug() << "读取线圈失败：" << modbus.getLastError();
        }
        
        // 写入单个线圈
        if (modbus.writeSingleCoil(0, true)) {
            qDebug() << "写入线圈成功";
        } else {
            qDebug() << "写入线圈失败：" << modbus.getLastError();
        }
        
        modbus.disconnect();
    } else {
        qDebug() << "RTU连接失败：" << modbus.getLastError();
        
        // 进行Modbus RTU特定诊断
        if (!ModbusTroubleshooter::testModbusRTU(portName, 1, 9600, 8, 'N', 1)) {
            qDebug() << "Modbus RTU连接测试失败";
            
            QString diagnostic = ModbusTroubleshooter::diagnoseModbusParameters(1, 9600, 'N', 8, 1);
            qDebug() << "参数诊断：" << diagnostic;
        }
    }
    
    return 0;
}
```

### ModbusRwManager高级示例

```cpp
#include "modbus_rw_manager.h"
#include "modbusmanager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    ModbusManager modbusManager;
    ModbusRwManager rwManager;
    
    // 设置Modbus管理器
    rwManager.setModbusManager(&modbusManager);
    
    // 连接信号
    QObject::connect(&rwManager, &ModbusRwManager::operationCompleted,
                     [](const ModbusRwManager::OperationResult& result) {
        if (result.success) {
            qDebug() << "操作成功：" << result.formattedResult;
            qDebug() << "执行时间：" << result.executionTime << "ms";
        } else {
            qDebug() << "操作失败：" << result.message;
            qDebug() << "错误代码：" << result.errorCode;
        }
    });
    
    QObject::connect(&rwManager, &ModbusRwManager::batchProgress,
                     [](int current, int total) {
        qDebug() << QString("批量操作进度：%1/%2").arg(current).arg(total);
    });
    
    QObject::connect(&rwManager, &ModbusRwManager::batchCompleted,
                     [](const QList<ModbusRwManager::OperationResult>& results) {
        qDebug() << "批量操作完成，总共" << results.size() << "个操作";
        int successCount = 0;
        for (const auto& result : results) {
            if (result.success) successCount++;
        }
        qDebug() << "成功：" << successCount << "失败：" << (results.size() - successCount);
    });
    
    // 连接到设备
    if (modbusManager.connectTCP("192.168.1.100", 502)) {
        modbusManager.setSlaveID(1);
        
        // 单个读取操作
        ModbusRwManager::OperationParams readParams(
            ModbusRwManager::ReadHoldingRegisters, 1, 0, 10);
        
        ModbusRwManager::OperationResult result = rwManager.performRead(readParams);
        if (result.success) {
            qDebug() << "读取结果：" << result.formattedResult;
            qDebug() << "原始数据：" << result.data.toList();
        }
        
        // 单个写入操作
        ModbusRwManager::OperationParams writeParams(
            ModbusRwManager::WriteSingleRegister, 1, 0, 1, "1234");
        
        result = rwManager.performWrite(writeParams);
        if (result.success) {
            qDebug() << "写入成功";
        }
        
        // 批量操作
        QList<ModbusRwManager::OperationParams> batchParams;
        
        // 添加多个读取操作
        for (int i = 0; i < 5; ++i) {
            batchParams.append(ModbusRwManager::OperationParams(
                ModbusRwManager::ReadHoldingRegisters, 1, i * 10, 5));
        }
        
        // 添加一些写入操作
        batchParams.append(ModbusRwManager::OperationParams(
            ModbusRwManager::WriteSingleRegister, 1, 100, 1, "500"));
        batchParams.append(ModbusRwManager::OperationParams(
            ModbusRwManager::WriteSingleRegister, 1, 101, 1, "600"));
        
        // 执行批量操作
        QList<ModbusRwManager::OperationResult> batchResults = 
            rwManager.performBatchOperations(batchParams);
        
        // 处理批量结果
        for (int i = 0; i < batchResults.size(); ++i) {
            const auto& batchResult = batchResults[i];
            qDebug() << QString("操作 %1: %2").arg(i + 1)
                        .arg(batchResult.success ? "成功" : "失败");
            if (!batchResult.success) {
                qDebug() << "  错误：" << batchResult.message;
            }
        }
        
        modbusManager.disconnect();
    } else {
        qDebug() << "连接失败：" << modbusManager.getLastError();
    }
    
    return 0;
}
```

### 完整诊断示例

```cpp
#include "serial_diagnostic.h"
#include "modbus_troubleshooter.h"
#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Modbus连接诊断工具 ===\n";
    
    // 1. 系统信息
    qDebug() << "1. 系统信息：";
    QString systemInfo = ModbusTroubleshooter::generateSystemInfo();
    qDebug() << systemInfo << "\n";
    
    // 2. 串口扫描
    qDebug() << "2. 串口扫描：";
    QStringList ports = SerialDiagnostic::getAvailablePorts();
    if (ports.isEmpty()) {
        qDebug() << "   没有发现可用的串口\n";
    } else {
        qDebug() << QString("   发现 %1 个串口：").arg(ports.size());
        
        for (const QString& port : ports) {
            SerialDiagnostic::PortInfo info = SerialDiagnostic::getPortInfo(port);
            
            qDebug() << QString("   - %1").arg(info.portName);
            qDebug() << QString("     描述: %1").arg(info.description);
            qDebug() << QString("     制造商: %1").arg(info.manufacturer);
            qDebug() << QString("     可访问: %1").arg(info.accessible ? "是" : "否");
            qDebug() << QString("     使用中: %1").arg(info.inUse ? "是" : "否");
            
            if (!info.errorString.isEmpty()) {
                qDebug() << QString("     错误: %1").arg(info.errorString);
            }
            
            // 权限检查
            bool hasPermission = SerialDiagnostic::checkPortPermissions(port);
            qDebug() << QString("     权限: %1").arg(hasPermission ? "正常" : "不足");
            
            // 连接测试
            qDebug() << "     连接测试:";
            if (SerialDiagnostic::testPortConnection(port, 9600, 8, 'N', 1)) {
                qDebug() << "       9600,8,N,1: 通过";
                
                // Modbus RTU测试
                if (ModbusTroubleshooter::testModbusRTU(port, 1, 9600, 8, 'N', 1)) {
                    qDebug() << "       Modbus RTU: 通过";
                } else {
                    qDebug() << "       Modbus RTU: 失败";
                }
            } else {
                qDebug() << "       9600,8,N,1: 失败";
            }
            
            // 获取建议
            QStringList recommendations = SerialDiagnostic::getRecommendations(port);
            if (!recommendations.isEmpty()) {
                qDebug() << "     建议:";
                for (const QString& rec : recommendations) {
                    qDebug() << QString("       - %1").arg(rec);
                }
            }
            
            qDebug() << "";
        }
    }
    
    // 3. 网络连接测试
    qDebug() << "3. 网络连接测试：";
    QStringList testIPs = {"127.0.0.1", "192.168.1.100", "192.168.1.1"};
    QList<int> testPorts = {502, 503, 5020};
    
    for (const QString& ip : testIPs) {
        for (int port : testPorts) {
            bool connected = ModbusTroubleshooter::checkNetworkConnectivity(ip, port);
            qDebug() << QString("   %1:%2 - %3").arg(ip).arg(port)
                        .arg(connected ? "可连接" : "不可连接");
            
            if (connected) {
                // Modbus TCP测试
                bool modbusOk = ModbusTroubleshooter::testModbusTCP(ip, port, 1);
                qDebug() << QString("     Modbus TCP: %1").arg(modbusOk ? "通过" : "失败");
            }
        }
    }
    qDebug() << "";
    
    // 4. 生成完整诊断报告
    qDebug() << "4. 生成诊断报告：";
    QString fullReport = SerialDiagnostic::generateDiagnosticReport();
    qDebug() << fullReport << "\n";
    
    // 5. 快速修复建议
    qDebug() << "5. 快速修复建议：";
    QStringList quickFixes = ModbusTroubleshooter::getQuickFixes();
    for (int i = 0; i < quickFixes.size(); ++i) {
        qDebug() << QString("   %1. %2").arg(i + 1).arg(quickFixes[i]);
    }
    
    // 6. 导出JSON报告
    QJsonObject reportJson;
    reportJson["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    reportJson["system_info"] = systemInfo;
    
    QJsonArray portsArray;
    for (const QString& port : ports) {
        SerialDiagnostic::PortInfo info = SerialDiagnostic::getPortInfo(port);
        portsArray.append(info.toJson());
    }
    reportJson["ports"] = portsArray;
    reportJson["quick_fixes"] = QJsonArray::fromStringList(quickFixes);
    
    QJsonDocument doc(reportJson);
    QString jsonReport = doc.toJson(QJsonDocument::Indented);
    
    qDebug() << "\n=== JSON诊断报告 ===";
    qDebug() << jsonReport;
    
    return 0;
}
```

## 错误处理

### 错误代码定义

```cpp
// 系统错误代码
#define MODBUS_ERROR_TIMEOUT        -1    // 超时
#define MODBUS_ERROR_CONNECTION     -2    // 连接错误
#define MODBUS_ERROR_INVALID_PARAM  -3    // 无效参数
#define MODBUS_ERROR_PERMISSION     -4    // 权限不足
#define MODBUS_ERROR_PORT_BUSY      -5    // 端口被占用

// Modbus异常代码
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION         0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS     0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE       0x03
#define MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE     0x04
#define MODBUS_EXCEPTION_ACKNOWLEDGE              0x05
#define MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY        0x06
#define MODBUS_EXCEPTION_MEMORY_PARITY_ERROR      0x08
#define MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE 0x0A
#define MODBUS_EXCEPTION_GATEWAY_TARGET_NO_RESPONSE 0x0B
```

### 错误处理最佳实践

```cpp
#include "modbusmanager.h"
#include <QDebug>

class ModbusErrorHandler {
public:
    static void handleModbusError(const ModbusManager& modbus) {
        QString error = modbus.getLastError();
        int errorCode = modbus.getLastErrorCode();
        
        qDebug() << "Modbus错误：" << error;
        qDebug() << "错误代码：" << errorCode;
        
        // 根据错误代码采取不同处理策略
        switch (errorCode) {
        case ECONNREFUSED:
            qDebug() << "处理建议：检查服务器是否运行，防火墙设置";
            break;
            
        case ETIMEDOUT:
            qDebug() << "处理建议：增加超时时间，检查网络连接";
            break;
            
        case EACCES:
            qDebug() << "处理建议：检查串口权限，运行sudo或管理员权限";
            break;
            
        case ENOENT:
            qDebug() << "处理建议：检查串口设备是否存在";
            break;
            
        case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
            qDebug() << "处理建议：检查寄存器地址是否在有效范围内";
            break;
            
        case MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY:
            qDebug() << "处理建议：稍后重试，设备可能正在处理其他请求";
            break;
            
        default:
            qDebug() << "处理建议：查看详细错误信息，联系技术支持";
            break;
        }
    }
    
    static bool retryOperation(std::function<bool()> operation, 
                              int maxRetries = 3, int delayMs = 1000) {
        for (int i = 0; i < maxRetries; ++i) {
            if (operation()) {
                return true;
            }
            
            qDebug() << QString("操作失败，第%1次重试...").arg(i + 1);
            QThread::msleep(delayMs);
        }
        
        qDebug() << "操作最终失败，已达到最大重试次数";
        return false;
    }
};

// 使用示例
ModbusManager modbus;
modbus.setDebugMode(true);

// 带重试的连接
bool connected = ModbusErrorHandler::retryOperation([&]() {
    return modbus.connectTCP("192.168.1.100", 502);
}, 3, 2000);

if (connected) {
    modbus.setSlaveID(1);
    
    // 带重试的读取操作
    QVector<quint16> values;
    bool readSuccess = ModbusErrorHandler::retryOperation([&]() {
        return modbus.readHoldingRegisters(0, 10, values);
    });
    
    if (!readSuccess) {
        ModbusErrorHandler::handleModbusError(modbus);
    }
} else {
    ModbusErrorHandler::handleModbusError(modbus);
}
```

## 编译配置

### CMake配置

```cmake
cmake_minimum_required(VERSION 3.16)
project(ModbusProject)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    SerialPort 
    Network
    Test
)

# libmodbus路径配置
set(LIBMODBUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libmodbus)
set(LIBMODBUS_INCLUDE_DIR ${LIBMODBUS_DIR}/inc)
set(LIBMODBUS_LIB_DIR ${LIBMODBUS_DIR}/lib)

# 根据平台设置库文件
if(WIN32)
    set(LIBMODBUS_LIBRARIES ${LIBMODBUS_LIB_DIR}/modbus.lib)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(LIBMODBUS_LIBRARIES ${LIBMODBUS_LIB_DIR}/modbusd.lib)
    endif()
elseif(UNIX)
    set(LIBMODBUS_LIBRARIES ${LIBMODBUS_LIB_DIR}/libmodbus.a)
endif()

# 包含头文件目录
include_directories(
    ${LIBMODBUS_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# 源文件
set(SOURCES
    src/modbusmanager.cpp
    src/modbus_rw_manager.cpp
    src/serial_diagnostic.cpp
    src/modbus_troubleshooter.cpp
)

# 头文件
set(HEADERS
    inc/modbus/modbusmanager.h
    inc/modbus/modbus_rw_manager.h
    inc/modbus/serial_diagnostic.h
    inc/modbus/modbus_troubleshooter.h
)

# 创建静态库
add_library(ModbusLib STATIC ${SOURCES} ${HEADERS})

# 设置库的包含目录
target_include_directories(ModbusLib PUBLIC 
    ${LIBMODBUS_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# 链接Qt库
target_link_libraries(ModbusLib
    Qt5::Core
    Qt5::SerialPort
    Qt5::Network
    ${LIBMODBUS_LIBRARIES}
)

# Windows特定设置
if(WIN32)
    target_link_libraries(ModbusLib wsock32 ws2_32)
endif()

# 示例程序
add_executable(modbus_example examples/main.cpp)
target_link_libraries(modbus_example ModbusLib)

# 测试程序
add_executable(modbus_test tests/test_main.cpp)
target_link_libraries(modbus_test ModbusLib Qt5::Test)

# 安装配置
install(TARGETS ModbusLib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include/modbus
)
```

### qmake配置

```pro
QT += core serialport network testlib
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# 项目信息
TARGET = ModbusProject
VERSION = 1.0.0

# libmodbus路径
LIBMODBUS_DIR = $$PWD/thirdparty/libmodbus
INCLUDEPATH += $$LIBMODBUS_DIR/inc

# 平台特定配置
win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L$$LIBMODBUS_DIR/lib -lmodbusd
    } else {
        LIBS += -L$$LIBMODBUS_DIR/lib -lmodbus
    }
    LIBS += -lws2_32 -lwsock32
}

unix {
    LIBS += -L$$LIBMODBUS_DIR/lib -lmodbus
}

# 源文件
SOURCES += \
    src/modbusmanager.cpp \
    src/modbus_rw_manager.cpp \
    src/serial_diagnostic.cpp \
    src/modbus_troubleshooter.cpp \
    main.cpp

# 头文件
HEADERS += \
    inc/modbus/modbusmanager.h \
    inc/modbus/modbus_rw_manager.h \
    inc/modbus/serial_diagnostic.h \
    inc/modbus/modbus_troubleshooter.h

# 输出目录
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui
```

## 常见问题

### Q1: TCP连接失败
**问题**：无法连接到Modbus TCP服务器

**解决方案**：
1. 检查IP地址和端口是否正确
2. 确认服务器是否运行
3. 检查防火墙设置
4. 使用 `ModbusTroubleshooter::testModbusTCP()` 进行诊断

### Q2: RTU连接失败
**问题**：串口连接失败或通信异常

**解决方案**：
1. 检查串口是否存在：`SerialDiagnostic::getAvailablePorts()`
2. 检查串口权限：`SerialDiagnostic::checkPortPermissions()`
3. 验证波特率、数据位、停止位、校验位设置
4. 使用诊断工具：`ModbusTroubleshooter::diagnoseSerialPort()`

### Q3: 读写操作超时
**问题**：读写操作经常超时

**解决方案**：
1. 增加超时时间：`modbus.setTimeout(5000)`
2. 检查网络延迟或串口通信质量
3. 减少单次读写的数据量
4. 使用重试机制

### Q4: 权限不足
**问题**：Linux/macOS下串口权限不足

**解决方案**：
```bash
# 添加用户到dialout组
sudo usermod -a -G dialout $USER

# 或者直接设置权限
sudo chmod 666 /dev/ttyUSB0
```

### Q5: 从站无响应
**问题**：从站设备不响应

**解决方案**：
1. 检查从站ID是否正确
2. 确认设备是否上电
3. 检查物理连接
4. 验证通信参数匹配

## 附录

### A. Modbus功能码对照表

| 功能码 | 名称 | 描述 | 对应方法 |
|--------|------|------|----------|
| 01 | Read Coils | 读线圈状态 | `readCoils()` |
| 02 | Read Discrete Inputs | 读离散输入 | `readDiscreteInputs()` |
| 03 | Read Holding Registers | 读保持寄存器 | `readHoldingRegisters()` |
| 04 | Read Input Registers | 读输入寄存器 | `readInputRegisters()` |
| 05 | Write Single Coil | 写单个线圈 | `writeSingleCoil()` |
| 06 | Write Single Register | 写单个寄存器 | `writeSingleRegister()` |
| 15 | Write Multiple Coils | 写多个线圈 | `writeMultipleCoils()` |
| 16 | Write Multiple Registers | 写多个寄存器 | `writeMultipleRegisters()` |
| 22 | Mask Write Register | 掩码写寄存器 | `maskWriteRegister()` |
| 23 | Read/Write Multiple Registers | 读写多个寄存器 | `readWriteRegisters()` |

### B. 常用波特率设置

| 波特率 | 用途 | 推荐场景 |
|--------|------|----------|
| 9600 | 标准 | 一般工业设备 |
| 19200 | 较快 | 现代PLC |
| 38400 | 快速 | 高速采集 |
| 115200 | 高速 | 计算机通信 |

### C. 串口参数组合

| 参数组合 | 描述 | 适用设备 |
|----------|------|----------|
| 9600,8,N,1 | 最常用 | 大多数工业设备 |
| 9600,8,E,1 | 偶校验 | 部分老设备 |
| 19200,8,N,1 | 较快速度 | 现代设备 |
| 115200,8,N,1 | 高速通信 | 计算机串口 |

### D. 版本历史

| 版本 | 日期 | 变更内容 |
|------|------|----------|
| 1.0.0 | 2025-06-03 | 初始版本，基于libmodbus 3.1.1 |
| 1.0.1 | 2025-06-03 | 添加诊断工具和故障排除功能 |
| 1.0.2 | 2025-06-03 | 完善文档和示例代码 |

---

**文档编写**: GitHub Copilot  
**最后更新**: 2025年6月3日  
**文档版本**: 1.0.2