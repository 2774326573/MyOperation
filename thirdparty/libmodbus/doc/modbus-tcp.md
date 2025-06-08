# Modbus TCP 通信文档

## 概述

`modbus-tcp.h` 提供了 Modbus TCP/IP 协议的实现，支持通过以太网进行 Modbus 通信。

## 文件信息

- **文件路径**: `inc/modbus/modbus-tcp.h`
- **版权**: © Stéphane Raimbault <stephane.raimbault@gmail.com>
- **许可证**: LGPL-2.1-or-later

## 主要特性

### TCP/IP 协议支持
- 标准 Modbus TCP 协议
- IPv4 网络通信
- 多客户端支持
- 无需 CRC 校验（TCP 自带错误检测）

### 跨平台网络支持
- Windows Winsock2 支持
- Unix/Linux socket 支持
- 标准错误码映射

## 网络错误码定义

### Windows 平台错误码映射
```c
#if defined(_WIN32) && !defined(__CYGWIN__)
#if !defined(ECONNRESET)
#define ECONNRESET WSAECONNRESET      // 连接被重置
#endif
#if !defined(ECONNREFUSED)
#define ECONNREFUSED WSAECONNREFUSED  // 连接被拒绝
#endif
#if !defined(ETIMEDOUT)
#define ETIMEDOUT WSAETIMEDOUT        // 连接超时
#endif
#if !defined(ENOPROTOOPT)
#define ENOPROTOOPT WSAENOPROTOOPT    // 协议选项不支持
#endif
#if !defined(EINPROGRESS)
#define EINPROGRESS WSAEINPROGRESS    // 操作正在进行
#endif
#endif
```

## 核心函数

### 创建 TCP 连接
```c
MODBUS_API modbus_t *modbus_new_tcp(
    const char *ip,       // 服务器 IP 地址
    int port             // 端口号（通常为 502）
);
```

### TCP 服务器相关
```c
// 创建 TCP 服务器监听
MODBUS_API int modbus_tcp_listen(modbus_t *ctx, int nb_connection);

// 接受客户端连接
MODBUS_API int modbus_tcp_accept(modbus_t *ctx, int *socket);

// 创建 PI（协议无关）连接
MODBUS_API modbus_t *modbus_new_tcp_pi(
    const char *node,     // 主机名或 IP
    const char *service   // 服务名或端口号
);
```

## 使用示例

### TCP 客户端
```c
#include "modbus-tcp.h"

// 创建 TCP 连接
modbus_t *ctx = modbus_new_tcp("192.168.1.100", 502);
if (ctx == NULL) {
    fprintf(stderr, "无法创建 TCP 连接\n");
    return -1;
}

// 设置从站地址（对于 TCP 通常为 1）
modbus_set_slave(ctx, 1);

// 连接到服务器
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
} else {
    printf("成功读取 %d 个寄存器\n", rc);
    for (int i = 0; i < rc; i++) {
        printf("寄存器 %d: %d\n", i, registers[i]);
    }
}

// 清理资源
modbus_close(ctx);
modbus_free(ctx);
```

### TCP 服务器
```c
#include "modbus-tcp.h"

// 创建 TCP 服务器
modbus_t *ctx = modbus_new_tcp("127.0.0.1", 1502);
if (ctx == NULL) {
    fprintf(stderr, "无法创建 TCP 服务器\n");
    return -1;
}

// 设置数据映射
modbus_mapping_t *mb_mapping = modbus_mapping_new(
    500,   // 线圈数量
    500,   // 离散输入数量
    500,   // 保持寄存器数量
    500    // 输入寄存器数量
);

if (mb_mapping == NULL) {
    fprintf(stderr, "无法分配内存\n");
    modbus_free(ctx);
    return -1;
}

// 开始监听
if (modbus_tcp_listen(ctx, 1) == -1) {
    fprintf(stderr, "监听失败\n");
    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);
    return -1;
}

printf("服务器启动，等待连接...\n");

// 主循环
for (;;) {
    int socket = modbus_tcp_accept(ctx, NULL);
    if (socket == -1) {
        break;
    }
    
    printf("客户端已连接\n");
    
    // 处理请求循环
    for (;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc = modbus_receive(ctx, query);
        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc == -1) {
            break;  // 连接断开
        }
    }
    
    printf("客户端已断开\n");
    close(socket);
}

// 清理资源
modbus_mapping_free(mb_mapping);
modbus_close(ctx);
modbus_free(ctx);
```

### 使用 PI 接口
```c
// 使用主机名和服务名
modbus_t *ctx = modbus_new_tcp_pi("server.example.com", "modbus");

// 或使用 IP 和端口号字符串
modbus_t *ctx = modbus_new_tcp_pi("192.168.1.100", "502");
```

## 协议特点

### Modbus TCP 帧结构
```
| MBAP 头部 (7字节) | PDU (可变长度) |
```

### MBAP 头部结构
```
字节 0-1: 事务标识符 (Transaction Identifier)
字节 2-3: 协议标识符 (Protocol Identifier) - 固定为 0
字节 4-5: 长度字段 (Length Field)
字节 6:   单元标识符 (Unit Identifier)
```

## 网络配置

### 默认端口
- **标准端口**: 502
- **备用端口**: 503, 1502（测试用）

### 防火墙配置
确保 Modbus TCP 端口已开放：
```bash
# Linux 防火墙配置示例
sudo ufw allow 502/tcp
sudo firewall-cmd --add-port=502/tcp --permanent
```

### 网络性能优化
```c
// 设置 TCP 无延迟
modbus_set_socket_nodelay(ctx, TRUE);

// 设置接收超时
struct timeval timeout;
timeout.tv_sec = 5;
timeout.tv_usec = 0;
modbus_set_response_timeout(ctx, timeout.tv_sec, timeout.tv_usec);
```

## 常见问题排除

### 连接问题
1. **连接被拒绝 (ECONNREFUSED)**
   - 检查服务器是否运行
   - 确认端口号正确
   - 验证防火墙设置

2. **连接超时 (ETIMEDOUT)**
   - 检查网络连通性
   - 验证 IP 地址正确
   - 调整超时设置

3. **连接被重置 (ECONNRESET)**
   - 服务器主动断开连接
   - 检查服务器负载
   - 验证协议版本兼容性

### 性能问题
1. **响应慢**
   - 减少批量读取大小
   - 优化网络配置
   - 使用连接池

2. **频繁断线**
   - 检查网络稳定性
   - 调整心跳机制
   - 实现重连逻辑

## 安全注意事项

1. **网络安全**
   - Modbus TCP 无内置加密
   - 考虑使用 VPN 或 SSL 隧道
   - 限制网络访问权限

2. **访问控制**
   - 实现客户端认证
   - 限制功能码访问
   - 记录操作日志

## 高级功能

### 多客户端支持
```c
// 服务器支持多个同时连接
if (modbus_tcp_listen(ctx, 10) == -1) {  // 最多10个连接
    // 错误处理
}
```

### 异步操作
```c
// 设置非阻塞模式
fcntl(socket, F_SETFL, O_NONBLOCK);
```

## 相关文档

- [Modbus 核心文档](modbus.md)
- [Modbus RTU 文档](modbus-rtu.md)
- [Modbus 管理器文档](modbusmanager.md)
