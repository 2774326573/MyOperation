# Modbus 读写管理器文档

## 概述

`ModbusRwManager` 是一个专门处理 Modbus 读写操作的业务逻辑管理器，与UI层分离，提供了完整的 Modbus 功能码支持和操作封装。

## 文件信息

- **文件路径**: `inc/modbus/modbus_rw_manager.h`
- **作者**: jinxi
- **创建日期**: 2025-05-26
- **基类**: `QObject`

## 主要特性

### 功能码支持
- 完整的 Modbus 功能码实现
- 读取和写入操作分离
- 参数验证和错误处理

### 业务逻辑分离
- 与UI层解耦
- 独立的操作结果处理
- 统一的参数和结果格式

### 诊断功能
- 串口状态检测
- 操作结果格式化
- 错误信息提供

## 枚举定义

### 功能码枚举
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

## 数据结构

### 操作参数结构体
```cpp
struct OperationParams {
    FunctionCode functionCode;  // 功能码
    int slaveId;               // 从站ID
    int startAddress;          // 起始地址
    int quantity;              // 数量
    QString writeValue;        // 写入值（仅写操作使用）
    
    OperationParams() : functionCode(ReadHoldingRegisters), slaveId(1), 
                       startAddress(0), quantity(1) {}
};
```

### 操作结果结构体
```cpp
struct OperationResult {
    bool success;              // 操作是否成功
    QString message;           // 结果消息
    QVariant data;             // 读取的数据（仅读操作）
    QString formattedResult;   // 格式化的结果字符串
    
    OperationResult() : success(false) {}
};
```

## 核心方法

### 基本操作
```cpp
class ModbusRwManager : public QObject {
public:
    explicit ModbusRwManager(QObject *parent = nullptr);
    ~ModbusRwManager();
    
    // 设置Modbus管理器
    void setModbusManager(ModbusManager* modbusManager);
    
    // 执行读取操作
    OperationResult performRead(const OperationParams& params);
    
    // 执行写入操作
    OperationResult performWrite(const OperationParams& params);
    
    // 参数验证
    QString validateParams(const OperationParams& params);
};
```

### 静态工具方法
```cpp
// 功能码类型检查
static bool isReadOperation(FunctionCode functionCode);
static bool isWriteOperation(FunctionCode functionCode);
static bool isSingleOperation(FunctionCode functionCode);
static bool isCoilOperation(FunctionCode functionCode);

// 功能码名称转换
static QString getFunctionCodeName(FunctionCode functionCode);
static FunctionCode getFunctionCodeFromIndex(int index);
static int getIndexFromFunctionCode(FunctionCode functionCode);

// 串口诊断
static QStringList getAvailablePorts();
```

## 使用示例

### 基本读取操作
```cpp
#include "modbus_rw_manager.h"
#include "modbusmanager.h"

// 创建管理器
ModbusRwManager *rwManager = new ModbusRwManager(this);
ModbusManager *modbusManager = new ModbusManager(this);

// 连接Modbus设备
if (modbusManager->connectRTU("COM1", 9600, 8, 'N', 1)) {
    rwManager->setModbusManager(modbusManager);
    
    // 设置操作参数
    ModbusRwManager::OperationParams params;
    params.functionCode = ModbusRwManager::ReadHoldingRegisters;
    params.slaveId = 1;
    params.startAddress = 0;
    params.quantity = 10;
    
    // 验证参数
    QString validation = rwManager->validateParams(params);
    if (!validation.isEmpty()) {
        qDebug() << "参数验证失败:" << validation;
        return;
    }
    
    // 执行读取操作
    ModbusRwManager::OperationResult result = rwManager->performRead(params);
    
    if (result.success) {
        qDebug() << "读取成功:" << result.formattedResult;
        
        // 获取原始数据
        QVector<quint16> values = result.data.value<QVector<quint16>>();
        for (int i = 0; i < values.size(); ++i) {
            qDebug() << QString("寄存器 %1: %2").arg(i).arg(values[i]);
        }
    } else {
        qDebug() << "读取失败:" << result.message;
    }
}
```

### 写入操作示例
```cpp
// 写入单个寄存器
ModbusRwManager::OperationParams writeParams;
writeParams.functionCode = ModbusRwManager::WriteSingleRegister;
writeParams.slaveId = 1;
writeParams.startAddress = 100;
writeParams.quantity = 1;
writeParams.writeValue = "1234";

ModbusRwManager::OperationResult writeResult = rwManager->performWrite(writeParams);

if (writeResult.success) {
    qDebug() << "写入成功:" << writeResult.message;
} else {
    qDebug() << "写入失败:" << writeResult.message;
}

// 写入多个寄存器
ModbusRwManager::OperationParams multiWriteParams;
multiWriteParams.functionCode = ModbusRwManager::WriteMultipleRegisters;
multiWriteParams.slaveId = 1;
multiWriteParams.startAddress = 200;
multiWriteParams.quantity = 5;
multiWriteParams.writeValue = "100,200,300,400,500"; // 逗号分隔

ModbusRwManager::OperationResult multiResult = rwManager->performWrite(multiWriteParams);
qDebug() << "批量写入结果:" << multiResult.message;
```

### 线圈操作示例
```cpp
// 读取线圈状态
ModbusRwManager::OperationParams coilReadParams;
coilReadParams.functionCode = ModbusRwManager::ReadCoils;
coilReadParams.slaveId = 1;
coilReadParams.startAddress = 0;
coilReadParams.quantity = 8;

ModbusRwManager::OperationResult coilResult = rwManager->performRead(coilReadParams);

if (coilResult.success) {
    QVector<bool> coilStates = coilResult.data.value<QVector<bool>>();
    for (int i = 0; i < coilStates.size(); ++i) {
        qDebug() << QString("线圈 %1: %2").arg(i).arg(coilStates[i] ? "ON" : "OFF");
    }
}

// 写入单个线圈
ModbusRwManager::OperationParams coilWriteParams;
coilWriteParams.functionCode = ModbusRwManager::WriteSingleCoil;
coilWriteParams.slaveId = 1;
coilWriteParams.startAddress = 5;
coilWriteParams.writeValue = "1"; // 1=ON, 0=OFF

ModbusRwManager::OperationResult coilWriteResult = rwManager->performWrite(coilWriteParams);
qDebug() << "线圈写入结果:" << coilWriteResult.message;
```

### 功能码工具方法示例
```cpp
// 检查功能码类型
FunctionCode fc = ModbusRwManager::ReadHoldingRegisters;

bool isRead = ModbusRwManager::isReadOperation(fc);        // true
bool isWrite = ModbusRwManager::isWriteOperation(fc);      // false
bool isSingle = ModbusRwManager::isSingleOperation(fc);    // false
bool isCoil = ModbusRwManager::isCoilOperation(fc);        // false

// 获取功能码名称
QString fcName = ModbusRwManager::getFunctionCodeName(fc);
qDebug() << "功能码名称:" << fcName; // "读保持寄存器"

// UI下拉框索引转换
int index = ModbusRwManager::getIndexFromFunctionCode(fc);
FunctionCode fcFromIndex = ModbusRwManager::getFunctionCodeFromIndex(index);
```

### 参数验证示例
```cpp
// 创建无效参数进行测试
ModbusRwManager::OperationParams invalidParams;
invalidParams.functionCode = ModbusRwManager::ReadHoldingRegisters;
invalidParams.slaveId = 0;        // 无效：从站ID不能为0
invalidParams.startAddress = -1;   // 无效：地址不能为负数
invalidParams.quantity = 0;        // 无效：数量不能为0

QString validation = rwManager->validateParams(invalidParams);
if (!validation.isEmpty()) {
    qDebug() << "参数验证错误:" << validation;
    // 输出：参数验证错误: 从站ID必须在1-247之间; 起始地址不能为负数; 数量必须大于0
}

// 修正参数
invalidParams.slaveId = 1;
invalidParams.startAddress = 0;
invalidParams.quantity = 10;

validation = rwManager->validateParams(invalidParams);
if (validation.isEmpty()) {
    qDebug() << "参数验证通过";
}
```

### 串口诊断示例
```cpp
// 获取可用串口列表
QStringList availablePorts = ModbusRwManager::getAvailablePorts();
qDebug() << "可用串口:" << availablePorts;

// 通常配合连接测试使用
for (const QString &port : availablePorts) {
    qDebug() << "测试串口:" << port;
    
    ModbusManager testManager;
    if (testManager.connectRTU(port, 9600, 8, 'N', 1)) {
        qDebug() << "串口" << port << "连接成功";
        testManager.disconnect();
    } else {
        qDebug() << "串口" << port << "连接失败";
    }
}
```

## 高级用法

### 批量操作封装
```cpp
class BatchModbusOperations : public QObject {
public:
    explicit BatchModbusOperations(ModbusRwManager *rwManager, QObject *parent = nullptr)
        : QObject(parent), m_rwManager(rwManager) {}
    
    // 批量读取多个地址段
    QVector<ModbusRwManager::OperationResult> batchRead(
        const QVector<QPair<int, int>>& addressRanges, 
        int slaveId = 1) {
        
        QVector<ModbusRwManager::OperationResult> results;
        
        for (const auto& range : addressRanges) {
            ModbusRwManager::OperationParams params;
            params.functionCode = ModbusRwManager::ReadHoldingRegisters;
            params.slaveId = slaveId;
            params.startAddress = range.first;
            params.quantity = range.second;
            
            results.append(m_rwManager->performRead(params));
        }
        
        return results;
    }
    
    // 配置数据写入
    bool writeConfiguration(const QMap<int, quint16>& configData, int slaveId = 1) {
        for (auto it = configData.begin(); it != configData.end(); ++it) {
            ModbusRwManager::OperationParams params;
            params.functionCode = ModbusRwManager::WriteSingleRegister;
            params.slaveId = slaveId;
            params.startAddress = it.key();
            params.writeValue = QString::number(it.value());
            
            ModbusRwManager::OperationResult result = m_rwManager->performWrite(params);
            if (!result.success) {
                qDebug() << QString("写入地址 %1 失败: %2").arg(it.key()).arg(result.message);
                return false;
            }
        }
        return true;
    }

private:
    ModbusRwManager *m_rwManager;
};

// 使用示例
BatchModbusOperations batch(rwManager, this);

// 批量读取
QVector<QPair<int, int>> ranges = {{0, 10}, {100, 5}, {200, 20}};
QVector<ModbusRwManager::OperationResult> results = batch.batchRead(ranges);

// 配置写入
QMap<int, quint16> config = {{100, 1234}, {101, 5678}, {102, 9012}};
bool success = batch.writeConfiguration(config);
```

### 数据格式化工具
```cpp
class ModbusDataFormatter {
public:
    // 格式化寄存器数据
    static QString formatRegisters(const QVector<quint16>& values, 
                                 const QString& format = "decimal") {
        QStringList formattedValues;
        
        for (quint16 value : values) {
            if (format == "hex") {
                formattedValues << QString("0x%1").arg(value, 4, 16, QChar('0')).toUpper();
            } else if (format == "binary") {
                formattedValues << QString("0b%1").arg(value, 16, 2, QChar('0'));
            } else {
                formattedValues << QString::number(value);
            }
        }
        
        return formattedValues.join(", ");
    }
    
    // 格式化线圈数据
    static QString formatCoils(const QVector<bool>& coils) {
        QStringList formattedCoils;
        
        for (int i = 0; i < coils.size(); ++i) {
            formattedCoils << QString("Coil%1:%2").arg(i).arg(coils[i] ? "ON" : "OFF");
        }
        
        return formattedCoils.join(", ");
    }
};

// 使用示例
QVector<quint16> registerValues = {1234, 5678, 9012};
QString decimal = ModbusDataFormatter::formatRegisters(registerValues, "decimal");
QString hex = ModbusDataFormatter::formatRegisters(registerValues, "hex");
QString binary = ModbusDataFormatter::formatRegisters(registerValues, "binary");

qDebug() << "十进制:" << decimal;  // 1234, 5678, 9012
qDebug() << "十六进制:" << hex;    // 0x04D2, 0x162E, 0x2334
qDebug() << "二进制:" << binary;   // 0b0000010011010010, ...
```

## 错误处理

### 常见错误情况
1. **连接错误**: Modbus管理器未设置或未连接
2. **参数错误**: 无效的从站ID、地址或数量
3. **通信错误**: 设备无响应或通信超时
4. **数据错误**: 写入数据格式不正确

### 错误处理示例
```cpp
ModbusRwManager::OperationResult result = rwManager->performRead(params);

if (!result.success) {
    // 分析错误类型
    if (result.message.contains("连接")) {
        qDebug() << "连接问题，尝试重新连接";
        // 重连逻辑
    } else if (result.message.contains("超时")) {
        qDebug() << "通信超时，检查设备状态";
        // 超时处理
    } else if (result.message.contains("参数")) {
        qDebug() << "参数错误，检查配置";
        // 参数修正
    } else {
        qDebug() << "未知错误:" << result.message;
    }
}
```

## 性能优化建议

### 1. 批量操作
```cpp
// 推荐：批量读取
params.quantity = 50; // 一次读取50个寄存器

// 避免：单个读取循环
for (int i = 0; i < 50; ++i) {
    params.startAddress = i;
    params.quantity = 1;
    // 效率低下
}
```

### 2. 参数缓存
```cpp
// 缓存验证结果
static QMap<QString, QString> validationCache;

QString cacheKey = QString("%1_%2_%3_%4")
    .arg(params.functionCode)
    .arg(params.slaveId)
    .arg(params.startAddress)
    .arg(params.quantity);

if (validationCache.contains(cacheKey)) {
    return validationCache[cacheKey];
}
```

### 3. 结果重用
```cpp
// 对于读取频繁但变化不大的数据，考虑缓存结果
QTime lastRead;
ModbusRwManager::OperationResult cachedResult;

if (lastRead.isValid() && lastRead.msecsTo(QTime::currentTime()) < 1000) {
    return cachedResult; // 使用缓存结果
}
```

## 相关文档

- [Modbus 管理器文档](modbusmanager.md)
- [Modbus 性能优化文档](modbus_performance.md)
- [优化 Modbus 管理器文档](optimized_modbus_manager.md)
- [Modbus 核心文档](modbus.md)
