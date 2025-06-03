#include "../../inc/modbus/modbusManager.h"
#include "../../inc/modbus/serial_diagnostic.h"
#include <QSerialPortInfo>
#include <QSerialPort>

#define LOG_INFO(msg) qDebug() << "[INFO]" << msg
#define LOG_ERROR(msg) qDebug() << "[ERROR]" << msg
#define LOG_WARN(msg) qDebug() << "[WARN]" << msg


ModbusManager::ModbusManager(QObject* parent): QObject(parent)
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
    SerialDiagnostic::PortInfo portInfo = SerialDiagnostic::getPortInfo(port);
    if (!portInfo.exists) {
        setLastError(tr("串口 %1 不存在").arg(port));
        LOG_ERROR(tr("串口 %1 不存在，请检查设备连接").arg(port));
        
        // 显示可用串口
        QStringList availablePorts = SerialDiagnostic::getAvailablePorts();
        if (!availablePorts.isEmpty()) {
            LOG_INFO(tr("可用串口: %1").arg(availablePorts.join(", ")));
        } else {
            LOG_WARN(tr("系统中未检测到任何串口设备"));
        }
        return false;
    }
    
    // 检查串口是否可访问
    if (!portInfo.accessible) {
        if (portInfo.inUse) {
            setLastError(tr("串口 %1 被其他程序占用: %2").arg(port).arg(portInfo.errorString));
            LOG_ERROR(tr("串口被占用，请关闭其他串口程序"));
        } else {
            setLastError(tr("串口 %1 无法访问: %2").arg(port).arg(portInfo.errorString));
            LOG_ERROR(tr("串口无法访问，可能需要管理员权限"));
        }
        
        // 显示解决建议
        QStringList recommendations = SerialDiagnostic::getRecommendations(port);
        for (const QString &rec : recommendations) {
            LOG_INFO(rec);
        }
        return false;
    }
    
    LOG_INFO(tr("串口 %1 可用性检查通过").arg(port));
    LOG_INFO(tr("串口描述: %1").arg(portInfo.description));
    LOG_INFO(tr("制造商: %1").arg(portInfo.manufacturer));
    
    // 使用Qt串口进行预连接测试
    if (!SerialDiagnostic::testPortConnection(port, baudRate, dataBits, parity, stopBits)) {
        setLastError(tr("串口参数测试失败，请检查波特率和其他参数"));
        LOG_ERROR(tr("串口参数测试失败"));
        return false;
    }
    
    // 创建新的Modbus上下文 (create a new Modbus context)
    m_connectionType = ConnectionType::RTU; // 设置连接类型为RTU (set connection type to RTU)
    m_rtuPort        = port;                // 串口号          (serial port)
    m_rtuBaudRate    = baudRate;            // 波特率          (baud rate)
    m_rtuDataBits    = dataBits;            // 数据位          (data bits)
    m_rtuParity      = parity;              // 校验位          (parity)
    m_rtuStopBits    = stopBits;            // 停止位          (stop bits)
    
    // 记录连接参数
    LOG_INFO(tr("RTU连接参数 - 端口: %1, 波特率: %2, 数据位: %3, 校验: %4, 停止位: %5")
             .arg(m_rtuPort).arg(m_rtuBaudRate).arg(m_rtuDataBits).arg(m_rtuParity).arg(m_rtuStopBits));
    
    // 创建libmodbus上下文 - Windows需要特殊的串口名称格式
    QString windowsPortName = m_rtuPort;
    if (windowsPortName.startsWith("COM") && !windowsPortName.startsWith("\\\\.\\")) {
        // Windows上COM端口号大于9时需要特殊格式
        bool ok;
        int portNumber = windowsPortName.mid(3).toInt(&ok);
        if (ok && portNumber > 9) {
            windowsPortName = "\\\\.\\" + m_rtuPort;
        }
    }
    
    LOG_INFO(tr("使用串口名称: %1 (原始: %2)").arg(windowsPortName).arg(m_rtuPort));
    
    m_modbusCtx = modbus_new_rtu(windowsPortName.toStdString().c_str(), m_rtuBaudRate, m_rtuParity, m_rtuDataBits,
                                 m_rtuStopBits);
    // 检查上下文是否创建成功 (check if context is created successfully)
    if (m_modbusCtx == nullptr)
    {
        setLastError(tr("无法创建Modbus RTU 上下文"));
        LOG_ERROR(tr("libmodbus上下文创建失败"));
        return false;
    }

    // 设置超时 (set timeout) - 增加超时时间以适应慢速设备
    modbus_set_response_timeout(m_modbusCtx, 3, 0);     // 3秒超时 (3 second timeout)
    modbus_set_byte_timeout(m_modbusCtx, 0, 500000);    // 设置字节超时 (set byte timeout)
    
    // 启用调试模式以获取更多信息
    modbus_set_debug(m_modbusCtx, 1);
    
    // 设置错误恢复模式
    modbus_set_error_recovery(m_modbusCtx, static_cast<modbus_error_recovery_mode>(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));

    // 设置从机地址 (set slave ID)
    modbus_set_slave(m_modbusCtx, m_slaveID);
    LOG_INFO(tr("设置从机地址: %1").arg(m_slaveID));

    // 尝试连接 (try to connect)
    LOG_INFO(tr("正在尝试连接串口: %1").arg(m_rtuPort));
    if (modbus_connect(m_modbusCtx) == -1)
    {
        QString errorMsg = QString::fromLocal8Bit(modbus_strerror(errno));
        if (errorMsg.isEmpty() || errorMsg == "No error") {
            errorMsg = tr("串口可能被其他程序占用或不存在");
        }
        
        // 提供更详细的错误分析
        QString detailedError = tr("Modbus RTU连接失败: %1 (端口: %2, errno: %3)")
                               .arg(errorMsg).arg(m_rtuPort).arg(errno);
        
        // 根据errno提供具体建议
        if (errno == ENOENT || errno == 2) {
            detailedError += tr("\n建议: 串口设备不存在，请检查设备连接和驱动");
        } else if (errno == EACCES || errno == 13) {
            detailedError += tr("\n建议: 权限不足，请以管理员身份运行或检查串口权限");
        } else if (errno == EBUSY || errno == 16) {
            detailedError += tr("\n建议: 串口被占用，请关闭其他串口程序");
        }
        
        setLastError(detailedError);
        LOG_ERROR(detailedError);
        
        modbus_free(m_modbusCtx); // 释放上下文 (free context)
        m_modbusCtx = nullptr; // 清空上下文指针 (clear context pointer)
        return false;
    }

    m_connected = true;
    
    // 尝试简单的通信测试
    LOG_INFO(tr("串口连接成功，正在测试Modbus通信..."));
    
    // 尝试读取一个保持寄存器来测试通信
    uint16_t testValue;
    int result = modbus_read_registers(m_modbusCtx, 0, 1, &testValue);
    if (result == 1) {
        LOG_INFO(tr("Modbus通信测试成功，读取到寄存器0的值: %1").arg(testValue));
    } else {
        LOG_WARN(tr("Modbus通信测试失败: %1 (这可能是正常的，如果地址0不存在)").arg(modbus_strerror(errno)));
        LOG_INFO(tr("串口连接成功，但建议检查设备地址和从机ID设置"));
    }
    
    emit connected(); // 发送连接成功信号 (emit connected signal)
    LOG_INFO(tr("=== Modbus RTU连接成功: %1 ===").arg(m_rtuPort)); // 输出连接成功信息 (output connection success message)
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
    m_connectionType = ConnectionType::TCP;  // 设置连接类型为TCP (set connection type to TCP)
    m_tcpIp          = ip;                   // TCP IP地址 (TCP IP address)
    m_tcpPort        = port;                 // TCP端口号 (TCP port number)
    m_modbusCtx = modbus_new_tcp(m_tcpIp.toStdString().c_str(), m_tcpPort);
    if (m_modbusCtx == nullptr)
    {
        setLastError(tr("无法创建Modbus TCP 上下文"));
        LOG_ERROR(tr("无法创建Modbus TCP 上下文"));
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
        LOG_ERROR(tr("Modbus TCP链接失败: %1").arg(modbus_strerror(errno))); // 输出错误信息 (output error message)
        modbus_free(m_modbusCtx); // 释放上下文 (free context)
        m_modbusCtx = nullptr; // 清空上下文指针 (clear context pointer)
        return false;
    }
    m_connected = true; // 设置连接状态 (set connected state)
    emit connected(); // 发送连接成功信号 (emit connected signal)

    LOG_INFO("Modbus TCP连接成功:" << m_tcpIp << ":" << m_tcpPort); // 输出连接成功信息 (output connection success message)
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
        LOG_INFO("Modbus连接已断开");
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
    LOG_INFO(tr("设置从机地址为: %1").arg(m_slaveID));
    
    // 如果上下文已经存在，立即更新 (if context exists, update immediately)
    if (m_modbusCtx != nullptr)
    {
        modbus_set_slave(m_modbusCtx, m_slaveID); // 更新Modbus上下文中的从机地址 (update slave ID in Modbus context)
        LOG_INFO(tr("已更新Modbus上下文中的从机地址"));
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
    LOG_INFO(tr("设置响应超时为: %1 毫秒").arg(timeoutMsec));
    
    // 如果上下文已经存在，立即应用设置 (if context exists, apply setting immediately)
    if (m_modbusCtx != nullptr)
    {
        // 将毫秒转换为秒和微秒 (convert milliseconds to seconds and microseconds)
        modbus_set_response_timeout(m_modbusCtx, timeoutMsec / 1000, (timeoutMsec % 1000) * 1000); // 设置响应超时 (set response timeout)
        LOG_INFO(tr("已更新Modbus上下文中的响应超时"));
    }
}

void ModbusManager::setDebugMode(bool enable)
{
    QMutexLocker locker(&m_mutex);
    m_debugMode = enable; // 保存调试模式设置 (save debug mode setting)
    LOG_INFO(tr("设置调试模式为: %1").arg(enable ? tr("启用") : tr("禁用")));
    
    // 如果上下文已经存在，立即应用设置 (if context exists, apply setting immediately)
    if (m_modbusCtx != nullptr)
    {
        modbus_set_debug(m_modbusCtx, enable ? 1 : 0); // 设置调试模式 (set debug mode)
        LOG_INFO(tr("已更新Modbus上下文中的调试模式"));
    }
}

/* ==================== 数据读取 | en:Data reading ====================== */
// 单个线圈/多个线圈用 uint8_t，单个/多个寄存器用 uint16_t，是因为它们在 Modbus 协议中的数据宽度不同。

bool ModbusManager::readCoils(int address, int count, QVector<bool> &values)
{
    // QVariant 在 Qt 中是一个通用的数据类型容器，可以存储多种类型的数据。
    QMutexLocker locker(&m_mutex);
    if (!checkConnection())
    {
        return false; // 检查连接 (check connection)
    }

    QVector<uint8_t> buffer(count);
    int rseult = modbus_read_bits(m_modbusCtx, address, count, buffer.data()); // 读取线圈 (read coils)
    if (rseult == -1)
    {
        setLastError(tr("读取线圈失败: %1").arg(modbus_strerror(errno)));
        LOG_ERROR(tr("读取线圈失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    values.clear();
    values.reserve(count); // 预留空间 (reserve space)
    for (int i = 0; i < rseult; ++i)
    {
        values.append(buffer[i] != 0); // 将读取的线圈数据转换为布尔值 (convert read coil data to boolean)
    }
    LOG_INFO("成功读取线圈数据");
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
        LOG_ERROR(tr("读取离散输入失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    values.clear();
    values.reserve(count); // 预留空间 (reserve space)
    // 将读取的离散输入数据转换为布尔值 (convert read discrete input data to boolean)
    for (int i = 0; i < count; ++i)
    {
        values.append(buffer[i] != 0);
    }
    LOG_INFO("成功读取离散输入数据");
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
    QVector<uint16_t> buffer(count); // 创建缓冲区 (create buffer)
    // 读取保持寄存器 (read holding registers)
    int result = modbus_read_registers(m_modbusCtx, address, count, buffer.data());
    if (result == -1)
    {
        setLastError(tr("读取保持寄存器失败: %1").arg(modbus_strerror(errno)));
        LOG_ERROR(tr("读取保持寄存器失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    values.clear();
    values.reserve(count); // 预留空间 (reserve space)
    // 将读取的保持寄存器数据存储到 QVector 中 (store read holding register data into QVector))
    for (int i = 0; i < result; ++i)
    {
        values.append(buffer[i]);
    }
    LOG_INFO("成功读取保持寄存器数据");
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
        LOG_ERROR(tr("读取输入寄存器失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    values.clear();
    values.reserve(count); // 预留空间 (reserve space)
    // 将读取的输入寄存器数据转换为有符号整数 (convert read input register data to signed integers)
    for (int i = 0; i < result; ++i)
    {
        values.append(buffer[i]);
    }
    LOG_INFO("成功读取输入寄存器数据");
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
        LOG_ERROR(tr("写入单个线圈失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    LOG_INFO("成功写入单个线圈");
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
        LOG_ERROR(tr("写入单个寄存器失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    LOG_INFO("成功写入单个寄存器");
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
        LOG_ERROR(tr("写入多个线圈失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    LOG_INFO("成功写入多个线圈");
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
        LOG_ERROR(tr("写入多个寄存器失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    LOG_INFO("成功写入多个寄存器");
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
        LOG_ERROR(tr("读写寄存器失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    readValues.clear();
    readValues.reserve(readCount); // 预留空间 (reserve space)
    // 将读取的寄存器数据存储到 QVector 中 (store read register data into QVector)
    for (int i = 0; i < result; ++i)
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
        LOG_ERROR(tr("掩码写入寄存器失败: %1").arg(modbus_strerror(errno)));
        return false;
    }
    LOG_INFO("成功掩码写入寄存器");
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

void ModbusManager::setLastError(const QString &error)
{
    m_lastError = error; // 设置最后一次错误信息 (set last error message)
    m_lastErrorCode = errno; // 设置最后一次错误代码 (set last error code)
    emit errorOccurred(m_lastError); // 发送错误发生信号 (emit error occurred signal)
    LOG_ERROR(m_lastError); // 输出错误信息 (output error message)
}

bool ModbusManager::checkConnection()
{
    if (!m_modbusCtx || !m_connected)
    {
        setLastError(tr("Modbus未连接初始化或已断开"));
        LOG_ERROR(tr("Modbus连接未初始化或已断开"));
        emit errorOccurred(m_lastError);
        return false; // 连接未初始化或已断开 (connection not initialized or disconnected)
    }
    return true; // 连接有效 (connection is valid)
}

void ModbusManager::handleConnectionTimeout()
{
    setLastError(tr("Modbus 连接超时"));
    LOG_ERROR(tr("Modbus 连接超时"));
    emit errorOccurred(m_lastError);
}

void ModbusManager::setupContext()
{
    if (!m_modbusCtx) return;
    // 设置从机地址 (set slave ID)
    modbus_set_slave(m_modbusCtx, m_slaveID);
    // 设置响应超时 (set response timeout)
    modbus_set_response_timeout(m_modbusCtx, m_responseTimeout / 1000, (m_responseTimeout % 1000) * 1000);
    // 设置字节超时 (set byte timeout)
    modbus_set_byte_timeout(m_modbusCtx, m_byteTimeout / 1000, (m_byteTimeout % 1000) * 1000);
    // 设置调试模式 (set debug mode)
    modbus_set_debug(m_modbusCtx, m_debugMode ? 1 : 0);
    // 设置错误信息 (set error message)
    modbus_set_error_recovery(m_modbusCtx, static_cast<modbus_error_recovery_mode>(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));
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
                LOG_INFO(tr("操作成功: %1 重试次数 %2").arg(operationName).arg(attempt));
            }
            return true;
        }
        LOG_ERROR(tr("%1失败: %2").arg(operationName).arg(modbus_strerror(errno)));
        setLastError(tr("%1失败: %2").arg(operationName).arg(modbus_strerror(errno)));
        if (attempt < m_retryCount - 1)
        {
            LOG_INFO(
                tr("操作失败,正在重试: %1 尝试: %2 / %3").arg(operationName).arg(attempt + 1).arg(m_retryCount));
            QThread::msleep(100); // 等待一段时间后重试 (wait for a while before retrying)
        }
    }
    LOG_ERROR(tr("操作失败: %1").arg(operationName));
    return false;
}

