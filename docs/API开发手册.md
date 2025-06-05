# MyOperation API开发手册

## 概述

MyOperation是一个基于Qt的机器视觉应用系统，集成了Halcon视觉库和Modbus通信功能。本手册详细介绍了系统核心API的使用方法和开发指南。

## 系统架构

### 核心组件
- **UI层**: 主窗口、可视化处理界面、串口通信对话框
- **配置管理**: 设置管理器、配置管理器
- **通信层**: Modbus通信管理器（TCP/RTU）
- **视觉处理**: Halcon集成组件、视觉工作线程
- **第三方库**: 日志管理、动态UI构建器

## 核心API参考

### 1. 主窗口API (MainWindow)

#### 类定义
```cpp
class MainWindow : public QMainWindow
```

#### 主要功能
- 应用程序主界面管理
- 菜单和工具栏控制
- 子窗口管理
- 状态栏信息显示

#### 关键方法

##### 构造函数
```cpp
MainWindow(QWidget *parent = nullptr);
```

##### 窗口初始化
```cpp
void initializeWindow();
void setupMenuBar();
void setupToolBar();
void setupStatusBar();
```

##### 事件处理
```cpp
void closeEvent(QCloseEvent *event) override;
void resizeEvent(QResizeEvent *event) override;
```

#### 使用示例
```cpp
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

### 2. 配置管理API (SettingManager)

#### 类定义
```cpp
class SettingManager : public QObject
```

#### 主要功能
- 应用程序配置的读取和保存
- 参数验证和类型转换
- 配置文件管理
- 默认值处理

#### 关键方法

##### 单例获取
```cpp
static SettingManager* getInstance();
```

##### 配置读取
```cpp
QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant());
QString getString(const QString& key, const QString& defaultValue = QString());
int getInt(const QString& key, int defaultValue = 0);
bool getBool(const QString& key, bool defaultValue = false);
double getDouble(const QString& key, double defaultValue = 0.0);
```

##### 配置保存
```cpp
void setValue(const QString& key, const QVariant& value);
void setString(const QString& key, const QString& value);
void setInt(const QString& key, int value);
void setBool(const QString& key, bool value);
void setDouble(const QString& key, double value);
```

##### 配置文件管理
```cpp
bool loadFromFile(const QString& filePath);
bool saveToFile(const QString& filePath);
void sync();
```

#### 使用示例
```cpp
#include "SettingManager.h"

// 获取配置管理器实例
SettingManager* settings = SettingManager::getInstance();

// 读取配置
QString serverIP = settings->getString("network/server_ip", "127.0.0.1");
int serverPort = settings->getInt("network/server_port", 502);
bool autoConnect = settings->getBool("network/auto_connect", false);

// 保存配置
settings->setString("network/server_ip", "192.168.1.100");
settings->setInt("network/server_port", 1502);
settings->setBool("network/auto_connect", true);
settings->sync();
```

### 3. 视觉处理API (VisualProcess)

#### 类定义
```cpp
class VisualProcess : public QWidget
```

#### 主要功能
- 图像显示和处理界面
- 视觉算法参数配置
- 实时图像预览
- 处理结果显示

#### 关键方法

##### 图像处理
```cpp
void loadImage(const QString& imagePath);
void processImage();
void clearImage();
```

##### 参数设置
```cpp
void setProcessingParameters(const QVariantMap& params);
QVariantMap getProcessingParameters() const;
```

##### 结果获取
```cpp
QList<QPointF> getDetectedPoints() const;
QRectF getBoundingBox() const;
double getMeasuredValue() const;
```

#### 信号
```cpp
signals:
    void imageLoaded(const QString& imagePath);
    void processingCompleted(bool success);
    void measurementReady(double value);
    void errorOccurred(const QString& error);
```

#### 使用示例
```cpp
#include "visualprocess.h"

VisualProcess* visualWidget = new VisualProcess(this);

// 连接信号槽
connect(visualWidget, &VisualProcess::processingCompleted,
        this, &MainWindow::onProcessingCompleted);
connect(visualWidget, &VisualProcess::measurementReady,
        this, &MainWindow::onMeasurementReady);

// 加载和处理图像
visualWidget->loadImage("test_image.bmp");
visualWidget->processImage();
```

### 4. 视觉工作线程API (VisualWorkThread)

#### 类定义
```cpp
class VisualWorkThread : public QThread
```

#### 主要功能
- 后台图像处理
- 多线程视觉算法执行
- 处理结果异步返回
- 线程安全的数据交换

#### 关键方法

##### 线程控制
```cpp
void startProcessing(const QString& imagePath);
void stopProcessing();
void pauseProcessing();
void resumeProcessing();
```

##### 参数设置
```cpp
void setWorkingDirectory(const QString& directory);
void setProcessingMode(ProcessingMode mode);
void setThreadPriority(QThread::Priority priority);
```

#### 信号
```cpp
signals:
    void processingStarted();
    void processingFinished(const ProcessingResult& result);
    void progressUpdated(int percentage);
    void errorOccurred(const QString& error);
```

#### 使用示例
```cpp
#include "visualWorkThread.h"

VisualWorkThread* workThread = new VisualWorkThread(this);

// 连接信号槽
connect(workThread, &VisualWorkThread::processingFinished,
        this, &MainWindow::onProcessingFinished);
connect(workThread, &VisualWorkThread::progressUpdated,
        this, &MainWindow::onProgressUpdated);

// 启动处理
workThread->setWorkingDirectory("./images");
workThread->startProcessing("sample.bmp");
```

### 5. 串口通信API (SerialDialog)

#### 类定义
```cpp
class SerialDialog : public QDialog
```

#### 主要功能
- 串口设备管理
- 通信参数配置
- 数据发送和接收
- 通信状态监控

#### 关键方法

##### 串口控制
```cpp
bool openSerialPort();
void closeSerialPort();
bool isSerialPortOpen() const;
```

##### 数据传输
```cpp
qint64 sendData(const QByteArray& data);
QByteArray receiveData();
void clearBuffer();
```

##### 配置管理
```cpp
void setPortName(const QString& portName);
void setBaudRate(qint32 baudRate);
void setDataBits(QSerialPort::DataBits dataBits);
void setParity(QSerialPort::Parity parity);
void setStopBits(QSerialPort::StopBits stopBits);
```

#### 信号
```cpp
signals:
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& error);
    void connectionStateChanged(bool connected);
```

#### 使用示例
```cpp
#include "serialdialog.h"

SerialDialog* serialDialog = new SerialDialog(this);

// 配置串口参数
serialDialog->setPortName("COM1");
serialDialog->setBaudRate(9600);
serialDialog->setDataBits(QSerialPort::Data8);
serialDialog->setParity(QSerialPort::NoParity);
serialDialog->setStopBits(QSerialPort::OneStop);

// 连接信号槽
connect(serialDialog, &SerialDialog::dataReceived,
        this, &MainWindow::onSerialDataReceived);

// 打开串口并发送数据
if (serialDialog->openSerialPort()) {
    QByteArray command = "READ_STATUS\r\n";
    serialDialog->sendData(command);
}
```

## 错误处理

### 异常类型
- **ConfigurationError**: 配置相关错误
- **CommunicationError**: 通信相关错误
- **ProcessingError**: 图像处理相关错误
- **HardwareError**: 硬件设备相关错误

### 错误码定义
```cpp
enum ErrorCode {
    NO_ERROR = 0,
    CONFIG_FILE_NOT_FOUND = 1001,
    INVALID_PARAMETER = 1002,
    COMMUNICATION_TIMEOUT = 2001,
    CONNECTION_FAILED = 2002,
    IMAGE_LOAD_FAILED = 3001,
    PROCESSING_FAILED = 3002,
    HARDWARE_NOT_FOUND = 4001,
    DEVICE_BUSY = 4002
};
```

### 错误处理示例
```cpp
try {
    visualWidget->processImage();
} catch (const ProcessingError& e) {
    qWarning() << "Processing error:" << e.what();
    // 处理错误逻辑
}
```

## 最佳实践

### 1. 内存管理
- 使用智能指针管理对象生命周期
- 及时释放大型图像数据
- 避免内存泄漏

### 2. 线程安全
- 使用互斥锁保护共享数据
- 通过信号槽进行线程间通信
- 避免直接跨线程访问UI

### 3. 错误处理
- 始终检查函数返回值
- 使用异常处理机制
- 提供详细的错误信息

### 4. 性能优化
- 缓存频繁使用的配置
- 使用异步处理避免界面阻塞
- 优化图像处理算法

## 版本兼容性

- Qt版本: 5.12+
- Halcon版本: 20.11+
- libmodbus版本: 3.1.1+
- 编译器: MSVC 2019+

## 更新日志

### v1.0.0 (2025-06-05)
- 初始版本发布
- 完整的API文档
- 基础示例代码

---

*本文档最后更新时间: 2025年6月5日*
