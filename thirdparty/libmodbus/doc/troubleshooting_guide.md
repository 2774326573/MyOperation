# 🔧 libmodbus 故障排除指南

> 快速解决 Modbus 通信中的常见问题

![Troubleshooting](https://img.shields.io/badge/Level-Maintenance-orange.svg) ![Guide](https://img.shields.io/badge/Type-Guide-blue.svg)

---

## 📋 目录

1. [连接问题](#-连接问题)
2. [通信错误](#-通信错误)
3. [性能问题](#-性能问题)
4. [数据错误](#-数据错误)
5. [硬件问题](#-硬件问题)
6. [诊断工具](#-诊断工具)

---

## 🔌 连接问题

### TCP 连接失败

**症状**: 无法建立 TCP 连接
```
ERROR: 连接失败: Connection refused (10061)
```

**解决方案**:
```cpp
// 1. 检查网络连通性
QString command = QString("ping -n 1 %1").arg(ip);
QProcess::execute(command);

// 2. 验证端口是否开放
QTcpSocket socket;
socket.connectToHost(ip, port);
if (socket.waitForConnected(3000)) {
    qDebug() << "端口可访问";
} else {
    qDebug() << "端口被阻塞:" << socket.errorString();
}

// 3. 增加连接超时
modbus_set_response_timeout(ctx, 5, 0);  // 5秒超时
```

### RTU 串口连接失败

**症状**: 串口无法打开
```
ERROR: 无法打开串口: Permission denied
```

**解决方案**:
```cpp
// 1. 检查串口是否被占用
QSerialPortInfo info("COM1");
if (info.isBusy()) {
    qWarning() << "串口被占用";
}

// 2. 检查串口权限 (Linux)
QFile::setPermissions("/dev/ttyS0", 
    QFile::ReadOwner | QFile::WriteOwner | 
    QFile::ReadGroup | QFile::WriteGroup);

// 3. 重试机制
int retryCount = 3;
while (retryCount-- > 0) {
    if (modbus_connect(ctx) != -1) break;
    QThread::msleep(1000);
}
```

---

## 📡 通信错误

### 响应超时

**症状**: 
```
ERROR: Response timeout
```

**诊断步骤**:
```cpp
// 1. 检查超时设置
uint32_t sec, usec;
modbus_get_response_timeout(ctx, &sec, &usec);
qDebug() << "当前超时:" << sec << "秒" << usec << "微秒";

// 2. 动态调整超时
modbus_set_response_timeout(ctx, 2, 0);  // 2秒

// 3. 检查从站地址
int slave = modbus_get_slave(ctx);
qDebug() << "当前从站地址:" << slave;
```

### 数据校验错误

**症状**:
```
ERROR: CRC check failed
```

**解决方案**:
```cpp
// 1. 启用调试模式查看原始数据
modbus_set_debug(ctx, TRUE);

// 2. 检查波特率匹配
// 确保客户端和服务器波特率一致

// 3. 检查电缆质量
// 使用示波器检查信号质量

// 4. 添加重试机制
int retries = 3;
while (retries-- > 0) {
    int rc = modbus_read_registers(ctx, addr, nb, dest);
    if (rc != -1) break;
    
    if (errno == EMBBADCRC) {
        qWarning() << "CRC错误，重试中...";
        QThread::msleep(100);
    } else {
        break;  // 其他错误不重试
    }
}
```

---

## ⚡ 性能问题

### 通信速度慢

**诊断**:
```cpp
// 1. 测量响应时间
QElapsedTimer timer;
timer.start();

int rc = modbus_read_registers(ctx, 0, 10, registers);

qint64 elapsed = timer.elapsed();
qDebug() << "读取耗时:" << elapsed << "毫秒";
```

**优化方案**:
```cpp
// 1. 批量读取
// 不要: 多次单个读取
for (int i = 0; i < 10; i++) {
    modbus_read_registers(ctx, i, 1, &registers[i]);
}

// 推荐: 一次批量读取
modbus_read_registers(ctx, 0, 10, registers);

// 2. 调整超时
modbus_set_response_timeout(ctx, 0, 500000);  // 500ms

// 3. 使用连接池
class ModbusPool {
private:
    QQueue<modbus_t*> pool;
    QMutex mutex;
    
public:
    modbus_t* acquire() {
        QMutexLocker locker(&mutex);
        if (pool.isEmpty()) {
            return createNewConnection();
        }
        return pool.dequeue();
    }
    
    void release(modbus_t* ctx) {
        QMutexLocker locker(&mutex);
        pool.enqueue(ctx);
    }
};
```

---

## 📊 数据错误

### 数据不一致

**问题**: 读取的数据与期望不符

**诊断步骤**:
```cpp
// 1. 验证数据类型
uint16_t raw_value = registers[0];
qDebug() << "原始值 (16进制):" << Qt::hex << raw_value;
qDebug() << "原始值 (10进制):" << Qt::dec << raw_value;

// 2. 检查字节序
float value1 = modbus_get_float_abcd(registers);
float value2 = modbus_get_float_dcba(registers);
qDebug() << "ABCD字节序:" << value1;
qDebug() << "DCBA字节序:" << value2;

// 3. 数据范围验证
if (raw_value > 4000) {
    qWarning() << "数据超出正常范围";
}
```

### 位操作错误

**解决方案**:
```cpp
// 1. 位状态检查
uint8_t bits[8];
modbus_read_bits(ctx, 0, 8, bits);

for (int i = 0; i < 8; i++) {
    qDebug() << QString("位 %1: %2").arg(i).arg(bits[i] ? "ON" : "OFF");
}

// 2. 位掩码操作
uint16_t mask = 0x00FF;  // 保持低8位
uint16_t value = 0xFF00; // 设置高8位
modbus_mask_write_register(ctx, 0, mask, value);
```

---

## 🔧 硬件问题

### RS485 通信问题

**检查清单**:
- [ ] 检查接线: A+ 对 A+, B- 对 B-
- [ ] 验证终端电阻 (120Ω)
- [ ] 检查地线连接
- [ ] 确认电源稳定性

**测试代码**:
```cpp
// 1. 设置 RS485 模式
modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);

// 2. 配置 RTS 控制
modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);
modbus_rtu_set_rts_delay(ctx, 1000);  // 1ms延迟

// 3. 自定义 RTS 控制
void custom_rts_control(modbus_t* ctx, int on) {
    // 硬件相关的RTS控制逻辑
    if (on) {
        // 发送前设置RTS
    } else {
        // 发送后清除RTS
    }
}
modbus_rtu_set_custom_rts(ctx, custom_rts_control);
```

---

## 🔍 诊断工具

### 内置诊断器

```cpp
#include "ModbusTroubleshooter.h"

ModbusTroubleshooter troubleshooter;

// 全面诊断
DiagnosticReport report = troubleshooter.diagnoseSystem();

qDebug() << "=== 诊断报告 ===";
qDebug() << "连接状态:" << report.connectionStatus;
qDebug() << "信号质量:" << report.signalQuality;
qDebug() << "错误计数:" << report.errorCount;

// 获取修复建议
for (const QString& fix : report.suggestedFixes) {
    qDebug() << "建议:" << fix;
}
```

### 网络诊断

```cpp
// TCP 连接测试
bool testTcpConnection(const QString& ip, int port) {
    QTcpSocket socket;
    socket.connectToHost(ip, port);
    
    if (!socket.waitForConnected(3000)) {
        qWarning() << "TCP连接失败:" << socket.errorString();
        return false;
    }
    
    socket.disconnectFromHost();
    return true;
}

// 串口诊断
bool testSerialPort(const QString& portName) {
    QSerialPort port(portName);
    
    if (!port.open(QIODevice::ReadWrite)) {
        qWarning() << "串口打开失败:" << port.errorString();
        return false;
    }
    
    // 测试回环
    QByteArray testData = "TEST";
    port.write(testData);
    port.waitForBytesWritten(1000);
    
    if (port.waitForReadyRead(1000)) {
        QByteArray received = port.readAll();
        return received == testData;
    }
    
    return false;
}
```

### 日志分析

```cpp
// 启用详细日志
modbus_set_debug(ctx, TRUE);

// 自定义日志输出
void modbus_logger(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    QString message = QString::vasprintf(format, args);
    
    // 写入日志文件
    QFile logFile("modbus_debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&logFile);
        stream << QDateTime::currentDateTime().toString()
               << " - " << message << Qt::endl;
    }
    
    va_end(args);
}
```

---

## 📋 常见错误代码

| 错误代码 | 含义 | 解决方案 |
|---------|------|----------|
| EMBXILFUN | 非法功能码 | 检查Modbus功能码是否支持 |
| EMBXILADD | 非法地址 | 验证寄存器地址范围 |
| EMBXILVAL | 非法数据值 | 检查数据值是否在有效范围内 |
| EMBXSFAIL | 从站故障 | 检查从站设备状态 |
| EMBXACK | 确认 | 等待从站处理完成后重试 |
| EMBXSBUSY | 从站忙 | 增加请求间隔时间 |
| EMBXNACK | 否定确认 | 检查命令格式和参数 |
| EMBXMEMPAR | 内存奇偶校验错误 | 检查设备内存状态 |
| EMBXGPATH | 网关路径不可用 | 检查网络路由配置 |
| EMBXGTAR | 目标无响应 | 检查目标设备连接状态 |

---

## 🚀 预防措施

### 1. 连接监控
```cpp
class ConnectionMonitor : public QObject {
    Q_OBJECT
    
private:
    QTimer* heartbeatTimer;
    modbus_t* ctx;
    
public slots:
    void checkConnection() {
        uint16_t test_reg;
        if (modbus_read_registers(ctx, 0, 1, &test_reg) == -1) {
            emit connectionLost();
            attemptReconnect();
        }
    }
    
    void attemptReconnect() {
        if (modbus_connect(ctx) != -1) {
            emit connectionRestored();
        }
    }
    
signals:
    void connectionLost();
    void connectionRestored();
};
```

### 2. 错误统计
```cpp
class ErrorTracker {
private:
    QMap<int, int> errorCounts;
    
public:
    void recordError(int errorCode) {
        errorCounts[errorCode]++;
        
        if (errorCounts[errorCode] > 10) {
            qWarning() << "错误频发:" << modbus_strerror(errorCode);
        }
    }
    
    void printStatistics() {
        for (auto it = errorCounts.begin(); it != errorCounts.end(); ++it) {
            qDebug() << "错误" << it.key() << "发生" << it.value() << "次";
        }
    }
};
```

---

**💡 提示**: 定期检查这些常见问题可以避免大部分通信故障。建议建立监控系统实时检测连接状态。

---

📝 **文档版本**: v1.0  
📅 **最后更新**: 2025年6月5日  
👨‍💻 **维护者**: MyOperation 开发团队
