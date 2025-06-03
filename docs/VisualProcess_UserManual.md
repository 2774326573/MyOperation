# 📋 VisualProcess 类使用手册 | VisualProcess Class User Manual

## 🌟 概述 | Overview

`VisualProcess` 是一个基于 Qt 的视觉处理类，专门用于工业视觉检测、测量和二维码识别等任务。该类集成了 Halcon 图像处理库，提供了丰富的视觉处理功能和直观的用户界面。

The `VisualProcess` class is a Qt-based visual processing widget designed for industrial vision inspection, measurement, and QR code recognition tasks. It integrates the Halcon image processing library and provides comprehensive visual processing capabilities with an intuitive user interface.

---

## 🎯 主要功能 | Main Features

### 1. 🔍 二维码处理 | QR Code Processing
- **二维码识别** | QR Code Recognition
- **批量处理** | Batch Processing
- **模板创建** | Template Creation
- **参数配置** | Parameter Configuration

### 2. 📏 测量功能 | Measurement Functions
- **距离测量** | Distance Measurement
- **角度测量** | Angle Measurement
- **圆形测量** | Circle Measurement
- **轮廓分析** | Contour Analysis

### 3. 🔎 检测功能 | Detection Functions
- **特征匹配** | Feature Matching
- **缺陷检测** | Defect Detection
- **尺寸检测** | Size Detection
- **对齐检测** | Alignment Detection

### 4. 🚀 批处理支持 | Batch Processing Support
- **任务队列管理** | Task Queue Management
- **进度监控** | Progress Monitoring
- **结果统计** | Result Statistics
- **错误处理** | Error Handling

---

## 🏗️ 类结构 | Class Structure

### 📝 核心数据结构 | Core Data Structures

#### VisualTaskParams 任务参数结构体
```cpp
struct VisualTaskParams {
    QString taskType;           // 任务类型：QRCode, Measure, Detection
    QString templateName;       // 模板名称 | Template name
    QVariantMap parameters;     // 任务特定参数 | Task-specific parameters
    QString regionPath;         // 区域文件路径 | Region file path
    QString modelPath;          // 模型文件路径 | Model file path
    bool isValid = false;       // 参数是否有效 | Parameter validity
    
    // 二维码特定参数 | QR Code specific parameters
    QString qrCodeType;         
    QString qrPolarity;         
    
    // 测量特定参数 | Measurement specific parameters
    QString measureType;        
    int edgeThreshold;          
    double measurePrecision;    
    
    // 检测特定参数 | Detection specific parameters
    int maxContrast;            
    int minContrast;            
    double matchThreshold;      
    QString pyramidLevels;      
};
```

#### VisualProcessResult 处理结果结构体
```cpp
struct VisualProcessResult {
    QString taskType;           // 任务类型 | Task type
    bool success;               // 处理是否成功 | Processing success
    QString errorMessage;       // 错误信息 | Error message
    QVariantMap resultData;     // 结果数据 | Result data
    QString imagePath;          // 处理的图像路径 | Processed image path
    QDateTime processTime;      // 处理时间 | Processing time
    
    // 测量结果特定数据 | Measurement specific data
    double minDistance = 0.0;
    double maxDistance = 0.0;
    double centroidDistance = 0.0;
    
    // 二维码结果特定数据 | QR Code specific data
    QStringList decodedTexts;   
    QList<QPointF> codePositions; 
    
    // 检测结果特定数据 | Detection specific data
    int matchCount = 0;         
    QList<QPointF> matchPositions; 
    QList<double> matchScores;  
};
```

---

## 🚀 快速开始 | Quick Start

### 1. 创建实例 | Creating Instance
```cpp
// 创建 VisualProcess 实例
VisualProcess* visualProcess = new VisualProcess(this);

// 显示界面
visualProcess->show();
```

### 2. 信号连接 | Signal Connection
```cpp
// 连接主要信号
connect(visualProcess, &VisualProcess::taskParamsReady,
        this, &MainWindow::onTaskParamsReady);
        
connect(visualProcess, &VisualProcess::processingRequested,
        this, &MainWindow::onProcessingRequested);
        
connect(visualProcess, &VisualProcess::resultGenerated,
        this, &MainWindow::onResultGenerated);
```

---

## 📖 详细使用指南 | Detailed Usage Guide

### 🔍 二维码处理模式 | QR Code Processing Mode

#### 1. 进入二维码模式
```cpp
// 通过下拉框选择二维码模式
// 触发 onComboBoxAction(QRCODE) 槽函数
visualProcess->onComboBoxAction(VisualProcess::QRCODE);
```

#### 2. 创建二维码模板
```cpp
// 点击创建模板按钮
// 这将打开模板创建窗口，允许用户：
// - 选择二维码类型 | Select QR code type
// - 设置极性参数 | Set polarity parameters
// - 保存模板配置 | Save template configuration
```

#### 3. 获取任务参数
```cpp
VisualTaskParams params = visualProcess->getCurrentTaskParams();
if (params.isValid && params.taskType == "QRCode") {
    // 使用参数进行处理
    QString qrType = params.qrCodeType;
    QString polarity = params.qrPolarity;
}
```

### 📏 测量功能模式 | Measurement Mode

#### 1. 进入测量模式
```cpp
visualProcess->onComboBoxAction(VisualProcess::MEASURE);
```

#### 2. 配置测量参数
```cpp
// 获取当前测量参数
VisualTaskParams params = visualProcess->getCurrentTaskParams();
params.measureType = "Distance";        // 测量类型
params.edgeThreshold = 30;              // 边缘阈值
params.measurePrecision = 0.01;         // 测量精度
```

#### 3. 创建测量模板
```cpp
// 通过 UI 创建测量模板
// 系统会自动保存模板信息到 JSON 文件
```

### 🔎 检测功能模式 | Detection Mode

#### 1. 进入检测模式
```cpp
visualProcess->onComboBoxAction(VisualProcess::CHECK);
```

#### 2. 配置检测参数
```cpp
VisualTaskParams params = visualProcess->getCurrentTaskParams();
params.maxContrast = 255;               // 最大对比度
params.minContrast = 0;                 // 最小对比度
params.matchThreshold = 0.8;            // 匹配阈值
params.pyramidLevels = "auto";          // 金字塔层数
```

---

## 🚀 批处理功能 | Batch Processing

### 1. 启动批处理
```cpp
// 配置好任务参数后启动批处理
visualProcess->startBatchProcessing();
```

### 2. 监控进度
```cpp
// 连接进度信号
connect(visualProcess, &VisualProcess::processingProgress,
        [](int current, int total, const QString& currentFile) {
    qDebug() << QString("处理进度: %1/%2, 当前文件: %3")
                .arg(current).arg(total).arg(currentFile);
});
```

### 3. 停止批处理
```cpp
visualProcess->stopBatchProcessing();
```

---

## 🎛️ 高级功能 | Advanced Features

### 1. 参数验证 | Parameter Validation
```cpp
VisualTaskParams params = visualProcess->getCurrentTaskParams();
QString errorMessage;
bool isValid = visualProcess->validateTaskParams(params, errorMessage);
if (!isValid) {
    qWarning() << "参数验证失败:" << errorMessage;
}
```

### 2. 结果处理 | Result Processing
```cpp
void MainWindow::onResultGenerated(const VisualProcessResult& result) {
    if (result.success) {
        if (result.taskType == "QRCode") {
            // 处理二维码结果
            for (const QString& text : result.decodedTexts) {
                qDebug() << "解码结果:" << text;
            }
        } else if (result.taskType == "Measure") {
            // 处理测量结果
            qDebug() << "最小距离:" << result.minDistance;
            qDebug() << "最大距离:" << result.maxDistance;
        }
    } else {
        qWarning() << "处理失败:" << result.errorMessage;
    }
}
```

### 3. 模板管理 | Template Management
```cpp
// 模板保存在以下位置：
// QRCode 模板: config/templates/qrcode/
// Measure 模板: config/templates/measure/
// Detection 模板: config/templates/detection/

// 模板文件格式为 JSON，包含所有必要的配置信息
```

---

## ⚠️ 注意事项 | Important Notes

### 🔧 初始化要求 | Initialization Requirements
1. **Halcon 库依赖** | Halcon Library Dependency
   - 确保 Halcon 库正确安装和配置
   - 验证 HalconCpp 头文件路径

2. **文件路径配置** | File Path Configuration
   - 设置正确的模板保存路径
   - 确保图像文件路径可访问

### 🎯 性能优化建议 | Performance Optimization Tips

1. **图像预处理** | Image Preprocessing
   ```cpp
   // 在处理前对图像进行预处理以提高性能
   // 建议的图像大小：不超过 2048x2048
   ```

2. **批处理优化** | Batch Processing Optimization
   ```cpp
   // 为大批量处理设置合理的线程数
   // 监控内存使用情况
   ```

### 🛠️ 故障排除 | Troubleshooting

#### 常见问题 | Common Issues

1. **模板创建失败** | Template Creation Failed
   ```cpp
   // 检查路径权限
   // 验证图像格式支持
   // 确认 Halcon 库版本兼容性
   ```

2. **处理结果异常** | Abnormal Processing Results
   ```cpp
   // 检查输入参数有效性
   // 验证图像质量
   // 调整处理参数
   ```

---

## 📊 API 参考 | API Reference

### 🔗 主要公有方法 | Main Public Methods

| 方法名 | 参数 | 返回值 | 描述 |
|--------|------|--------|------|
| `getCurrentTaskParams()` | 无 | `VisualTaskParams` | 获取当前任务参数 |
| `validateTaskParams()` | `params`, `errorMsg` | `bool` | 验证任务参数 |
| `startBatchProcessing()` | 无 | `void` | 启动批处理 |
| `stopBatchProcessing()` | 无 | `void` | 停止批处理 |

### 📡 主要信号 | Main Signals

| 信号名 | 参数 | 描述 |
|--------|------|------|
| `taskParamsReady` | `VisualTaskParams` | 任务参数准备就绪 |
| `processingRequested` | `VisualTaskParams` | 请求开始处理 |
| `processingStopRequested` | 无 | 请求停止处理 |
| `resultGenerated` | `VisualProcessResult` | 生成处理结果 |

### 🎰 主要槽函数 | Main Slots

| 槽函数名 | 参数 | 描述 |
|----------|------|------|
| `onComboBoxAction` | `int index` | 处理模式选择 |
| `onProcessingResult` | `VisualProcessResult` | 处理结果接收 |
| `onProcessingProgress` | `int`, `int`, `QString` | 处理进度更新 |

---

## 🎨 示例代码 | Example Code

### 完整使用示例 | Complete Usage Example

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupVisualProcess();
    }
    
private:
    void setupVisualProcess() {
        // 创建 VisualProcess 实例
        visualProcess = new VisualProcess(this);
        
        // 连接信号槽
        connect(visualProcess, &VisualProcess::taskParamsReady,
                this, &MainWindow::onTaskParamsReady);
        connect(visualProcess, &VisualProcess::processingRequested,
                this, &MainWindow::onProcessingRequested);
        connect(visualProcess, &VisualProcess::resultGenerated,
                this, &MainWindow::onResultGenerated);
        
        // 显示界面
        visualProcess->show();
    }
    
private slots:
    void onTaskParamsReady(const VisualTaskParams& params) {
        qDebug() << "任务参数准备完成，类型:" << params.taskType;
        
        // 验证参数
        QString errorMsg;
        if (!visualProcess->validateTaskParams(params, errorMsg)) {
            QMessageBox::warning(this, "参数错误", errorMsg);
            return;
        }
        
        // 开始处理
        emit visualProcess->processingRequested(params);
    }
    
    void onProcessingRequested(const VisualTaskParams& params) {
        qDebug() << "开始处理任务:" << params.taskType;
        // 在这里调用实际的处理逻辑
    }
    
    void onResultGenerated(const VisualProcessResult& result) {
        if (result.success) {
            qDebug() << "处理成功完成";
            displayResults(result);
        } else {
            qWarning() << "处理失败:" << result.errorMessage;
        }
    }
    
    void displayResults(const VisualProcessResult& result) {
        // 根据任务类型显示不同的结果
        if (result.taskType == "QRCode") {
            for (const QString& text : result.decodedTexts) {
                ui->resultList->addItem("二维码内容: " + text);
            }
        } else if (result.taskType == "Measure") {
            ui->distanceLabel->setText(
                QString("距离: %1 - %2 mm")
                .arg(result.minDistance, 0, 'f', 2)
                .arg(result.maxDistance, 0, 'f', 2)
            );
        }
    }
    
private:
    VisualProcess* visualProcess;
    Ui::MainWindow* ui;
};
```

---

## 🎓 最佳实践 | Best Practices

### 1. 🏗️ 项目结构建议 | Recommended Project Structure
```
project/
├── src/                    # 源代码
├── inc/                    # 头文件
├── config/                 # 配置文件
│   ├── templates/          # 模板文件
│   │   ├── qrcode/        # 二维码模板
│   │   ├── measure/       # 测量模板
│   │   └── detection/     # 检测模板
│   └── params/            # 参数文件
├── images/                # 测试图像
└── results/              # 处理结果
```

### 2. 🔄 错误处理模式 | Error Handling Pattern
```cpp
// 推荐的错误处理方式
try {
    VisualTaskParams params = visualProcess->getCurrentTaskParams();
    QString errorMsg;
    
    if (!visualProcess->validateTaskParams(params, errorMsg)) {
        throw std::runtime_error(errorMsg.toStdString());
    }
    
    // 执行处理...
    
} catch (const std::exception& e) {
    qCritical() << "视觉处理错误:" << e.what();
    // 记录日志并通知用户
}
```

### 3. 📝 日志记录建议 | Logging Recommendations
```cpp
// 在关键处理节点添加日志
qDebug() << "开始处理图像:" << imagePath;
qInfo() << "模板加载成功:" << templateName;
qWarning() << "处理参数可能不合适:" << paramValue;
qCritical() << "严重错误，处理失败:" << errorDetails;
```

---

## 📞 技术支持 | Technical Support

### 🐛 问题报告 | Bug Reports
如果您遇到任何问题，请提供以下信息：
- Qt 版本和编译器信息
- Halcon 库版本
- 详细的错误信息和日志
- 复现步骤

### 📧 联系方式 | Contact Information
- 开发团队邮箱：dev-team@company.com
- 技术支持热线：400-xxx-xxxx
- 在线文档：https://docs.company.com/visualprocess

---

*📅 最后更新时间 | Last Updated: 2024-12-19*
*📝 文档版本 | Document Version: v1.0.0*
*👨‍💻 作者 | Author: Development Team* 