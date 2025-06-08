/**
 * @file serialdialog.h
 * @brief SerialDialog类的头文件
 * @author jinxi
 * @date 2025-05-17
 */

#ifndef SERIALDIALOG_H
#define SERIALDIALOG_H

#include <QEvent>
#include <QLabel>
#include <QMap>
#include <QRandomGenerator>
#include <QSerialPort>
#include <QTimer>
#include <QWidget>

#include "../thirdparty/libmodbus/inc/modbus/modbusmanager.h" // 引入Modbus管理器头文件
#include "config/SettingManager.h"

class SimpleCategoryLogger;
QT_BEGIN_NAMESPACE

namespace Ui
{
  class SerialDialog;
}

QT_END_NAMESPACE

// LED状态枚举
enum class LEDState
{
  Gray, // 灰色 - 未连接
  Red, // 红色 - 已连接未触发
  Green // 绿色 - 已连接且触发
};

enum class LOGType
{
  INFO, // 信息
  WARNING, // 警告
  ERR // 错误
};

struct serialParameters
{
  QString portName; // 串口名称
  int baudRate; // 波特率
  int dataBits; // 数据位
  int parity; // 校验位
  double stopBits; // 停止位
  int flowControl; // 流控方式
};

struct modbusParameters
{
  int slaveID; // 从站地址
  int timeout; // 超时时间
  ModbusManager::DataType dataType; // 数据类型
};

class SerialDialog : public QWidget
{
  Q_OBJECT

public:
  explicit SerialDialog(QWidget* parent = nullptr);
  ~SerialDialog() override;


  QString configFilePath;

private slots:
  // 保存配置按钮响应
  void onSaveConfig();
  // 打开串口按钮响应
  void onOpenSerialPort();
  // 关闭串口按钮响应
  void onCloseSerialPort();
  // 串口数据接收
  void onReadData();
  // 模拟数据更新（仅用于演示）
  void onSimulateData();
  // 刷新端口按钮响应
  void onRefreshPorts();
  // 读取寄存器按钮响应
  void onReadRegister();
  // 写入寄存器按钮响应
  void onWriteRegister();
  // 清空日志按钮响应
  void onClearLog();
  // 发送数据按钮响应
  void onSendData();
  // 重置计数按钮响应
  void onResetCount();

private:
  // 初始化串口配置下拉框
  void initSerialPortConfig();
  // 初始化连接
  void initConnection();
  // 初始化modbus
  void initModbusManager();
  // 初始化日志
  void initLog();
  // 应用日志
  void appLog(const QString &message, LOGType type = LOGType::INFO);
  // 初始化配置
  void initConfiguration();
  /* =============================初始化LED指示灯相关函数============================= */
  // 初始化LED指示灯
  void initLEDs();
  // 为LED创建标签
  void createLEDLabels();
  // 更新LED状态
  void updateLEDState(const QString& ledName, LEDState state);
  // 更新所有IO点状态
  void updateAllLEDs(bool connected);  
  /* =============================加载函数============================= */
  // 加载配置文件
  void loadConfig();
  // 分析 Modbus 报文
  QString analyzeModbusFrame(const QByteArray& data);
  quint16 calculateCRC16(const quint8* data, int length);
  bool validateAndFixCRC(QByteArray& data);

  Ui::SerialDialog* ui;
  QSerialPort* serialPort; // 串口对象
  QTimer* simulationTimer; // 用于模拟数据的定时器
  QMap<QString, QLabel*> leds; // 保存所有LED指示灯的引用
  serialParameters serialParameters;  // 串口参数
  modbusParameters modbusParameters; // Modbus参数

  ModbusManager* m_modbusManager; // Modbus管理器对象
  SettingManager* settingManager; // 配置管理器对象
};

#endif // SERIALDIALOG_H
