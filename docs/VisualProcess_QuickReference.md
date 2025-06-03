# 🚀 VisualProcess 快速参考 | Quick Reference Card

## 🔗 核心连接模式 | Core Connection Pattern

```cpp
// 🎯 标准初始化模式 | Standard Initialization Pattern
VisualProcess* vp = new VisualProcess(this);

// 🔌 必要信号连接 | Essential Signal Connections
connect(vp, &VisualProcess::taskParamsReady, this, &YourClass::onTaskReady);
connect(vp, &VisualProcess::processingRequested, this, &YourClass::onProcessStart);
connect(vp, &VisualProcess::resultGenerated, this, &YourClass::onResult);
connect(vp, &VisualProcess::processingStopRequested, this, &YourClass::onStop);
```

---

## 🎛️ 工作模式切换 | Mode Switching

| 模式 | 代码 | 用途 |
|------|------|------|
| 🔍 二维码 | `vp->onComboBoxAction(VisualProcess::QRCODE)` | QR码识别 |
| 📏 测量 | `vp->onComboBoxAction(VisualProcess::MEASURE)` | 距离/角度测量 |
| 🔎 检测 | `vp->onComboBoxAction(VisualProcess::CHECK)` | 特征匹配/缺陷检测 |

---

## 📋 参数获取与验证 | Parameter Retrieval & Validation

```cpp
// ✅ 获取并验证参数 | Get & Validate Parameters
VisualTaskParams params = vp->getCurrentTaskParams();
QString error;
bool valid = vp->validateTaskParams(params, error);

if (valid) {
    // 🎯 处理有效参数 | Process valid parameters
    switch (params.taskType) {
        case "QRCode":
            // 使用 params.qrCodeType, params.qrPolarity
            break;
        case "Measure": 
            // 使用 params.measureType, params.edgeThreshold
            break;
        case "Detection":
            // 使用 params.matchThreshold, params.maxContrast
            break;
    }
} else {
    qWarning() << "参数错误:" << error;
}
```

---

## 🚀 批处理操作 | Batch Operations

```cpp
// ▶️ 启动批处理 | Start Batch Processing
vp->startBatchProcessing();

// ⏸️ 停止批处理 | Stop Batch Processing  
vp->stopBatchProcessing();

// 📊 进度监控 | Progress Monitoring
connect(vp, &VisualProcess::processingProgress,
    [](int current, int total, const QString& file) {
        qDebug() << QString("进度: %1/%2, 文件: %3")
                    .arg(current).arg(total).arg(file);
    });
```

---

## 📊 结果处理模板 | Result Processing Template

```cpp
void YourClass::onResultGenerated(const VisualProcessResult& result) {
    if (!result.success) {
        qWarning() << "处理失败:" << result.errorMessage;
        return;
    }
    
    // 🎯 根据任务类型处理结果 | Process by task type
    if (result.taskType == "QRCode") {
        // 📱 二维码结果
        for (const QString& text : result.decodedTexts) {
            ui->resultList->addItem("QR: " + text);
        }
        
    } else if (result.taskType == "Measure") {
        // 📏 测量结果
        ui->minDistanceLabel->setText(QString::number(result.minDistance, 'f', 2));
        ui->maxDistanceLabel->setText(QString::number(result.maxDistance, 'f', 2));
        
    } else if (result.taskType == "Detection") {
        // 🔍 检测结果
        ui->matchCountLabel->setText(QString::number(result.matchCount));
        // 显示匹配位置 result.matchPositions
    }
}
```

---

## ⚡ 常用代码片段 | Common Code Snippets

### 🏗️ 基础设置 | Basic Setup
```cpp
// 创建实例并显示
VisualProcess* visualProcess = new VisualProcess(this);
visualProcess->show();
```

### 🔧 错误处理 | Error Handling
```cpp
try {
    VisualTaskParams params = visualProcess->getCurrentTaskParams();
    // ... 处理逻辑
} catch (const std::exception& e) {
    qCritical() << "错误:" << e.what();
}
```

### 📝 日志记录 | Logging
```cpp
qDebug() << "开始处理:" << imagePath;      // 调试信息
qInfo() << "模板加载:" << templateName;    // 常规信息  
qWarning() << "参数异常:" << paramValue;   // 警告
qCritical() << "严重错误:" << errorMsg;    // 错误
```

---

## 🎯 任务参数速查 | Task Parameters Quick Reference

### 🔍 二维码参数 | QR Code Parameters
```cpp
params.taskType = "QRCode";
params.qrCodeType = "QR_Code";     // 二维码类型
params.qrPolarity = "light_on_dark"; // 极性
```

### 📏 测量参数 | Measurement Parameters  
```cpp
params.taskType = "Measure";
params.measureType = "Distance";   // 测量类型
params.edgeThreshold = 30;         // 边缘阈值
params.measurePrecision = 0.01;    // 测量精度
```

### 🔎 检测参数 | Detection Parameters
```cpp
params.taskType = "Detection";
params.maxContrast = 255;          // 最大对比度
params.minContrast = 0;            // 最小对比度  
params.matchThreshold = 0.8;       // 匹配阈值
params.pyramidLevels = "auto";     // 金字塔层数
```

---

## 🚨 常见问题速查 | Common Issues Quick Fix

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| 模板创建失败 | 路径权限 | 检查文件夹写权限 |
| 参数验证失败 | 必要参数缺失 | 检查 `isValid` 标志 |
| 处理结果异常 | 图像质量问题 | 调整预处理参数 |
| 批处理中断 | 内存不足 | 减少批处理数量 |

---

## 📁 文件路径约定 | File Path Conventions

```
config/
├── templates/
│   ├── qrcode/           # 二维码模板
│   ├── measure/          # 测量模板
│   └── detection/        # 检测模板
├── params/               # 参数配置
└── logs/                # 日志文件
```

---

## 🎪 演示代码 | Demo Code

```cpp
class QuickDemo : public QWidget {
public:
    QuickDemo() {
        auto* vp = new VisualProcess(this);
        
        // 快速设置二维码模式
        vp->onComboBoxAction(VisualProcess::QRCODE);
        
        // 连接结果处理
        connect(vp, &VisualProcess::resultGenerated, 
            [](const VisualProcessResult& result) {
                if (result.success && result.taskType == "QRCode") {
                    for (const QString& text : result.decodedTexts) {
                        qDebug() << "发现二维码:" << text;
                    }
                }
            });
            
        vp->show();
    }
};
```

---

*�� 提示：保存此文件作为日常开发参考！* 