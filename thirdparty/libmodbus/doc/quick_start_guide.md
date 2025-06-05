# 🚀 libmodbus 快速开始指南

> 5分钟快速上手 Modbus 通信开发

![Beginner Friendly](https://img.shields.io/badge/Level-Beginner-green.svg) ![Time](https://img.shields.io/badge/Time-5%20minutes-blue.svg)

---

## 📋 目录

1. [环境准备](#-环境准备)
2. [第一个程序](#-第一个程序)
3. [TCP 连接示例](#-tcp-连接示例)
4. [RTU 串口示例](#-rtu-串口示例)
5. [常用操作](#-常用操作)
6. [错误处理](#-错误处理)
7. [下一步](#-下一步)

---

## 🛠️ 环境准备

### 1. 检查依赖
确保您的开发环境已安装：
- ✅ Qt 5.12 或更高版本
- ✅ CMake 3.16 或更高版本
- ✅ MSVC 2017+ 或 MinGW 7.3+

### 2. 包含头文件
```cpp
#include "modbusmanager.h"
#include <QApplication>
#include <QDebug>
```

### 3. 链接配置
在您的 `CMakeLists.txt` 中：
```cmake
find_package(Qt5 REQUIRED COMPONENTS Core Network SerialPort)
target_link_libraries(your_target 
    Qt5::Core Qt5::Network Qt5::SerialPort
    modbus  # libmodbus 库
)
```

---

## 🎯 第一个程序

创建一个简单的 Modbus 客户端：

```cpp
#include "modbusmanager.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 1. 创建 Modbus 管理器
    ModbusManager modbus;
    
    // 2. 连接到 Modbus 设备
    if (!modbus.connectTCP("192.168.1.100", 502)) {
        qCritical() << "连接失败:" << modbus.getLastError();
        return -1;
    }
    
    // 3. 设置从站地址
    modbus.setSlaveID(1);
    
    // 4. 读取数据
    QVector<quint16> values;
    if (modbus.readHoldingRegisters(0, 5, values)) {
        qDebug() << "读取成功，数据:" << values;
    } else {
        qCritical() << "读取失败:" << modbus.getLastError();
    }
    
    // 5. 断开连接
    modbus.disconnect();
    
    return 0;
}
```

---

## 🌐 TCP 连接示例

### 基本 TCP 连接
```cpp
class ModbusTcpExample : public QObject
{
    Q_OBJECT
    
public:
    ModbusTcpExample()
    {
        // 连接到 Modbus TCP 设备
        if (m_modbus.connectTCP("192.168.1.100", 502)) {
            qDebug() << "✅ TCP 连接成功";
            setupDevice();
        } else {
            qCritical() << "❌ TCP 连接失败:" << m_modbus.getLastError();
        }
    }
    
private:
    void setupDevice()
    {
        // 设置设备参数
        m_modbus.setSlaveID(1);               // 从站地址
        m_modbus.setResponseTimeout(3000);    // 响应超时 3 秒
        m_modbus.setDebugMode(true);          // 启用调试模式
        
        // 开始数据操作
        readSensorData();
        writeControlData();
    }
    
    void readSensorData()
    {
        // 读取传感器数据（输入寄存器）
        QVector<quint16> sensorValues;
        if (m_modbus.readInputRegisters(100, 10, sensorValues)) {
            qDebug() << "🌡️ 传感器数据:" << sensorValues;
            
            // 解析数据
            float temperature = sensorValues[0] / 10.0f;  // 假设温度数据
            float humidity = sensorValues[1] / 10.0f;     // 假设湿度数据
            
            qDebug() << QString("温度: %1°C, 湿度: %2%")
                        .arg(temperature).arg(humidity);
        }
    }
    
    void writeControlData()
    {
        // 写入控制数据（保持寄存器）
        QVector<quint16> controlValues = {1000, 2000, 3000};
        if (m_modbus.writeMultipleRegisters(200, controlValues)) {
            qDebug() << "✅ 控制数据写入成功";
        } else {
            qCritical() << "❌ 控制数据写入失败:" << m_modbus.getLastError();
        }
    }
    
private:
    ModbusManager m_modbus;
};
```

---

## 📡 RTU 串口示例

### 基本 RTU 连接
```cpp
class ModbusRtuExample : public QObject
{
    Q_OBJECT
    
public:
    ModbusRtuExample()
    {
        // 连接到 Modbus RTU 设备
        if (m_modbus.connectRTU("COM1", 9600, 8, 'N', 1)) {
            qDebug() << "✅ RTU 连接成功";
            setupDevice();
        } else {
            qCritical() << "❌ RTU 连接失败:" << m_modbus.getLastError();
        }
    }
    
private:
    void setupDevice()
    {
        // RTU 特殊配置
        m_modbus.setSlaveID(1);               // 从站地址
        m_modbus.setResponseTimeout(1000);    // RTU 通常需要较短超时
        
        // 测试连接
        testConnection();
    }
    
    void testConnection()
    {
        // 读取设备信息（线圈状态）
        QVector<bool> coilStatus;
        if (m_modbus.readCoils(0, 8, coilStatus)) {
            qDebug() << "📟 线圈状态:" << coilStatus;
            
            // 切换第一个线圈状态
            bool newState = !coilStatus[0];
            if (m_modbus.writeSingleCoil(0, newState)) {
                qDebug() << QString("🔄 线圈 0 状态更改为: %1")
                            .arg(newState ? "ON" : "OFF");
            }
        }
    }
    
private:
    ModbusManager m_modbus;
};
```

---

## 🔧 常用操作

### 数据类型转换
```cpp
// 1. 读取 32 位浮点数（占用 2 个寄存器）
QVector<quint16> rawData;
if (modbus.readHoldingRegisters(0, 2, rawData)) {
    // 合并两个 16 位值为 32 位
    quint32 combined = (rawData[1] << 16) | rawData[0];
    float value = *reinterpret_cast<float*>(&combined);
    qDebug() << "浮点值:" << value;
}

// 2. 写入 32 位整数
quint32 intValue = 123456;
QVector<quint16> writeData = {
    static_cast<quint16>(intValue & 0xFFFF),        // 低 16 位
    static_cast<quint16>((intValue >> 16) & 0xFFFF) // 高 16 位
};
modbus.writeMultipleRegisters(10, writeData);

// 3. 位操作
QVector<bool> bits;
if (modbus.readCoils(0, 16, bits)) {
    // 检查特定位
    if (bits[5]) {
        qDebug() << "第 6 位（索引 5）为 ON";
    }
    
    // 设置多个位
    bits[0] = true;
    bits[1] = false;
    bits[2] = true;
    modbus.writeMultipleCoils(0, bits);
}
```

### 批量操作
```cpp
class BatchOperations
{
public:
    static void readAllSensors(ModbusManager& modbus)
    {
        const int SENSOR_COUNT = 10;
        const int BASE_ADDRESS = 1000;
        
        QVector<quint16> allSensors;
        if (modbus.readHoldingRegisters(BASE_ADDRESS, SENSOR_COUNT, allSensors)) {
            for (int i = 0; i < SENSOR_COUNT; ++i) {
                qDebug() << QString("传感器 %1: %2").arg(i+1).arg(allSensors[i]);
            }
        }
    }
    
    static void writeConfiguration(ModbusManager& modbus, const QMap<int, quint16>& config)
    {
        for (auto it = config.begin(); it != config.end(); ++it) {
            if (!modbus.writeSingleRegister(it.key(), it.value())) {
                qWarning() << QString("写入地址 %1 失败").arg(it.key());
            }
        }
    }
};

// 使用示例
QMap<int, quint16> deviceConfig = {
    {100, 1500},  // 设置点 1
    {101, 2000},  // 设置点 2
    {102, 500}    // 设置点 3
};
BatchOperations::writeConfiguration(modbus, deviceConfig);
```

---

## ⚠️ 错误处理

### 基本错误处理
```cpp
class ErrorHandling
{
public:
    static bool safeRead(ModbusManager& modbus, int address, int count, QVector<quint16>& values)
    {
        if (!modbus.isConnected()) {
            qCritical() << "❌ 设备未连接";
            return false;
        }
        
        if (!modbus.readHoldingRegisters(address, count, values)) {
            QString error = modbus.getLastError();
            int errorCode = modbus.getLastErrorCode();
            
            qCritical() << QString("❌ 读取失败 - 地址: %1, 数量: %2")
                           .arg(address).arg(count);
            qCritical() << QString("❌ 错误信息: %1 (代码: %2)")
                           .arg(error).arg(errorCode);
            
            // 根据错误码决定处理策略
            handleError(errorCode);
            return false;
        }
        
        return true;
    }
    
private:
    static void handleError(int errorCode)
    {
        switch (errorCode) {
        case ETIMEDOUT:
            qWarning() << "⏰ 超时错误 - 建议检查网络连接";
            break;
        case ECONNREFUSED:
            qWarning() << "🚫 连接被拒绝 - 检查设备地址和端口";
            break;
        default:
            qWarning() << "🔧 其他错误 - 检查设备状态";
            break;
        }
    }
};
```

### 重连机制
```cpp
class ReconnectManager : public QObject
{
    Q_OBJECT
    
public:
    ReconnectManager(ModbusManager* modbus) : m_modbus(modbus)
    {
        m_reconnectTimer = new QTimer(this);
        m_reconnectTimer->setSingleShot(true);
        connect(m_reconnectTimer, &QTimer::timeout, this, &ReconnectManager::attemptReconnect);
    }
    
    void startReconnect()
    {
        m_retryCount = 0;
        m_reconnectTimer->start(RECONNECT_INTERVAL);
    }
    
private slots:
    void attemptReconnect()
    {
        if (m_retryCount >= MAX_RETRIES) {
            qCritical() << "❌ 重连失败，已达到最大重试次数";
            emit reconnectFailed();
            return;
        }
        
        qDebug() << QString("🔄 尝试重连 (%1/%2)...").arg(m_retryCount + 1).arg(MAX_RETRIES);
        
        if (m_modbus->isConnected() || reconnectDevice()) {
            qDebug() << "✅ 重连成功";
            emit reconnectSuccess();
        } else {
            m_retryCount++;
            m_reconnectTimer->start(RECONNECT_INTERVAL);
        }
    }
    
private:
    bool reconnectDevice()
    {
        // 这里需要根据实际的连接方式重新连接
        // return m_modbus->connectTCP("192.168.1.100", 502);
        return false; // 示例代码
    }
    
signals:
    void reconnectSuccess();
    void reconnectFailed();
    
private:
    ModbusManager* m_modbus;
    QTimer* m_reconnectTimer;
    int m_retryCount = 0;
    
    static const int MAX_RETRIES = 5;
    static const int RECONNECT_INTERVAL = 3000; // 3 秒
};
```

---

## 🎓 下一步

恭喜！您已经掌握了 libmodbus 的基本使用方法。接下来您可以：

### 📚 深入学习
- 📖 阅读 [API 使用手册](./libmodbus_api_manage.md) 了解高级功能
- 🔧 查看 [性能优化指南](./performance_guide.md) 提升性能
- 🐛 学习 [故障排除指南](./troubleshooting_guide.md) 解决问题

### 🛠️ 实践项目
- 🏭 构建工业数据采集系统
- 📊 开发实时监控界面
- 🔄 实现设备控制逻辑

### 🤝 参与社区
- 🐛 报告 Bug 或提出改进建议
- 💡 分享您的使用经验
- 🤝 贡献代码和文档

---

## 📞 获取帮助

如果您在使用过程中遇到问题：

1. **📖 查看文档**: 先查阅相关文档章节
2. **🔍 搜索问题**: 在 [故障排除指南](./troubleshooting_guide.md) 中查找解决方案
3. **💬 社区支持**: 在项目的 Issues 页面提问
4. **📧 技术支持**: 联系开发团队获取专业支持

---

<div align="center">

**🎉 开始您的 Modbus 开发之旅吧！**

[⬆️ 回到顶部](#-libmodbus-快速开始指南) | [📖 查看完整文档](./README.md)

</div>
