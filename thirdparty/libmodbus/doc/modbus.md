# Modbus 核心库文档

## 概述

`modbus.h` 是 libmodbus 库的核心头文件，提供了 Modbus 协议的基础功能和数据结构定义。

## 文件信息

- **文件路径**: `inc/modbus/modbus.h`
- **版权**: © Stéphane Raimbault <stephane.raimbault@gmail.com>
- **许可证**: LGPL-2.1-or-later

## 主要特性

### 平台支持
- 支持 Unix/Linux 系统
- 支持 Windows 系统（Visual Studio）
- 跨平台兼容性

### 数据类型定义
- 标准整数类型支持（stdint.h）
- 布尔值定义（TRUE/FALSE）
- Modbus 特定数据类型

### API 导出
- Windows DLL 支持
- 动态库导入/导出宏定义
- C++ 兼容性支持

## 核心宏定义

### 布尔值
```c
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
```

### API 导出
```c
#if defined(_MSC_VER)
# if defined(DLLBUILD)
#  define MODBUS_API __declspec(dllexport)
# else
#  define MODBUS_API __declspec(dllimport)
# endif
#else
# define MODBUS_API
#endif
```

### C++ 兼容性
```c
#ifdef  __cplusplus
# define MODBUS_BEGIN_DECLS  extern "C" {
# define MODBUS_END_DECLS    }
#else
# define MODBUS_BEGIN_DECLS
# define MODBUS_END_DECLS
#endif
```

## 使用方法

1. **包含头文件**
   ```c
   #include "modbus.h"
   ```

2. **在 C++ 项目中使用**
   ```cpp
   extern "C" {
   #include "modbus.h"
   }
   ```

## 依赖关系

- `modbus-version.h` - 版本信息
- `stdint.h` - 标准整数类型
- 系统相关头文件（Unix/Windows）

## 编译配置

### Windows (Visual Studio)
- 定义 `DLLBUILD` 用于构建 DLL
- 自动处理 DLL 导入/导出

### Unix/Linux
- 使用标准的 GCC/Clang 编译器
- 支持系统参数检测

## 注意事项

1. 确保正确包含版本头文件
2. 在 Windows 下注意 DLL 导入/导出设置
3. C++ 项目需要使用 extern "C" 包装
4. 遵循 LGPL-2.1-or-later 许可证要求

## 相关文档

- [Modbus RTU 文档](modbus-rtu.md)
- [Modbus TCP 文档](modbus-tcp.md)
- [Modbus 版本信息](modbus-version.md)
- [Modbus 管理器文档](modbusmanager.md)
