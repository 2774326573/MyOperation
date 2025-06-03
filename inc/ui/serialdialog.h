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

QT_BEGIN_NAMESPACE
namespace Ui {
class SerialDialog;
}
QT_END_NAMESPACE

// LED状态枚举
enum class LEDState {
  Gray, // 灰色 - 未连接
  Red,  // 红色 - 已连接未触发
  Green // 绿色 - 已连接且触发
};

class SerialDialog : public QWidget {
  Q_OBJECT

public:
  explicit SerialDialog(QWidget *parent = nullptr);
  ~SerialDialog() override;

#ifdef ENABLE_MULTILANGUAGE
  /// 更新UI文本翻译
  void retranslateUi();
#endif

protected:
  // 重写changeEvent处理语言改变事件
  void changeEvent(QEvent *event) override;

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
  // 初始化LED指示灯
  void initLEDs();
  // 初始化连接
  void initConnection();
  // 为LED创建标签
  void createLEDLabels();
  // 更新LED状态
  void updateLEDState(const QString &ledName, LEDState state);
  // 更新所有IO点状态
  void updateAllLEDs(bool connected);


  Ui::SerialDialog *ui;
  QSerialPort *serialPort;      // 串口对象
  QTimer *simulationTimer;      // 用于模拟数据的定时器
  QMap<QString, QLabel *> leds; // 保存所有LED指示灯的引用

  ModbusManager *m_modbusManager; // Modbus管理器对象
};

#endif // SERIALDIALOG_H
