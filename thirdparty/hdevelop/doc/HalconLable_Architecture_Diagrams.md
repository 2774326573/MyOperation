# 🏗️ HalconLable 架构图表集合 | Architecture Diagrams Collection

<div align="center">

![Architecture](https://img.shields.io/badge/Architecture-Diagrams-2196F3?style=for-the-badge&logo=sitemap&logoColor=white)
![Mermaid](https://img.shields.io/badge/Mermaid-Diagrams-FF6B35?style=for-the-badge&logo=mermaid&logoColor=white)
![Vision](https://img.shields.io/badge/Machine-Vision-4CAF50?style=for-the-badge&logo=eye&logoColor=white)

### 🎯 基于 HalconLable 的机器视觉系统架构分析

**时序图 • 流程图 • 交互图 | Sequence • Flow • Interaction Diagrams**

</div>

---

## 📋 目录 | Table of Contents

- [🔄 系统组件时序图](#-系统组件时序图--system-component-sequence)
- [📷 图像处理流程图](#-图像处理流程图--image-processing-flowchart)
- [🎮 用户交互时序图](#-用户交互时序图--user-interaction-sequence)
- [🧵 多线程协作时序图](#-多线程协作时序图--multi-thread-collaboration)
- [📊 视觉检测完整流程](#-视觉检测完整流程--complete-vision-detection-flow)
- [🔧 系统架构概览](#-系统架构概览--system-architecture-overview)

---

## 🔄 系统组件时序图 | System Component Sequence

### 🏗️ 主要组件交互时序

```mermaid
sequenceDiagram
    participant U as User/用户
    participant MW as MainWindow/主窗口
    participant HL as HalconLable/视觉控件
    participant VT as VisualWorkThread/视觉线程
    participant VP as VisualProcess/视觉处理
    participant CAM as Camera/相机
    participant HAL as Halcon Engine/Halcon引擎

    Note over U,HAL: 系统初始化阶段 | System Initialization
    U->>MW: 启动应用程序 | Start Application
    MW->>HL: 创建视觉控件 | Create Vision Widget
    HL->>HAL: 初始化Halcon引擎 | Initialize Halcon Engine
    MW->>VT: 创建工作线程 | Create Worker Thread
    VT->>VP: 初始化视觉处理 | Initialize Vision Processing

    Note over U,HAL: 相机连接阶段 | Camera Connection Phase
    U->>MW: 点击连接相机 | Click Connect Camera
    MW->>HL: QtOpenCam()
    HL->>CAM: 打开相机设备 | Open Camera Device
    CAM-->>HL: 返回连接状态 | Return Connection Status
    HL-->>MW: 相机状态反馈 | Camera Status Feedback
    MW-->>U: 显示连接结果 | Show Connection Result

    Note over U,HAL: 图像采集阶段 | Image Acquisition Phase
    MW->>VT: 启动采集线程 | Start Acquisition Thread
    loop 实时采集 | Real-time Acquisition
        VT->>HL: QtGrabImg()
        HL->>CAM: 抓取图像帧 | Grab Image Frame
        CAM-->>HL: 返回图像数据 | Return Image Data
        HL->>VP: 处理图像 | Process Image
        VP->>HAL: 调用算法 | Call Algorithms
        HAL-->>VP: 返回处理结果 | Return Processing Results
        VP-->>VT: 结果反馈 | Result Feedback
        VT->>HL: 显示结果 | Display Results
        HL-->>MW: 更新UI | Update UI
        MW-->>U: 显示处理结果 | Show Processing Results
    end

    Note over U,HAL: 检测操作阶段 | Detection Operation Phase
    U->>MW: 配置检测参数 | Configure Detection Parameters
    MW->>VP: 设置检测模式 | Set Detection Mode
    U->>MW: 执行检测 | Execute Detection
    MW->>VT: 触发检测任务 | Trigger Detection Task
    VT->>VP: 执行视觉算法 | Execute Vision Algorithms
    VP->>HAL: 形状检测/模板匹配 | Shape Detection/Template Matching
    HAL-->>VP: 返回检测结果 | Return Detection Results
    VP->>HL: 显示检测标记 | Display Detection Markers
    HL-->>MW: 更新检测状态 | Update Detection Status
    MW-->>U: 显示最终结果 | Show Final Results
```

---

## 📷 图像处理流程图 | Image Processing Flowchart

### 🔄 完整图像处理流程

```mermaid
flowchart TD
    Start([开始 | Start]) --> InitCamera{初始化相机<br/>Initialize Camera}
    
    InitCamera -->|成功 Success| CameraReady[相机就绪<br/>Camera Ready]
    InitCamera -->|失败 Failed| ErrorHandle[错误处理<br/>Error Handling]
    
    CameraReady --> GrabImage[抓取图像<br/>Grab Image]
    
    GrabImage --> ImageValid{图像有效?<br/>Image Valid?}
    ImageValid -->|无效 Invalid| GrabImage
    ImageValid -->|有效 Valid| PreProcess[图像预处理<br/>Image Preprocessing]
    
    PreProcess --> FilterNoise[噪声滤波<br/>Noise Filtering]
    FilterNoise --> EnhanceContrast[对比度增强<br/>Contrast Enhancement]
    EnhanceContrast --> EdgeDetection[边缘检测<br/>Edge Detection]
    
    EdgeDetection --> ROISelection{ROI选择<br/>ROI Selection}
    ROISelection -->|手动 Manual| DrawROI[绘制ROI<br/>Draw ROI]
    ROISelection -->|自动 Auto| AutoROI[自动ROI<br/>Auto ROI]
    
    DrawROI --> ProcessROI[处理ROI区域<br/>Process ROI]
    AutoROI --> ProcessROI
    
    ProcessROI --> DetectionType{检测类型<br/>Detection Type}
    
    DetectionType -->|形状检测 Shape| ShapeDetection[形状检测算法<br/>Shape Detection Algorithm]
    DetectionType -->|模板匹配 Template| TemplateMatching[模板匹配算法<br/>Template Matching Algorithm]
    DetectionType -->|二维码 QR Code| QRDetection[二维码识别<br/>QR Code Recognition]
    DetectionType -->|3D检测 3D| 3DDetection[3D检测算法<br/>3D Detection Algorithm]
    
    ShapeDetection --> ExtractFeatures[特征提取<br/>Feature Extraction]
    TemplateMatching --> MatchScore[匹配评分<br/>Match Scoring]
    QRDetection --> DecodeQR[解码内容<br/>Decode Content]
    3DDetection --> PointCloud[点云处理<br/>Point Cloud Processing]
    
    ExtractFeatures --> Measurement[测量计算<br/>Measurement Calculation]
    MatchScore --> Measurement
    DecodeQR --> Measurement
    PointCloud --> Measurement
    
    Measurement --> QualityCheck{质量检查<br/>Quality Check}
    QualityCheck -->|合格 Pass| ResultPass[检测通过<br/>Detection Pass]
    QualityCheck -->|不合格 Fail| ResultFail[检测失败<br/>Detection Fail]
    
    ResultPass --> DisplayResult[显示结果<br/>Display Results]
    ResultFail --> DisplayResult
    
    DisplayResult --> SaveData{保存数据?<br/>Save Data?}
    SaveData -->|是 Yes| SaveResults[保存检测结果<br/>Save Detection Results]
    SaveData -->|否 No| ContinueLoop[继续循环<br/>Continue Loop]
    
    SaveResults --> ContinueLoop
    ContinueLoop --> GrabImage
    
    ErrorHandle --> End([结束 | End])
    
    style Start fill:#e1f5fe
    style End fill:#ffebee
    style ResultPass fill:#e8f5e8
    style ResultFail fill:#ffebee
    style ErrorHandle fill:#ffebee
```

---

## 🎮 用户交互时序图 | User Interaction Sequence

### 🖱️ 用户操作交互流程

```mermaid
sequenceDiagram
    participant U as User/用户
    participant UI as MainWindow UI/主窗口界面
    participant HL as HalconLable/视觉控件
    participant Menu as ContextMenu/右键菜单
    participant Dialog as Dialog/对话框
    participant File as FileSystem/文件系统

    Note over U,File: 界面初始化 | UI Initialization
    U->>UI: 启动程序 | Launch Program
    UI->>HL: 初始化控件 | Initialize Widget
    HL-->>UI: 控件就绪 | Widget Ready
    UI-->>U: 显示主界面 | Show Main Interface

    Note over U,File: 图像加载交互 | Image Loading Interaction
    U->>UI: 点击加载图像 | Click Load Image
    UI->>Dialog: 显示文件对话框 | Show File Dialog
    Dialog-->>U: 选择图像文件 | Select Image File
    U->>Dialog: 确认选择 | Confirm Selection
    Dialog->>File: 读取图像文件 | Read Image File
    File-->>HL: 返回图像数据 | Return Image Data
    HL->>HL: QtReadImage()
    HL->>HL: showImage()
    HL-->>UI: 图像显示完成 | Image Display Complete
    UI-->>U: 图像加载成功 | Image Loaded Successfully

    Note over U,File: 鼠标交互操作 | Mouse Interaction
    U->>HL: 鼠标移动 | Mouse Move
    HL->>HL: updatePixelInfoDisplay()
    HL-->>U: 显示像素信息 | Display Pixel Info

    U->>HL: 鼠标滚轮 | Mouse Wheel
    HL->>HL: 缩放图像 | Zoom Image
    HL-->>U: 更新显示 | Update Display

    U->>HL: 左键拖拽 | Left Drag
    HL->>HL: 平移图像 | Pan Image
    HL-->>U: 更新视图 | Update View

    U->>HL: 双击左键 | Double Click
    HL->>HL: zoomToFit()
    HL-->>U: 重置视图 | Reset View

    Note over U,File: 右键菜单交互 | Context Menu Interaction
    U->>HL: 右键点击 | Right Click
    HL->>Menu: showContextMenu()
    Menu-->>U: 显示菜单选项 | Show Menu Options

    alt 保存图像 | Save Image
        U->>Menu: 选择保存图像 | Select Save Image
        Menu->>Dialog: 显示保存对话框 | Show Save Dialog
        Dialog-->>U: 输入文件名 | Input Filename
        U->>Dialog: 确认保存 | Confirm Save
        Dialog->>HL: QtSaveImage()
        HL->>File: 写入文件 | Write File
        File-->>UI: 保存完成 | Save Complete
    else 缩放操作 | Zoom Operation
        U->>Menu: 选择缩放 | Select Zoom
        Menu->>HL: zoomToActualSize()
        HL-->>U: 更新缩放 | Update Zoom
    else 清除显示 | Clear Display
        U->>Menu: 选择清除 | Select Clear
        Menu->>HL: RemoveShow()
        HL-->>U: 清除完成 | Clear Complete
    end

    Note over U,File: ROI绘制交互 | ROI Drawing Interaction
    U->>UI: 点击绘制ROI | Click Draw ROI
    UI->>HL: genAngleRec()
    HL-->>U: 进入绘制模式 | Enter Drawing Mode

    U->>HL: 鼠标点击定位 | Mouse Click Position
    HL->>HL: 记录坐标点 | Record Coordinate Points
    U->>HL: 移动鼠标 | Move Mouse
    HL->>HL: 实时显示ROI | Real-time Display ROI
    U->>HL: 完成绘制 | Complete Drawing
    HL->>HL: 生成ROI区域 | Generate ROI Region
    HL-->>UI: ROI创建完成 | ROI Creation Complete
    UI-->>U: 显示ROI结果 | Show ROI Result

    Note over U,File: 检测执行交互 | Detection Execution Interaction
    U->>UI: 配置检测参数 | Configure Detection Parameters
    U->>UI: 点击执行检测 | Click Execute Detection
    UI->>HL: 执行检测算法 | Execute Detection Algorithm
    HL->>HL: 视觉处理 | Vision Processing
    HL->>HL: 显示检测结果 | Display Detection Results
    HL-->>UI: 检测完成 | Detection Complete
    UI-->>U: 显示最终结果 | Show Final Results
```

---

## 🧵 多线程协作时序图 | Multi-Thread Collaboration

### 🔄 线程间协作机制

```mermaid
sequenceDiagram
    participant MT as MainThread/主线程
    participant WT as WorkerThread/工作线程
    participant GT as GrabThread/采集线程
    participant PT as ProcessThread/处理线程
    participant HL as HalconLable/视觉控件
    participant CAM as Camera/相机
    participant HAL as Halcon/算法引擎

    Note over MT,HAL: 线程初始化 | Thread Initialization
    MT->>WT: 创建工作线程 | Create Worker Thread
    MT->>GT: 创建采集线程 | Create Grab Thread
    MT->>PT: 创建处理线程 | Create Process Thread
    
    WT-->>MT: 线程就绪信号 | Thread Ready Signal
    GT-->>MT: 采集线程就绪 | Grab Thread Ready
    PT-->>MT: 处理线程就绪 | Process Thread Ready

    Note over MT,HAL: 相机启动 | Camera Startup
    MT->>GT: 启动采集 | Start Acquisition
    GT->>CAM: 打开相机 | Open Camera
    CAM-->>GT: 相机就绪 | Camera Ready
    GT-->>MT: emit cameraReady() | Emit Camera Ready

    Note over MT,HAL: 图像采集循环 | Image Acquisition Loop
    loop 连续采集 | Continuous Acquisition
        GT->>CAM: 抓取图像 | Grab Image
        CAM-->>GT: 图像数据 | Image Data
        GT->>WT: emit newImageAvailable() | Emit New Image
        
        WT->>PT: 发送处理任务 | Send Processing Task
        PT->>HAL: 执行算法 | Execute Algorithms
        HAL-->>PT: 返回结果 | Return Results
        PT->>WT: emit processingComplete() | Emit Processing Complete
        
        WT->>MT: invoke displayResults() | Invoke Display Results
        MT->>HL: 更新显示 | Update Display
        HL-->>MT: 显示完成 | Display Complete
    end

    Note over MT,HAL: 检测任务处理 | Detection Task Processing
    MT->>WT: 发送检测任务 | Send Detection Task
    WT->>PT: 执行特定检测 | Execute Specific Detection
    
    par 并行处理 | Parallel Processing
        PT->>HAL: 形状检测 | Shape Detection
        PT->>HAL: 模板匹配 | Template Matching
        PT->>HAL: 测量计算 | Measurement Calculation
    end
    
    HAL-->>PT: 检测结果 | Detection Results
    PT->>WT: emit detectionComplete() | Emit Detection Complete
    WT->>MT: 结果回调 | Result Callback
    MT->>HL: 显示检测标记 | Display Detection Markers

    Note over MT,HAL: 异常处理 | Exception Handling
    alt 相机异常 | Camera Exception
        CAM-xGT: 连接断开 | Connection Lost
        GT->>MT: emit cameraError() | Emit Camera Error
        MT->>HL: 显示错误信息 | Display Error Message
    else 算法异常 | Algorithm Exception
        HAL-xPT: 处理失败 | Processing Failed
        PT->>WT: emit processingError() | Emit Processing Error
        WT->>MT: 错误回调 | Error Callback
        MT->>HL: 显示错误状态 | Display Error Status
    end

    Note over MT,HAL: 线程清理 | Thread Cleanup
    MT->>GT: 停止采集 | Stop Acquisition
    GT->>CAM: 关闭相机 | Close Camera
    MT->>WT: 停止工作线程 | Stop Worker Thread
    MT->>PT: 停止处理线程 | Stop Process Thread
    
    WT-->>MT: 线程退出 | Thread Exit
    GT-->>MT: 采集停止 | Acquisition Stopped
    PT-->>MT: 处理停止 | Processing Stopped
```

---

## 📊 视觉检测完整流程 | Complete Vision Detection Flow

### 🔍 端到端检测流程

```mermaid
flowchart TD
    A[系统启动<br/>System Startup] --> B[初始化Halcon引擎<br/>Initialize Halcon Engine]
    B --> C[连接工业相机<br/>Connect Industrial Camera]
    C --> D{相机连接成功?<br/>Camera Connected?}
    
    D -->|否 No| E[显示错误信息<br/>Display Error Message]
    D -->|是 Yes| F[启动图像采集<br/>Start Image Acquisition]
    
    F --> G[实时图像显示<br/>Real-time Image Display]
    G --> H[用户操作选择<br/>User Operation Selection]
    
    H --> I{操作类型<br/>Operation Type}
    
    I -->|加载图像 Load Image| J[选择本地图像<br/>Select Local Image]
    I -->|实时检测 Live Detection| K[配置检测参数<br/>Configure Detection Parameters]
    I -->|模板创建 Template Creation| L[创建检测模板<br/>Create Detection Template]
    
    J --> M[显示图像<br/>Display Image]
    K --> N[设置检测区域<br/>Set Detection Region]
    L --> O[选择模板区域<br/>Select Template Region]
    
    M --> P[选择检测功能<br/>Select Detection Function]
    N --> P
    O --> Q[保存模板文件<br/>Save Template File]
    Q --> P
    
    P --> R{检测类型<br/>Detection Type}
    
    R -->|形状检测<br/>Shape Detection| S[几何形状识别<br/>Geometric Shape Recognition]
    R -->|模板匹配<br/>Template Matching| T[模板匹配算法<br/>Template Matching Algorithm]
    R -->|二维码识别<br/>QR Code Recognition| U[二维码解码<br/>QR Code Decoding]
    R -->|边缘检测<br/>Edge Detection| V[边缘轮廓提取<br/>Edge Contour Extraction]
    R -->|缺陷检测<br/>Defect Detection| W[表面缺陷分析<br/>Surface Defect Analysis]
    
    S --> X[特征参数提取<br/>Feature Parameter Extraction]
    T --> Y[匹配度计算<br/>Match Score Calculation]
    U --> Z[解码结果验证<br/>Decode Result Validation]
    V --> AA[轮廓精度分析<br/>Contour Accuracy Analysis]
    W --> BB[缺陷分类识别<br/>Defect Classification]
    
    X --> CC[测量计算<br/>Measurement Calculation]
    Y --> CC
    Z --> CC
    AA --> CC
    BB --> CC
    
    CC --> DD[质量判定<br/>Quality Judgment]
    DD --> EE{检测结果<br/>Detection Result}
    
    EE -->|合格 Pass| FF[标记合格产品<br/>Mark Pass Product]
    EE -->|不合格 Fail| GG[标记不合格产品<br/>Mark Fail Product]
    
    FF --> HH[显示检测结果<br/>Display Detection Results]
    GG --> HH
    
    HH --> II[结果数据记录<br/>Record Result Data]
    II --> JJ{保存结果?<br/>Save Results?}
    
    JJ -->|是 Yes| KK[保存检测报告<br/>Save Detection Report]
    JJ -->|否 No| LL[继续下一次检测<br/>Continue Next Detection]
    
    KK --> LL
    LL --> MM{继续检测?<br/>Continue Detection?}
    
    MM -->|是 Yes| G
    MM -->|否 No| NN[结束检测<br/>End Detection]
    
    E --> NN
    NN --> OO[系统关闭<br/>System Shutdown]
    
    style A fill:#e1f5fe
    style OO fill:#ffebee
    style FF fill:#e8f5e8
    style GG fill:#ffebee
    style E fill:#ffebee
```

---

## 🔧 系统架构概览 | System Architecture Overview

### 🏗️ 整体架构组件关系

```mermaid
graph TB
    subgraph "用户界面层 | User Interface Layer"
        MW[MainWindow<br/>主窗口]
        HL[HalconLable<br/>视觉控件]
        Menu[ContextMenu<br/>右键菜单]
        Dialog[Dialogs<br/>对话框组件]
    end
    
    subgraph "业务逻辑层 | Business Logic Layer"
        VP[VisualProcess<br/>视觉处理器]
        TM[TemplateManager<br/>模板管理器]
        QR[QRCodeProcessor<br/>二维码处理器]
        MD[MeasurementProcessor<br/>测量处理器]
    end
    
    subgraph "线程管理层 | Thread Management Layer"
        VWT[VisualWorkThread<br/>视觉工作线程]
        GT[GrabThread<br/>图像采集线程]
        PT[ProcessThread<br/>图像处理线程]
        ST[SaveThread<br/>数据保存线程]
    end
    
    subgraph "硬件抽象层 | Hardware Abstraction Layer"
        CAM[Camera Interface<br/>相机接口]
        IO[I/O Interface<br/>输入输出接口]
        CALIB[Calibration<br/>标定系统]
    end
    
    subgraph "算法引擎层 | Algorithm Engine Layer"
        HAL[Halcon Engine<br/>Halcon算法引擎]
        CV[OpenCV Library<br/>OpenCV库]
        CUSTOM[Custom Algorithms<br/>自定义算法]
    end
    
    subgraph "数据存储层 | Data Storage Layer"
        DB[Database<br/>数据库]
        FILE[File System<br/>文件系统]
        CONFIG[Configuration<br/>配置文件]
    end
    
    %% 连接关系 | Connections
    MW --> HL
    MW --> Menu
    MW --> Dialog
    HL --> VP
    HL --> VWT
    
    VP --> TM
    VP --> QR
    VP --> MD
    VP --> HAL
    
    VWT --> GT
    VWT --> PT
    VWT --> ST
    
    GT --> CAM
    PT --> HAL
    PT --> CV
    PT --> CUSTOM
    ST --> FILE
    ST --> DB
    
    CAM --> IO
    HAL --> CALIB
    
    TM --> FILE
    QR --> HAL
    MD --> HAL
    
    CONFIG --> MW
    CONFIG --> VP
    CONFIG --> CAM
    
    %% 样式定义 | Style Definitions
    classDef uiLayer fill:#e3f2fd,stroke:#1976d2,stroke-width:2px
    classDef businessLayer fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef threadLayer fill:#e8f5e8,stroke:#388e3c,stroke-width:2px
    classDef hardwareLayer fill:#fff3e0,stroke:#f57c00,stroke-width:2px
    classDef algorithmLayer fill:#fce4ec,stroke:#c2185b,stroke-width:2px
    classDef dataLayer fill:#e0f2f1,stroke:#00695c,stroke-width:2px
    
    class MW,HL,Menu,Dialog uiLayer
    class VP,TM,QR,MD businessLayer
    class VWT,GT,PT,ST threadLayer
    class CAM,IO,CALIB hardwareLayer
    class HAL,CV,CUSTOM algorithmLayer
    class DB,FILE,CONFIG dataLayer
```

---

## 📋 图表使用说明 | Diagram Usage Guide

### 🎯 如何阅读这些图表 | How to Read These Diagrams

#### 1. **时序图 (Sequence Diagrams)**
- **箭头方向**: 消息流向和调用关系
- **生命线**: 对象的存在时间
- **激活框**: 对象处于活跃状态的时间
- **注释框**: 不同阶段的说明

#### 2. **流程图 (Flowcharts)**
- **矩形**: 处理步骤
- **菱形**: 判断节点
- **圆角矩形**: 开始/结束节点
- **箭头**: 流程方向

#### 3. **架构图 (Architecture Diagrams)**
- **子图**: 不同的系统层次
- **节点**: 系统组件
- **连线**: 组件间的依赖关系
- **颜色**: 不同层次的区分

### 📝 图表应用场景 | Diagram Application Scenarios

| 图表类型 | 适用场景 | 目标用户 |
|---------|---------|---------|
| **系统组件时序图** | 理解组件交互顺序 | 架构师，高级开发者 |
| **图像处理流程图** | 了解算法处理流程 | 算法工程师，测试工程师 |
| **用户交互时序图** | 设计用户体验流程 | UI设计师，产品经理 |
| **多线程协作时序图** | 解决并发编程问题 | 系统开发者，性能优化工程师 |
| **视觉检测完整流程** | 理解业务处理逻辑 | 业务分析师，项目经理 |
| **系统架构概览** | 系统整体架构设计 | 技术总监，架构师 |

---

<div align="center">

## 🎉 架构图表使用建议 | Architecture Diagram Usage Recommendations

### 💡 开发流程建议 | Development Process Recommendations

**📋 需求分析阶段**: 参考"视觉检测完整流程"理解业务需求  
**🏗️ 架构设计阶段**: 参考"系统架构概览"进行模块划分  
**🧵 并发设计阶段**: 参考"多线程协作时序图"设计线程策略  
**🎮 界面设计阶段**: 参考"用户交互时序图"优化用户体验  
**🔧 开发实现阶段**: 参考"系统组件时序图"理解调用关系  
**🧪 测试验证阶段**: 参考"图像处理流程图"设计测试用例

### 🔗 相关文档链接 | Related Documentation Links

- [📘 HalconLable API Manual](HalconLable_API_Manual.md)
- [⚡ 快速参考卡片](HalconLable_Quick_Reference.md)
- [📖 完整使用手册](HalconLable使用手册.md)
- [📚 文档中心](README.md)

---

**📊 这些图表帮助您深入理解 HalconLable 系统的架构设计和工作原理**  
**These diagrams help you understand the architecture design and working principles of HalconLable system**

**文档版本：v1.0** | **更新日期：2025-05-31** | **维护者：开发团队**

</div> 