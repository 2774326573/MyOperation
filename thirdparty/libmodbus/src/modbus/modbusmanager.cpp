#include "../../inc/modbus/modbusManager.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QThread>
#include <QCoreApplication>
#include <errno.h>

#define LOG_INFO(msg) qDebug() << "[INFO]" << msg
#define LOG_ERROR(msg) qDebug() << "[ERROR]" << msg
#define LOG_WARN(msg) qDebug() << "[WARN]" << msg


ModbusManager::ModbusManager(QObject* parent):
  QObject(parent)
  , m_modbusCtx(nullptr)
  , m_connectionType(ConnectionType::TCP)
  , m_connected(false)
  , m_slaveID(1)
  , m_lastError("")
  , m_lastErrorCode(0)
  , m_tcpIp("")
  , m_tcpPort(502)
  , m_connectionTimer(nullptr)
  , m_rtuPort("")
  , m_rtuBaudRate(9600)
  , m_rtuDataBits(8)
  , m_rtuParity('N')
  , m_rtuStopBits(1)
  , m_byteTimeout(500)
  // 默认字节超时为500毫秒 (default byte timeout is 500 milliseconds)
  , m_responseTimeout(1000)
  // 默认响应超时为1000毫秒 (default response timeout is 1000 milliseconds)
  , m_timeoutMsec(1000)
  // 默认超时时间为1000毫秒 (default timeout is 1000 milliseconds)
  , m_retryCount(3) // 默认重试次数为3次 (default retry count is 3)
  , m_debugMode(false)
{
  m_connectionTimer = new QTimer(this); // 创建连接定时器
  m_connectionTimer->setSingleShot(true); // 设置单次触发
  // 连接定时器超时信号到处理函数
  connect(m_connectionTimer, &QTimer::timeout, this, &ModbusManager::handleConnectionTimeout);
}

ModbusManager::~ModbusManager()
{
  disconnect();
}

// 辅助函数：将整数校验位转换为字符校验位
char ModbusManager::convertParityToChar(int parity)
{
  switch (parity)
  {
  case 0:
    return 'N'; // QSerialPort::NoParity
  case 1:
    return 'O'; // QSerialPort::OddParity
  case 2:
    return 'E'; // QSerialPort::EvenParity
  case 3:
    return 'S'; // QSerialPort::SpaceParity
  case 4:
    return 'M'; // QSerialPort::MarkParity
  default:
    emit infoLog(tr("未知的校验位值，使用无校验(N)"));
    return 'N'; // 默认无校验
  }
}

// 重载版本：接受整数校验位参数，内部转换为字符
bool ModbusManager::connectRTU(const QString& port, int baudRate, int dataBits, int parity, int stopBits)
{
  // 将整数校验位转换为字符校验位
  char parityChar = convertParityToChar(parity);

  // 调用原始的字符版本
  return connectRTU(port, baudRate, dataBits, parityChar, stopBits);
}

bool ModbusManager::connectRTU(const QString& port, int baudRate, int dataBits, char parity, int stopBits)
{
  QMutexLocker lock(&m_mutex);

  // 如果当前已经连接，则先断开连接 (if already connected, disconnect first)
  if (m_connected)
  {
    disconnect();
  }

  // 详细的串口诊断 (Detailed serial port diagnostics)
  LOG_INFO(tr("=== 开始串口连接诊断 ==="));
  // 检查串口是否存在
  PortInfo portInfo = getPortInfo(port);
  if (!portInfo.exists)
  {
    setLastError(tr("串口 %1 不存在").arg(port));
    // 显示可用串口
    QStringList availablePorts = getAvailablePorts();
    if (!availablePorts.isEmpty())
    {
      emit infoLog(tr("可用串口: %1").arg(availablePorts.join(", ")));
    }
    else
    {
      emit infoLog(tr("系统中未检测到任何串口设备"));
    }
    return false;
  }

  // 检查串口是否可访问
  if (!portInfo.accessible)
  {
    if (portInfo.inUse)
    {
      setLastError(tr("串口 %1 被其他程序占用: %2").arg(port).arg(portInfo.errorString));
    }
    else
    {
      setLastError(tr("串口 %1 无法访问: %2").arg(port).arg(portInfo.errorString));
    }    // 显示解决建议
    QStringList recommendations = getRecommendations(port);
    for (const QString& rec : recommendations)
    {
      emit infoLog(tr("rec: %1").arg(rec));
    }
    return false;
  }

  emit infoLog(tr("串口 %1 可用性检查通过").arg(port));
  emit infoLog(tr("串口描述: %1").arg(portInfo.description));
  emit infoLog(tr("制造商: %1").arg(portInfo.manufacturer));
  // 使用Qt串口进行预连接测试
  if (!testPortConnection(port, baudRate, dataBits, parity, stopBits))
  {
    setLastError(tr("串口参数测试失败，请检查波特率和其他参数"));
    return false;
  }

  // 创建新的Modbus上下文 (create a new Modbus context)
  m_connectionType = ConnectionType::RTU; // 设置连接类型为RTU (set connection type to RTU)
  m_rtuPort = port; // 串口号          (serial port)
  m_rtuBaudRate = baudRate; // 波特率          (baud rate)
  m_rtuDataBits = dataBits; // 数据位          (data bits)
  m_rtuParity = parity; // 校验位          (parity)
  m_rtuStopBits = stopBits; // 停止位          (stop bits)
  // 记录连接参数
  emit infoLog(tr("RTU连接参数 - 端口: %1, 波特率: %2, 数据位: %3, 校验: %4, 停止位: %5")
               .arg(m_rtuPort).arg(m_rtuBaudRate).arg(m_rtuDataBits).arg(m_rtuParity).arg(m_rtuStopBits));

  // 创建libmodbus上下文 - Windows需要特殊的串口名称格式
  QString windowsPortName = m_rtuPort;
  if (windowsPortName.startsWith("COM") && !windowsPortName.startsWith("\\\\.\\"))
  {
    // Windows上COM端口号大于9时需要特殊格式
    bool ok;
    int portNumber = windowsPortName.mid(3).toInt(&ok);
    if (ok && portNumber > 9)
    {
      windowsPortName = "\\\\.\\" + m_rtuPort;
    }
  }
  emit infoLog(tr("使用串口名称: %1 (原始: %2)").arg(windowsPortName).arg(m_rtuPort));
  emit infoLog(tr("RTU参数: 波特率=%1, 数据位=%2, 校验=%3, 停止位=%4")
               .arg(m_rtuBaudRate).arg(m_rtuDataBits).arg(m_rtuParity).arg(m_rtuStopBits));

      // 清除errno以确保能正确捕获错误
      errno = 0;
  m_modbusCtx = modbus_new_rtu(windowsPortName.toStdString().c_str(), m_rtuBaudRate, m_rtuParity, m_rtuDataBits,
                               m_rtuStopBits);
  // 检查上下文是否创建成功 (check if context is created successfully)
  if (m_modbusCtx == nullptr)
  {
    QString errorMsg = QString::fromLocal8Bit(modbus_strerror(errno));
    if (errorMsg.isEmpty() || errorMsg == "No error")
    {
      errorMsg = tr("未知错误 - 可能是参数无效或系统资源不足");
    }

    QString detailedError = tr("无法创建Modbus RTU 上下文: %1 (errno: %2)")
                            .arg(errorMsg).arg(errno);
    setLastError(detailedError);
    emit infoLog(tr("libmodbus上下文创建失败 - %1").arg(detailedError));

    // 输出更多诊断信息
    emit infoLog(tr("参数详情 - 串口: %1, 波特率: %2, 校验: %3, 数据位: %4, 停止位: %5")
                 .arg(windowsPortName).arg(m_rtuBaudRate).arg(m_rtuParity).arg(m_rtuDataBits).arg(m_rtuStopBits));
    return false;
  }

  // 设置超时 (set timeout) - 增加超时时间以适应慢速设备
  modbus_set_response_timeout(m_modbusCtx, 3, 0); // 3秒超时 (3 second timeout)
  modbus_set_byte_timeout(m_modbusCtx, 0, 500000); // 设置字节超时 (set byte timeout)

  // 启用调试模式以获取更多信息
  modbus_set_debug(m_modbusCtx, 1);

  // 设置错误恢复模式
  modbus_set_error_recovery(m_modbusCtx,
                            static_cast<modbus_error_recovery_mode>(MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL));

  // 设置从机地址 (set slave ID)
  modbus_set_slave(m_modbusCtx, m_slaveID);
  emit infoLog(tr("设置从机地址: %1").arg(m_slaveID));

  // 尝试连接 (try to connect)
  emit infoLog(tr("正在尝试连接串口: %1").arg(m_rtuPort));
  if (modbus_connect(m_modbusCtx) == -1)
  {
    QString errorMsg = QString::fromLocal8Bit(modbus_strerror(errno));
    if (errorMsg.isEmpty() || errorMsg == "No error")
    {
      errorMsg = tr("串口可能被其他程序占用或不存在");
    }

    // 提供更详细的错误分析
    QString detailedError = tr("Modbus RTU连接失败: %1 (端口: %2, errno: %3)")
                            .arg(errorMsg).arg(m_rtuPort).arg(errno);

    // 根据errno提供具体建议
    if (errno == ENOENT || errno == 2)
    {
      detailedError += tr("\n建议: 串口设备不存在，请检查设备连接和驱动");
    }
    else if (errno == EACCES || errno == 13)
    {
      detailedError += tr("\n建议: 权限不足，请以管理员身份运行或检查串口权限");
    }
    else if (errno == EBUSY || errno == 16)
    {
      detailedError += tr("\n建议: 串口被占用，请关闭其他串口程序");
    }

    setLastError(detailedError);

    modbus_free(m_modbusCtx); // 释放上下文 (free context)
    m_modbusCtx = nullptr; // 清空上下文指针 (clear context pointer)
    return false;
  }

  m_connected = true;

  // 尝试简单的通信测试
  emit infoLog(tr("串口连接成功，正在测试Modbus通信..."));

  // 尝试读取一个保持寄存器来测试通信
  uint16_t testValue;
  int result = modbus_read_registers(m_modbusCtx, 0, 1, &testValue);
  if (result == 1)
  {
    emit infoLog(tr("Modbus通信测试成功，读取到寄存器0的值: %1").arg(testValue));
  }
  else
  {
    emit infoLog(tr("Modbus通信测试失败: %1 (这可能是正常的，如果地址0不存在)").arg(modbus_strerror(errno)));
    emit infoLog(tr("串口连接成功，但建议检查设备地址和从机ID设置"));
  }
  emit connected(); // 发送连接成功信号 (emit connected signal)
  emit infoLog(tr("=== Modbus RTU连接成功: %1 ===").arg(m_rtuPort)); // 输出连接成功信息 (output connection success message)
  return true;
}

bool ModbusManager::connectTCP(const QString& ip, int port)
{
  QMutexLocker locker(&m_mutex);
  // 如果当前已经连接，则先断开连接 (if already connected, disconnect first)
  if (m_connected)
  {
    disconnect();
  }
  // 创建新的Modbus上下文 (create a new Modbus context)
  m_connectionType = ConnectionType::TCP; // 设置连接类型为TCP (set connection type to TCP)
  m_tcpIp = ip; // TCP IP地址 (TCP IP address)
  m_tcpPort = port; // TCP端口号 (TCP port number)
  m_modbusCtx = modbus_new_tcp(m_tcpIp.toStdString().c_str(), m_tcpPort);
  if (m_modbusCtx == nullptr)
  {
    setLastError(tr("无法创建Modbus TCP 上下文"));
    return false;
  }

  // 设置超时 (set timeout)
  modbus_set_response_timeout(m_modbusCtx, 1, 0); // 1秒超时

  // 设置从机地址 (set slave ID)
  modbus_set_slave(m_modbusCtx, m_slaveID);

  // 尝试连接 (try to connect)
  if (modbus_connect(m_modbusCtx) == -1)
  {
    setLastError(tr("Modbus TCP链接失败: %1").arg(modbus_strerror(errno)));
    modbus_free(m_modbusCtx); // 释放上下文 (free context)
    m_modbusCtx = nullptr; // 清空上下文指针 (clear context pointer)
    return false;
  }
  m_connected = true; // 设置连接状态 (set connected state)
  emit connected(); // 发送连接成功信号 (emit connected signal)
  emit infoLog(tr("Modbus TCP连接成功: %1:%2").arg(m_tcpIp).arg(m_tcpPort)); // 输出连接成功信息 (output connection success message)
  return true;
}

void ModbusManager::disconnect()
{
  QMutexLocker locker(&m_mutex);
  if (m_connected && m_modbusCtx != nullptr)
  {
    modbus_close(m_modbusCtx); // 关闭连接 (close connection)
    modbus_free(m_modbusCtx); // 释放上下文 (free context)
    m_modbusCtx = nullptr; // 清空上下文指针 (clear context pointer)
    m_connected = false; // 设置连接状态为未连接 (set connected state to false)
    emit disconnected(); // 发送断开连接信号 (emit disconnected signal)
    emit infoLog(tr("Modbus连接已断开"));
  }
}

bool ModbusManager::isConnected() const
{
  return m_connected; // 返回连接状态 (return connected state)
}

void ModbusManager::setSlaveID(int slaveID)
{
  QMutexLocker locker(&m_mutex);

  m_slaveID = slaveID; // 设置从机地址 (set slave ID)
  emit infoLog(tr("设置从机地址为: %1").arg(m_slaveID));

  // 如果上下文已经存在，立即更新 (if context exists, update immediately)
  if (m_modbusCtx != nullptr)
  {
    modbus_set_slave(m_modbusCtx, m_slaveID); // 更新Modbus上下文中的从机地址 (update slave ID in Modbus context)
    emit infoLog(tr("已更新Modbus上下文中的从机地址"));
  }
}

int ModbusManager::getSlaveID() const
{
  return m_slaveID; // 返回从机地址 (return slave ID)
}

void ModbusManager::setTimeout(int timeoutMsec)
{
  QMutexLocker locker(&m_mutex);
  m_timeoutMsec = timeoutMsec; // 保存超时设置 (save timeout setting)
  emit infoLog(tr("设置响应超时为: %1 毫秒").arg(timeoutMsec));

  // 如果上下文已经存在，立即应用设置 (if context exists, apply setting immediately)
  if (m_modbusCtx != nullptr)
  {
    // 将毫秒转换为秒和微秒 (convert milliseconds to seconds and microseconds)
    modbus_set_response_timeout(m_modbusCtx, timeoutMsec / 1000, (timeoutMsec % 1000) * 1000);
    // 设置响应超时 (set response timeout)
    emit infoLog(tr("已更新Modbus上下文中的响应超时"));
  }
}

void ModbusManager::setDebugMode(bool enable)
{
  QMutexLocker locker(&m_mutex);
  m_debugMode = enable; // 保存调试模式设置 (save debug mode setting)
  emit infoLog(tr("设置调试模式为: %1").arg(enable ? tr("启用") : tr("禁用")));

  // 如果上下文已经存在，立即应用设置 (if context exists, apply setting immediately)
  if (m_modbusCtx != nullptr)
  {
    modbus_set_debug(m_modbusCtx, enable ? 1 : 0); // 设置调试模式 (set debug mode)
    emit infoLog(tr("已更新Modbus上下文中的调试模式"));
  }
}

/* ==================== 数据读取 | en:Data reading ====================== */
// 单个线圈/多个线圈用 uint8_t，单个/多个寄存器用 uint16_t，是因为它们在 Modbus 协议中的数据宽度不同。

bool ModbusManager::readCoils(int address, int count, QVector<bool>& values)
{
  // QVariant 在 Qt 中是一个通用的数据类型容器，可以存储多种类型的数据。
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  QVector<uint8_t> buffer(count);
  int result = modbus_read_bits(m_modbusCtx, address, count, buffer.data()); // 读取线圈 (read coils)
  if (result == -1)
  {
    setLastError(tr("读取线圈失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  values.clear();
  values.reserve(count); // 预留空间 (reserve space)
  // 防止缓冲区溢出 - 使用较小的值作为循环边界 (prevent buffer overflow - use smaller value as loop boundary)
  int safeBound = qMin(result, count);
  for (int i = 0; i < safeBound; ++i)
  {
    values.append(buffer[i] != 0); // 将读取的线圈数据转换为布尔值 (convert read coil data to boolean)
  }
  emit infoLog(tr("成功读取线圈数据"));
  // 发送数据接收信号 (emit data received signal)
  emit dataReceived(DataType::Coils, address, QVariant::fromValue(values));
  return true;
}

bool ModbusManager::readDiscreteInputs(int address, int count, QVector<bool>& values)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  QVector<uint8_t> buffer(count); // 创建缓冲区 (create buffer)
  // 读取离散输入 (read discrete inputs)
  int result = modbus_read_input_bits(m_modbusCtx, address, count, buffer.data());
  if (result == -1)
  {
    setLastError(tr("读取离散输入失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  values.clear();
  values.reserve(count); // 预留空间 (reserve space)  
  // 将读取的离散输入数据转换为布尔值 (convert read discrete input data to boolean)
  // 防止缓冲区溢出 - 使用较小的值作为循环边界 (prevent buffer overflow - use smaller value as loop boundary)
  int safeBound = qMin(result, count);
  for (int i = 0; i < safeBound; ++i)
  {
    values.append(buffer[i] != 0);
  }
  emit infoLog(tr("成功读取离散输入数据"));
  // 发送数据接收信号 (emit data received signal)
  emit dataReceived(DataType::DiscreteInputs, address, QVariant::fromValue(values));
  return true;
}

bool ModbusManager::readHoldingRegisters(int address, int count, QVector<quint16>& values)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  
  // 参数验证 (parameter validation)
  if (count <= 0 || count > 125) // Modbus标准限制 (Modbus standard limit)
  {
    setLastError(tr("无效的读取数量: %1 (范围: 1-125)").arg(count));
    return false;
  }
  
  QVector<uint16_t> buffer(count); // 创建缓冲区 (create buffer)
  // 读取保持寄存器 (read holding registers)
  int result = modbus_read_registers(m_modbusCtx, address, count, buffer.data());  if (result == -1)
  {
    setLastError(tr("读取保持寄存器失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  
  // 验证返回的数据量 (validate returned data count)
  if (result != count)
  {
    emit infoLog(tr("警告: 请求读取 %1 个寄存器，实际返回 %2 个").arg(count).arg(result));
  }
  
  values.clear();
  values.reserve(count); // 预留空间 (reserve space)  // 将读取的保持寄存器数据存储到 QVector 中 (store read holding register data into QVector))
  // 防止缓冲区溢出 - 使用较小的值作为循环边界 (prevent buffer overflow - use smaller value as loop boundary)
  int safeBound = qMin(result, count);
  for (int i = 0; i < safeBound; ++i)
  {
    values.append(buffer[i]);
  }
  emit infoLog(tr("成功读取保持寄存器数据"));
  // 发送数据接收信号 (emit data received signal)
  emit dataReceived(DataType::HoldingRegisters, address, QVariant::fromValue(values));
  return true;
}

bool ModbusManager::readInputRegisters(int address, int count, QVector<quint16>& values)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  QVector<uint16_t> buffer(count); // 创建缓冲区 (create buffer)
  // 读取输入寄存器 (read input registers)
  int result = modbus_read_input_registers(m_modbusCtx, address, count, buffer.data());
  if (result == -1)
  {
    setLastError(tr("读取输入寄存器失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  values.clear();
  values.reserve(count); // 预留空间 (reserve space)  // 将读取的输入寄存器数据转换为有符号整数 (convert read input register data to signed integers)
  // 防止缓冲区溢出 - 使用较小的值作为循环边界 (prevent buffer overflow - use smaller value as loop boundary)
  int safeBound = qMin(result, count);
  for (int i = 0; i < safeBound; ++i)
  {
    values.append(buffer[i]);
  }
  emit infoLog(tr("成功读取输入寄存器数据"));
  // 发送数据接收信号 (emit data received signal)
  emit dataReceived(InputRegisters, address, QVariant::fromValue(values));
  return true;
}

bool ModbusManager::writeSingleCoil(int address, bool value)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  // 写入单个线圈 (write single coil)
  int result = modbus_write_bit(m_modbusCtx, address, value ? TRUE : FALSE);
  if (result == -1)
  {
    setLastError(tr("写入单个线圈失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  emit infoLog(tr("成功写入单个线圈"));
  return true;
}

bool ModbusManager::writeSingleRegister(int address, quint16 value)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  // 写入单个寄存器 (write single register)
  int result = modbus_write_register(m_modbusCtx, address, value);
  if (result == -1)
  {
    setLastError(tr("写入单个寄存器失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  emit infoLog(tr("成功写入单个寄存器"));
  return true;
}

bool ModbusManager::writeMultipleCoils(int address, const QVector<bool>& values)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  QVector<uint8_t> buffer(values.size());
  for (int i = 0; i < values.size(); ++i)
  {
    buffer[i] = values[i] ? TRUE : FALSE; // 将布尔值转换为线圈值 (convert boolean to coil value)
  }
  // 写入多个线圈 (write multiple coils)
  int result = modbus_write_bits(m_modbusCtx, address, values.size(), buffer.data());
  if (result == -1)
  {
    setLastError(tr("写入多个线圈失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  emit infoLog(tr("成功写入多个线圈"));
  return true;
}

bool ModbusManager::writeMultipleRegisters(int address, const QVector<quint16>& values)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  QVector<uint16_t> buffer(values.size());
  for (int i = 0; i < values.size(); ++i)
  {
    buffer[i] = values[i]; // 将 QVector 中的值复制到缓冲区 (copy values from QVector to buffer)
  }
  // 写入多个寄存器 (write multiple registers)
  int result = modbus_write_registers(m_modbusCtx, address, values.size(), buffer.data());
  if (result == -1)
  {
    setLastError(tr("写入多个寄存器失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  emit infoLog(tr("成功写入多个寄存器"));
  return true;
}

bool ModbusManager::readWriteRegisters(int readAddress, int readCount, QVector<quint16>& readValues,
                                       int writeAddress, const QVector<quint16>& writeValues)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  QVector<uint16_t> readbuffer(readCount);
  QVector<uint16_t> writebuffer(writeValues.size());
  // 将写入值复制到缓冲区 (copy write values to buffer)
  int result = modbus_write_and_read_registers(m_modbusCtx,
                                               writeAddress, writeValues.size(), writebuffer.data(),
                                               readAddress, readCount, readbuffer.data());
  if (result == -1)
  {
    setLastError(tr("读写寄存器失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  readValues.clear();
  readValues.reserve(readCount); // 预留空间 (reserve space)  // 将读取的寄存器数据存储到 QVector 中 (store read register data into QVector)
  // 防止缓冲区溢出 - 使用较小的值作为循环边界 (prevent buffer overflow - use smaller value as loop boundary)
  int safeBound = qMin(result, readCount);
  for (int i = 0; i < safeBound; ++i)
  {
    readValues.append(readbuffer[i]);
  }
  return true;
}

// 掩码写入寄存器 (mask write register)
bool ModbusManager::maskWriteRegister(int address, quint16 andMask, quint16 orMask)
{
  QMutexLocker locker(&m_mutex);
  if (!checkConnection())
  {
    return false; // 检查连接 (check connection)
  }
  // 掩码写入寄存器 (mask write register)
  int result = modbus_mask_write_register(m_modbusCtx, address, andMask, orMask);
  if (result == -1)
  {
    setLastError(tr("掩码写入寄存器失败: %1").arg(modbus_strerror(errno)));
    return false;
  }
  emit infoLog(tr("成功掩码写入寄存器"));
  return true;
}

QString ModbusManager::getLastError() const
{
  return m_lastError; // 返回最后一次错误信息 (return last error message)
}

int ModbusManager::getLastErrorCode() const
{
  return m_lastErrorCode; // 返回最后一次错误代码 (return last error code)
}

QString ModbusManager::getConnectionInfo() const
{
  return m_connectionInfo;
}

void ModbusManager::setResponseTimeout(int timeoutMsec)
{
  m_responseTimeout = timeoutMsec;
  if (m_modbusCtx)
  {
    modbus_set_response_timeout(m_modbusCtx, m_responseTimeout / 1000, (m_responseTimeout % 1000) * 1000);
  }
}

void ModbusManager::setLastError(const QString& error)
{
  m_lastError = error; // 设置最后一次错误信息 (set last error message)
  m_lastErrorCode = errno; // 设置最后一次错误代码 (set last error code)
  emit errorOccurred(m_lastError); // 发送错误发生信号 (emit error occurred signal)
}

bool ModbusManager::checkConnection()
{
  if (!m_modbusCtx || !m_connected)
  {
    setLastError(tr("Modbus未连接初始化或已断开"));
    emit errorOccurred(m_lastError);
    return false; // 连接未初始化或已断开 (connection not initialized or disconnected)
  }
  return true; // 连接有效 (connection is valid)
}

void ModbusManager::handleConnectionTimeout()
{
  setLastError(tr("Modbus 连接超时"));
  emit errorOccurred(m_lastError);
}

void ModbusManager::setupContext()
{
  if (!m_modbusCtx)
    return;
  // 设置从机地址 (set slave ID)
  modbus_set_slave(m_modbusCtx, m_slaveID);
  // 设置响应超时 (set response timeout)
  modbus_set_response_timeout(m_modbusCtx, m_responseTimeout / 1000, (m_responseTimeout % 1000) * 1000);
  // 设置字节超时 (set byte timeout)
  modbus_set_byte_timeout(m_modbusCtx, m_byteTimeout / 1000, (m_byteTimeout % 1000) * 1000);
  // 设置调试模式 (set debug mode)
  modbus_set_debug(m_modbusCtx, m_debugMode ? 1 : 0);
  // 设置错误信息 (set error message)
  modbus_set_error_recovery(m_modbusCtx,
                            static_cast<modbus_error_recovery_mode>(MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL));
}

void ModbusManager::cleanupContext()
{
  if (m_modbusCtx)
  {
    modbus_free(m_modbusCtx); // 释放上下文 (free context)
    m_modbusCtx = nullptr; // 清空上下文指针 (clear context pointer)
  }
}

bool ModbusManager::executWithRetry(std::function<int()> operation, const QString& operationName)
{
  for (int attempt = 0; attempt < m_retryCount; ++attempt)
  {
    int result = operation();
    if (result != -1)
    {
      if (attempt > 0)
      {
        emit infoLog(tr("操作成功: %1 重试次数 %2").arg(operationName).arg(attempt));
      }
      return true;
    }
    setLastError(tr("%1失败: %2").arg(operationName).arg(modbus_strerror(errno)));
    if (attempt < m_retryCount - 1)
    {
      emit infoLog(
          tr("操作失败,正在重试: %1 尝试: %2 / %3").arg(operationName).arg(attempt + 1).arg(m_retryCount));
      QThread::msleep(100); // 等待一段时间后重试 (wait for a while before retrying)
    }
  }
  emit infoLog(tr("操作失败: %1").arg(operationName));
  return false;
}

// ================= 串口诊断功能实现 (Serial Diagnostic Functions Implementation) =================

QStringList ModbusManager::getAvailablePorts()
{
  QStringList portList;
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

  for (const QSerialPortInfo& port : ports)
  {
    portList << port.portName();
  }

  return portList;
}

ModbusManager::PortInfo ModbusManager::getPortInfo(const QString& portName)
{
  PortInfo info;
  info.portName = portName;
  info.exists = false;
  info.accessible = false;
  info.inUse = false;

  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

  for (const QSerialPortInfo& portInfo : ports)
  {
    if (portInfo.portName() == portName)
    {
      info.exists = true;
      info.description = portInfo.description();
      info.manufacturer = portInfo.manufacturer();
      info.serialNumber = portInfo.serialNumber();
      info.vendorId = portInfo.vendorIdentifier();
      info.productId = portInfo.productIdentifier();
      info.systemLocation = portInfo.systemLocation();
      break;
    }
  }

  if (info.exists)
  {
    // 测试串口是否可以打开
    QSerialPort testPort;
    testPort.setPortName(portName);

    if (testPort.open(QIODevice::ReadWrite))
    {
      info.accessible = true;
      testPort.close();
    }
    else
    {
      info.accessible = false;
      info.errorString = testPort.errorString();
      info.errorCode = static_cast<int>(testPort.error());

      // 判断是否被占用
      if (testPort.error() == QSerialPort::PermissionError)
      {
        info.inUse = true;
      }
    }
  }

  return info;
}

bool ModbusManager::testPortConnection(const QString& portName, int baudRate, int dataBits, char parity, int stopBits)
{
  QSerialPort testPort;
  testPort.setPortName(portName);
  testPort.setBaudRate(baudRate);
  testPort.setDataBits(static_cast<QSerialPort::DataBits>(dataBits));

  // 设置校验位
  switch (parity)
  {
  case 'N':
    testPort.setParity(QSerialPort::NoParity);
    break;
  case 'O':
    testPort.setParity(QSerialPort::OddParity);
    break;
  case 'E':
    testPort.setParity(QSerialPort::EvenParity);
    break;
  case 'M':
    testPort.setParity(QSerialPort::MarkParity);
    break;
  case 'S':
    testPort.setParity(QSerialPort::SpaceParity);
    break;
  default:
    testPort.setParity(QSerialPort::NoParity);
    break;
  }

  // 设置停止位
  if (stopBits == 2)
  {
    testPort.setStopBits(QSerialPort::TwoStop);
  }
  else
  {
    testPort.setStopBits(QSerialPort::OneStop);
  }

  testPort.setFlowControl(QSerialPort::NoFlowControl);
  if (!testPort.open(QIODevice::ReadWrite))
  {
    LOG_INFO(tr("串口测试失败: %1 - %2").arg(portName).arg(testPort.errorString()));
    return false;
  }

  // 简单的读写测试
  testPort.clear();
  QThread::msleep(100);

  testPort.close();
  LOG_INFO(tr("串口测试成功: %1").arg(portName));
  return true;
}

QString ModbusManager::generateDiagnosticReport()
{
  QString report;
  report += tr("=== 串口诊断报告 ===\n\n");

  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

  if (ports.isEmpty())
  {
    report += tr("❌ 未检测到任何串口设备\n");
    report += tr("建议:\n");
    report += tr("• 检查设备连接\n");
    report += tr("• 安装USB转串口驱动\n");
    report += tr("• 检查设备管理器\n\n");
    return report;
  }

  report += tr("检测到 %1 个串口设备:\n\n").arg(ports.size());

  for (const QSerialPortInfo& portInfo : ports)
  {
    PortInfo info = getPortInfo(portInfo.portName());

    report += tr("端口: %1\n").arg(info.portName);
    report += tr("描述: %1\n").arg(info.description);
    report += tr("制造商: %1\n").arg(info.manufacturer);
    report += tr("系统位置: %1\n").arg(info.systemLocation);

    if (info.accessible)
    {
      report += tr("状态: ✅ 可用\n");
    }
    else if (info.inUse)
    {
      report += tr("状态: ⚠️  被占用\n");
      report += tr("错误: %1\n").arg(info.errorString);
    }
    else
    {
      report += tr("状态: ❌ 不可用\n");
      report += tr("错误: %1\n").arg(info.errorString);
    }

    // 测试常见波特率
    report += tr("波特率测试:\n");
    QList<int> commonBaudRates = {9600, 19200, 38400, 57600, 115200};

    for (int baudRate : commonBaudRates)
    {
      if (info.accessible && testPortConnection(info.portName, baudRate, 8, 'N', 1))
      {
        report += tr("  %1: ✅\n").arg(baudRate);
      }
      else
      {
        report += tr("  %1: ❌\n").arg(baudRate);
      }
    }

    report += "\n";
  }

  return report;
}

QString ModbusManager::diagnoseSerialPort(const QString& portName)
{
  QString report;
  report += tr("=== Modbus RTU 串口诊断报告 ===\n\n");

  // 1. 检查串口是否存在
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
  bool portExists = false;
  QSerialPortInfo targetPort;

  for (const QSerialPortInfo& port : ports)
  {
    if (port.portName() == portName)
    {
      portExists = true;
      targetPort = port;
      break;
    }
  }

  if (!portExists)
  {
    report += tr("❌ 问题: 串口 %1 不存在\n\n").arg(portName);
    report += tr("可用串口列表:\n");
    if (ports.isEmpty())
    {
      report += tr("  (无可用串口)\n");
    }
    else
    {
      for (const QSerialPortInfo& port : ports)
      {
        report += tr("  • %1 - %2\n").arg(port.portName()).arg(port.description());
      }
    }
    report += tr("\n解决方案:\n");
    report += tr("1. 检查设备是否正确连接\n");
    report += tr("2. 检查USB转串口驱动是否安装\n");
    report += tr("3. 在设备管理器中查看串口状态\n");
    report += tr("4. 尝试重新插拔USB设备\n\n");
    return report;
  }

  report += tr("✅ 串口 %1 存在\n").arg(portName);
  report += tr("   描述: %1\n").arg(targetPort.description());
  report += tr("   制造商: %1\n").arg(targetPort.manufacturer());
  report += tr("   系统位置: %1\n\n").arg(targetPort.systemLocation());

  // 2. 测试串口访问权限
  QSerialPort testPort;
  testPort.setPortName(portName);

  if (!testPort.open(QIODevice::ReadWrite))
  {
    report += tr("❌ 问题: 无法打开串口\n");
    report += tr("   错误: %1\n\n").arg(testPort.errorString());

    if (testPort.error() == QSerialPort::PermissionError)
    {
      report += tr("🔒 权限问题解决方案:\n");
      report += tr("1. 以管理员身份运行程序\n");
      report += tr("2. 检查串口是否被其他程序占用:\n");
      report += tr("   • 串口调试工具 (SecureCRT, Putty, 超级终端)\n");
      report += tr("   • 其他Modbus工具\n");
      report += tr("   • 设备配置软件\n");
      report += tr("3. 重启计算机释放串口资源\n\n");
    }
    else if (testPort.error() == QSerialPort::DeviceNotFoundError)
    {
      report += tr("🔌 设备问题解决方案:\n");
      report += tr("1. 重新插拔USB设备\n");
      report += tr("2. 更换USB端口\n");
      report += tr("3. 检查USB线缆是否损坏\n");
      report += tr("4. 更新或重新安装驱动程序\n\n");
    }
    else
    {
      report += tr("⚠️  其他问题解决方案:\n");
      report += tr("1. 检查设备管理器中的串口状态\n");
    }
    return report;
  }

  testPort.close();
  report += tr("✅ 串口可以正常打开\n\n");

  // 3. 测试常见Modbus参数
  report += tr("📋 Modbus RTU 参数测试:\n");

  QList<QPair<int, QString>> baudRates = {
      {1200, "1200 (很慢)"},
      {2400, "2400 (慢)"},
      {4800, "4800 (较慢)"},
      {9600, "9600 (标准)"},
      {19200, "19200 (快)"},
      {38400, "38400 (较快)"},
      {57600, "57600 (很快)"},
      {115200, "115200 (最快)"}
  };

  for (const auto& baudRate : baudRates)
  {
    bool success = testPortConnection(portName, baudRate.first, 8, 'N', 1);
    report += tr("  %1: %2\n").arg(baudRate.second).arg(success ? "✅" : "❌");
  }

  report += tr("\n🔧 Modbus 连接建议:\n");
  report += tr("1. 最常用参数: 9600,8,N,1\n");
  report += tr("2. 检查设备手册确认正确参数\n");
  report += tr("3. 确认从机ID设置 (通常为1)\n");
  report += tr("4. 检查RS485接线:\n");
  report += tr("   • A+ 连接 A+\n");
  report += tr("   • B- 连接 B-\n");
  report += tr("   • 确保接地良好\n");
  report += tr("5. 检查设备供电状态\n");
  report += tr("6. 如果是RS232，确认RXD/TXD/GND连接正确\n\n");

  return report;
}

QStringList ModbusManager::getRecommendations(const QString& portName)
{
  QStringList recommendations;
  PortInfo info = getPortInfo(portName);

  if (!info.exists)
  {
    recommendations << tr("串口不存在，请检查设备连接");
    recommendations << tr("确认USB转串口驱动已正确安装");
    recommendations << tr("在设备管理器中检查串口状态");
    return recommendations;
  }

  if (info.inUse)
  {
    recommendations << tr("串口被其他程序占用，请关闭以下可能的程序:");
    recommendations << tr("• 串口调试工具 (如SecureCRT, Putty, 超级终端)");
    recommendations << tr("• 其他Modbus工具");
    recommendations << tr("• 设备配置软件");
    recommendations << tr("• 重启计算机可能有助于释放串口");
    return recommendations;
  }

  if (!info.accessible)
  {
    recommendations << tr("串口无法访问，可能的解决方案:");
    recommendations << tr("• 以管理员身份运行程序");
    recommendations << tr("• 检查串口权限设置");
    recommendations << tr("• 重新插拔USB设备");
    recommendations << tr("• 更新或重新安装串口驱动");
    return recommendations;
  }

  // 如果串口可用，提供Modbus连接建议
  recommendations << tr("串口可用，Modbus连接建议:");
  recommendations << tr("• 尝试标准参数: 9600,8,N,1");
  recommendations << tr("• 检查从机ID设置 (通常为1)");
  recommendations << tr("• 确认RS485接线正确 (A+/B-)");
  recommendations << tr("• 检查设备供电状态");

  return recommendations;
}

QStringList ModbusManager::getQuickFixes()
{
  QStringList fixes;

  fixes << tr("🚀 快速解决方案:");
  fixes << tr("");
  fixes << tr("1. 检查串口占用:");
  fixes << tr("   • 关闭所有串口调试工具");
  fixes << tr("   • 关闭其他Modbus软件");
  fixes << tr("   • 重启计算机");
  fixes << tr("");
  fixes << tr("2. 权限问题:");
  fixes << tr("   • 右键程序 → 以管理员身份运行");
  fixes << tr("");
  fixes << tr("3. 驱动问题:");
  fixes << tr("   • 设备管理器 → 端口 → 更新驱动");
  fixes << tr("   • 重新插拔USB设备");
  fixes << tr("");
  fixes << tr("4. 参数设置:");
  fixes << tr("   • 尝试 9600,8,N,1 (最常用)");
  fixes << tr("   • 从机ID设为1");
  fixes << tr("   • 检查设备手册");
  fixes << tr("");
  fixes << tr("5. 硬件检查:");
  fixes << tr("   • RS485: 确认A+/B-接线");
  fixes << tr("   • RS232: 确认RXD/TXD/GND");
  fixes << tr("   • 检查设备供电");

  return fixes;
}

QString ModbusManager::generateSystemInfo()
{
  QString info;
  info += tr("=== 系统信息 ===\n");

  // 获取可用串口
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
  info += tr("检测到 %1 个串口设备:\n").arg(ports.size());

  for (const QSerialPortInfo& port : ports)
  {
    info += tr("\n端口: %1\n").arg(port.portName());
    info += tr("  描述: %1\n").arg(port.description());
    info += tr("  制造商: %1\n").arg(port.manufacturer());
    info += tr("  序列号: %1\n").arg(port.serialNumber());
    info += tr("  系统位置: %1\n").arg(port.systemLocation());
    info += tr("  VID: 0x%1\n").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
    info += tr("  PID: 0x%1\n").arg(port.productIdentifier(), 4, 16, QChar('0'));

    // 测试可访问性
    QSerialPort testPort;
    testPort.setPortName(port.portName());
    if (testPort.open(QIODevice::ReadWrite))
    {
      info += tr("  状态: ✅ 可用\n");
      testPort.close();
    }
    else
    {
      info += tr("  状态: ❌ 不可用 (%1)\n").arg(testPort.errorString());
    }
  }

  return info;
}

// ========== 新增的诊断方法实现 ==========

bool ModbusManager::isPortAvailable(const QString& portName)
{
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo& port : ports)
  {
    if (port.portName() == portName)
    {
      // 尝试打开端口检查可用性
      QSerialPort testPort;
      testPort.setPortName(portName);
      if (testPort.open(QIODevice::ReadWrite))
      {
        testPort.close();
        return true;
      }
    }
  }
  return false;
}

bool ModbusManager::isPortBusy(const QString& portName)
{
  QSerialPort testPort;
  testPort.setPortName(portName);
  // 尝试打开端口，如果失败可能表示端口忙碌
  if (!testPort.open(QIODevice::ReadWrite))
  {
    // 检查是否是因为端口被占用
    return testPort.error() == QSerialPort::PermissionError ||
        testPort.error() == QSerialPort::ResourceError;
  }
  testPort.close();
  return false;
}

bool ModbusManager::testPortConnectivity(const QString& portName)
{
  QSerialPort testPort;
  testPort.setPortName(portName);
  testPort.setBaudRate(9600);
  testPort.setDataBits(QSerialPort::Data8);
  testPort.setParity(QSerialPort::NoParity);
  testPort.setStopBits(QSerialPort::OneStop);
  testPort.setFlowControl(QSerialPort::NoFlowControl);

  if (testPort.open(QIODevice::ReadWrite))
  {
    // 简单的连通性测试
    testPort.write("test");
    testPort.waitForBytesWritten(1000);
    testPort.close();
    return true;
  }
  return false;
}

QVariantMap ModbusManager::getPortDiagnostics(const QString& portName)
{
  QVariantMap diagnostics;

  // 基本信息
  diagnostics["portName"] = portName;
  diagnostics["exists"] = false;
  diagnostics["available"] = false;
  diagnostics["busy"] = false;
  diagnostics["error"] = "";

  // 查找端口信息
  QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo& port : ports)
  {
    if (port.portName() == portName)
    {
      diagnostics["exists"] = true;
      diagnostics["description"] = port.description();
      diagnostics["manufacturer"] = port.manufacturer();
      diagnostics["serialNumber"] = port.serialNumber();
      diagnostics["vendorId"] = port.vendorIdentifier();
      diagnostics["productId"] = port.productIdentifier();

      // 测试可用性
      QSerialPort testPort;
      testPort.setPortName(portName);
      if (testPort.open(QIODevice::ReadWrite))
      {
        diagnostics["available"] = true;
        testPort.close();
      }
      else
      {
        diagnostics["error"] = testPort.errorString();
        diagnostics["busy"] = (testPort.error() == QSerialPort::PermissionError ||
          testPort.error() == QSerialPort::ResourceError);
      }
      break;
    }
  }

  return diagnostics;
}
