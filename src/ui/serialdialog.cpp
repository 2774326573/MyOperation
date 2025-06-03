//
// Created by jinxi on 25-5-20.
//

#include "../inc/ui/serialdialog.h"
#include "ui_SerialDialog.h"
#include "../thirdparty/config/inc/config_manager.h" // 引入Modbus管理器头文件
#include "../thirdparty/log_manager/inc/simplecategorylogger.h" // 引入日志管理器头文件

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

// 日志重定义
// 定义优化版的日志宏，基于编译模式和构建设置自动调整行为
#ifdef _DEBUG // 调试模式
// 调试模式下，日志同时输出到控制台和文件，更详细
#define LOG_DEBUG(message) SIMPLE_DEBUG_LOG_DEBUG("serialModbus", message)
#define LOG_INFO(message) SIMPLE_DEBUG_LOG_INFO("serialModbus", message)
#define LOG_WARNING(message) SIMPLE_DEBUG_LOG_WARNING("serialModbus", message)
#define LOG_ERROR(message) SIMPLE_DEBUG_LOG_ERROR("serialModbus", message)
#define LOG_ASSERT(message) SIMPLE_ASSERT((condition), (message))
// 添加对TRACE日志的支持
#define LOG_TRACE(message) SIMPLE_LOG_TRACE("serialModbus", message)

#else // 发布模式
// 发布模式下，根据CMake配置决定日志行为

// 其他级别的日志使用配置控制的版本
#define LOG_INFO(message) SIMPLE_LOG_INFO_CONFIG("serialModbus", message, false, WRITE_TO_FILE)
#define LOG_WARNING(message) SIMPLE_LOG_WARNING_CONFIG("serialModbus", message, false, WRITE_TO_FILE)
#define LOG_ERROR(message) SIMPLE_LOG_ERROR_CONFIG("serialModbus", message, false, WRITE_TO_FILE)
#define LOG_ASSERT(condition, message, "serialModbus")                                                                       \
  do                                                                                                                   \
  {                                                                                                                    \
    if (!(condition))                                                                                                  \
    {                                                                                                                  \
      LOG_ERROR(category, "断言失败: " + QString(message));                                                            \
    }                                                                                                                  \
  }                                                                                                                    \
  while (0)

#endif

#ifdef ENABLE_MULTILANGUAGE
#include "../../../inc/language/translation_helper.h"
#endif

SerialDialog::SerialDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::SerialDialog), serialPort(nullptr),
      simulationTimer(nullptr) {
  ui->setupUi(this);

 auto *configManager = ConfigManager::getInstance();
  QString configFilePath = QApplication::applicationDirPath() + "/config/communication";
  
  if (!configManager->init(configFilePath+"/serialdialog.ini"))
  {
    LOG_ERROR("配置管理器初始化失败，无法加载串口配置");
    QMessageBox::critical(this, tr("错误"), tr("无法加载串口配置，请检查配置文件是否存在"));
    return;
  }

  // 初始化串口配置
  initSerialPortConfig();

  // 初始化LED指示灯
  initLEDs();

  // 初始化连接
  initConnection();

  // 创建模拟数据定时器（仅用于演示）
  simulationTimer = new QTimer(this);
  connect(simulationTimer, &QTimer::timeout, this,
          &SerialDialog::onSimulateData);

#ifdef ENABLE_MULTILANGUAGE
  // 注册翻译更新回调
  TranslationHelper::registerWindow(this, [this]() { this->retranslateUi(); });
#endif
}

SerialDialog::~SerialDialog() {
  if (simulationTimer && simulationTimer->isActive()) {
    simulationTimer->stop();
  }

  if (serialPort && serialPort->isOpen()) {
    serialPort->close();
  }
  delete simulationTimer;
  delete serialPort;
  delete ui;
}

void SerialDialog::initSerialPortConfig() {
  // 1. 查找可用的串口
  const auto infos = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &info : infos) {
    ui->comboBox_com->addItem(info.portName());
  }

  // 2. 添加常用波特率
  QList<int> baudrates = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
  for (int rate : baudrates) {
    ui->comboBox_baudrate->addItem(QString::number(rate));
  }
  // 默认选择9600
  ui->comboBox_baudrate->setCurrentText("9600");

  // 3. 添加数据位
  ui->comboBox_databits->addItem("5", QSerialPort::Data5);
  ui->comboBox_databits->addItem("6", QSerialPort::Data6);
  ui->comboBox_databits->addItem("7", QSerialPort::Data7);
  ui->comboBox_databits->addItem("8", QSerialPort::Data8);
  // 默认选择8位
  ui->comboBox_databits->setCurrentIndex(3);

  // 4. 添加校验位
  ui->comboBox_parity->addItem(tr("无"), QSerialPort::NoParity);
  ui->comboBox_parity->addItem(tr("奇校验"), QSerialPort::OddParity);
  ui->comboBox_parity->addItem(tr("偶校验"), QSerialPort::EvenParity);
  ui->comboBox_parity->addItem(tr("空校验"), QSerialPort::SpaceParity);
  ui->comboBox_parity->addItem(tr("标志校验"), QSerialPort::MarkParity);

  // 5. 添加停止位
  ui->comboBox_stopbits->addItem("1", QSerialPort::OneStop);
  ui->comboBox_stopbits->addItem("1.5", QSerialPort::OneAndHalfStop);
  ui->comboBox_stopbits->addItem("2", QSerialPort::TwoStop);

  // 初始化串口对象
  serialPort = new QSerialPort(this);

  // 初始时禁用关闭按钮，因为串口尚未打开
  ui->pushButton_close->setEnabled(false);
}

void SerialDialog::onSaveConfig() {
  // 这里可以保存配置到文件或数据库
  QMessageBox::information(this, tr("保存配置"), tr("串口配置已保存"));
}

/* * 初始化LED指示灯
 * 获取所有LED指示灯的引用并存入map中
 * 输入点X00-X07, X10-X17
 * 输出点Y00-Y07, Y10-Y17
 */
void SerialDialog::initLEDs() {
  // 获取所有LED指示灯的引用并存入map中
  // 输入点X00-X07
  leds["X00"] = ui->led_X00;
  leds["X01"] = ui->led_X01;
  leds["X02"] = ui->led_X02;
  leds["X03"] = ui->led_X03;
  leds["X04"] = ui->led_X04;
  leds["X05"] = ui->led_X05;
  leds["X06"] = ui->led_X06;
  leds["X07"] = ui->led_X07;

  // 输入点X10-X17
  leds["X10"] = ui->led_X10;
  leds["X11"] = ui->led_X11;
  leds["X12"] = ui->led_X12;
  leds["X13"] = ui->led_X13;
  leds["X14"] = ui->led_X14;
  leds["X15"] = ui->led_X15;
  leds["X16"] = ui->led_X16;
  leds["X17"] = ui->led_X17;

  // 输出点Y00-Y07
  leds["Y00"] = ui->led_Y00;
  leds["Y01"] = ui->led_Y01;
  leds["Y02"] = ui->led_Y02;
  leds["Y03"] = ui->led_Y03;
  leds["Y04"] = ui->led_Y04;
  leds["Y05"] = ui->led_Y05;
  leds["Y06"] = ui->led_Y06;
  leds["Y07"] = ui->led_Y07;

  // 输出点Y10-Y17
  leds["Y10"] = ui->led_Y10;
  leds["Y11"] = ui->led_Y11;
  leds["Y12"] = ui->led_Y12;
  leds["Y13"] = ui->led_Y13;
  leds["Y14"] = ui->led_Y14;
  leds["Y15"] = ui->led_Y15;
  leds["Y16"] = ui->led_Y16;
  leds["Y17"] = ui->led_Y17;

  // 为每个LED创建标签
  createLEDLabels();

  // 初始化所有LED为灰色（未连接状态）
  updateAllLEDs(false);
}
// 初始化连接信号和槽
void SerialDialog::initConnection()
{
  // 连接信号和槽
  connect(ui->pushButton_saveConfig, &QPushButton::clicked, this,
          &SerialDialog::onSaveConfig);
  connect(ui->pushButton_open, &QPushButton::clicked, this,
          &SerialDialog::onOpenSerialPort);
  connect(ui->pushButton_close, &QPushButton::clicked, this,
          &SerialDialog::onCloseSerialPort);
  connect(ui->pushButton_refresh, &QPushButton::clicked, this,
          &SerialDialog::onRefreshPorts);
  connect(ui->pushButton_readReg, &QPushButton::clicked, this,
          &SerialDialog::onReadRegister);
  connect(ui->pushButton_writeReg, &QPushButton::clicked, this,
          &SerialDialog::onWriteRegister);
  connect(ui->pushButton_clearLog, &QPushButton::clicked, this,
          &SerialDialog::onClearLog);
  connect(ui->pushButton_send, &QPushButton::clicked, this,
          &SerialDialog::onSendData);
  connect(ui->pushButton_resetCount, &QPushButton::clicked, this,
          &SerialDialog::onResetCount);
}

void SerialDialog::updateLEDState(const QString &ledName, LEDState state) {
  if (!leds.contains(ledName)) {
    return;
  }

  QLabel *led = leds[ledName];
  QString stateText;

  switch (state) {
  case LEDState::Gray:
    led->setStyleSheet("background-color: gray; border-radius: 12px;");
    stateText = tr("未连接");
    break;
  case LEDState::Red:
    led->setStyleSheet("background-color: red; border-radius: 12px;");
    stateText = tr("已连接未触发");
    break;
  case LEDState::Green:
    led->setStyleSheet("background-color: green; border-radius: 12px;");
    stateText = tr("已触发");
    break;
  }

  // 在tooltip中显示状态信息
  led->setToolTip(QString("%1 - %2").arg(ledName).arg(stateText));
}
/**
 * 为LED创建标签
 * 获取LED所在的父容器（通过VBoxLayout->GridLayout）
 * 并为每个LED设置样式和提示
 */
void SerialDialog::createLEDLabels() {
  // 获取LED所在的父容器（通过VBoxLayout->GridLayout）
  QVBoxLayout *inputVLayout =
      qobject_cast<QVBoxLayout *>(ui->groupBox_input->layout());
  QVBoxLayout *outputVLayout =
      qobject_cast<QVBoxLayout *>(ui->groupBox_output->layout());

  if (!inputVLayout || !outputVLayout) {
    qWarning() << "无法找到LED所在的VBoxLayout";
    return;
  }

  // 获取GridLayout（VBoxLayout的第一个item）
  QGridLayout *inputLayout = nullptr;
  QGridLayout *outputLayout = nullptr;

  if (inputVLayout->count() > 0) {
    QLayoutItem *item = inputVLayout->itemAt(0);
    if (item) {
      inputLayout = qobject_cast<QGridLayout *>(item->layout());
    }
  }

  if (outputVLayout->count() > 0) {
    QLayoutItem *item = outputVLayout->itemAt(0);
    if (item) {
      outputLayout = qobject_cast<QGridLayout *>(item->layout());
    }
  }

  if (!inputLayout || !outputLayout) {
    qWarning() << "无法找到LED所在的GridLayout";
    return;
  }

  // 为每个LED设置样式和提示
  for (auto it = leds.begin(); it != leds.end(); ++it) {
    QString ledName = it.key();
    QLabel *led = it.value();

    // 设置LED的固定大小和样式
    led->setMinimumSize(30, 30);
    led->setMaximumSize(30, 30);

    // 设置LED为圆形
    led->setStyleSheet("background-color: #95a5a6; border-radius: 15px; "
                       "border: 2px solid #7f8c8d;");

    // 设置悬停提示
    led->setToolTip(QString("%1").arg(ledName));
  }
}

/**
 * 更新全部LED状态 传入的值是假全部为灰色， 如果为真全部为红色
 * @param connected
 */
void SerialDialog::updateAllLEDs(bool connected) {
  LEDState initialState = connected ? LEDState::Red : LEDState::Gray;

  for (auto it = leds.begin(); it != leds.end(); ++it) {
    updateLEDState(it.key(), initialState);
  }
}

/**
 * 打开串口
 */
void SerialDialog::onOpenSerialPort() {
  if (serialPort->isOpen()) {
    QMessageBox::warning(this, tr("错误"), tr("串口已经打开"));
    return;
  }

  // 配置串口参数
  serialPort->setPortName(ui->comboBox_com->currentText());
  serialPort->setBaudRate(ui->comboBox_baudrate->currentText().toInt());

  // 设置数据位
  QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(
      ui->comboBox_databits->currentData().toInt());
  serialPort->setDataBits(dataBits);

  // 设置校验位
  QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(
      ui->comboBox_parity->currentData().toInt());
  serialPort->setParity(parity);

  // 设置停止位
  QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(
      ui->comboBox_stopbits->currentData().toInt());
  serialPort->setStopBits(stopBits);

  // 打开串口
  if (serialPort->open(QIODevice::ReadWrite)) {
    ui->pushButton_open->setEnabled(false);
    ui->pushButton_close->setEnabled(true);

    // 禁用参数下拉框，防止在串口打开时更改参数
    ui->comboBox_com->setEnabled(false);
    ui->comboBox_baudrate->setEnabled(false);
    ui->comboBox_databits->setEnabled(false);
    ui->comboBox_parity->setEnabled(false);
    ui->comboBox_stopbits->setEnabled(false);

    // 连接数据接收信号
    connect(serialPort, &QSerialPort::readyRead, this,
            &SerialDialog::onReadData);

    // 更新LED状态为红色（已连接未触发）
    updateAllLEDs(true);

    // 启动模拟定时器（仅用于演示）
    simulationTimer->start(2000); // 每2秒更新一次

    // 更新状态标签
    QLabel *statusLabel = findChild<QLabel *>("label_status");
    if (statusLabel) {
      statusLabel->setText(tr("状态: 已连接"));
      statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    }

    QMessageBox::information(this, tr("成功"), tr("串口已成功打开"));
  } else {
    QMessageBox::critical(
        this, tr("错误"),
        tr("无法打开串口: %1").arg(serialPort->errorString()));
  }
}

void SerialDialog::onCloseSerialPort() {
  if (serialPort->isOpen()) {
    serialPort->close();

    // 停止模拟
    if (simulationTimer->isActive()) {
      simulationTimer->stop();
    }

    ui->pushButton_open->setEnabled(true);
    ui->pushButton_close->setEnabled(false);

    // 重新启用参数下拉框
    ui->comboBox_com->setEnabled(true);
    ui->comboBox_baudrate->setEnabled(true);
    ui->comboBox_databits->setEnabled(true);
    ui->comboBox_parity->setEnabled(true);
    ui->comboBox_stopbits->setEnabled(true);

    // 更新所有LED为灰色（未连接状态）
    updateAllLEDs(false);

    // 更新状态标签
    QLabel *statusLabel = findChild<QLabel *>("label_status");
    if (statusLabel) {
      statusLabel->setText(tr("状态: 未连接"));
      statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
    }

    QMessageBox::information(this, tr("成功"), tr("串口已关闭"));
  }
}

#ifdef ENABLE_MULTILANGUAGE
void SerialDialog::retranslateUi() {
  ui->retranslateUi(this);

  // 更新可能需要动态翻译的内容
  ui->comboBox_parity->setItemText(0, tr("无"));
  ui->comboBox_parity->setItemText(1, tr("奇校验"));
  ui->comboBox_parity->setItemText(2, tr("偶校验"));
  ui->comboBox_parity->setItemText(3, tr("空校验"));
  ui->comboBox_parity->setItemText(4, tr("标志校验"));
}
#endif

void SerialDialog::onReadData() {
  // 读取所有可用数据
  QByteArray data = serialPort->readAll();

  // 在实际应用中，这里应该解析接收到的数据，然后根据解析结果更新LED状态
  // 例如，可能需要检查数据包格式、校验和等

  // 这里只是简单地演示如何基于接收到的数据更新LED状态
  if (!data.isEmpty()) {
    // 假设第一个字节表示X00-X07的状态
    if (data.size() > 0) {
      uint8_t xStatus = static_cast<uint8_t>(data[0]);
      for (int i = 0; i < 8; ++i) {
        bool bitSet = (xStatus & (1 << i)) != 0;
        updateLEDState(QString("X0%1").arg(i),
                       bitSet ? LEDState::Green : LEDState::Red);
      }
    }

    // 假设第二个字节表示X10-X17的状态
    if (data.size() > 1) {
      uint8_t xStatus = static_cast<uint8_t>(data[1]);
      for (int i = 0; i < 8; ++i) {
        bool bitSet = (xStatus & (1 << i)) != 0;
        updateLEDState(QString("X1%1").arg(i),
                       bitSet ? LEDState::Green : LEDState::Red);
      }
    }

    // 假设第三个字节表示Y00-Y07的状态
    if (data.size() > 2) {
      uint8_t yStatus = static_cast<uint8_t>(data[2]);
      for (int i = 0; i < 8; ++i) {
        bool bitSet = (yStatus & (1 << i)) != 0;
        updateLEDState(QString("Y0%1").arg(i),
                       bitSet ? LEDState::Green : LEDState::Red);
      }
    }

    // 假设第四个字节表示Y10-Y17的状态
    if (data.size() > 3) {
      uint8_t yStatus = static_cast<uint8_t>(data[3]);
      for (int i = 0; i < 8; ++i) {
        bool bitSet = (yStatus & (1 << i)) != 0;
        updateLEDState(QString("Y1%1").arg(i),
                       bitSet ? LEDState::Green : LEDState::Red);
      }
    }
  }
}

void SerialDialog::onSimulateData() {
  // 仅用于演示：随机生成一些IO点状态变化
  static QRandomGenerator rand;

  // 随机选择一些点变为绿色（触发状态）
  int numPoints = rand.bounded(3, 8); // 每次随机 3-7 个点

  for (int i = 0; i < numPoints; i++) {
    int type = rand.bounded(4);  // 0=X0, 1=X1, 2=Y0, 3=Y1
    int point = rand.bounded(8); // 0-7

    QString ledName;

    switch (type) {
    case 0:
      ledName = QString("X0%1").arg(point);
      break;
    case 1:
      ledName = QString("X1%1").arg(point);
      break;
    case 2:
      ledName = QString("Y0%1").arg(point);
      break;
    case 3:
      ledName = QString("Y1%1").arg(point);
      break;
    }

    // 随机设置状态（绿色或红色）
    bool isTriggered = rand.bounded(2) == 1;
    updateLEDState(ledName, isTriggered ? LEDState::Green : LEDState::Red);
  }
}

void SerialDialog::onRefreshPorts() {
  // 清空现有的端口列表
  ui->comboBox_com->clear();

  // 重新扫描可用端口
  const auto infos = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &info : infos) {
    ui->comboBox_com->addItem(info.portName());
  }

  if (ui->comboBox_com->count() == 0) {
    QMessageBox::warning(this, tr("警告"), tr("未找到可用的串口"));
  } else {
    QMessageBox::information(
        this, tr("成功"),
        tr("已刷新串口列表，找到 %1 个端口").arg(ui->comboBox_com->count()));
  }
}

void SerialDialog::onReadRegister() {
  if (!serialPort || !serialPort->isOpen()) {
    QMessageBox::warning(this, tr("错误"), tr("请先打开串口"));
    return;
  }

  // 检查UI控件是否存在（在UI文件重新生成前可能不存在）
  QComboBox *regTypeCombo = findChild<QComboBox *>("comboBox_regType");
  QLineEdit *addrEdit = findChild<QLineEdit *>("lineEdit_regAddr");
  QLineEdit *countEdit = findChild<QLineEdit *>("lineEdit_regCount");
  QTextEdit *logEdit = findChild<QTextEdit *>("textEdit_log");
  QCheckBox *autoScrollCheck = findChild<QCheckBox *>("checkBox_autoScroll");

  if (!regTypeCombo || !addrEdit || !countEdit) {
    QMessageBox::warning(this, tr("错误"),
                         tr("UI控件未找到，请重新生成UI文件"));
    return;
  }

  QString regType = regTypeCombo->currentText();
  QString address = addrEdit->text();
  QString count = countEdit->text();

  if (address.isEmpty()) {
    QMessageBox::warning(this, tr("错误"), tr("请输入寄存器地址"));
    return;
  }

  // 构造读取命令（这里是示例，实际协议需要根据具体设备调整）
  QString command =
      QString("READ %1 %2 %3\r\n").arg(regType).arg(address).arg(count);
  QByteArray data = command.toUtf8();

  // 发送命令
  qint64 bytesWritten = serialPort->write(data);
  if (bytesWritten == -1) {
    QMessageBox::critical(this, tr("错误"), tr("发送读取命令失败"));
    return;
  }

  // 记录到日志
  if (logEdit) {
    QString logText =
        QString("[%1] 发送读取命令: %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
            .arg(command.trimmed());
    logEdit->append(logText);

    if (autoScrollCheck && autoScrollCheck->isChecked()) {
      logEdit->moveCursor(QTextCursor::End);
    }
  }
}

void SerialDialog::onWriteRegister() {
  if (!serialPort || !serialPort->isOpen()) {
    QMessageBox::warning(this, tr("错误"), tr("请先打开串口"));
    return;
  }

  // 检查UI控件是否存在
  QComboBox *regTypeCombo = findChild<QComboBox *>("comboBox_regType");
  QLineEdit *addrEdit = findChild<QLineEdit *>("lineEdit_regAddr");
  QLineEdit *valueEdit = findChild<QLineEdit *>("lineEdit_regValue");
  QTextEdit *logEdit = findChild<QTextEdit *>("textEdit_log");
  QCheckBox *autoScrollCheck = findChild<QCheckBox *>("checkBox_autoScroll");

  if (!regTypeCombo || !addrEdit || !valueEdit) {
    QMessageBox::warning(this, tr("错误"),
                         tr("UI控件未找到，请重新生成UI文件"));
    return;
  }

  QString regType = regTypeCombo->currentText();
  QString address = addrEdit->text();
  QString value = valueEdit->text();

  if (address.isEmpty() || value.isEmpty()) {
    QMessageBox::warning(this, tr("错误"), tr("请输入寄存器地址和写入值"));
    return;
  }

  // 构造写入命令（这里是示例，实际协议需要根据具体设备调整）
  QString command =
      QString("WRITE %1 %2 %3\r\n").arg(regType).arg(address).arg(value);
  QByteArray data = command.toUtf8();

  // 发送命令
  qint64 bytesWritten = serialPort->write(data);
  if (bytesWritten == -1) {
    QMessageBox::critical(this, tr("错误"), tr("发送写入命令失败"));
    return;
  }

  // 记录到日志
  if (logEdit) {
    QString logText =
        QString("[%1] 发送写入命令: %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
            .arg(command.trimmed());
    logEdit->append(logText);

    if (autoScrollCheck && autoScrollCheck->isChecked()) {
      logEdit->moveCursor(QTextCursor::End);
    }
  }
}

void SerialDialog::onClearLog() {
  QTextEdit *logEdit = findChild<QTextEdit *>("textEdit_log");
  if (logEdit) {
    logEdit->clear();
  }
}

void SerialDialog::onSendData() {
  if (!serialPort || !serialPort->isOpen()) {
    QMessageBox::warning(this, tr("错误"), tr("请先打开串口"));
    return;
  }

  // 检查UI控件是否存在
  QLineEdit *sendEdit = findChild<QLineEdit *>("lineEdit_send");
  QCheckBox *hexSendCheck = findChild<QCheckBox *>("checkBox_hexSend");
  QTextEdit *logEdit = findChild<QTextEdit *>("textEdit_log");
  QCheckBox *autoScrollCheck = findChild<QCheckBox *>("checkBox_autoScroll");

  if (!sendEdit) {
    QMessageBox::warning(this, tr("错误"),
                         tr("UI控件未找到，请重新生成UI文件"));
    return;
  }

  QString text = sendEdit->text();
  if (text.isEmpty()) {
    QMessageBox::warning(this, tr("错误"), tr("请输入要发送的数据"));
    return;
  }

  QByteArray data;

  if (hexSendCheck && hexSendCheck->isChecked()) {
    // HEX格式发送
    QString hexString = text.remove(' ').remove('\n').remove('\r');
    if (hexString.length() % 2 != 0) {
      QMessageBox::warning(this, tr("错误"), tr("HEX数据长度必须为偶数"));
      return;
    }

    for (int i = 0; i < hexString.length(); i += 2) {
      bool ok;
      QString byteString = hexString.mid(i, 2);
      quint8 byte = byteString.toUInt(&ok, 16);
      if (!ok) {
        QMessageBox::warning(this, tr("错误"),
                             tr("无效的HEX数据: %1").arg(byteString));
        return;
      }
      data.append(byte);
    }
  } else {
    // 文本格式发送
    data = text.toUtf8();
  }

  // 发送数据
  qint64 bytesWritten = serialPort->write(data);
  if (bytesWritten == -1) {
    QMessageBox::critical(this, tr("错误"), tr("发送数据失败"));
    return;
  }

  // 记录到日志
  if (logEdit) {
    QString logText =
        QString("[%1] 发送: %2")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
            .arg((hexSendCheck && hexSendCheck->isChecked())
                     ? data.toHex(' ').toUpper()
                     : text);
    logEdit->append(logText);

    if (autoScrollCheck && autoScrollCheck->isChecked()) {
      logEdit->moveCursor(QTextCursor::End);
    }
  }

  // 清空输入框
  sendEdit->clear();
}

void SerialDialog::onResetCount() {
  // 检查UI控件是否存在
  QLabel *rxLabel = findChild<QLabel *>("label_rxCount");
  QLabel *txLabel = findChild<QLabel *>("label_txCount");

  if (rxLabel) {
    rxLabel->setText(tr("接收: 0 字节"));
  }
  if (txLabel) {
    txLabel->setText(tr("发送: 0 字节"));
  }
}

void SerialDialog::changeEvent(QEvent *event) {
  if (event->type() == QEvent::LanguageChange) {
    // 调用UI文件自动生成的函数
    ui->retranslateUi(this);

    // 手动更新其他动态翻译
#ifdef ENABLE_MULTILANGUAGE
    retranslateUi();
#endif
  }

  // 调用父类的changeEvent处理
  QWidget::changeEvent(event);
}
