# Libmodbus 库文档

## 概述

欢迎使用 Libmodbus 库文档中心。本库提供了完整的 Modbus RTU/TCP 通信解决方案，包含基础通信、性能优化、诊断工具等功能。

## 📚 文档目录

### 核心组件

| 文档 | 描述 | 主要功能 |
|------|------|----------|
| [📡 modbus.md](modbus.md) | 核心 Modbus 库文档 | 基础数据类型、错误处理、核心API |
| [🔧 modbusmanager.md](modbusmanager.md) | Modbus 管理器文档 | 连接管理、数据读写、串口诊断 |
| [📊 modbus_rw_manager.md](modbus_rw_manager.md) | 读写管理器文档 | 高级读写操作、批量处理、静态工具 |

### 通信协议

| 文档 | 描述 | 适用场景 |
|------|------|----------|
| [🔌 modbus-rtu.md](modbus-rtu.md) | RTU 通信协议文档 | 串口通信、RS485 总线 |
| [🌐 modbus-tcp.md](modbus-tcp.md) | TCP/IP 通信协议文档 | 网络通信、以太网连接 |

### 性能优化

| 文档 | 描述 | 优化特性 |
|------|------|----------|
| [⚡ modbus_performance.md](modbus_performance.md) | 性能优化组件文档 | 连接池、缓存、异步操作、智能重连 |
| [🚀 optimized_modbus_manager.md](optimized_modbus_manager.md) | 优化管理器文档 | 综合性能优化、统一高级API |
| [📈 modbus_benchmark.md](modbus_benchmark.md) | 性能基准测试文档 | 性能测试工具、基准测试、性能分析 |

### 工具和辅助

| 文档 | 描述 | 用途 |
|------|------|------|
| [ℹ️ modbus-version.md](modbus-version.md) | 版本信息文档 | 版本检查、兼容性验证 |

## 🚀 快速开始

### 1. 基础 Modbus 通信

```cpp
#include "modbusmanager.h"

// 创建管理器
ModbusManager* manager = new ModbusManager(this);

// RTU 连接
bool success = manager->connectRTU("COM1", 9600, 8, 'N', 1);
if (success) {
    // 设置从机地址
    manager->setSlaveID(1);
    
    // 读取保持寄存器
    QVector<quint16> values;
    bool readSuccess = manager->readHoldingRegisters(0, 10, values);
    if (readSuccess) {
        qDebug() << "读取成功：" << values;
    }
}
```

### 2. 高性能 Modbus 通信

```cpp
#include "optimized_modbus_manager.h"

// 创建优化管理器
OptimizedModbusManager* manager = new OptimizedModbusManager(this);

// 配置优化参数
OptimizedModbusManager::OptimizationConfig config;
config.cacheEnabled = true;
config.asyncEnabled = true;
config.autoReconnectEnabled = true;
manager->setOptimizationConfig(config);

// 连接设备
bool success = manager->connectDevice("device1", "192.168.1.100:502", 1);

// 异步读取
QString operationId = manager->readHoldingRegistersAsync("device1", 0, 10,
    [](bool success, const QVector<quint16>& values) {
        if (success) {
            qDebug() << "异步读取成功：" << values;
        }
    });
```

### 3. 串口诊断

```cpp
#include "modbusmanager.h"

// 获取可用串口
QStringList ports = ModbusManager::getAvailablePorts();
qDebug() << "可用串口：" << ports;

// 测试串口连接
bool canConnect = ModbusManager::testPortConnection("COM1", 9600, 8, 'N', 1);
qDebug() << "串口测试：" << (canConnect ? "成功" : "失败");

// 生成诊断报告
QString report = ModbusManager::generateDiagnosticReport();
qDebug() << "诊断报告：\n" << report;
```

## 📋 功能特性

### ✅ 核心功能
- **多协议支持**：RTU (串口) 和 TCP/IP 两种通信协议
- **完整数据类型**：支持线圈、离散输入、输入寄存器、保持寄存器
- **异常处理**：完善的错误检测和异常处理机制
- **调试支持**：内置调试模式和详细日志

### ✅ 性能优化
- **连接池管理**：自动管理和复用连接，减少连接开销
- **智能缓存**：缓存读取结果，避免重复请求
- **异步操作**：非阻塞异步 I/O 操作
- **批量处理**：合并多个请求，提高通信效率
- **智能重连**：自动检测连接状态并智能重连

### ✅ 诊断工具
- **串口检测**：自动检测系统可用串口
- **连接测试**：测试串口连接参数和通信状态
- **设备扫描**：扫描 Modbus 总线上的设备
- **故障诊断**：提供详细的故障诊断和解决建议
- **性能监控**：实时监控通信性能和系统健康状态

### ✅ 开发友好
- **Qt 集成**：完全基于 Qt 框架，支持信号槽机制
- **线程安全**：内置线程安全保护，支持多线程使用
- **配置持久化**：支持配置的保存和加载
- **丰富示例**：提供详细的使用示例和最佳实践

## 🔧 系统要求

### 基础要求
- **操作系统**：Windows 7+, Linux, macOS
- **Qt 版本**：Qt 5.12+ 或 Qt 6.x
- **编译器**：MSVC 2017+, GCC 7+, Clang 6+
- **CMake**：3.16+ （如果使用 CMake 构建）

### 依赖库
- **libmodbus**：核心 Modbus C 库
- **Qt5Core**：Qt 核心库
- **Qt5Network**：网络通信库（TCP 协议）
- **Qt5SerialPort**：串口通信库（RTU 协议）

## 📖 使用指南

### 选择合适的组件

| 需求场景 | 推荐组件 | 说明 |
|----------|----------|------|
| 简单 Modbus 通信 | `ModbusManager` | 基础功能，易于使用 |
| 高频数据读取 | `OptimizedModbusManager` | 包含缓存和连接池优化 |
| 批量数据处理 | `ModbusRwManager` | 提供批量操作和静态工具 |
| 性能要求极高 | `modbus_performance.h` 组件 | 可单独使用各种性能优化组件 |
| 开发调试阶段 | 诊断功能 | 使用串口诊断和性能监控工具 |

### 开发流程建议

1. **需求分析**：确定通信协议（RTU/TCP）和性能要求
2. **组件选择**：根据需求选择合适的管理器组件
3. **连接配置**：配置连接参数和优化选项
4. **功能实现**：实现具体的数据读写逻辑
5. **测试调试**：使用诊断工具进行测试和调试
6. **性能优化**：根据性能监控结果进行优化
7. **部署上线**：配置生产环境参数

## 🔍 故障排除

### 常见问题

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 连接失败 | 串口被占用、参数错误 | 使用诊断工具检查串口状态 |
| 读写超时 | 网络延迟、设备无响应 | 增加超时时间，检查设备状态 |
| 性能较差 | 频繁连接、无缓存 | 启用连接池和缓存优化 |
| 内存泄露 | 未释放资源 | 定期调用清理函数 |

### 调试工具

```cpp
// 启用调试模式
manager->setDebugMode(true);

// 生成诊断报告
QString report = ModbusManager::generateDiagnosticReport();

// 获取性能统计
QMap<QString, QVariant> stats = optimizedManager->getPerformanceStats();

// 系统健康检查
QMap<QString, QVariant> health = optimizedManager->getSystemHealth();
```

## 📝 最佳实践

### 1. 错误处理
```cpp
// 始终检查操作结果
bool success = manager->readHoldingRegisters(0, 10, values);
if (!success) {
    QString error = manager->getLastError();
    qWarning() << "读取失败：" << error;
    // 处理错误...
}
```

### 2. 资源管理
```cpp
// 使用智能指针管理资源
std::unique_ptr<ModbusManager> manager(new ModbusManager());

// 或使用 Qt 的父子对象管理
ModbusManager* manager = new ModbusManager(this);
```

### 3. 性能优化
```cpp
// 高频读取场景使用缓存
OptimizedModbusManager::OptimizationConfig config;
config.cacheEnabled = true;
config.defaultCacheTtlMs = 1000;
optimizedManager->setOptimizationConfig(config);
```

### 4. 信号槽连接
```cpp
// 监听连接状态变化
connect(manager, &ModbusManager::connected, this, &MyClass::onConnected);
connect(manager, &ModbusManager::disconnected, this, &MyClass::onDisconnected);
connect(manager, &ModbusManager::errorOccurred, this, &MyClass::onError);
```

## 🤝 技术支持

### 文档反馈
如果您在使用过程中发现文档问题或有改进建议，请：
1. 检查相关文档是否有更新
2. 查看示例代码和最佳实践
3. 参考故障排除指南

### 开发支持
- 详细的 API 文档和使用示例
- 完整的故障排除指南
- 性能优化建议和配置指南

## 📄 版本信息

当前文档版本对应的库版本信息请参考 [modbus-version.md](modbus-version.md)。

---

**更新时间**: 2025-06-07  
**文档版本**: 1.0.0  
**维护状态**: 活跃维护中 ✅
