# 视觉处理工作流程指南 | Visual Processing Workflow Guide

## 📋 概述 | Overview

本系统实现了一个完整的视觉处理工作流程，通过三个核心类的协作来完成从参数配置到结果显示的全过程：

- **VisualProcess** - 用户交互和参数配置界面
- **VisualWorkThread** - 后台处理和算法执行引擎  
- **MainWindow** - 结果显示和整体控制中心

## 🏗️ 系统架构 | System Architecture

```
┌─────────────────┐    信号/槽    ┌─────────────────┐    信号/槽    ┌─────────────────┐
│   VisualProcess │ ──────────→ │   MainWindow    │ ──────────→ │ VisualWorkThread│
│   (参数配置)     │             │   (协调控制)     │             │   (算法处理)     │
└─────────────────┘             └─────────────────┘             └─────────────────┘
        ↑                               ↓                               ↓
        │                        ┌─────────────┐                ┌─────────────┐
        └────── 结果反馈 ──────────│ 结果显示     │←───── 处理结果 ──│ 算法输出     │
                                └─────────────┘                └─────────────┘
```

## 🔄 工作流程 | Workflow

### 1. 参数配置阶段 (VisualProcess)

#### 🎯 支持的任务类型
- **二维码识别** (QRCode): 识别和解码各种二维码
- **测量任务** (Measure): 距离、角度、轮廓测量
- **检测任务** (Detection): 形状匹配和缺陷检测

#### 📝 参数配置步骤
1. **选择任务模式**: 在下拉框中选择二维码/测量/检测
2. **创建模板**: 点击"创建模板"按钮
3. **配置参数**: 
   - 设置模板名称
   - 配置任务特定参数
   - 绘制识别/测量区域
4. **启动处理**: 点击"开始批处理"按钮

#### 🔧 参数结构 (VisualTaskParams)
```cpp
struct VisualTaskParams {
    QString taskType;           // 任务类型：QRCode, Measure, Detection
    QString templateName;       // 模板名称
    QString regionPath;         // 区域文件路径（HOBJ格式）
    QString modelPath;          // 模型文件路径
    
    // 二维码特定参数
    QString qrCodeType;         // 二维码类型
    QString qrPolarity;         // 极性设置
    
    // 测量特定参数
    QString measureType;        // 测量类型
    int edgeThreshold;          // 边缘阈值
    double measurePrecision;    // 测量精度
    
    // 检测特定参数
    int maxContrast;            // 最大对比度
    int minContrast;            // 最小对比度
    double matchThreshold;      // 匹配阈值
    QString pyramidLevels;      // 金字塔层数
};
```

### 2. 协调控制阶段 (MainWindow)

#### 🔗 信号槽连接
MainWindow作为中央协调器，负责连接VisualProcess和VisualWorkThread：

```cpp
// VisualProcess -> MainWindow
connect(m_visualProcess, &VisualProcess::processingRequested, 
        this, &Mainwindow::onProcessingRequested);

// MainWindow -> VisualWorkThread  
connect(this, &Mainwindow::startProcessing,
        m_visualWorkThread, &visualWorkThread::startBatchProcessing);

// VisualWorkThread -> MainWindow -> VisualProcess
connect(m_visualWorkThread, &visualWorkThread::processingResult,
        this, &Mainwindow::onProcessingResult);
```

#### 📊 协调功能
- **参数传递**: 将VisualProcess的参数传递给VisualWorkThread
- **任务启动**: 根据任务类型启动相应的处理流程
- **进度监控**: 实时显示处理进度和统计信息
- **结果分发**: 将处理结果分发给显示组件

### 3. 算法处理阶段 (VisualWorkThread)

#### 🚀 处理流程
1. **接收参数**: 通过`setTaskParams()`接收任务参数
2. **验证参数**: 检查参数完整性和有效性
3. **批量处理**: 遍历图像文件夹中的所有图像
4. **算法执行**: 根据任务类型调用相应的处理方法
5. **结果输出**: 发送处理结果和统计信息

#### 🔍 算法实现

##### 二维码检测 (processQRCodeDetection)
```cpp
VisualProcessResult visualWorkThread::processQRCodeDetection(const HObject& image, const QString& imagePath)
{
    // 1. 加载识别区域
    HObject qrRegion;
    ReadObject(&qrRegion, params.regionPath.toStdString().c_str());
    
    // 2. 创建二维码数据模型
    HTuple dataCodeHandle;
    CreateDataCode2dModel(params.qrCodeType.toStdString().c_str(), 
                         "default_parameters", "maximum_recognition", &dataCodeHandle);
    
    // 3. 查找二维码
    HTuple resultHandles, decodedDataStrings;
    HObject symbolXLDs;
    FindDataCode2d(image, &symbolXLDs, dataCodeHandle, "stop_after_first", 
                   qrRegion, &resultHandles, &decodedDataStrings);
    
    // 4. 处理结果
    return createSuccessResult("QRCode", imagePath, resultData);
}
```

##### 测量处理 (processMeasurement)
```cpp
VisualProcessResult visualWorkThread::processMeasurement(const HObject& image, const QString& imagePath)
{
    // 1. 加载测量区域
    HObject measureRegion1, measureRegion2;
    ReadObject(&measureRegion1, regionPath1.toStdString().c_str());
    ReadObject(&measureRegion2, regionPath2.toStdString().c_str());
    
    // 2. 提取轮廓
    HObject xld1 = workThreadHalcon->QtGetLengthMaxXld(image, measureRegion1, edgeThreshold);
    HObject xld2 = workThreadHalcon->QtGetLengthMaxXld(image, measureRegion2, edgeThreshold);
    
    // 3. 计算距离
    HTuple disMin, disMax;
    DistanceCc(xld1, xld2, "point_to_point", &disMin, &disMax);
    
    // 4. 返回结果
    return createSuccessResult("Measure", imagePath, resultData);
}
```

##### 形状检测 (processShapeDetection)
```cpp
VisualProcessResult visualWorkThread::processShapeDetection(const HObject& image, const QString& imagePath)
{
    // 1. 加载形状模型
    HTuple modelID;
    ReadShapeModel(params.modelPath.toStdString().c_str(), &modelID);
    
    // 2. 查找形状模型
    HTuple row, column, angle, score;
    FindShapeModel(image, modelID, HTuple(-M_PI).TupleRad(), HTuple(2*M_PI).TupleRad(), 
                   matchThreshold, 0, 0.5, "least_squares", 0, 0.9, 
                   &row, &column, &angle, &score);
    
    // 3. 处理匹配结果
    return createSuccessResult("Detection", imagePath, resultData);
}
```

### 4. 结果显示阶段

#### 📊 结果结构 (VisualProcessResult)
```cpp
struct VisualProcessResult {
    QString taskType;           // 任务类型
    bool success;               // 处理是否成功
    QString errorMessage;       // 错误信息
    QVariantMap resultData;     // 结果数据
    QString imagePath;          // 处理的图像路径
    QDateTime processTime;      // 处理时间
    
    // 任务特定结果数据
    QStringList decodedTexts;   // 二维码解码文本
    QList<QPointF> codePositions; // 二维码位置
    double minDistance, maxDistance, centroidDistance; // 测量结果
    int matchCount;             // 检测匹配数量
    QList<QPointF> matchPositions; // 匹配位置
    QList<double> matchScores;  // 匹配分数
};
```

#### 🎨 显示方式
- **VisualProcess**: 在日志列表和图像窗口显示详细结果
- **MainWindow**: 在主窗口图像区域显示汇总信息
- **统计信息**: 实时更新处理进度、成功率等统计数据

## 🛠️ 使用指南 | Usage Guide

### 快速开始

1. **启动应用程序**
   ```cpp
   // 在MainWindow构造函数中自动初始化
   initVisionProcess();
   initThread();
   initVisualWorkflow();
   ```

2. **打开视觉处理界面**
   - 点击主窗口的"视觉处理"按钮
   - 选择任务类型（二维码/测量/检测）

3. **创建处理模板**
   - 点击"创建模板"按钮
   - 加载参考图像
   - 绘制识别/测量区域
   - 配置算法参数
   - 保存模板

4. **执行批处理**
   - 将待处理图像放入`/img`文件夹
   - 点击"开始批处理"按钮
   - 实时查看处理进度和结果

### 高级功能

#### 🔧 自定义算法参数
```cpp
// 二维码识别参数
params.qrCodeType = "Data Matrix ECC 200";
params.qrPolarity = "light_on_dark";

// 测量参数
params.measureType = "edge_detection";
params.edgeThreshold = 30;
params.measurePrecision = 0.01;

// 检测参数
params.maxContrast = 128;
params.minContrast = 30;
params.matchThreshold = 0.7;
params.pyramidLevels = "auto";
```

#### 📁 文件管理
- **模板文件**: 保存在`/config/models/`目录
- **区域文件**: 使用HOBJ通用格式，支持中文文件名
- **参数文件**: 使用Halcon Tuple格式保存
- **结果导出**: 支持CSV格式导出测量数据

#### 🔍 结果分析
```cpp
// 访问处理结果
void onProcessingResult(const VisualProcessResult& result) {
    if (result.success) {
        if (result.taskType == "QRCode") {
            // 处理二维码结果
            for (const QString& text : result.decodedTexts) {
                qDebug() << "解码内容:" << text;
            }
        }
        else if (result.taskType == "Measure") {
            // 处理测量结果
            qDebug() << "距离:" << result.minDistance << "-" << result.maxDistance;
        }
        else if (result.taskType == "Detection") {
            // 处理检测结果
            qDebug() << "匹配数量:" << result.matchCount;
        }
    }
}
```

## 🚨 错误处理 | Error Handling

### 常见错误类型
1. **参数验证失败**: 检查模板名称、文件路径等
2. **文件读取失败**: 确认图像文件格式和路径
3. **算法执行失败**: 检查Halcon许可证和算法参数
4. **内存不足**: 处理大图像时可能出现

### 错误恢复机制
- **自动重试**: 对于临时性错误自动重试
- **跳过处理**: 单个图像失败不影响批处理继续
- **详细日志**: 记录详细的错误信息和堆栈跟踪
- **用户提示**: 在界面显示友好的错误提示

## 📈 性能优化 | Performance Optimization

### 处理速度优化
- **多线程处理**: VisualWorkThread在独立线程中运行
- **内存管理**: 及时释放Halcon对象和图像内存
- **算法优化**: 使用合适的金字塔层数和匹配阈值

### 内存使用优化
- **对象复用**: 重复使用Halcon对象
- **批量处理**: 避免频繁的文件I/O操作
- **缓存管理**: 限制结果历史记录数量

## 🔧 扩展开发 | Extension Development

### 添加新的任务类型
1. **扩展参数结构**: 在`VisualTaskParams`中添加新参数
2. **实现处理方法**: 在`VisualWorkThread`中添加处理函数
3. **更新UI界面**: 在`VisualProcess`中添加相应按钮和配置
4. **添加结果显示**: 更新结果显示逻辑

### 自定义算法集成
```cpp
// 在VisualWorkThread中添加新的处理方法
VisualProcessResult processCustomAlgorithm(const HObject& image, const QString& imagePath) {
    try {
        // 1. 获取算法参数
        VisualTaskParams params = getCurrentTaskParams();
        
        // 2. 执行自定义算法
        // ... 算法实现 ...
        
        // 3. 构建结果
        QVariantMap resultData;
        resultData["custom_result"] = algorithmOutput;
        
        return createSuccessResult("Custom", imagePath, resultData);
    } catch (...) {
        return createErrorResult("Custom", imagePath, "算法执行失败");
    }
}
```

## 📚 参考资料 | References

- [Halcon算法库文档](https://www.mvtec.com/products/halcon/)
- [Qt信号槽机制](https://doc.qt.io/qt-5/signalsandslots.html)
- [多线程编程指南](https://doc.qt.io/qt-5/threads.html)
- [HOBJ文件格式说明](./HOBJ_Format_Guide.md)

---

## 🎯 总结 | Summary

本视觉处理工作流程通过三个类的精密协作，实现了从用户交互到算法处理再到结果显示的完整闭环。系统具有以下特点：

✅ **模块化设计**: 职责分离，易于维护和扩展  
✅ **异步处理**: 多线程架构，界面响应流畅  
✅ **错误恢复**: 完善的错误处理和恢复机制  
✅ **用户友好**: 直观的界面和详细的进度反馈  
✅ **高性能**: 优化的算法和内存管理  
✅ **可扩展**: 支持自定义算法和任务类型  

通过这个工作流程，开发者可以快速构建专业的视觉检测应用，满足工业自动化和质量控制的需求。 