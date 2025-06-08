//
// Created by jinxi on 25-5-20.
//

#include "../inc/ui/serialdialog.h"
#include "ui_SerialDialog.h"
#include "../thirdparty/log_manager/inc/simplecategorylogger.h" // 引入日志管理器头文件
#include "../thirdparty/libmodbus/inc/modbus/modbusmanager.h"

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
#include <QSettings>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTextDocument>

#define SYSTEM "serialModbus"

const int MAX_LOG_LINE_LENGTH = 120; // 设置最大行长度

// 日志重定义
// 定义优化版的日志宏，基于编译模式和构建设置自动调整行为
#ifdef _DEBUG // 调试模式
// 调试模式下，日志同时输出到控制台和文件，更详细
#define LOG_DEBUG(message) SIMPLE_DEBUG_LOG_DEBUG(SYSTEM, message)
#define LOG_INFO(message) SIMPLE_DEBUG_LOG_INFO(SYSTEM, message)
#define LOG_WARNING(message) SIMPLE_DEBUG_LOG_WARNING(SYSTEM, message)
#define LOG_ERROR(message) SIMPLE_DEBUG_LOG_ERROR(SYSTEM, message)
#define LOG_ASSERT(message) SIMPLE_ASSERT((condition), (message))
// 添加对TRACE日志的支持
#define LOG_TRACE(message) SIMPLE_LOG_TRACE(SYSTEM, message)

#else // 发布模式
// 发布模式下，根据CMake配置决定日志行为

// 其他级别的日志使用配置控制的版本
#define LOG_INFO(message) SIMPLE_LOG_INFO_CONFIG(SYSTEM, message, false, WRITE_TO_FILE)
#define LOG_WARNING(message) SIMPLE_LOG_WARNING_CONFIG(SYSTEM, message, false, WRITE_TO_FILE)
#define LOG_ERROR(message) SIMPLE_LOG_ERROR_CONFIG(SYSTEM, message, false, WRITE_TO_FILE)
#define LOG_ASSERT(condition, message, SYSTEM)                                                                       \
  do                                                                                                                   \
  {                                                                                                                    \
    if (!(condition))                                                                                                  \
    {                                                                                                                  \
      LOG_ERROR(category, "断言失败: " + QString(message));                                                            \
    }                                                                                                                  \
  }                                                                                                                    \
  while (0)

#endif

SerialDialog::SerialDialog(QWidget* parent) :
  QWidget(parent), ui(new Ui::SerialDialog), serialPort(nullptr),
  simulationTimer(nullptr)
{
  ui->setupUi(this);
  // 初始化日志
  initLog();
  // 初始化串口配置
  initSerialPortConfig();
  // 初始化LED指示灯
  initLEDs();
  // 初始化配置
  initConfiguration();
  // 加载配置文件
  loadConfig();
  // 初始化Modbus管理器
  initModbusManager();
  // 初始化连接
  initConnection();

  // 创建模拟数据定时器（仅用于演示）
  simulationTimer = new QTimer(this);
  connect(simulationTimer, &QTimer::timeout, this,
          &SerialDialog::onReadData);

#ifdef ENABLE_MULTILANGUAGE
  // 注册翻译更新回调
  TranslationHelper::registerWindow(this, [this]() { this->retranslateUi(); });
#endif
}

SerialDialog::~SerialDialog()
{
  if (simulationTimer && simulationTimer->isActive())
  {
    simulationTimer->stop();
    delete simulationTimer;
    simulationTimer = nullptr;
  }

  if (serialPort && serialPort->isOpen())
  {
    serialPort->close();
    delete serialPort;
    serialPort = nullptr;
  }
  if (m_modbusManager != nullptr)
  {
    m_modbusManager->disconnect();
    delete m_modbusManager;
    m_modbusManager = nullptr;
  }
  delete ui;
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

void SerialDialog::initConfiguration()
{
  configFilePath = QApplication::applicationDirPath() + "/config/communication/";
  QString serialConfigFilePath = configFilePath + "serialdialog.ini";
  // 检查配置文件是否存在
  settingManager = new SettingManager(this);
  if (!settingManager->init(configFilePath + "serialdialog.ini"))
  {
    LOG_ERROR("配置管理器初始化失败, 无法加载配置信息");
    QMessageBox::critical(this, tr("错误"), tr("无法加载配置信息，请检查配置文件是否存在"));
    return;
  }
  else
  {
    QFileInfo fileInfo(serialConfigFilePath);
    if (!fileInfo.exists() || fileInfo.size() == 0) // 如果配置文件不存在
    {
      // 创建配置目录
      QDir dir;
      dir.mkpath(configFilePath);

      // 创建默认配置文件
      QSettings defaultSettings(serialConfigFilePath, QSettings::IniFormat);
      defaultSettings.beginGroup("SerialPort");
      defaultSettings.setValue("Port", "COM1");
      defaultSettings.setValue("BaudRate", 9600);
      defaultSettings.setValue("DataBits", 8);
      defaultSettings.setValue("Parity", 0); // 0=无校验
      defaultSettings.setValue("StopBits", 1);
      defaultSettings.setValue("Timeout", 1000); // 超时时间(ms)
      defaultSettings.endGroup();

      defaultSettings.beginGroup("Modbus");
      defaultSettings.setValue("SlaveAddress", 1);
      defaultSettings.setValue("RegisterStartAddress", 0);
      defaultSettings.setValue("RegisterCount", 10);
      defaultSettings.endGroup();

      defaultSettings.sync();
      LOG_INFO("已创建默认配置文件");
    }

  }
}

void SerialDialog::initSerialPortConfig()
{
  // 1. 查找可用的串口
  const auto infos = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo& info : infos)
  {
    ui->comboBox_com->addItem(info.portName());
  }

  // 2. 添加常用波特率
  QList<int> baudrates = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
  for (int rate : baudrates)
  {
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

  // 6. 从站地址
  ui->lineEdit_slaveID->setPlaceholderText("1~254"); // 设置从站地址输入框的提示文本

  // 7. 超时设置
  ui->lineEdit_Response_timeOut->setPlaceholderText("单位豪秒/ms "); // 设置超时范围为100ms到5000ms
  // 初始化串口对象
  serialPort = new QSerialPort(this);

  // 初始时禁用关闭按钮，因为串口尚未打开
  ui->pushButton_close->setEnabled(false);
}

void SerialDialog::initModbusManager()
{
  m_modbusManager = new ModbusManager(this);
  m_modbusManager->setSlaveID(modbusParameters.slaveID);
  m_modbusManager->setDebugMode(true);
  m_modbusManager->setTimeout(modbusParameters.timeout);

  connect(m_modbusManager, &ModbusManager::connected, this, [this]()
  {
    LOG_INFO("Modbus连接成功");
    updateLEDState("modbus", LEDState::Red);
    ui->pushButton_open->setEnabled(false);
    ui->pushButton_close->setEnabled(true);
    ui->pushButton_send->setEnabled(true);
    ui->pushButton_readReg->setEnabled(true);
    ui->pushButton_writeReg->setEnabled(true);
    ui->pushButton_resetCount->setEnabled(false);
    ui->pushButton_clearLog->setEnabled(true);
    ui->comboBox_com->setEnabled(false);
    ui->comboBox_baudrate->setEnabled(false);
    ui->comboBox_databits->setEnabled(false);
    ui->comboBox_parity->setEnabled(false);
    ui->comboBox_stopbits->setEnabled(false);
    ui->lineEdit_slaveID->setEnabled(false);
    ui->lineEdit_Response_timeOut->setEnabled(false);

    // 更新状态标签
    QLabel* statusLabel = findChild<QLabel*>("label_status");
    if (statusLabel)
    {
      statusLabel->setText(tr("状态: 已连接"));
      statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
      updateAllLEDs(true);
    }

  });
  connect(m_modbusManager, &ModbusManager::disconnected, this, [this]()
  {
    LOG_INFO("Modbus连接已断开");
    updateLEDState("modbus", LEDState::Gray);
    ui->pushButton_open->setEnabled(true);
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_send->setEnabled(false);
    ui->pushButton_readReg->setEnabled(false);
    ui->pushButton_writeReg->setEnabled(false);
    ui->pushButton_resetCount->setEnabled(false);
    ui->pushButton_clearLog->setEnabled(false);
    ui->comboBox_com->setEnabled(true);
    ui->comboBox_baudrate->setEnabled(true);
    ui->comboBox_databits->setEnabled(true);
    ui->comboBox_parity->setEnabled(true);
    ui->comboBox_stopbits->setEnabled(true);
    ui->lineEdit_slaveID->setEnabled(true);
    ui->lineEdit_Response_timeOut->setEnabled(true);
    // 更新状态标签
    QLabel* statusLabel = findChild<QLabel*>("label_status");
    if (statusLabel)
    {
      statusLabel->setText(tr("状态: 未连接"));
      statusLabel->setStyleSheet("color: #ff0000; font-weight: bold;");
      updateAllLEDs(false);
    }
  });
  // connect(m_modbusManager, &ModbusManager::dataReceived, this, &SerialDialog::onReadData);
  connect(m_modbusManager, &ModbusManager::dataSent, this, &SerialDialog::onSendData);
  connect(m_modbusManager, &ModbusManager::errorOccurred, this, [this](const QString& error)
  {
    if (error.length() > MAX_LOG_LINE_LENGTH)
    {

      for (int i = 0; i < error.length(); i += MAX_LOG_LINE_LENGTH)
      {
        QString segment = error.mid(i, qMin(MAX_LOG_LINE_LENGTH, error.length() - i));
        appLog(tr("%1").arg(segment), LOGType::INFO);
      }
    }
    else
    {
      LOG_ERROR("Modbus错误: " + error);
      appLog(tr("Modbus错误: %1").arg(error), LOGType::ERR);
    }
  });
  connect(m_modbusManager, &ModbusManager::infoLog, this, [this](const QString& info)
  {
    // 检查信息长度，如果过长则分段显示
    if (info.length() > MAX_LOG_LINE_LENGTH)
    {
      for (int i = 0; i < info.length(); i += MAX_LOG_LINE_LENGTH)
      {
        QString segment = info.mid(i, qMin(MAX_LOG_LINE_LENGTH, info.length() - i));
        appLog(tr("%1").arg(segment), LOGType::INFO);
      }
    }
    else
    {
      appLog(tr("%1").arg(info), LOGType::INFO);
    }
  });
}

void SerialDialog::initLog()
{
  // 初始化日志管理类
  auto& logger = SimpleCategoryLogger::instance();
  // 初始化日志系统 | en : Initialize the logging system
  QString logFilePath = QApplication::applicationDirPath() + "/logs/" + SYSTEM;
  logger.initCategory(SYSTEM, logFilePath);
  // 设置日志大小 | en : Set the log file size
  logger.setDefaultMaxLogFileSize(10 * 1024 * 1024); // 10MB
  // 设置历史文件数量 | en : Set the number of historical log files
  logger.setMaxHistoryFileCount(30); // 保留30个历史日志文件
  // 配置调试选项 | en : Configure debug options
  logger.setDebugConfig(true, true, true); // 显示在控制台、写入文件、启用调试日志
  // 设置定期清理日志 | en : Set up periodic log cleanup
  logger.setPeriodicCleanup(true); // 默认7天清理一次, 日志天数超过30天自动清理

}

void SerialDialog::appLog(const QString& message, LOGType type)
{
  // 初始化日志显示区域
  ui->textEdit_log->setReadOnly(true);
  ui->textEdit_log->setLineWrapMode(QTextEdit::NoWrap); // 禁用自动换行
  ui->textEdit_log->setFont(QFont("Consolas", 10)); // 设置字体和大小
  // 设置日志样式
  QString styleSheet = "QTextEdit {"
      "background-color: #f0f0f0;"
      "color: #333;"
      "border: 1px solid #ccc;"
      "padding: 5px;"
      "}";
  ui->textEdit_log->setStyleSheet(styleSheet);
  // 获取当前时间
  QDateTime currentTime = QDateTime::currentDateTime();
  QString timeString = currentTime.toString("yyyy-MM-dd HH:mm:ss");

  // 将日志消息添加到文本编辑器
  QString logMessage = QString("[%1] %2\n").arg(timeString, message);
  switch (type)
  {
  case LOGType::INFO:
    logMessage.prepend("<span style='color: blue;'>[INFO] </span>");
    LOG_INFO(logMessage);
    break;
  case LOGType::WARNING:
    logMessage.prepend("<span style='color: orange;'>[WARNING] </span>");
    LOG_WARNING(logMessage);
    break;
  case LOGType::ERR:
    logMessage.prepend("<span style='color: red;'>[ERROR] </span>");
    LOG_ERROR(logMessage);
    break;
  default:
    logMessage.prepend("<span style='color: black;'>[LOG] </span>");
    LOG_DEBUG(logMessage);
    break;
  }
  ui->textEdit_log->append(logMessage);

  // 滚动到最新日志
  QTextCursor cursor = ui->textEdit_log->textCursor();
  cursor.movePosition(QTextCursor::End);
  ui->textEdit_log->setTextCursor(cursor);
  // 确保日志显示区域滚动到最新位置
  ui->textEdit_log->ensureCursorVisible();
}

void SerialDialog::onSaveConfig()
{
  // 获取当前配置
  serialParameters.portName = ui->comboBox_com->currentText();
  serialParameters.baudRate = ui->comboBox_baudrate->currentText().toInt();
  serialParameters.dataBits = ui->comboBox_databits->currentData().toInt();
  serialParameters.parity = ui->comboBox_parity->currentData().toInt();
  serialParameters.stopBits = ui->comboBox_stopbits->currentData().toInt();

  modbusParameters.timeout = ui->lineEdit_Response_timeOut->text().toInt();
  modbusParameters.slaveID = ui->lineEdit_slaveID->text().toInt();
  if (modbusParameters.slaveID < 1 || modbusParameters.slaveID > 254)
  {
    QMessageBox::warning(this, tr("错误"), tr("从站地址必须在1到254之间"));
    return;
  }
  if (modbusParameters.timeout < 100 || modbusParameters.timeout > 5000)
  {
    QMessageBox::warning(this, tr("错误"), tr("超时时间必须在100到5000毫秒之间"));
    return;
  }
  // 保存到配置文件
  if (!settingManager->init(configFilePath + "serialdialog.ini"))
  {
    LOG_ERROR("配置管理器初始化失败, 无法存储配置信息");
    QMessageBox::critical(this, tr("错误"), tr("无法存储配置信息，请检查配置文件是否存在"));
    return;
  }
  settingManager->setValue("SerialPort/Port", serialParameters.portName);
  settingManager->setValue("SerialPort/BaudRate", serialParameters.baudRate);
  settingManager->setValue("SerialPort/DataBits", serialParameters.dataBits);
  settingManager->setValue("SerialPort/Parity", serialParameters.parity);
  settingManager->setValue("SerialPort/StopBits", serialParameters.stopBits);
  settingManager->setValue("SerialPort/Timeout", modbusParameters.timeout);
  settingManager->setValue("SerialPort/SlaveID", modbusParameters.slaveID);

  QMessageBox::information(this, tr("保存配置"), tr("串口配置已保存"));
}

/* * 初始化LED指示灯
 * 获取所有LED指示灯的引用并存入map中
 * 输入点X00-X07, X10-X17
 * 输出点Y00-Y07, Y10-Y17
 */
void SerialDialog::initLEDs()
{
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

void SerialDialog::updateLEDState(const QString& ledName, LEDState state)
{
  if (!leds.contains(ledName))
  {
    return;
  }

  QLabel* led = leds[ledName];
  QString stateText;

  switch (state)
  {
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
void SerialDialog::createLEDLabels()
{
  // 获取LED所在的父容器（通过VBoxLayout->GridLayout）
  QVBoxLayout* inputVLayout =
      qobject_cast<QVBoxLayout*>(ui->groupBox_input->layout());
  QVBoxLayout* outputVLayout =
      qobject_cast<QVBoxLayout*>(ui->groupBox_output->layout());

  if (!inputVLayout || !outputVLayout)
  {
    qWarning() << "无法找到LED所在的VBoxLayout";
    return;
  }

  // 获取GridLayout（VBoxLayout的第一个item）
  QGridLayout* inputLayout = nullptr;
  QGridLayout* outputLayout = nullptr;

  if (inputVLayout->count() > 0)
  {
    QLayoutItem* item = inputVLayout->itemAt(0);
    if (item)
    {
      inputLayout = qobject_cast<QGridLayout*>(item->layout());
    }
  }

  if (outputVLayout->count() > 0)
  {
    QLayoutItem* item = outputVLayout->itemAt(0);
    if (item)
    {
      outputLayout = qobject_cast<QGridLayout*>(item->layout());
    }
  }

  if (!inputLayout || !outputLayout)
  {
    qWarning() << "无法找到LED所在的GridLayout";
    return;
  }

  // 为每个LED设置样式和提示
  for (auto it = leds.begin(); it != leds.end(); ++it)
  {
    QString ledName = it.key();
    QLabel* led = it.value();

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
void SerialDialog::updateAllLEDs(bool connected)
{
  LEDState initialState = connected ? LEDState::Red : LEDState::Gray;

  for (auto it = leds.begin(); it != leds.end(); ++it)
  {
    updateLEDState(it.key(), initialState);
  }
}

void SerialDialog::loadConfig()
{
  if (settingManager == nullptr)
  {
    LOG_ERROR("配置管理器未初始化，无法加载配置");
    return;
  }
  // 从配置文件加载串口设置
  serialParameters.portName = settingManager->getValue("SerialPort/Port").toString();
  serialParameters.baudRate = settingManager->getValue("SerialPort/BaudRate").toInt();
  serialParameters.dataBits = settingManager->getValue("SerialPort/DataBits").toInt();
  serialParameters.parity = settingManager->getValue("SerialPort/Parity").toInt();
  serialParameters.stopBits = settingManager->getValue("SerialPort/StopBits").toInt();
  modbusParameters.timeout = settingManager->getValue("SerialPort/Timeout").toInt();
  modbusParameters.slaveID = settingManager->getValue("SerialPort/SlaveID").toInt();
  modbusParameters.dataType = ModbusManager::DataType::HoldingRegisters; // 默认数据类型为保持寄存器
  if (modbusParameters.slaveID < 1 || modbusParameters.slaveID > 254)
  {
    QMessageBox::warning(this, tr("错误"), tr("从站地址必须在1到254之间"));
    return;
  }
  if (modbusParameters.timeout < 100 || modbusParameters.timeout > 5000)
  {
    QMessageBox::warning(this, tr("错误"), tr("超时时间必须在100到5000毫秒之间"));
    return;
  }
  // 设置串口参数
  ui->comboBox_com->setCurrentText(serialParameters.portName);
  ui->comboBox_baudrate->setCurrentText(QString::number(serialParameters.baudRate));
  ui->comboBox_databits->setCurrentText(QString::number(serialParameters.dataBits));
  ui->comboBox_parity->setCurrentText(QString::number(serialParameters.parity));
  ui->comboBox_stopbits->setCurrentText(QString::number(serialParameters.stopBits));
  ui->lineEdit_Response_timeOut->setText(QString::number(modbusParameters.timeout));
  ui->lineEdit_slaveID->setText(QString::number(modbusParameters.slaveID));
  // 更新LED状态
  updateAllLEDs(false); // 初始状态为灰色（未连接状态）
  // 更新状态标签
  QLabel* statusLabel = findChild<QLabel*>("label_status");
  if (statusLabel)
  {
    statusLabel->setText(tr("状态: 未连接"));
    statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
  }
  LOG_INFO("配置加载成功: " + serialParameters.portName + ", 波特率: " + QString::number(serialParameters.baudRate) +
      ", 数据位: " + QString::number(serialParameters.dataBits) +
      ", 校验位: " + QString::number(serialParameters.parity) +
      ", 停止位: " + QString::number(serialParameters.stopBits) +
      ", 超时: " + QString::number(modbusParameters.timeout) +
      ", 从站地址: " + QString::number(modbusParameters.slaveID));
}

/**
 * 打开串口
 */
void SerialDialog::onOpenSerialPort()
{
  if (m_modbusManager->isConnected())
  {
    QMessageBox::warning(this, tr("错误"), tr("Modbus连接已经打开"));
    return;
  }
  serialParameters.portName = ui->comboBox_com->currentText();
  serialParameters.baudRate = ui->comboBox_baudrate->currentText().toInt();
  serialParameters.dataBits = ui->comboBox_databits->currentData().toInt();
  serialParameters.parity = ui->comboBox_parity->currentData().toInt();
  serialParameters.stopBits = ui->comboBox_stopbits->currentData().toInt();
  modbusParameters.timeout = ui->lineEdit_Response_timeOut->text().toInt();
  modbusParameters.slaveID = ui->lineEdit_slaveID->text().toInt();
  if (modbusParameters.slaveID < 1 || modbusParameters.slaveID > 254)
  {
    QMessageBox::warning(this, tr("错误"), tr("从站地址必须在1到254之间"));
    return;
  }
  if (modbusParameters.timeout < 100 || modbusParameters.timeout > 5000)
  {
    QMessageBox::warning(this, tr("错误"), tr("超时时间必须在100到5000毫秒之间"));
    return;
  }
  m_modbusManager->connectRTU(serialParameters.portName, serialParameters.baudRate,
                              serialParameters.dataBits, serialParameters.parity,
                              serialParameters.stopBits);
  if (m_modbusManager->isConnected())
  {
    m_modbusManager->getPortInfo(serialParameters.portName);
    // simulationTimer->start(50); // 每50毫秒读取一次数据
  }
}

void SerialDialog::onCloseSerialPort()
{
  if (m_modbusManager->isConnected())
  {
    appLog(tr("正在关闭串口..."), LOGType::INFO);
    m_modbusManager->disconnect();
    // simulationTimer->stop(); // 停止模拟数据读取
  }
}

void SerialDialog::onReadData()
{
  // 检查Modbus连接状态
  if (!m_modbusManager || !m_modbusManager->isConnected())
  {
    appLog(tr("Modbus设备未连接"), LOGType::WARNING);
    return;
  }

  // 设置从站地址（如果需要）
  m_modbusManager->setSlaveID(modbusParameters.slaveID);
  
  // 先尝试读取线圈状态 - 只读取支持的功能码
  QVector<bool> coilValues;
  bool success = m_modbusManager->readCoils(0, 32, coilValues);
  
  if (!success)
  {
    QString errorMsg = m_modbusManager->getLastError();
    // 如果读取线圈失败，尝试读取保持寄存器来模拟IO状态
    QVector<quint16> registerValues;
    bool regSuccess = m_modbusManager->readHoldingRegisters(0, 4, registerValues);
    
    if (regSuccess && registerValues.size() >= 4)
    {
      // 将寄存器值转换为位状态来模拟IO点
      for (int reg = 0; reg < 4 && reg < registerValues.size(); ++reg)
      {
        quint16 regValue = registerValues[reg];
        for (int bit = 0; bit < 8; ++bit)
        {
          bool bitState = (regValue >> bit) & 0x01;
          
          if (reg == 0) // X00-X07
          {
            updateLEDState(QString("X0%1").arg(bit),
                          bitState ? LEDState::Green : LEDState::Red);
          }
          else if (reg == 1) // X10-X17  
          {
            updateLEDState(QString("X1%1").arg(bit),
                          bitState ? LEDState::Green : LEDState::Red);
          }
          else if (reg == 2) // Y00-Y07
          {
            updateLEDState(QString("Y0%1").arg(bit),
                          bitState ? LEDState::Green : LEDState::Red);
          }
          else if (reg == 3) // Y10-Y17
          {
            updateLEDState(QString("Y1%1").arg(bit),
                          bitState ? LEDState::Green : LEDState::Red);
          }
        }
      }
      appLog(tr("成功读取IO状态（通过保持寄存器）"), LOGType::INFO);
    }
    else
    {
      appLog(tr("读取设备状态失败: %1").arg(errorMsg), LOGType::ERR);
    }
    return;
  }

  // 确保我们有足够的数据
  if (coilValues.size() < 32)
  {
    appLog(tr("读取的线圈数据不足，期望32个，实际%1个").arg(coilValues.size()), LOGType::WARNING);
    return;
  }

  // 更新X00-X07的状态
  for (int i = 0; i < 8; ++i)
  {
    if (i < coilValues.size())
    {
      bool coilState = coilValues[i];
      updateLEDState(QString("X0%1").arg(i),
                     coilState ? LEDState::Green : LEDState::Red);
    }
  }

  // 更新X10-X17的状态
  for (int i = 0; i < 8; ++i)
  {
    int coilIndex = 8 + i; // X10-X17 对应线圈索引 8-15
    if (coilIndex < coilValues.size())
    {
      bool coilState = coilValues[coilIndex];
      updateLEDState(QString("X1%1").arg(i),
                     coilState ? LEDState::Green : LEDState::Red);
    }
  }

  // 更新Y00-Y07的状态
  for (int i = 0; i < 8; ++i)
  {
    int coilIndex = 16 + i; // Y00-Y07 对应线圈索引 16-23
    if (coilIndex < coilValues.size())
    {
      bool coilState = coilValues[coilIndex];
      updateLEDState(QString("Y0%1").arg(i),
                     coilState ? LEDState::Green : LEDState::Red);
    }
  }

  // 更新Y10-Y17的状态
  for (int i = 0; i < 8; ++i)
  {
    int coilIndex = 24 + i; // Y10-Y17 对应线圈索引 24-31
    if (coilIndex < coilValues.size())
    {
      bool coilState = coilValues[coilIndex];
      updateLEDState(QString("Y1%1").arg(i),
                     coilState ? LEDState::Green : LEDState::Red);
    }
  }

  // 记录成功读取的信息
  appLog(tr("成功读取32个线圈状态"), LOGType::INFO);
}

void SerialDialog::onSimulateData()
{
  // 仅用于演示：随机生成一些IO点状态变化
  static QRandomGenerator rand;

  // 随机选择一些点变为绿色（触发状态）
  int numPoints = rand.bounded(3, 8); // 每次随机 3-7 个点

  for (int i = 0; i < numPoints; i++)
  {
    int type = rand.bounded(4); // 0=X0, 1=X1, 2=Y0, 3=Y1
    int point = rand.bounded(8); // 0-7

    QString ledName;

    switch (type)
    {
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

void SerialDialog::onRefreshPorts()
{
  // 清空现有的端口列表
  ui->comboBox_com->clear();

  // 重新扫描可用端口
  const auto infos = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo& info : infos)
  {
    ui->comboBox_com->addItem(info.portName());
  }

  if (ui->comboBox_com->count() == 0)
  {
    QMessageBox::warning(this, tr("警告"), tr("未找到可用的串口"));
  }
  else
  {
    QMessageBox::information(
        this, tr("成功"),
        tr("已刷新串口列表，找到 %1 个端口").arg(ui->comboBox_com->count()));
  }
}

void SerialDialog::onReadRegister()
{
  if (!m_modbusManager->isConnected())
  {
    appLog(tr("请连接串口"), LOGType::ERR);
    return;
  }

  // 检查UI控件是否存在（在UI文件重新生成前可能不存在）
  QComboBox* regTypeCombo = findChild<QComboBox*>("comboBox_regType");
  QLineEdit* addrEdit = findChild<QLineEdit*>("lineEdit_regAddr");
  QLineEdit* countEdit = findChild<QLineEdit*>("lineEdit_regCount");
  QTextEdit* logEdit = findChild<QTextEdit*>("textEdit_log");
  QCheckBox* autoScrollCheck = findChild<QCheckBox*>("checkBox_autoScroll");

  if (!regTypeCombo || !addrEdit || !countEdit)
  {
    QMessageBox::warning(this, tr("错误"),
                         tr("UI控件未找到，请重新生成UI文件"));
    return;
  }

  QString regType = regTypeCombo->currentText();
  QString address = addrEdit->text();
  QString count = countEdit->text();

  if (address.isEmpty())
  {
    QMessageBox::warning(this, tr("错误"), tr("请输入寄存器地址"));
    return;
  }

  // 解析参数
  bool addressOk = false;
  bool countOk = false;
  int startAddress = address.toInt(&addressOk);
  int registerCount = count.isEmpty() ? 1 : count.toInt(&countOk);

  if (!addressOk || startAddress < 0)
  {
    QMessageBox::warning(this, tr("错误"), tr("寄存器地址格式错误"));
    return;
  }

  if (!count.isEmpty() && (!countOk || registerCount <= 0 || registerCount > 125))
  {
    QMessageBox::warning(this, tr("错误"), tr("寄存器数量必须在1-125之间"));
    return;
  }

  // 根据寄存器类型执行相应的读取操作
  QVector<quint16> values;
  QVector<bool> coilValues;
  bool success = false;
  int modbusAddress = startAddress;

  // 解析PLC地址格式并转换为Modbus地址
  if (regType == "X点" || regType == "X Points" || regType == "输入点")
  {
    // X点 -> 离散输入, 八进制转十进制
    if (address.startsWith("X") || address.startsWith("x"))
    {
      QString octalStr = address.mid(1); // 去掉X前缀
      bool ok;
      modbusAddress = octalStr.toInt(&ok, 8); // 八进制转换
      if (!ok) {
        QMessageBox::warning(this, tr("错误"), tr("X点地址格式错误，应为X00-X77格式"));
        return;
      }
    }
    success = m_modbusManager->readDiscreteInputs(modbusAddress, registerCount, coilValues);
  }
  else if (regType == "Y点" || regType == "Y Points" || regType == "输出点")
  {
    // Y点 -> 线圈, 八进制转十进制
    if (address.startsWith("Y") || address.startsWith("y"))
    {
      QString octalStr = address.mid(1); // 去掉Y前缀
      bool ok;
      modbusAddress = octalStr.toInt(&ok, 8); // 八进制转换
      if (!ok) {
        QMessageBox::warning(this, tr("错误"), tr("Y点地址格式错误，应为Y00-Y77格式"));
        return;
      }
    }
    success = m_modbusManager->readCoils(modbusAddress, registerCount, coilValues);
  }
  else if (regType == "D寄存器" || regType == "D Registers" || regType == "数据寄存器")
  {
    // D寄存器 -> 保持寄存器
    if (address.startsWith("D") || address.startsWith("d"))
    {
      QString numStr = address.mid(1); // 去掉D前缀
      bool ok;
      modbusAddress = numStr.toInt(&ok);
      if (!ok || modbusAddress < 0 || modbusAddress > 7999) {
        QMessageBox::warning(this, tr("错误"), tr("D寄存器地址格式错误，应为D0-D7999格式"));
        return;
      }
    }
    success = m_modbusManager->readHoldingRegisters(modbusAddress, registerCount, values);
  }
  else if (regType == "M寄存器" || regType == "M Registers" || regType == "内部继电器")
  {
    // M寄存器 -> 线圈 (地址偏移1000避免与Y点冲突)
    if (address.startsWith("M") || address.startsWith("m"))
    {
      QString numStr = address.mid(1); // 去掉M前缀
      bool ok;
      int mAddr = numStr.toInt(&ok);
      if (!ok || mAddr < 0 || mAddr > 7999) {
        QMessageBox::warning(this, tr("错误"), tr("M寄存器地址格式错误，应为M0-M7999格式"));
        return;
      }
      modbusAddress = mAddr + 1000; // 地址偏移
    }
    success = m_modbusManager->readCoils(modbusAddress, registerCount, coilValues);
  }
  else if (regType == "Holding Registers" || regType == "保持寄存器")
  {
    success = m_modbusManager->readHoldingRegisters(modbusAddress, registerCount, values);
  }
  else if (regType == "Input Registers" || regType == "输入寄存器")
  {
    success = m_modbusManager->readInputRegisters(modbusAddress, registerCount, values);
  }
  else if (regType == "Coils" || regType == "线圈")
  {
    success = m_modbusManager->readCoils(modbusAddress, registerCount, coilValues);
  }
  else if (regType == "Discrete Inputs" || regType == "离散输入")
  {
    success = m_modbusManager->readDiscreteInputs(modbusAddress, registerCount, coilValues);
  }
  else
  {
    QMessageBox::warning(this, tr("错误"), tr("不支持的寄存器类型"));
    return;
  }

  if (!success)
  {
    QString errorMsg = m_modbusManager->getLastError();
    QMessageBox::critical(this, tr("错误"), tr("读取寄存器失败: %1").arg(errorMsg));
    appLog(tr("读取寄存器失败: %1").arg(errorMsg), LOGType::ERR);
    return;
  }

  // 记录到日志并显示结果
  if (logEdit)
  {
    QString resultText;
    QString originalAddress = address;
    
    if (regType.contains("X点") || regType.contains("Y点") || regType.contains("M寄存器") ||
        regType == "Coils" || regType == "线圈" || 
        regType == "Discrete Inputs" || regType == "离散输入")
    {
      // 显示线圈/离散输入的结果
      QStringList boolResults;
      for (int i = 0; i < coilValues.size(); ++i)
      {
        QString addr;
        if (regType.contains("X点")) {
          addr = QString("X%1").arg(startAddress + i, 2, 8, QChar('0')); // 八进制显示
        } else if (regType.contains("Y点")) {
          addr = QString("Y%1").arg(startAddress + i, 2, 8, QChar('0')); // 八进制显示
        } else if (regType.contains("M寄存器")) {
          addr = QString("M%1").arg(startAddress + i);
        } else {
          addr = QString("%1").arg(modbusAddress + i);
        }
        boolResults << QString("%1:%2").arg(addr).arg(coilValues[i] ? "1" : "0");
      }
      resultText = boolResults.join(", ");
    }
    else
    {
      // 显示寄存器的结果
      QStringList regResults;
      for (int i = 0; i < values.size(); ++i)
      {
        QString addr;
        if (regType.contains("D寄存器")) {
          addr = QString("D%1").arg(startAddress + i);
        } else {
          addr = QString("%1").arg(modbusAddress + i);
        }
        regResults << QString("%1:%2").arg(addr).arg(values[i]);
      }
      resultText = regResults.join(", ");
    }

    QString logText =
        QString("[%1] 读取%2成功 - 地址:%3, 数量:%4, 结果:[%5]")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(regType)
        .arg(originalAddress)
        .arg(registerCount)
        .arg(resultText);
    logEdit->append(logText);

    if (autoScrollCheck && autoScrollCheck->isChecked())
    {
      logEdit->moveCursor(QTextCursor::End);
    }
  }

  // 记录成功信息到应用日志
  appLog(tr("成功读取%1, 地址:%2, 数量:%3").arg(regType).arg(address).arg(registerCount), LOGType::INFO);
}

void SerialDialog::onWriteRegister()
{
  if (!m_modbusManager->isConnected())
  {
    QMessageBox::warning(this, tr("错误"), tr("请先打开串口"));
    return;
  }

  // 检查UI控件是否存在
  QComboBox* regTypeCombo = findChild<QComboBox*>("comboBox_regType");
  QLineEdit* addrEdit = findChild<QLineEdit*>("lineEdit_regAddr");
  QLineEdit* valueEdit = findChild<QLineEdit*>("lineEdit_regValue");
  QTextEdit* logEdit = findChild<QTextEdit*>("textEdit_log");
  QCheckBox* autoScrollCheck = findChild<QCheckBox*>("checkBox_autoScroll");

  if (!regTypeCombo || !addrEdit || !valueEdit)
  {
    QMessageBox::warning(this, tr("错误"),
                         tr("UI控件未找到，请重新生成UI文件"));
    return;
  }

  QString regType = regTypeCombo->currentText();
  QString addressStr = addrEdit->text();
  QString valueStr = valueEdit->text();

  if (addressStr.isEmpty() || valueStr.isEmpty())
  {
    QMessageBox::warning(this, tr("错误"), tr("请输入寄存器地址和写入值"));
    return;
  }

  // 解析地址
  bool addressOk;
  int address = addressStr.toInt(&addressOk);
  
  if (!addressOk || address < 0)
  {
    QMessageBox::warning(this, tr("错误"), tr("无效的寄存器地址"));
    return;
  }

  bool success = false;
  QString operationDesc;

  // 解析值字符串，支持多个值 (用逗号、空格或分号分隔)
  QStringList valueStrList = valueStr.split(QRegExp("[,;\\s]+"), QString::SkipEmptyParts);
  if (valueStrList.isEmpty())
  {
    QMessageBox::warning(this, tr("错误"), tr("请输入有效的写入值"));
    return;
  }

  // 根据寄存器类型执行相应的Modbus写入操作
  if (regType == "线圈" || regType == "Coil" || regType.contains("线圈"))
  {
    if (valueStrList.size() == 1)
    {
      // 写入单个线圈
      QString singleValue = valueStrList.first();
      bool coilValue = false;
      if (singleValue == "1" || singleValue.toLower() == "true" || singleValue.toLower() == "on")
      {
        coilValue = true;
      }
      else if (singleValue == "0" || singleValue.toLower() == "false" || singleValue.toLower() == "off")
      {
        coilValue = false;
      }
      else
      {
        QMessageBox::warning(this, tr("错误"), tr("线圈值必须为 0/1 或 true/false"));
        return;
      }
      
      success = m_modbusManager->writeSingleCoil(address, coilValue);
      operationDesc = QString("写入单个线圈 地址:%1 值:%2").arg(address).arg(coilValue ? "ON" : "OFF");
    }
    else
    {
      // 写入多个线圈
      QVector<bool> coilValues;
      for (const QString& valStr : valueStrList)
      {
        if (valStr == "1" || valStr.toLower() == "true" || valStr.toLower() == "on")
        {
          coilValues.append(true);
        }
        else if (valStr == "0" || valStr.toLower() == "false" || valStr.toLower() == "off")
        {
          coilValues.append(false);
        }
        else
        {
          QMessageBox::warning(this, tr("错误"), tr("线圈值必须为 0/1 或 true/false: %1").arg(valStr));
          return;
        }
      }
      
      success = m_modbusManager->writeMultipleCoils(address, coilValues);
      QStringList valueDescs;
      for (int i = 0; i < coilValues.size(); ++i)
      {
        valueDescs << QString("%1:%2").arg(address + i).arg(coilValues[i] ? "ON" : "OFF");
      }
      operationDesc = QString("写入多个线圈 [%1]").arg(valueDescs.join(", "));
    }
  }
  else if (regType == "保持寄存器" || regType == "Holding Register" || regType.contains("寄存器"))
  {
    if (valueStrList.size() == 1)
    {
      // 写入单个保持寄存器
      bool valueOk;
      quint16 regValue = valueStrList.first().toUShort(&valueOk);
      if (!valueOk)
      {
        QMessageBox::warning(this, tr("错误"), tr("无效的寄存器值 (应为0-65535)"));
        return;
      }
      
      success = m_modbusManager->writeSingleRegister(address, regValue);
      operationDesc = QString("写入单个寄存器 地址:%1 值:%2").arg(address).arg(regValue);
    }
    else
    {
      // 写入多个保持寄存器
      QVector<quint16> regValues;
      for (const QString& valStr : valueStrList)
      {
        bool valueOk;
        quint16 regValue = valStr.toUShort(&valueOk);
        if (!valueOk)
        {
          QMessageBox::warning(this, tr("错误"), tr("无效的寄存器值 (应为0-65535): %1").arg(valStr));
          return;
        }
        regValues.append(regValue);
      }
      
      if (regValues.size() > 123) // Modbus限制
      {
        QMessageBox::warning(this, tr("错误"), tr("一次最多只能写入123个寄存器"));
        return;
      }
      
      success = m_modbusManager->writeMultipleRegisters(address, regValues);
      QStringList valueDescs;
      for (int i = 0; i < regValues.size(); ++i)
      {
        valueDescs << QString("%1:%2").arg(address + i).arg(regValues[i]);
      }
      operationDesc = QString("写入多个寄存器 [%1]").arg(valueDescs.join(", "));
    }
  }
  else
  {
    QMessageBox::warning(this, tr("错误"), tr("不支持的寄存器类型: %1\n支持类型: 线圈、保持寄存器").arg(regType));
    return;
  }

  // 记录到日志
  if (logEdit)
  {
    QString logText;
    if (success)
    {
      logText = QString("[%1] 成功: %2")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(operationDesc);
    }
    else
    {
      logText = QString("[%1] 失败: %2 - 错误: %3")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(operationDesc)
                .arg(m_modbusManager->getLastError());
    }
    
    logEdit->append(logText);

    if (autoScrollCheck && autoScrollCheck->isChecked())
    {
      logEdit->moveCursor(QTextCursor::End);
    }
  }

  // 显示结果消息
  if (success)
  {
    appLog(tr("Modbus写入成功: %1").arg(operationDesc), LOGType::INFO);
  }
  else
  {
    QString errorMsg = tr("写入操作失败: %1\n错误信息: %2").arg(operationDesc).arg(m_modbusManager->getLastError());
    appLog(tr("Modbus写入失败: %1").arg(errorMsg), LOGType::ERR);
  }
}

void SerialDialog::onClearLog()
{
  QTextEdit* logEdit = findChild<QTextEdit*>("textEdit_log");
  if (logEdit)
  {
    logEdit->clear();
  }
}

void SerialDialog::onSendData()
{
  if (!m_modbusManager->isConnected())
  {
    QMessageBox::warning(this, tr("错误"), tr("请先打开串口"));
    return;
  }

  // 检查UI控件是否存在
  QLineEdit* sendEdit = findChild<QLineEdit*>("lineEdit_send");
  QCheckBox* hexSendCheck = findChild<QCheckBox*>("checkBox_hexSend");
  QTextEdit* logEdit = findChild<QTextEdit*>("textEdit_log");
  QCheckBox* autoScrollCheck = findChild<QCheckBox*>("checkBox_autoScroll");

  if (!sendEdit)
  {
    QMessageBox::warning(this, tr("错误"),
                         tr("UI控件未找到，请重新生成UI文件"));
    return;
  }

  QString text = sendEdit->text();
  if (text.isEmpty())
  {
    QMessageBox::warning(this, tr("错误"), tr("请输入要发送的数据"));
    return;
  }

  // 警告用户这是原始数据发送功能
  QMessageBox::StandardButton reply = QMessageBox::question(this, 
    tr("警告"), 
    tr("这是原始数据发送功能，仅用于调试。\n建议使用上方的Modbus读写功能。\n确定要继续吗？"),
    QMessageBox::Yes | QMessageBox::No, 
    QMessageBox::No);
    
  if (reply != QMessageBox::Yes)
  {
    return;
  }

  QByteArray data;

  if (hexSendCheck && hexSendCheck->isChecked())
  {
    // HEX格式发送
    QString hexString = text.remove(' ').remove('\n').remove('\r');
    if (hexString.length() % 2 != 0)
    {
      QMessageBox::warning(this, tr("错误"), tr("HEX数据长度必须为偶数"));
      return;
    }

    for (int i = 0; i < hexString.length(); i += 2)
    {
      bool ok;
      QString byteString = hexString.mid(i, 2);
      quint8 byte = byteString.toUInt(&ok, 16);
      if (!ok)
      {
        QMessageBox::warning(this, tr("错误"),
                             tr("无效的HEX数据: %1").arg(byteString));
        return;
      }
      data.append(byte);
    }
  }
  else
  {
    // 文本格式发送
    data = text.toUtf8();
  }

  // 解析 Modbus 报文
  QString analysisResult = analyzeModbusFrame(data);
  
  // 检查并修复CRC（如果需要）
  QByteArray fixedData = data;
  bool crcFixed = validateAndFixCRC(fixedData);
  
  QString crcInfo;
  if (crcFixed) {
    crcInfo = tr("\n⚠️ CRC已自动修复为正确值");
  } else {
    crcInfo = tr("\n✓ CRC校验正确");
  }
  
  // 提供发送选项
  QMessageBox msgBox(this);
  msgBox.setWindowTitle(tr("Modbus报文分析"));
  msgBox.setText(tr("报文分析结果：\n\n%1%2\n\n是否发送此报文？").arg(analysisResult).arg(crcInfo));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.button(QMessageBox::Yes)->setText(tr("发送"));
  msgBox.button(QMessageBox::No)->setText(tr("仅分析"));
  msgBox.button(QMessageBox::Cancel)->setText(tr("取消"));
  
  int result = msgBox.exec();
  
  if (result == QMessageBox::Yes) {
    // 发送修复后的报文
    // 注意：直接串口发送需要访问底层串口，这里我们通过ModbusManager的底层接口
    appLog(tr("发送原始报文: %1").arg(QString(fixedData.toHex(' ').toUpper())), LOGType::INFO);
    
    // TODO: 实现直接串口发送（需要访问ModbusManager的底层串口）
    // 目前只记录到日志
    QMessageBox::information(this, tr("提示"), 
      tr("原始报文发送功能需要进一步实现底层串口访问。\n已修复的报文：%1")
      .arg(QString(fixedData.toHex(' ').toUpper())));
  }

  // 记录到日志
  if (logEdit)
  {
    QString logText;
    if (result == QMessageBox::Yes) {
      logText = QString("[%1] 报文分析并发送: %2\n分析结果: %3")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg((hexSendCheck && hexSendCheck->isChecked())
                    ? fixedData.toHex(' ').toUpper()
                    : text)
                .arg(analysisResult);
    } else {
      logText = QString("[%1] 报文分析 (未发送): %2\n分析结果: %3")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg((hexSendCheck && hexSendCheck->isChecked())
                    ? data.toHex(' ').toUpper()
                    : text)
                .arg(analysisResult);
    }
    
    logEdit->append(logText);

    if (autoScrollCheck && autoScrollCheck->isChecked())
    {
      logEdit->moveCursor(QTextCursor::End);
    }
  }

  // 清空输入框
  sendEdit->clear();
}

void SerialDialog::onResetCount()
{
  // 检查UI控件是否存在
  QLabel* rxLabel = findChild<QLabel*>("label_rxCount");
  QLabel* txLabel = findChild<QLabel*>("label_txCount");

  if (rxLabel)
  {
    rxLabel->setText(tr("接收: 0 字节"));
  }
  if (txLabel)
  {
    txLabel->setText(tr("发送: 0 字节"));
  }
}

/**
 * @brief 分析 Modbus RTU 报文
 * @param data 原始报文数据
 * @return 分析结果字符串
 */
QString SerialDialog::analyzeModbusFrame(const QByteArray& data)
{
  if (data.size() < 4) {
    return tr("报文长度不足 (最少需要4字节: 从站地址+功能码+数据+CRC)");
  }

  quint8 slaveAddr = static_cast<quint8>(data[0]);
  quint8 functionCode = static_cast<quint8>(data[1]);
  
  QString result = tr("从站地址: %1 (0x%2)\n")
                   .arg(slaveAddr)
                   .arg(slaveAddr, 2, 16, QChar('0')).toUpper();

  // 分析功能码
  QString functionDesc;
  switch (functionCode) {
    case 0x01:
      functionDesc = tr("读取线圈状态 (Read Coils)");
      if (data.size() >= 6) {
        quint16 startAddr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 quantity = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        functionDesc += tr("\n  起始地址: %1, 数量: %2").arg(startAddr).arg(quantity);
      }
      break;
      
    case 0x02:
      functionDesc = tr("读取离散输入 (Read Discrete Inputs)");
      if (data.size() >= 6) {
        quint16 startAddr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 quantity = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        functionDesc += tr("\n  起始地址: %1, 数量: %2").arg(startAddr).arg(quantity);
      }
      break;
      
    case 0x03:
      functionDesc = tr("读取保持寄存器 (Read Holding Registers)");
      if (data.size() >= 6) {
        quint16 startAddr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 quantity = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        functionDesc += tr("\n  起始地址: %1, 数量: %2").arg(startAddr).arg(quantity);
      }
      break;
      
    case 0x04:
      functionDesc = tr("读取输入寄存器 (Read Input Registers)");
      if (data.size() >= 6) {
        quint16 startAddr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 quantity = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        functionDesc += tr("\n  起始地址: %1, 数量: %2").arg(startAddr).arg(quantity);
      }
      break;
      
    case 0x05:
      functionDesc = tr("写入单个线圈 (Write Single Coil)");
      if (data.size() >= 6) {
        quint16 addr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 value = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        functionDesc += tr("\n  地址: %1, 值: %2 (%3)")
                        .arg(addr)
                        .arg(value, 4, 16, QChar('0')).toUpper()
                        .arg(value == 0xFF00 ? "ON" : "OFF");
      }
      break;
      
    case 0x06:
      functionDesc = tr("写入单个寄存器 (Write Single Register)");
      if (data.size() >= 6) {
        quint16 addr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 value = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        functionDesc += tr("\n  地址: %1, 值: %2").arg(addr).arg(value);
      }
      break;
      
    case 0x0F:
      functionDesc = tr("写入多个线圈 (Write Multiple Coils)");
      if (data.size() >= 7) {
        quint16 startAddr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 quantity = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        quint8 byteCount = static_cast<quint8>(data[6]);
        functionDesc += tr("\n  起始地址: %1, 数量: %2, 数据字节数: %3")
                        .arg(startAddr).arg(quantity).arg(byteCount);
      }
      break;
      
    case 0x10:
      functionDesc = tr("写入多个寄存器 (Write Multiple Registers)");
      if (data.size() >= 7) {
        quint16 startAddr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 quantity = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        quint8 byteCount = static_cast<quint8>(data[6]);
        functionDesc += tr("\n  起始地址: %1, 数量: %2, 数据字节数: %3")
                        .arg(startAddr).arg(quantity).arg(byteCount);
      }
      break;
      
    case 0x17:
      functionDesc = tr("读写多个寄存器 (Read/Write Multiple Registers)");
      break;
      
    case 0x16:
      functionDesc = tr("掩码写入寄存器 (Mask Write Register)");
      if (data.size() >= 8) {
        quint16 addr = (static_cast<quint8>(data[2]) << 8) | static_cast<quint8>(data[3]);
        quint16 andMask = (static_cast<quint8>(data[4]) << 8) | static_cast<quint8>(data[5]);
        quint16 orMask = (static_cast<quint8>(data[6]) << 8) | static_cast<quint8>(data[7]);
        functionDesc += tr("\n  地址: %1, AND掩码: 0x%2, OR掩码: 0x%3")
                        .arg(addr)
                        .arg(andMask, 4, 16, QChar('0')).toUpper()
                        .arg(orMask, 4, 16, QChar('0')).toUpper();
      }
      break;
      
    default:
      if (functionCode >= 0x80) {
        quint8 exceptionCode = functionCode & 0x7F;
        functionDesc = tr("异常响应 (Exception Response)");
        functionDesc += tr("\n  原功能码: 0x%1").arg(exceptionCode, 2, 16, QChar('0')).toUpper();
        if (data.size() >= 3) {
          quint8 exceptionType = static_cast<quint8>(data[2]);
          QString exceptionDesc;
          switch (exceptionType) {
            case 0x01: exceptionDesc = tr("非法功能码"); break;
            case 0x02: exceptionDesc = tr("非法数据地址"); break;
            case 0x03: exceptionDesc = tr("非法数据值"); break;
            case 0x04: exceptionDesc = tr("从站设备故障"); break;
            case 0x05: exceptionDesc = tr("确认"); break;
            case 0x06: exceptionDesc = tr("从站设备忙"); break;
            default: exceptionDesc = tr("未知异常"); break;
          }
          functionDesc += tr("\n  异常代码: 0x%1 (%2)")
                          .arg(exceptionType, 2, 16, QChar('0')).toUpper()
                          .arg(exceptionDesc);
        }
      } else {
        functionDesc = tr("未知功能码: 0x%1").arg(functionCode, 2, 16, QChar('0')).toUpper();
      }
      break;
  }

  result += tr("功能码: 0x%1 - %2\n")
            .arg(functionCode, 2, 16, QChar('0')).toUpper()
            .arg(functionDesc);

  // CRC 校验分析
  if (data.size() >= 2) {
    quint16 receivedCRC = (static_cast<quint8>(data[data.size()-1]) << 8) | 
                          static_cast<quint8>(data[data.size()-2]);
    
    // 计算正确的CRC
    quint16 calculatedCRC = calculateCRC16(reinterpret_cast<const quint8*>(data.constData()), data.size() - 2);
    
    result += tr("CRC校验: 接收=0x%1, 计算=0x%2")
              .arg(receivedCRC, 4, 16, QChar('0')).toUpper()
              .arg(calculatedCRC, 4, 16, QChar('0')).toUpper();
    
    if (receivedCRC == calculatedCRC) {
      result += tr(" ✓ 校验正确");
    } else {
      result += tr(" ✗ 校验错误");
    }
  }

  result += tr("\n原始数据: %1").arg(QString(data.toHex(' ')).toUpper());
  result += tr("\n数据长度: %1 字节").arg(data.size());

  return result;
}

quint16 SerialDialog::calculateCRC16(const quint8* data, int length)
{
  quint16 crc = 0xFFFF;
  
  for (int i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  
  return crc;
}

/**
 * @brief 验证并修复 Modbus 报文的 CRC 校验
 * @param data 报文数据（会被修改）
 * @return 是否需要修复CRC
 */
bool SerialDialog::validateAndFixCRC(QByteArray& data)
{
  if (data.size() < 4) {
    return false; // 报文太短
  }
  
  // 计算正确的CRC
  quint16 calculatedCRC = calculateCRC16(reinterpret_cast<const quint8*>(data.constData()), data.size() - 2);
  
  // 获取报文中的CRC (低字节在前)
  quint16 receivedCRC = (static_cast<quint8>(data[data.size()-1]) << 8) | 
                        static_cast<quint8>(data[data.size()-2]);
  
  if (calculatedCRC != receivedCRC) {
    // CRC不匹配，修复它
    data[data.size()-2] = static_cast<char>(calculatedCRC & 0xFF);        // 低字节
    data[data.size()-1] = static_cast<char>((calculatedCRC >> 8) & 0xFF); // 高字节
    return true; // 需要修复
  }
  
  return false; // CRC正确
}
