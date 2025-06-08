# PLC寄存器手册

## 概述
本手册详细介绍了PLC中各种寄存器类型及其在Modbus通信中的对应关系。

## 1. 寄存器类型分类

### 1.1 X点 - 输入点 (Input Points)
**功能描述**: 外部输入信号接收点
- **类型**: 只读布尔型
- **地址范围**: X00-X07, X10-X17, X20-X27... (八进制)
- **用途**: 连接外部输入设备
  - 传感器信号
  - 按钮开关
  - 限位开关
  - 光电开关
  - 接近开关

**Modbus映射**:
- **功能码**: 02 (读离散输入)
- **寄存器类型**: Discrete Inputs
- **数据长度**: 1 bit per point

**编程示例**:
```
IF X00 THEN    // 如果X00输入为ON
    Y00 = ON   // 则Y00输出为ON
END_IF
```

### 1.2 Y点 - 输出点 (Output Points)
**功能描述**: 外部输出信号控制点
- **类型**: 可读写布尔型
- **地址范围**: Y00-Y07, Y10-Y17, Y20-Y27... (八进制)
- **用途**: 控制外部输出设备
  - 继电器
  - 电磁阀
  - 指示灯
  - 蜂鸣器
  - 电机启停

**Modbus映射**:
- **读取功能码**: 01 (读线圈)
- **写入功能码**: 05 (写单个线圈), 15 (写多个线圈)
- **寄存器类型**: Coils
- **数据长度**: 1 bit per point

**编程示例**:
```
Y00 = X00 AND X01    // Y00输出 = X00输入 AND X01输入
SET Y01              // 强制设置Y01为ON
RST Y01              // 强制复位Y01为OFF
```

### 1.3 D寄存器 - 数据寄存器 (Data Registers)
**功能描述**: 存储数值数据
- **类型**: 16位有符号/无符号整数
- **地址范围**: D0-D7999
- **数值范围**: -32768 ~ +32767 (有符号) 或 0 ~ 65535 (无符号)
- **用途**: 
  - 计数器当前值
  - 定时器设定值
  - 模拟量数据
  - 运算结果存储
  - 参数设置

**特殊区域**:
- **D0-D199**: 一般用途，断电不保持
- **D200-D511**: 断电保持区域
- **D512-D7999**: 一般用途，断电不保持

**Modbus映射**:
- **读取功能码**: 03 (读保持寄存器), 04 (读输入寄存器)
- **写入功能码**: 06 (写单个寄存器), 16 (写多个寄存器)
- **寄存器类型**: Holding Registers
- **数据长度**: 16 bits per register

**编程示例**:
```
D0 = D1 + D2         // D0 = D1加D2的结果
D10 = K1000          // D10设置为常数1000
IF D20 > K500 THEN   // 如果D20大于500
    Y00 = ON         // 则Y00输出
END_IF
```

### 1.4 M寄存器 - 内部继电器 (Internal Relays)
**功能描述**: 内部逻辑控制点
- **类型**: 布尔型 (ON/OFF)
- **地址范围**: M0-M7999
- **用途**:
  - 程序内部逻辑
  - 状态标志位
  - 中间计算结果
  - 条件判断标志

**特殊区域**:
- **M0-M499**: 一般用途，断电不保持
- **M500-M1023**: 断电保持区域
- **M1024-M7999**: 一般用途，断电不保持

**Modbus映射**:
- **读取功能码**: 01 (读线圈)
- **写入功能码**: 05 (写单个线圈), 15 (写多个线圈)
- **寄存器类型**: Coils
- **数据长度**: 1 bit per relay

**编程示例**:
```
M0 = X00 AND X01     // M0 = X00 AND X01
SET M1               // 强制设置M1
RST M1               // 强制复位M1
Y00 = M0 OR M1       // Y00 = M0 OR M1
```

## 2. Modbus功能码对应表

| PLC寄存器 | Modbus类型 | 读功能码 | 写功能码 | 地址范围 | 数据类型 |
|-----------|------------|----------|----------|----------|----------|
| X点 | Discrete Inputs | 02 | - | 10001-19999 | 1 bit |
| Y点 | Coils | 01 | 05/15 | 00001-09999 | 1 bit |
| M点 | Coils | 01 | 05/15 | 00001-09999 | 1 bit |
| D寄存器 | Holding Registers | 03 | 06/16 | 40001-49999 | 16 bit |

## 3. 地址转换规则

### 3.1 八进制转十进制
PLC中的X、Y点使用八进制编址:
```
X00 = 0,  X01 = 1,  ..., X07 = 7
X10 = 8,  X11 = 9,  ..., X17 = 15
X20 = 16, X21 = 17, ..., X27 = 23
```

### 3.2 Modbus地址映射
```cpp
// X点 -> Discrete Inputs (起始地址: 10001)
int modbusAddr = plcAddr + 10001;
// 例: X00 -> 10001, X07 -> 10008, X10 -> 10009

// Y点 -> Coils (起始地址: 00001) 
int modbusAddr = plcAddr + 1;
// 例: Y00 -> 1, Y07 -> 8, Y10 -> 9

// D寄存器 -> Holding Registers (起始地址: 40001)
int modbusAddr = plcAddr + 40001;
// 例: D0 -> 40001, D100 -> 40101

// M点 -> Coils (起始地址: 1000)
int modbusAddr = plcAddr + 1000;
// 例: M0 -> 1000, M100 -> 1100
```

## 4. 编程注意事项

### 4.1 数据范围
- **D寄存器**: 注意有符号/无符号范围
- **浮点数**: 需要使用两个连续D寄存器 (32位)
- **字符串**: 每个D寄存器存储2个ASCII字符

### 4.2 特殊寄存器
```
M8000: RUN状态标志
M8001: 初始化脉冲
M8002: 系统时钟(10ms)
M8011: 10秒时钟
M8012: 1分钟时钟
M8013: 1小时时钟

D8000: 系统寄存器
D8001: 系统时间-秒
D8002: 系统时间-分
D8003: 系统时间-时
```

### 4.3 通信配置
```cpp
// 典型Modbus RTU配置
struct ModbusConfig {
    QString portName = "COM1";
    int baudRate = 9600;
    int dataBits = 8;
    int parity = 0;      // 0:无校验, 1:奇校验, 2:偶校验
    int stopBits = 1;
    int slaveID = 1;     // 从站地址 1-247
    int timeout = 1000;  // 超时时间(ms)
};
```

## 5. 常用操作示例

### 5.1 读取输入点状态
```cpp
// 读取X00-X07状态
QVector<bool> inputStates;
bool success = modbusManager->readDiscreteInputs(0, 8, inputStates);
```

### 5.2 控制输出点
```cpp
// 设置Y00为ON
bool success = modbusManager->writeSingleCoil(0, true);

// 设置Y00-Y07
QVector<bool> outputStates = {true, false, true, false, true, false, true, false};
bool success = modbusManager->writeMultipleCoils(0, outputStates);
```

### 5.3 读写数据寄存器
```cpp
// 读取D0-D9
QVector<quint16> dataValues;
bool success = modbusManager->readHoldingRegisters(0, 10, dataValues);

// 写入D0
bool success = modbusManager->writeSingleRegister(0, 1234);
```

### 5.4 读写内部继电器
```cpp
// 读取M0-M15状态
QVector<bool> relayStates;
bool success = modbusManager->readCoils(1000, 16, relayStates);

// 设置M0为ON
bool success = modbusManager->writeSingleCoil(1000, true);
```

## 6. 故障排除

### 6.1 常见错误码
- **01**: 非法功能码
- **02**: 非法数据地址  
- **03**: 非法数据值
- **04**: 从站设备故障
- **05**: 确认
- **06**: 从站设备忙

### 6.2 调试建议
1. 检查波特率、校验位设置
2. 确认从站地址正确
3. 验证寄存器地址范围
4. 检查数据格式和长度
5. 调整超时时间设置

## 7. 参考资料
- Modbus协议规范
- PLC编程手册
- 设备通信手册
