# Modbus RTU 通信文档

## 概述

`modbus-rtu.h` 提供了 Modbus RTU（Remote Terminal Unit）协议的实现，支持串行通信（RS232/RS485）。

## 文件信息

- **文件路径**: `inc/modbus/modbus-rtu.h`
- **版权**: © Stéphane Raimbault <stephane.raimbault@gmail.com>
- **许可证**: LGPL-2.1-or-later

## 主要特性

### RTU 协议支持
- RS232 串行通信
- RS485 串行通信
- 标准 Modbus RTU 帧格式
- CRC 校验

### 硬件控制
- RTS（Request To Send）信号控制
- 串行模式配置
- 波特率、奇偶校验、数据位、停止位设置

## 常量定义

### ADU 长度限制
```c
#define MODBUS_RTU_MAX_ADU_LENGTH 256
```
- **说明**: RTU 模式下的最大应用数据单元长度
- **计算**: 253字节数据 + 1字节从站地址 + 2字节CRC = 256字节

### 串行模式
```c
#define MODBUS_RTU_RS232 0  // RS232 模式
#define MODBUS_RTU_RS485 1  // RS485 模式
```

### RTS 控制模式
```c
#define MODBUS_RTU_RTS_NONE 0   // 无 RTS 控制
#define MODBUS_RTU_RTS_UP   1   // RTS 高电平
#define MODBUS_RTU_RTS_DOWN 2   // RTS 低电平
```

## 核心函数

### 创建 RTU 连接
```c
MODBUS_API modbus_t *modbus_new_rtu(
    const char *device,    // 设备路径（如 "/dev/ttyUSB0" 或 "COM1"）
    int baud,             // 波特率（如 9600, 19200, 115200）
    char parity,          // 奇偶校验（'N', 'E', 'O'）
    int data_bit,         // 数据位（7 或 8）
    int stop_bit          // 停止位（1 或 2）
);
```

### 串行模式配置
```c
MODBUS_API int modbus_rtu_set_serial_mode(modbus_t *ctx, int mode);
MODBUS_API int modbus_rtu_get_serial_mode(modbus_t *ctx);
```

## 使用示例

### 基本 RTU 连接
```c
#include "modbus-rtu.h"

// 创建 RTU 连接
modbus_t *ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
if (ctx == NULL) {
    fprintf(stderr, "无法创建 RTU 连接\n");
    return -1;
}

// 设置从站地址
modbus_set_slave(ctx, 1);

// 连接到设备
if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "连接失败: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

// 读取保持寄存器
uint16_t registers[10];
int rc = modbus_read_registers(ctx, 0, 10, registers);
if (rc == -1) {
    fprintf(stderr, "读取失败: %s\n", modbus_strerror(errno));
}

// 清理资源
modbus_close(ctx);
modbus_free(ctx);
```

### RS485 配置
```c
modbus_t *ctx = modbus_new_rtu("/dev/ttyUSB0", 19200, 'E', 8, 1);

// 设置为 RS485 模式
modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);

// 设置 RTS 控制
modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);
```

## 配置参数说明

### 波特率（Baud Rate）
常用值：1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200

### 奇偶校验（Parity）
- `'N'` - 无校验（None）
- `'E'` - 偶校验（Even）
- `'O'` - 奇校验（Odd）

### 数据位（Data Bits）
- `7` - 7位数据
- `8` - 8位数据（推荐）

### 停止位（Stop Bits）
- `1` - 1个停止位（推荐）
- `2` - 2个停止位

## 硬件连接

### RS232 连接
```
设备1 TXD  ----  RXD 设备2
设备1 RXD  ----  TXD 设备2
设备1 GND  ----  GND 设备2
```

### RS485 连接
```
设备1 A/+  ----  A/+ 设备2
设备1 B/-  ----  B/- 设备2
设备1 GND  ----  GND 设备2
```

## 故障排除

### 常见问题
1. **设备文件不存在**
   - 检查设备路径是否正确
   - 确认设备已正确连接

2. **权限不足**
   - 确保用户有访问串口的权限
   - Linux 下可能需要添加到 dialout 组

3. **通信失败**
   - 检查波特率、奇偶校验等参数
   - 验证硬件连接
   - 确认从站地址正确

4. **RS485 方向控制**
   - 正确配置 RTS 信号
   - 检查硬件 RTS 连接

## 性能优化

1. **选择合适的波特率**
   - 根据传输距离和可靠性要求选择
   - 距离越远，波特率应越低

2. **响应超时设置**
   ```c
   modbus_set_response_timeout(ctx, 1, 0);  // 1秒超时
   ```

3. **字节间超时**
   ```c
   modbus_set_byte_timeout(ctx, 0, 100000); // 100ms
   ```

## 相关文档

- [Modbus 核心文档](modbus.md)
- [Modbus TCP 文档](modbus-tcp.md)
- [Modbus 管理器文档](modbusmanager.md)
