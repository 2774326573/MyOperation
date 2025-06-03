# libmodbus API 使用手册

## 目录
1. [简介](#简介)
2. [Modbus协议概述](#modbus协议概述)
3. [通用API](#通用api)
   - [连接管理](#连接管理)
   - [设置操作](#设置操作)
   - [数据读取](#数据读取)
   - [数据写入](#数据写入)
   - [数据映射](#数据映射)
   - [错误处理](#错误处理)
4. [Modbus TCP](#modbus-tcp)
5. [Modbus RTU](#modbus-rtu)
6. [实用函数](#实用函数)
7. [使用示例](#使用示例)
8. [Python示例](#python示例)
9. [Qt集成示例](#qt集成示例)

## 简介

libmodbus是一个开源的Modbus协议实现库，支持RTU和TCP协议。本手册详细介绍了libmodbus的API使用方法，帮助开发者快速上手Modbus协议通信开发。

## Modbus协议概述

Modbus是一种工业自动化领域广泛使用的通信协议，主要用于可编程逻辑控制器（PLC）和其他设备之间的通信。Modbus协议定义了以下几种主要功能：

- 读取线圈状态（Read Coil Status）
- 读取输入状态（Read Input Status）
- 读取保持寄存器（Read Holding Registers）
- 读取输入寄存器（Read Input Registers）
- 写单个线圈（Force Single Coil）
- 写单个寄存器（Preset Single Register）
- 写多个线圈（Force Multiple Coils）
- 写多个寄存器（Preset Multiple Registers）

## 通用API

### 连接管理

```c
// 建立Modbus连接
int modbus_connect(modbus_t *ctx);

// 关闭Modbus连接
void modbus_close(modbus_t *ctx);

// 释放Modbus上下文
void modbus_free(modbus_t *ctx);
```

**参数说明：**
- `ctx`：Modbus上下文，由`modbus_new_tcp`或`modbus_new_rtu`创建

**示例：**
```c
modbus_t *ctx = modbus_new_tcp("192.168.1.1", 502);
if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "连接失败: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

// 通信代码...

modbus_close(ctx);
modbus_free(ctx);
```

### 设置操作

```c
// 设置从站地址
int modbus_set_slave(modbus_t *ctx, int slave);

// 获取当前从站地址
int modbus_get_slave(modbus_t *ctx);

// 设置错误恢复模式
int modbus_set_error_recovery(modbus_t *ctx, modbus_error_recovery_mode error_recovery);

// 设置套接字描述符
int modbus_set_socket(modbus_t *ctx, int s);

// 获取套接字描述符
int modbus_get_socket(modbus_t *ctx);

// 设置响应超时
int modbus_set_response_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

// 获取响应超时
int modbus_get_response_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);

// 设置字节超时
int modbus_set_byte_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

// 获取字节超时
int modbus_get_byte_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);

// 设置指示超时
int modbus_set_indication_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

// 获取指示超时
int modbus_get_indication_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);

// 获取消息头长度
int modbus_get_header_length(modbus_t *ctx);

// 清空缓冲区
int modbus_flush(modbus_t *ctx);

// 设置调试模式
int modbus_set_debug(modbus_t *ctx, int flag);
```

**参数说明：**
- `ctx`：Modbus上下文
- `slave`：从站地址（0-247）
- `error_recovery`：错误恢复模式（MODBUS_ERROR_RECOVERY_NONE, MODBUS_ERROR_RECOVERY_LINK, MODBUS_ERROR_RECOVERY_PROTOCOL）
- `s`：套接字描述符
- `to_sec`：超时秒数
- `to_usec`：超时微秒数
- `flag`：调试模式开关（TRUE/FALSE）

### 数据读取

```c
// 读取线圈状态
int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);

// 读取输入状态
int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);

// 读取保持寄存器
int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);

// 读取输入寄存器
int modbus_read_input_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
```

**参数说明：**
- `ctx`：Modbus上下文
- `addr`：起始地址
- `nb`：读取数量
- `dest`：读取结果存储数组

**示例：**
```c
uint8_t bits[10];
// 读取从地址0开始的10个线圈状态
if (modbus_read_bits(ctx, 0, 10, bits) == -1) {
    fprintf(stderr, "读取线圈失败: %s\n", modbus_strerror(errno));
}

uint16_t registers[5];
// 读取从地址100开始的5个寄存器
if (modbus_read_registers(ctx, 100, 5, registers) == -1) {
    fprintf(stderr, "读取寄存器失败: %s\n", modbus_strerror(errno));
}
```

### 数据写入

```c
// 写单个线圈
int modbus_write_bit(modbus_t *ctx, int addr, int status);

// 写单个寄存器
int modbus_write_register(modbus_t *ctx, int addr, const uint16_t value);

// 写多个线圈
int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *data);

// 写多个寄存器
int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *data);

// 寄存器掩码写入
int modbus_mask_write_register(modbus_t *ctx, int addr, uint16_t and_mask, uint16_t or_mask);

// 写入并读取寄存器
int modbus_write_and_read_registers(modbus_t *ctx, int write_addr, int write_nb,
                                   const uint16_t *src, int read_addr, int read_nb,
                                   uint16_t *dest);
```

**参数说明：**
- `ctx`：Modbus上下文
- `addr`：起始地址
- `status`：线圈状态（TRUE/FALSE）
- `value`：寄存器值
- `nb`：数量
- `data`：源数据数组
- `and_mask`：与掩码
- `or_mask`：或掩码
- `write_addr`：写入起始地址
- `write_nb`：写入数量
- `src`：写入源数据
- `read_addr`：读取起始地址
- `read_nb`：读取数量
- `dest`：读取目标数组

**示例：**
```c
// 设置地址5的线圈状态为ON
if (modbus_write_bit(ctx, 5, TRUE) == -1) {
    fprintf(stderr, "写入线圈失败: %s\n", modbus_strerror(errno));
}

// 设置地址10的寄存器值为1234
if (modbus_write_register(ctx, 10, 1234) == -1) {
    fprintf(stderr, "写入寄存器失败: %s\n", modbus_strerror(errno));
}

// 掩码写入：保持寄存器的低8位，将高8位设置为0xFF
if (modbus_mask_write_register(ctx, 10, 0x00FF, 0xFF00) == -1) {
    fprintf(stderr, "掩码写入失败: %s\n", modbus_strerror(errno));
}
```

### 数据映射

```c
// 创建Modbus映射
modbus_mapping_t* modbus_mapping_new(int nb_bits, int nb_input_bits,
                                     int nb_registers, int nb_input_registers);

// 创建具有指定起始地址的Modbus映射
modbus_mapping_t* modbus_mapping_new_start_address(
    unsigned int start_bits, unsigned int nb_bits,
    unsigned int start_input_bits, unsigned int nb_input_bits,
    unsigned int start_registers, unsigned int nb_registers,
    unsigned int start_input_registers, unsigned int nb_input_registers);

// 释放Modbus映射
void modbus_mapping_free(modbus_mapping_t *mb_mapping);

// 使用映射响应Modbus请求
int modbus_reply(modbus_t *ctx, const uint8_t *req,
                int req_length, modbus_mapping_t *mb_mapping);
```

**参数说明：**
- `nb_bits`：线圈数量
- `nb_input_bits`：输入状态数量
- `nb_registers`：保持寄存器数量
- `nb_input_registers`：输入寄存器数量
- `start_bits`：线圈起始地址
- `start_input_bits`：输入状态起始地址
- `start_registers`：保持寄存器起始地址
- `start_input_registers`：输入寄存器起始地址
- `mb_mapping`：Modbus映射结构
- `ctx`：Modbus上下文
- `req`：请求数据
- `req_length`：请求数据长度

**示例（服务器模式）：**
```c
modbus_mapping_t *mb_mapping = modbus_mapping_new(10, 10, 10, 10);
if (mb_mapping == NULL) {
    fprintf(stderr, "创建映射失败: %s\n", modbus_strerror(errno));
    return -1;
}

// 初始化一些值
mb_mapping->tab_registers[0] = 1234;

// 监听并处理请求
uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
while (1) {
    int rc = modbus_receive(ctx, query);
    if (rc > 0) {
        modbus_reply(ctx, query, rc, mb_mapping);
    }
}

modbus_mapping_free(mb_mapping);
```

### 错误处理

```c
// 获取错误信息
const char *modbus_strerror(int errnum);

// 回复异常
int modbus_reply_exception(modbus_t *ctx, const uint8_t *req,
                          unsigned int exception_code);
```

**异常代码：**
- `MODBUS_EXCEPTION_ILLEGAL_FUNCTION`：非法功能码
- `MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS`：非法数据地址
- `MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE`：非法数据值
- `MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE`：从站或服务器故障
- `MODBUS_EXCEPTION_ACKNOWLEDGE`：确认
- `MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY`：从站或服务器忙
- `MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE`：否定确认
- `MODBUS_EXCEPTION_MEMORY_PARITY`：内存奇偶校验错误
- `MODBUS_EXCEPTION_GATEWAY_PATH`：网关路径不可用
- `MODBUS_EXCEPTION_GATEWAY_TARGET`：目标设备无响应

## Modbus TCP

```c
// 创建Modbus TCP上下文
modbus_t* modbus_new_tcp(const char *ip_address, int port);

// TCP服务器监听
int modbus_tcp_listen(modbus_t *ctx, int nb_connection);

// TCP服务器接受连接
int modbus_tcp_accept(modbus_t *ctx, int *s);

// 创建Modbus TCP IPv6上下文
modbus_t* modbus_new_tcp_pi(const char *node, const char *service);

// TCP IPv6服务器监听
int modbus_tcp_pi_listen(modbus_t *ctx, int nb_connection);

// TCP IPv6服务器接受连接
int modbus_tcp_pi_accept(modbus_t *ctx, int *s);
```

**参数说明：**
- `ip_address`：服务器IP地址
- `port`：端口号（默认为502）
- `nb_connection`：最大连接数
- `s`：用于存储接受的socket描述符
- `node`：IPv6节点名称或地址
- `service`：服务名称或端口号

**示例（客户端）：**
```c
// 创建TCP连接
modbus_t *ctx = modbus_new_tcp("192.168.1.1", 502);
if (ctx == NULL) {
    fprintf(stderr, "创建连接失败\n");
    return -1;
}

if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "连接失败: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

// 通信代码...

modbus_close(ctx);
modbus_free(ctx);
```

**示例（服务器）：**
```c
// 创建TCP服务器
modbus_t *ctx = modbus_new_tcp("0.0.0.0", 502);
if (ctx == NULL) {
    fprintf(stderr, "创建服务器失败\n");
    return -1;
}

// 创建数据映射
modbus_mapping_t *mb_mapping = modbus_mapping_new(10, 10, 10, 10);
if (mb_mapping == NULL) {
    fprintf(stderr, "创建映射失败: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

// 监听连接
socket = modbus_tcp_listen(ctx, 1);
if (socket == -1) {
    fprintf(stderr, "监听失败: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

int client_socket;
while (1) {
    if (modbus_tcp_accept(ctx, &client_socket) == -1) {
        break;
    }
    
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    while (1) {
        int rc = modbus_receive(ctx, query);
        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);
        } else {
            break;
        }
    }
}

modbus_mapping_free(mb_mapping);
modbus_close(ctx);
modbus_free(ctx);
```

## Modbus RTU

```c
// 创建Modbus RTU上下文
modbus_t* modbus_new_rtu(const char *device, int baud, char parity,
                         int data_bit, int stop_bit);

// 设置串口模式
int modbus_rtu_set_serial_mode(modbus_t *ctx, int mode);

// 获取串口模式
int modbus_rtu_get_serial_mode(modbus_t *ctx);

// 设置RTS模式
int modbus_rtu_set_rts(modbus_t *ctx, int mode);

// 获取RTS模式
int modbus_rtu_get_rts(modbus_t *ctx);

// 设置自定义RTS函数
int modbus_rtu_set_custom_rts(modbus_t *ctx, void (*set_rts) (modbus_t *ctx, int on));

// 设置RTS延迟
int modbus_rtu_set_rts_delay(modbus_t *ctx, int us);

// 获取RTS延迟
int modbus_rtu_get_rts_delay(modbus_t *ctx);
```

**参数说明：**
- `device`：串口设备名称（如"/dev/ttyS0"或"COM1"）
- `baud`：波特率（如9600, 19200, 115200等）
- `parity`：奇偶校验（'N'=无, 'E'=偶校验, 'O'=奇校验）
- `data_bit`：数据位（通常为8）
- `stop_bit`：停止位（通常为1）
- `mode`：串口模式（MODBUS_RTU_RS232或MODBUS_RTU_RS485）
- `mode`（RTS）：RTS模式（MODBUS_RTU_RTS_NONE, MODBUS_RTU_RTS_UP, MODBUS_RTU_RTS_DOWN）
- `set_rts`：自定义RTS控制函数
- `us`：RTS延迟微秒数

**示例：**
```c
// 创建RTU连接
modbus_t *ctx = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);
if (ctx == NULL) {
    fprintf(stderr, "创建RTU连接失败\n");
    return -1;
}

// 设置从站地址
modbus_set_slave(ctx, 1);

// 设置RS485模式
modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);

// 设置RTS模式
modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);

if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "连接失败: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

// 通信代码...

modbus_close(ctx);
modbus_free(ctx);
```

## 实用函数

```c
// 从int16数组获取int64值
int64_t MODBUS_GET_INT64_FROM_INT16(uint16_t *tab_int16, int index);

// 从bit数组获取byte
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int index, unsigned int nb_bits);

// 从byte设置bits
void modbus_set_bits_from_byte(uint8_t *dest, int index, const uint8_t value);

// 从bytes数组设置bits
void modbus_set_bits_from_bytes(uint8_t *dest, int index, unsigned int nb_bits,
                              const uint8_t *tab_byte);

// 从两个寄存器获取浮点值（不同字节顺序）
float modbus_get_float(const uint16_t *src);
float modbus_get_float_abcd(const uint16_t *src);
float modbus_get_float_dcba(const uint16_t *src);
float modbus_get_float_badc(const uint16_t *src);
float modbus_get_float_cdab(const uint16_t *src);

// 将浮点值设置到两个寄存器（不同字节顺序）
void modbus_set_float(float f, uint16_t *dest);
void modbus_set_float_abcd(float f, uint16_t *dest);
void modbus_set_float_dcba(float f, uint16_t *dest);
void modbus_set_float_badc(float f, uint16_t *dest);
void modbus_set_float_cdab(float f, uint16_t *dest);
```

**字节顺序说明：**
- `abcd`：大端序（Motorola，高位字节在前）
- `dcba`：小端序（Intel，低位字节在前）
- `badc`：中端序（高位字在后，字内高位字节在前）
- `cdab`：中端序（高位字在前，字内低位字节在前）

**示例：**
```c
// 读取保持寄存器并转换为浮点数
uint16_t registers[2];
if (modbus_read_registers(ctx, 0, 2, registers) == -1) {
    fprintf(stderr, "读取寄存器失败: %s\n", modbus_strerror(errno));
}

// 根据设备的字节序选择正确的转换函数
float value = modbus_get_float_abcd(registers);
printf("浮点值: %f\n", value);

// 将浮点数写入保持寄存器
float f = 123.45;
uint16_t regs[2];
modbus_set_float_abcd(f, regs);
if (modbus_write_registers(ctx, 0, 2, regs) == -1) {
    fprintf(stderr, "写入寄存器失败: %s\n", modbus_strerror(errno));
}
```

## 使用示例

### 客户端读取保持寄存器示例

```c
#include <stdio.h>
#include <modbus.h>

int main(void) {
    modbus_t *ctx;
    uint16_t tab_reg[64];
    int i, rc;
    
    // TCP连接
    ctx = modbus_new_tcp("192.168.1.1", 502);
    
    // 设置超时时间
    modbus_set_response_timeout(ctx, 0, 500000); // 0.5秒
    
    // 建立连接
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "连接失败: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
    // 读取10个保持寄存器，起始地址为0
    rc = modbus_read_registers(ctx, 0, 10, tab_reg);
    if (rc == -1) {
        fprintf(stderr, "读取失败: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    
    // 打印寄存器值
    for (i = 0; i < rc; i++) {
        printf("reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
    }
    
    // 关闭连接
    modbus_close(ctx);
    modbus_free(ctx);
    
    return 0;
}
```

### 服务器示例

```c
#include <stdio.h>
#include <modbus.h>
#include <signal.h>

static modbus_t *ctx = NULL;
static int server_socket = -1;
static modbus_mapping_t *mb_mapping = NULL;

void close_server(void) {
    if (server_socket != -1) {
        close(server_socket);
    }
    
    if (mb_mapping != NULL) {
        modbus_mapping_free(mb_mapping);
    }
    
    if (ctx != NULL) {
        modbus_close(ctx);
        modbus_free(ctx);
    }
}

void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("收到SIGINT信号，关闭服务器\n");
        close_server();
        exit(0);
    }
}

int main(void) {
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc;
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    
    // TCP服务器
    ctx = modbus_new_tcp("0.0.0.0", 502);
    
    // 创建数据映射
    mb_mapping = modbus_mapping_new(10, 10, 10, 10);
    if (mb_mapping == NULL) {
        fprintf(stderr, "创建映射失败: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
    // 初始化一些寄存器值
    mb_mapping->tab_registers[0] = 1234;
    mb_mapping->tab_registers[1] = 5678;
    
    // 监听连接
    server_socket = modbus_tcp_listen(ctx, 1);
    if (server_socket == -1) {
        fprintf(stderr, "监听失败: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
    printf("监听连接...\n");
    
    int client_socket;
    while (1) {
        if (modbus_tcp_accept(ctx, &client_socket) == -1) {
            break;
        }
        
        printf("客户端已连接\n");
        
        while (1) {
            rc = modbus_receive(ctx, query);
            if (rc > 0) {
                // 处理请求
                modbus_reply(ctx, query, rc, mb_mapping);
            } else if (rc == -1) {
                // 连接关闭或错误
                break;
            }
        }
    }
    
    printf("服务器终止\n");
    close_server();
    
    return 0;
}
```

### RTU主站示例

```c
#include <stdio.h>
#include <modbus.h>

int main(void) {
    modbus_t *ctx;
    uint16_t tab_reg[64];
    int rc;
    
    // RTU连接
    ctx = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);
    if (ctx == NULL) {
        fprintf(stderr, "创建RTU连接失败\n");
        return -1;
    }
    
    // 设置从站地址
    modbus_set_slave(ctx, 1);
    
    // 设置调试模式
    modbus_set_debug(ctx, TRUE);
    
    // 建立连接
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "连接失败: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
    // 读取输入寄存器
    rc = modbus_read_input_registers(ctx, 0, 10, tab_reg);
    if (rc == -1) {
        fprintf(stderr, "读取失败: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    
    // 写入单个线圈
    rc = modbus_write_bit(ctx, 0, TRUE);
    if (rc == -1) {
        fprintf(stderr, "写入线圈失败: %s\n", modbus_strerror(errno));
    }
    
    // 写入单个寄存器
    rc = modbus_write_register(ctx, 0, 1234);
    if (rc == -1) {
        fprintf(stderr, "写入寄存器失败: %s\n", modbus_strerror(errno));
    }
    
    // 关闭连接
    modbus_close(ctx);
    modbus_free(ctx);
    
    return 0;
}
```

## Python示例

### 安装Python Modbus库
```bash
pip install pymodbus
```

### 客户端读取保持寄存器示例（Python）
```python
from pymodbus.client.sync import ModbusTcpClient

def main():
    # 创建Modbus TCP客户端
    client = ModbusTcpClient('192.168.1.1', port=502)
    
    try:
        # 建立连接
        client.connect()
        print("连接成功")
        
        # 读取10个保持寄存器，起始地址为0
        response = client.read_holding_registers(address=0, count=10, unit=1)
        
        if not response.isError():
            print("读取成功")
            # 打印寄存器值
            for i, value in enumerate(response.registers):
                print(f"reg[{i}]={value} (0x{value:X})")
        else:
            print(f"读取失败: {response}")
            
    except Exception as e:
        print(f"异常：{e}")
    finally:
        # 关闭连接
        client.close()
        print("连接已关闭")

if __name__ == "__main__":
    main()
```

### 服务器示例（Python）
```python
from pymodbus.server.sync import StartTcpServer
from pymodbus.datastore import ModbusSequentialDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

def run_server():
    # 定义Modbus数据存储
    block = ModbusSequentialDataBlock(0, [1234, 5678, 9101, 1121, 3141])
    
    # 创建slave上下文
    store = ModbusSlaveContext(
        di=ModbusSequentialDataBlock(0, [0]*100),    # 离散输入
        co=ModbusSequentialDataBlock(0, [0]*100),    # 线圈
        hr=block,                                    # 保持寄存器
        ir=ModbusSequentialDataBlock(0, [0]*100)     # 输入寄存器
    )
    
    # 创建服务器上下文
    context = ModbusServerContext(slaves=store, single=True)
    
    # 启动服务器
    print("启动Modbus TCP服务器...")
    StartTcpServer(context, address=("0.0.0.0", 502))

if __name__ == "__main__":
    run_server()
```

### RTU主站示例（Python）
```python
from pymodbus.client.sync import ModbusSerialClient

def main():
    # 创建Modbus RTU客户端
    client = ModbusSerialClient(
        method='rtu',
        port='/dev/ttyS0',  # Windows下可能是'COM1'
        baudrate=9600,
        bytesize=8,
        parity='N',
        stopbits=1,
        timeout=1
    )
    
    try:
        # 建立连接
        client.connect()
        print("连接成功")
        
        # 读取输入寄存器
        response = client.read_input_registers(address=0, count=10, unit=1)
        if not response.isError():
            print("读取输入寄存器成功")
            print(response.registers)
            
        # 写入单个线圈
        response = client.write_coil(address=0, value=True, unit=1)
        if not response.isError():
            print("写入线圈成功")
            
        # 写入单个寄存器
        response = client.write_register(address=0, value=1234, unit=1)
        if not response.isError():
            print("写入寄存器成功")
            
    except Exception as e:
        print(f"异常：{e}")
    finally:
        # 关闭连接
        client.close()
        print("连接已关闭")

if __name__ == "__main__":
    main()
```

### 使用asyncio异步客户端示例
```python
import asyncio
from pymodbus.client.asynchronous.tcp import AsyncModbusTCPClient
from pymodbus.client.asynchronous import schedulers

async def async_modbus_client():
    # 创建异步客户端
    client = AsyncModbusTCPClient()
    
    # 连接到服务器
    await client.connect('192.168.1.1', port=502)
    print("连接成功")
    
    try:
        # 读取寄存器
        response = await client.read_holding_registers(0, 10, unit=1)
        if not response.isError():
            print("读取成功:")
            for i, value in enumerate(response.registers):
                print(f"reg[{i}]={value} (0x{value:X})")
                
        # 写入寄存器
        response = await client.write_register(0, 1234, unit=1)
        if not response.isError():
            print("写入成功")
            
        # 读取线圈
        response = await client.read_coils(0, 8, unit=1)
        if not response.isError():
            print("线圈状态:", response.bits)
    finally:
        # 关闭连接
        client.close()
        print("连接已关闭")

if __name__ == "__main__":
    # 运行异步任务
    asyncio.run(async_modbus_client())
```

### 使用PyQt5创建Modbus监控工具
```python
import sys
import time
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout
from PyQt5.QtWidgets import QPushButton, QLineEdit, QLabel, QTableWidget, QTableWidgetItem
from PyQt5.QtWidgets import QComboBox, QGroupBox, QSpinBox, QTabWidget
from PyQt5.QtCore import QTimer, Qt, pyqtSlot
from pymodbus.client.sync import ModbusTcpClient, ModbusSerialClient

class ModbusMonitor(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Modbus监控工具")
        self.setGeometry(100, 100, 800, 600)
        
        self.client = None
        self.connected = False
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update_data)
        
        self.init_ui()
    
    def init_ui(self):
        # 创建中央窗口部件
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 主布局
        main_layout = QVBoxLayout(central_widget)
        
        # 连接设置组
        connection_group = QGroupBox("连接设置")
        connection_layout = QHBoxLayout(connection_group)
        
        # 连接类型
        self.connection_type = QComboBox()
        self.connection_type.addItems(["TCP", "RTU"])
        self.connection_type.currentIndexChanged.connect(self.update_connection_options)
        connection_layout.addWidget(QLabel("连接类型:"))
        connection_layout.addWidget(self.connection_type)
        
        # IP地址/串口
        self.address_label = QLabel("IP地址:")
        connection_layout.addWidget(self.address_label)
        self.address_input = QLineEdit("127.0.0.1")
        connection_layout.addWidget(self.address_input)
        
        # 端口/波特率
        self.port_label = QLabel("端口:")
        connection_layout.addWidget(self.port_label)
        self.port_input = QLineEdit("502")
        connection_layout.addWidget(self.port_input)
        
        # 从站地址
        connection_layout.addWidget(QLabel("从站地址:"))
        self.slave_input = QSpinBox()
        self.slave_input.setRange(0, 255)
        self.slave_input.setValue(1)
        connection_layout.addWidget(self.slave_input)
        
        # 连接按钮
        self.connect_button = QPushButton("连接")
        self.connect_button.clicked.connect(self.toggle_connection)
        connection_layout.addWidget(self.connect_button)
        
        main_layout.addWidget(connection_group)
        
        # 标签页
        self.tab_widget = QTabWidget()
        main_layout.addWidget(self.tab_widget)
        
        # 寄存器页面
        registers_tab = QWidget()
        registers_layout = QVBoxLayout(registers_tab)
        
        # 寄存器设置
        register_settings = QHBoxLayout()
        register_settings.addWidget(QLabel("寄存器类型:"))
        self.register_type = QComboBox()
        self.register_type.addItems(["保持寄存器", "输入寄存器"])
        register_settings.addWidget(self.register_type)
        
        register_settings.addWidget(QLabel("起始地址:"))
        self.start_address = QSpinBox()
        self.start_address.setRange(0, 65535)
        register_settings.addWidget(self.start_address)
        
        register_settings.addWidget(QLabel("数量:"))
        self.register_count = QSpinBox()
        self.register_count.setRange(1, 125)
        self.register_count.setValue(10)
        register_settings.addWidget(self.register_count)
        
        register_settings.addWidget(QLabel("更新频率(ms):"))
        self.update_rate = QSpinBox()
        self.update_rate.setRange(100, 10000)
        self.update_rate.setValue(1000)
        self.update_rate.setSingleStep(100)
        register_settings.addWidget(self.update_rate)
        
        self.update_button = QPushButton("手动更新")
        self.update_button.clicked.connect(self.update_data)
        register_settings.addWidget(self.update_button)
        
        self.auto_update = QPushButton("自动更新")
        self.auto_update.setCheckable(True)
        self.auto_update.clicked.connect(self.toggle_auto_update)
        register_settings.addWidget(self.auto_update)
        
        registers_layout.addLayout(register_settings)
        
        # 寄存器表格
        self.register_table = QTableWidget()
        self.register_table.setColumnCount(3)
        self.register_table.setHorizontalHeaderLabels(["地址", "十进制", "十六进制"])
        self.register_table.verticalHeader().setVisible(False)
        registers_layout.addWidget(self.register_table)
        
        # 添加标签页
        self.tab_widget.addTab(registers_tab, "寄存器")
        
        # 线圈页面
        coils_tab = QWidget()
        coils_layout = QVBoxLayout(coils_tab)
        
        # 线圈设置
        coil_settings = QHBoxLayout()
        coil_settings.addWidget(QLabel("线圈类型:"))
        self.coil_type = QComboBox()
        self.coil_type.addItems(["线圈", "离散输入"])
        coil_settings.addWidget(self.coil_type)
        
        coil_settings.addWidget(QLabel("起始地址:"))
        self.coil_start_address = QSpinBox()
        self.coil_start_address.setRange(0, 65535)
        coil_settings.addWidget(self.coil_start_address)
        
        coil_settings.addWidget(QLabel("数量:"))
        self.coil_count = QSpinBox()
        self.coil_count.setRange(1, 2000)
        self.coil_count.setValue(16)
        coil_settings.addWidget(self.coil_count)
        
        self.update_coils_button = QPushButton("更新线圈")
        self.update_coils_button.clicked.connect(self.update_coils)
        coil_settings.addWidget(self.update_coils_button)
        
        coils_layout.addLayout(coil_settings)
        
        # 线圈表格
        self.coil_table = QTableWidget()
        self.coil_table.setColumnCount(2)
        self.coil_table.setHorizontalHeaderLabels(["地址", "状态"])
        self.coil_table.verticalHeader().setVisible(False)
        coils_layout.addWidget(self.coil_table)
        
        # 添加线圈标签页
        self.tab_widget.addTab(coils_tab, "线圈")
        
        # 状态栏
        self.statusBar().showMessage("未连接")
    
    def update_connection_options(self):
        if self.connection_type.currentText() == "TCP":
            self.address_label.setText("IP地址:")
            self.port_label.setText("端口:")
            self.address_input.setText("127.0.0.1")
            self.port_input.setText("502")
        else:
            self.address_label.setText("串口:")
            self.port_label.setText("波特率:")
            self.address_input.setText("COM1")
            self.port_input.setText("9600")
    
    def toggle_connection(self):
        if not self.connected:
            self.connect_to_device()
        else:
            self.disconnect_device()
    
    def connect_to_device(self):
        try:
            if self.connection_type.currentText() == "TCP":
                self.client = ModbusTcpClient(
                    self.address_input.text(),
                    port=int(self.port_input.text())
                )
            else:
                self.client = ModbusSerialClient(
                    method='rtu',
                    port=self.address_input.text(),
                    baudrate=int(self.port_input.text()),
                    bytesize=8,
                    parity='N',
                    stopbits=1,
                    timeout=1
                )
                
            if self.client.connect():
                self.connected = True
                self.connect_button.setText("断开连接")
                self.statusBar().showMessage("已连接")
            else:
                self.statusBar().showMessage("连接失败")
        except Exception as e:
            self.statusBar().showMessage(f"连接错误: {str(e)}")
    
    def disconnect_device(self):
        if self.client:
            self.client.close()
        self.connected = False
        self.connect_button.setText("连接")
        self.statusBar().showMessage("已断开连接")
        self.stop_auto_update()
    
    def toggle_auto_update(self):
        if self.auto_update.isChecked():
            self.start_auto_update()
        else:
            self.stop_auto_update()
    
    def start_auto_update(self):
        if not self.connected:
            self.auto_update.setChecked(False)
            return
            
        self.update_timer.start(self.update_rate.value())
        self.auto_update.setText("停止更新")
    
    def stop_auto_update(self):
        self.update_timer.stop()
        self.auto_update.setChecked(False)
        self.auto_update.setText("自动更新")
    
    def update_data(self):
        if not self.connected or not self.client:
            return
            
        try:
            start = self.start_address.value()
            count = self.register_count.value()
            unit = self.slave_input.value()
            
            if self.register_type.currentText() == "保持寄存器":
                response = self.client.read_holding_registers(start, count, unit=unit)
            else:
                response = self.client.read_input_registers(start, count, unit=unit)
                
            if not response.isError():
                self.register_table.setRowCount(count)
                for i, value in enumerate(response.registers):
                    self.register_table.setItem(i, 0, QTableWidgetItem(str(start + i)))
                    self.register_table.setItem(i, 1, QTableWidgetItem(str(value)))
                    self.register_table.setItem(i, 2, QTableWidgetItem(f"0x{value:04X}"))
                    
                self.statusBar().showMessage(f"数据更新成功: {time.strftime('%H:%M:%S')}")
            else:
                self.statusBar().showMessage(f"读取失败: {response}")
        except Exception as e:
            self.statusBar().showMessage(f"更新错误: {str(e)}")
    
    def update_coils(self):
        if not self.connected or not self.client:
            return
            
        try:
            start = self.coil_start_address.value()
            count = self.coil_count.value()
            unit = self.slave_input.value()
            
            if self.coil_type.currentText() == "线圈":
                response = self.client.read_coils(start, count, unit=unit)
            else:
                response = self.client.read_discrete_inputs(start, count, unit=unit)
                
            if not response.isError():
                self.coil_table.setRowCount(count)
                for i, value in enumerate(response.bits[:count]):
                    self.coil_table.setItem(i, 0, QTableWidgetItem(str(start + i)))
                    status_item = QTableWidgetItem("ON" if value else "OFF")
                    status_item.setBackground(Qt.green if value else Qt.red)
                    self.coil_table.setItem(i, 1, status_item)
                    
                self.statusBar().showMessage(f"线圈数据更新成功: {time.strftime('%H:%M:%S')}")
            else:
                self.statusBar().showMessage(f"读取线圈失败: {response}")
        except Exception as e:
            self.statusBar().showMessage(f"更新线圈错误: {str(e)}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ModbusMonitor()
    window.show()
    sys.exit(app.exec_())
```

## Qt集成示例

### Modbus TCP客户端（Qt C++）

```cpp
#include <QCoreApplication>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 创建Modbus TCP客户端
    QModbusTcpClient *modbusClient = new QModbusTcpClient(&app);
    
    // 设置连接参数
    modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "192.168.1.1");
    modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, 502);
    
    // 连接到服务器
    if (!modbusClient->connectDevice()) {
        qDebug() << "连接失败:" << modbusClient->errorString();
        return 1;
    }
    
    // 连接成功后创建读请求
    QModbusDataUnit readUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 0, 10);
    
    if (QModbusReply *reply = modbusClient->sendReadRequest(readUnit, 1)) {
        // 使用lambda表达式处理回复
        QObject::connect(reply, &QModbusReply::finished, [reply, modbusClient]() {
            if (!reply->error()) {
                const QModbusDataUnit unit = reply->result();
                for (uint i = 0; i < unit.valueCount(); i++) {
                    qDebug() << "reg[" << i << "]=" << unit.value(i) << "(0x" << QString::number(unit.value(i), 16).toUpper() << ")";
                }
            } else {
                qDebug() << "读取失败:" << reply->errorString();
            }
            
            reply->deleteLater();
            modbusClient->disconnectDevice();
            QCoreApplication::quit();
        });
    } else {
        qDebug() << "发送读请求失败:" << modbusClient->errorString();
        modbusClient->disconnectDevice();
        return 1;
    }
    
    return app.exec();
}
```

### Modbus服务器（Qt C++）

```cpp
#include <QCoreApplication>
#include <QModbusTcpServer>
#include <QModbusDataUnit>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 创建Modbus TCP服务器
    QModbusTcpServer *modbusServer = new QModbusTcpServer(&app);
    
    // 设置服务器数据
    QModbusDataUnitMap reg;
    reg.insert(QModbusDataUnit::RegisterType::DiscreteInputs, QModbusDataUnit(QModbusDataUnit::RegisterType::DiscreteInputs, 0, 100));
    reg.insert(QModbusDataUnit::RegisterType::Coils, QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, 0, 100));
    reg.insert(QModbusDataUnit::RegisterType::InputRegisters, QModbusDataUnit(QModbusDataUnit::RegisterType::InputRegisters, 0, 100));
    reg.insert(QModbusDataUnit::RegisterType::HoldingRegisters, QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 0, 100));
    
    modbusServer->setMap(reg);
    
    // 初始化一些保持寄存器的值
    for (int i = 0; i < 10; ++i) {
        modbusServer->setData(QModbusDataUnit::RegisterType::HoldingRegisters, i, i * 1000 + i);
    }
    
    // 设置服务器地址
    modbusServer->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "0.0.0.0");
    modbusServer->setConnectionParameter(QModbusDevice::NetworkPortParameter, 502);
    modbusServer->setServerAddress(1);
    
    // 启动服务器
    if (!modbusServer->connectDevice()) {
        qDebug() << "服务器启动失败:" << modbusServer->errorString();
        return 1;
    }
    
    qDebug() << "Modbus服务器已启动，按Ctrl+C终止...";
    
    return app.exec();
}
```

### Modbus客户端（PyQt5）

```python
import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QPushButton, QVBoxLayout, QWidget, QTextEdit
from PyQt5.QtCore import QTimer
import pymodbus
from pymodbus.client.sync import ModbusTcpClient

class ModbusClientWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Modbus TCP客户端")
        self.setGeometry(100, 100, 600, 400)
        
        # 创建中央窗口部件
        centralWidget = QWidget()
        self.setCentralWidget(centralWidget)
        
        # 创建垂直布局
        layout = QVBoxLayout(centralWidget)
        
        # 创建按钮
        self.readButton = QPushButton("读取寄存器", self)
        self.readButton.clicked.connect(self.readRegisters)
        
        self.writeButton = QPushButton("写入寄存器", self)
        self.writeButton.clicked.connect(self.writeRegister)
        
        # 创建文本框显示结果
        self.resultText = QTextEdit(self)
        self.resultText.setReadOnly(True)
        
        # 添加部件到布局
        layout.addWidget(self.readButton)
        layout.addWidget(self.writeButton)
        layout.addWidget(self.resultText)
        
        # 创建Modbus客户端
        self.client = ModbusTcpClient('192.168.1.1', port=502)
        self.connected = False
        
        # 定时检查连接状态
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.checkConnection)
        self.timer.start(5000)  # 每5秒检查一次
        
        # 尝试连接
        self.connectToServer()
    
    def connectToServer(self):
        try:
            self.connected = self.client.connect()
            status = "已连接" if self.connected else "连接失败"
            self.resultText.append(f"Modbus服务器状态: {status}")
        except Exception as e:
            self.resultText.append(f"连接异常: {e}")
            self.connected = False
    
    def checkConnection(self):
        if not self.client.is_socket_open():
            self.resultText.append("连接已断开，尝试重新连接...")
            self.connectToServer()
    
    def readRegisters(self):
        if not self.connected:
            self.resultText.append("未连接到服务器")
            return
        
        try:
            response = self.client.read_holding_registers(address=0, count=10, unit=1)
            if not response.isError():
                self.resultText.append("读取成功:")
                for i, value in enumerate(response.registers):
                    self.resultText.append(f"寄存器[{i}] = {value} (0x{value:04X})")
            else:
                self.resultText.append(f"读取失败: {response}")
        except Exception as e:
            self.resultText.append(f"读取异常: {e}")
    
    def writeRegister(self):
        if not self.connected:
            self.resultText.append("未连接到服务器")
            return
        
        try:
            response = self.client.write_register(address=0, value=1234, unit=1)
            if not response.isError():
                self.resultText.append("写入成功: 寄存器[0] = 1234")
            else:
                self.resultText.append(f"写入失败: {response}")
        except Exception as e:
            self.resultText.append(f"写入异常: {e}")
    
    def closeEvent(self, event):
        if self.client:
            self.client.close()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ModbusClientWindow()
    window.show()
    sys.exit(app.exec_()) 