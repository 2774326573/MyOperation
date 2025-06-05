# 📡 libmodbus 库文档中心

> **高性能 Modbus 通信库** - 基于 libmodbus 3.1.1 的 Qt C++ 封装

![Version](https://img.shields.io/badge/Version-3.1.1-blue.svg) ![Qt](https://img.shields.io/badge/Qt-5.12+-green.svg) ![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)

---

## 📋 文档目录

| 文档 | 描述 | 适用对象 |
|------|------|----------|
| [📖 快速开始指南](./quick_start_guide.md) | 5分钟快速上手 Modbus 通信 | 初学者 |
| [🔧 API 使用手册](./libmodbus_api_manual_cn.md) | 完整的 libmodbus C API 文档 | 开发者 |
| [⚙️ 管理器使用手册](./libmodbus_api_manage.md) | ModbusManager 高级封装使用 | Qt 开发者 |
| [🐛 故障排除指南](./troubleshooting_guide.md) | 常见问题解决方案 | 运维人员 |
| [📊 性能优化指南](./performance_guide.md) | 性能调优最佳实践 | 高级开发者 |
| [🔌 硬件集成手册](./hardware_integration.md) | 不同设备的连接配置 | 工程师 |
| [📝 协议规范说明](./protocol_specification.md) | Modbus 协议详细规范 | 协议专家 |
| [🧪 测试与验证](./testing_guide.md) | 单元测试和集成测试 | QA 工程师 |

---

## 🚀 快速开始

### 基本连接示例
```cpp
#include "modbusmanager.h"

// TCP 连接
ModbusManager modbus;
if (modbus.connectTCP("192.168.1.100", 502)) {
    modbus.setSlaveID(1);
    
    // 读取保持寄存器
    QVector<quint16> values;
    if (modbus.readHoldingRegisters(0, 10, values)) {
        qDebug() << "读取成功:" << values;
    }
}
```

### RTU 串口连接
```cpp
// RTU 连接
if (modbus.connectRTU("COM1", 9600)) {
    modbus.setSlaveID(1);
    
    // 写入单个寄存器
    modbus.writeSingleRegister(0, 1234);
}
```

---

## 🎯 核心特性

### ✨ 主要功能
- **🔗 双协议支持**: TCP/RTU 完整实现
- **🛡️ 异常处理**: 完善的错误检测和恢复机制
- **⚡ 高性能**: 异步操作和连接池支持
- **🔧 易于集成**: 简洁的 Qt C++ API
- **📊 实时监控**: 内置诊断和状态监控
- **🎨 灵活配置**: 丰富的参数配置选项

### 🏗️ 架构设计
```
┌─────────────────────────────────────┐
│           应用层 (Application)        │
├─────────────────────────────────────┤
│     ModbusManager (Qt C++ 封装)      │
├─────────────────────────────────────┤
│       libmodbus 核心库 (C)          │
├─────────────────────────────────────┤
│    TCP Socket / Serial Port        │
└─────────────────────────────────────┘
```

---

## 📚 支持的功能码

| 功能码 | 功能名称 | 操作类型 | 数据类型 |
|--------|----------|----------|----------|
| 01 | Read Coils | 读取 | 线圈状态 (bool) |
| 02 | Read Discrete Inputs | 读取 | 离散输入 (bool) |
| 03 | Read Holding Registers | 读取 | 保持寄存器 (uint16) |
| 04 | Read Input Registers | 读取 | 输入寄存器 (uint16) |
| 05 | Write Single Coil | 写入 | 单个线圈 (bool) |
| 06 | Write Single Register | 写入 | 单个寄存器 (uint16) |
| 15 | Write Multiple Coils | 写入 | 多个线圈 (bool[]) |
| 16 | Write Multiple Registers | 写入 | 多个寄存器 (uint16[]) |
| 22 | Mask Write Register | 写入 | 掩码写寄存器 |
| 23 | Read/Write Multiple Registers | 读写 | 批量读写寄存器 |

---

## 🔧 环境要求

### 系统要求
- **操作系统**: Windows 7/8/10/11
- **编译器**: MSVC 2017+ / MinGW 7.3+
- **Qt 版本**: 5.12 或更高
- **CMake**: 3.16 或更高

### 依赖库
- **libmodbus**: 3.1.1 (已包含)
- **Qt Core**: 核心模块
- **Qt Network**: TCP 通信
- **Qt SerialPort**: 串口通信

---

## 📦 安装配置

### 1. CMake 集成
```cmake
# 在您的 CMakeLists.txt 中添加
find_package(Qt5 REQUIRED COMPONENTS Core Network SerialPort)

# 添加 libmodbus 包含目录
include_directories(thirdparty/libmodbus/inc)

# 链接库
target_link_libraries(your_target
    Qt5::Core
    Qt5::Network
    Qt5::SerialPort
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libmodbus/libmodbus64/modbus.lib
)
```

### 2. 运行时配置
确保以下 DLL 文件在可执行文件目录：
- `modbus.dll` (Modbus 核心库)
- `modbusd.dll` (调试版本)

---

## 🏷️ 版本历史

| 版本 | 发布日期 | 主要更新 |
|------|----------|----------|
| 3.1.1 | 2024-12-01 | 初始版本，基础功能完善 |
| 3.1.2 | 计划中 | 性能优化，增强诊断功能 |

---

## 🤝 贡献指南

### 报告问题
- 使用 [GitHub Issues](../../issues) 报告 Bug
- 提供详细的重现步骤和环境信息
- 包含相关的日志输出

### 提交代码
1. Fork 本项目
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 开启 Pull Request

---

## 📞 技术支持

- **文档问题**: 查看 [FAQ](./troubleshooting_guide.md#faq)
- **使用教程**: 参考 [示例代码](../examples/)
- **API 参考**: 查阅 [API 文档](./libmodbus_api_manage.md)

---

## 📄 许可证

本项目基于 [LGPL v2.1](../LICENSE) 许可证发布。

---

<div align="center">

**🌟 如果这个项目对您有帮助，请给我们一个 Star！**

[⬆️ 回到顶部](#-libmodbus-库文档中心)

</div>
