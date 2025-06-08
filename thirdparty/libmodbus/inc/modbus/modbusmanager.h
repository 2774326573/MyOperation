#pragma once // 预定义：防止头文件被多次包含

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QDir>

#include "modbus.h"

class ModbusManager : public QObject
{
  Q_OBJECT

public:
  enum class ConnectionType
  {
    RTU,
    TCP,
  };

  enum DataType
  {
    Coils, /// 线圈
    DiscreteInputs, /// 离散输入
    HoldingRegisters, /// 保持寄存器
    InputRegisters, /// 输入寄存器
  };

  /**
   * @brief 串口信息结构体 (Port Information Structure)
   */
  struct PortInfo
  {
    QString portName; ///< 端口名称 (Port name)
    QString description; ///< 端口描述 (Port description)
    QString manufacturer; ///< 制造商 (Manufacturer)
    QString serialNumber; ///< 序列号 (Serial number)
    QString systemLocation; ///< 系统位置 (System location)
    quint16 vendorId; ///< 厂商ID (Vendor ID)
    quint16 productId; ///< 产品ID (Product ID)
    bool exists; ///< 端口是否存在 (Port exists)
    bool accessible; ///< 端口是否可访问 (Port accessible)
    bool inUse; ///< 端口是否被占用 (Port in use)
    bool isAvailable; ///< 端口是否可用 (Port available)
    bool isBusy; ///< 端口是否忙碌 (Port busy)
    QString errorString; ///< 错误信息 (Error message)
    int errorCode; ///< 错误代码 (Error code)
  };

public:
  explicit ModbusManager(QObject* parent = nullptr);
  ~ModbusManager();

  /**
   * @brief 连接RTU
   * @param port 串口号
   * @param baudRate 波特率
   * @param dataBits 数据位
   * @param parity 校验位
   * @param stopBits 停止位
   */
  bool connectRTU(const QString& port, int baudRate = 9600, int dataBits = 8, char parity = 'N', int stopBits = 1);
  /**
   * @brief 连接RTU (重载版本，接受整数校验位)
   * @param port 串口号
   * @param baudRate 波特率
   * @param dataBits 数据位
   * @param parity 校验位 (整数：0=无, 1=奇校验, 2=偶校验, 3=空校验, 4=标志校验)
   * @param stopBits 停止位
   */
  bool connectRTU(const QString& port, int baudRate, int dataBits, int parity, int stopBits);
  /**
   * @brief 连接TCP
   * @param ip IP地址
   * @param port 端口号
   */
  bool connectTCP(const QString& ip, int port = 502);
  ///  断开连接
  void disconnect();
  ///  是否连接
  bool isConnected() const;
  ///  设置从机地址
  void setSlaveID(int id);
  ///  获取从机地址
  int getSlaveID() const;
  ///  设置超时时间
  void setTimeout(int timeoutMsec);
  ///  设置调试模式
  void setDebugMode(bool enable);
  /// 读取线圈
  bool readCoils(int address, int count, QVector<bool>& values);
  /// 读取离散输入
  bool readDiscreteInputs(int address, int count, QVector<bool>& values);
  /// 读取保持寄存器
  bool readHoldingRegisters(int address, int count, QVector<quint16>& values);
  /// 读取输入寄存器
  bool readInputRegisters(int address, int count, QVector<quint16>& values);
  /// 写入单个线圈
  bool writeSingleCoil(int address, bool value);
  /// 写入单个寄存器
  bool writeSingleRegister(int address, quint16 value);
  /// 写入多个线圈
  bool writeMultipleCoils(int address, const QVector<bool>& values);
  /// 写入多个寄存器
  bool writeMultipleRegisters(int address, const QVector<quint16>& values);

  /**
   * @brief 读写寄存器
   * @param readAddress 读取地址
   * @param readCount 读取数量
   * @param writeAddress 写入地址
   * @param writeValues
   */
  bool readWriteRegisters(int readAddress, int readCount, QVector<quint16>& readValues, int writeAddress,
                          const QVector<quint16>& writeValues);
  /**
   * @brief 掩码写入寄存器
   * @param address 地址
   * @param andMask 与掩码
   * @param orMask 或掩码
   */
  bool maskWriteRegister(int address, quint16 andMask, quint16 orMask);
  /// 获取最后一次错误信息
  QString getLastError() const;
  /// 获取最后一次错误代码
  int getLastErrorCode() const;
  /// 获取连接信息
  QString getConnectionInfo() const;
  /// 设置响应超时时间
  void setResponseTimeout(int timeoutMsec);

  // ========== 串口诊断功能 (Serial Diagnostic Functions) ==========
  /**
   * @brief 获取可用串口列表 (Get available ports list)
   * @return 串口名称列表 (List of port names)
   */
  static QStringList getAvailablePorts();

  /**
   * @brief 获取指定串口的详细信息 (Get detailed information of specified port)
   * @param portName 串口名称 (Port name)
   * @return 串口信息 (Port information)
   */
  static PortInfo getPortInfo(const QString& portName);

  /**
   * @brief 测试串口连接 (Test port connection)
   * @param portName 串口名称 (Port name)
   * @param baudRate 波特率 (Baud rate)
   * @param dataBits 数据位 (Data bits)
   * @param parity 校验位 (Parity)
   * @param stopBits 停止位 (Stop bits)
   * @return 测试是否成功 (Test success)
   */
  static bool testPortConnection(const QString& portName, int baudRate, int dataBits, char parity, int stopBits);

  /**
   * @brief 生成串口诊断报告 (Generate serial diagnostic report)
   * @return 诊断报告文本 (Diagnostic report text)
   */
  static QString generateDiagnosticReport();

  /**
   * @brief 诊断指定串口的问题 (Diagnose issues with specified serial port)
   * @param portName 串口名称 (Port name)
   * @return 诊断报告 (Diagnostic report)
   */
  static QString diagnoseSerialPort(const QString& portName);

  /**
   * @brief 获取针对特定串口的建议 (Get recommendations for specific port)
   * @param portName 串口名称 (Port name)
   * @return 建议列表 (List of recommendations)
   */
  static QStringList getRecommendations(const QString& portName);

  /**
   * @brief 获取快速解决方案 (Get quick fixes)
   * @return 解决方案列表 (List of solutions)
   */
  static QStringList getQuickFixes();
  /**
   * @brief 生成系统信息报告 (Generate system information report)
   * @return 系统信息 (System information)
   */
  static QString generateSystemInfo();

  /**
   * @brief 检查端口是否可用 (Check if port is available)
   * @param portName 串口名称 (Port name)
   * @return 是否可用 (Is available)
   */
  static bool isPortAvailable(const QString& portName);

  /**
   * @brief 检查端口是否忙碌 (Check if port is busy)
   * @param portName 串口名称 (Port name)
   * @return 是否忙碌 (Is busy)
   */
  bool isPortBusy(const QString& portName);

  /**
   * @brief 测试端口连通性 (Test port connectivity)
   * @param portName 串口名称 (Port name)
   * @return 测试结果 (Test result)
   */
  bool testPortConnectivity(const QString& portName);

  /**
   * @brief 获取端口诊断信息 (Get port diagnostics)
   * @param portName 串口名称 (Port name)
   * @return 诊断信息 (Diagnostic information)
   */
  QVariantMap getPortDiagnostics(const QString& portName);

signals:
  /// 连接成功
  void connected();
  /// 断开连接
  void disconnected();
  /// 错误发生
  void errorOccurred(const QString& error);
  /// 数据接收
  void dataReceived(DataType type, int address, const QVariant& data);
  /// 数据发送
  void dataSent(DataType type, int address, const QVariant& data);
  /// 信息日志
  void infoLog(const QString& message);

private slots:
  void handleConnectionTimeout(); /// 处理连接超时

private:
  void setupContext(); /// 设置 Modbus 上下文
  void cleanupContext(); /// 清理 Modbus 上下文
  bool executWithRetry(std::function<int()> operation, const QString& operationName); // 执行操作并重试
  void setLastError(const QString& error); /// 设置最后一次错误信息
  bool checkConnection(); /// 检查连接
  char convertParityToChar(int parity); /// 将整数校验位转换为字符校验位

private:
  /// 连接上下文
  modbus_t* m_modbusCtx;
  /// 连接类型
  ConnectionType m_connectionType;
  // 调试模式
  bool m_debugMode;
  // 是否连接
  bool m_connected;
  // 从机地址
  int m_slaveID;
  // 最后一次错误信息
  QString m_lastError;
  // 连接信息
  QString m_connectionInfo;
  // 最后一次错误代码
  int m_lastErrorCode;
  // 连接定时器
  QTimer* m_connectionTimer;
  // 互斥锁
  QMutex m_mutex;
  // 字节超时时间（毫秒）
  int m_byteTimeout;
  // 响应超时时间（毫秒）
  int m_responseTimeout;
  // 超时时间（毫秒）
  int m_timeoutMsec;
  // 重试次数
  int m_retryCount;
  // 读取队列
  QQueue<QPair<int, QVector<quint16>>> m_readQueue;
  // 写入队列
  QQueue<QPair<int, QVector<quint16>>> m_writeQueue;

  // 链接参数
  // RTU 串口号
  QString m_rtuPort;
  // RTU 波特率
  int m_rtuBaudRate;
  // RTU 数据位
  int m_rtuDataBits;
  // RTU 校验位
  char m_rtuParity;
  // RTU 停止位
  int m_rtuStopBits;
  // TCP IP地址
  QString m_tcpIp;
  // TCP 端口号
  int m_tcpPort;
};
