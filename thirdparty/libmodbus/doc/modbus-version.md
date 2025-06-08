# Modbus 版本信息文档

## 概述

`modbus-version.h` 定义了 libmodbus 库的版本信息和版本检查宏。

## 文件信息

- **文件路径**: `inc/modbus/modbus-version.h`
- **版权**: © Stéphane Raimbault <stephane.raimbault@gmail.com>
- **许可证**: LGPL-2.1-or-later

## 当前版本

### 版本号 3.1.1

- **主版本号**: 3
- **次版本号**: 1  
- **补丁版本号**: 1

## 版本定义

### 版本组件
```c
#define LIBMODBUS_VERSION_MAJOR (3)    // 主版本号
#define LIBMODBUS_VERSION_MINOR (1)    // 次版本号  
#define LIBMODBUS_VERSION_MICRO (1)    // 补丁版本号
```

### 完整版本
```c
#define LIBMODBUS_VERSION 3.1.1                    // 数值版本
#define LIBMODBUS_VERSION_STRING "3.1.1"           // 字符串版本
```

### 十六进制版本
```c
#define LIBMODBUS_VERSION_HEX 0x030101
```
- **格式**: 0xMMmmpp (主版本.次版本.补丁版本)
- **计算**: (MAJOR << 16) | (MINOR << 8) | (MICRO << 0)

## 版本检查宏

### LIBMODBUS_VERSION_CHECK
```c
#define LIBMODBUS_VERSION_CHECK(major, minor, micro)
```

检查当前库版本是否大于或等于指定版本。

**参数:**
- `major` - 要检查的主版本号
- `minor` - 要检查的次版本号  
- `micro` - 要检查的补丁版本号

**返回值:**
- `真` - 当前版本 >= 指定版本
- `假` - 当前版本 < 指定版本

## 使用示例

### 版本信息获取
```c
#include "modbus-version.h"
#include <stdio.h>

void print_version_info() {
    printf("libmodbus 版本信息:\n");
    printf("主版本号: %d\n", LIBMODBUS_VERSION_MAJOR);
    printf("次版本号: %d\n", LIBMODBUS_VERSION_MINOR);
    printf("补丁版本号: %d\n", LIBMODBUS_VERSION_MICRO);
    printf("完整版本: %s\n", LIBMODBUS_VERSION_STRING);
    printf("十六进制版本: 0x%06X\n", LIBMODBUS_VERSION_HEX);
}
```

### 版本兼容性检查
```c
#include "modbus-version.h"

// 检查是否支持某个特性（假设该特性在 3.1.0 版本引入）
#if LIBMODBUS_VERSION_CHECK(3, 1, 0)
    // 使用新特性
    printf("支持新特性\n");
#else
    // 使用兼容代码
    printf("使用兼容模式\n");
#endif
```

### 运行时版本检查
```c
#include "modbus-version.h"

int check_minimum_version(int required_major, int required_minor, int required_micro) {
    if (LIBMODBUS_VERSION_CHECK(required_major, required_minor, required_micro)) {
        printf("版本检查通过: 需要 %d.%d.%d, 当前 %s\n", 
               required_major, required_minor, required_micro,
               LIBMODBUS_VERSION_STRING);
        return 1;
    } else {
        printf("版本检查失败: 需要 %d.%d.%d, 当前 %s\n", 
               required_major, required_minor, required_micro,
               LIBMODBUS_VERSION_STRING);
        return 0;
    }
}

// 使用示例
int main() {
    // 检查是否满足最低版本要求
    if (!check_minimum_version(3, 0, 0)) {
        fprintf(stderr, "libmodbus 版本过低\n");
        return -1;
    }
    
    // 继续执行程序
    return 0;
}
```

### 条件编译
```c
// 根据版本选择不同的实现
#if LIBMODBUS_VERSION_CHECK(3, 1, 0)
    // 3.1.0 及以上版本的实现
    #define USE_NEW_API 1
#elif LIBMODBUS_VERSION_CHECK(3, 0, 0)
    // 3.0.x 版本的实现
    #define USE_LEGACY_API 1
#else
    // 不支持的版本
    #error "不支持的 libmodbus 版本"
#endif
```

## 版本历史

### 版本演进
- **3.1.1** - 当前版本，Bug 修复和稳定性改进
- **3.1.0** - 新增功能和 API 改进
- **3.0.x** - 稳定版本系列
- **2.x.x** - 旧版本系列（不推荐）

### 版本兼容性
- **向前兼容**: 新版本保持对旧 API 的兼容
- **向后兼容**: 新编译的代码可能不兼容旧版本库
- **ABI 稳定性**: 同主版本内 ABI 保持稳定

## 最佳实践

### 1. 版本检查
在使用特定 API 前进行版本检查：
```c
#if LIBMODBUS_VERSION_CHECK(3, 1, 0)
    // 安全使用 3.1.0 引入的 API
#endif
```

### 2. 构建系统集成
在 CMakeLists.txt 中检查版本：
```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus>=3.1.0)
```

### 3. 运行时检查
```c
// 在程序启动时检查版本
if (!LIBMODBUS_VERSION_CHECK(3, 0, 0)) {
    fprintf(stderr, "libmodbus 版本过低，需要 3.0.0 或更高版本\n");
    exit(1);
}
```

### 4. 错误处理
```c
#include "modbus-version.h"

const char* get_version_string() {
    return LIBMODBUS_VERSION_STRING;
}

int get_version_hex() {
    return LIBMODBUS_VERSION_HEX;
}
```

## 升级指南

### 从 3.0.x 升级到 3.1.x
1. 检查 API 变更
2. 更新构建脚本
3. 测试兼容性
4. 更新版本检查代码

### 版本选择建议
- **生产环境**: 使用稳定版本（如 3.1.1）
- **开发环境**: 可以使用最新版本
- **兼容性要求**: 选择最低公共版本

## 相关文档

- [Modbus 核心文档](modbus.md)
- [Modbus RTU 文档](modbus-rtu.md)
- [Modbus TCP 文档](modbus-tcp.md)
- [Modbus 管理器文档](modbusmanager.md)
