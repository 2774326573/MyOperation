# 🚀 HalconLable 快速参考卡片 | Quick Reference Card

<div align="center">

![Quick Reference](https://img.shields.io/badge/Quick-Reference-FF6B35?style=for-the-badge&logo=bookmark&logoColor=white)
![HalconLable](https://img.shields.io/badge/HalconLable-API-4CAF50?style=for-the-badge&logo=code&logoColor=white)

### ⚡ 常用API快速查找 | Common API Quick Lookup

</div>

---

## 🎯 核心功能速查 | Core Functions Quick Reference

### 📷 相机操作 | Camera Operations
```cpp
bool QtOpenCam();                    // 开启相机 | Open camera
HObject QtGrabImg();                 // 抓取图像 | Grab image  
void QtCloseCam();                   // 关闭相机 | Close camera
```

### 🖼️ 图像操作 | Image Operations
```cpp
HObject QtReadImage(HTuple path);    // 读取图像 | Read image
void showImage(HObject image);       // 显示图像 | Show image
bool QtSaveImage(HObject image);     // 保存图像 | Save image
```

### 🔍 形状检测 | Shape Detection
```cpp
HObject genAngleRec(QString color);  // 有角度矩形 | Angled rectangle
HObject genNoAngleRec();             // 无角度矩形 | Normal rectangle
HObject GenCircleRegionOne();        // 圆形区域 | Circle region
HObject GenAnyShapeRegion();         // 任意形状 | Any shape
```

### 🎨 显示功能 | Display Functions
```cpp
void showHalconObject(HObject obj, QString color, double width = 2);
void DispCoorCross(double x, double y, int width, double angle = 0);
void dispHalconMessage(int x, int y, QString text, QString color = "green");
void RemoveShow();                   // 清除显示 | Clear display
```

---

## 📊 常用工作流程 | Common Workflows

### 🔄 基础图像处理流程 | Basic Image Processing

```cpp
// 1. 创建控件 | Create widget
HalconLable *widget = new HalconLable(this);

// 2. 加载图像 | Load image
HObject image = widget->QtReadImage("image.bmp");

// 3. 创建检测区域 | Create detection region
HObject region = widget->genAngleRec("yellow");

// 4. 显示结果 | Display results
widget->showHalconObject(region, "red", 2);
```

### 📷 相机实时采集流程 | Live Camera Workflow

```cpp
// 1. 开启相机 | Open camera
if (widget->QtOpenCam()) {
    // 2. 定时抓取 | Timer grab
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, [=]() {
        HObject frame = widget->QtGrabImg();
        widget->showImage(frame);
    });
    timer->start(33); // 30 FPS
}
```

---

## 🎯 模板匹配速查 | Template Matching Quick Reference

### 📋 创建模板 | Create Template
```cpp
ShapeModelData model = widget->QtCreateShapeModel(
    image,      // 模板图像 | Template image
    region,     // 模板区域 | Template region  
    30,         // 对比度 | Contrast
    10,         // 最小对比度 | Min contrast
    "model.shm" // 保存路径 | Save path
);
```

### 🔍 线条检测 | Line Detection
```cpp
LineData line = widget->QtGenLine(
    image,    // 输入图像 | Input image
    region,   // 检测区域 | Detection region
    30        // 阈值 | Threshold
);
```

---

## 🔲 二维码识别速查 | QR Code Recognition Quick Reference

```cpp
// 1. 创建二维码模型 | Create QR model
HTuple qrModel = widget->QtData2d("QR Code", "auto");

// 2. 识别二维码 | Recognize QR codes
CodeData result = widget->QtRecogied(image, qrModel, 10);

// 3. 获取结果 | Get results
for (const QString &code : result.codestring) {
    qDebug() << "检测到 | Detected:" << code;
}
```

---

## 📏 测量功能速查 | Measurement Quick Reference

### 📐 基础测量 | Basic Measurements
```cpp
// 两点距离 | Point distance
double dist = widget->calculatePointDistance(x1, y1, x2, y2);

// 三点角度 | Three point angle
double angle = widget->calculateThreePointAngle(x1, y1, x2, y2, x3, y3);

// 区域面积 | Region area
double area = widget->calculateRegionArea(region);

// 区域重心 | Region centroid
pointStruct center = widget->calculateRegionCentroid(region);
```

---

## 🎨 标注功能速查 | Annotation Quick Reference

```cpp
// 文本标注 | Text annotation
widget->addTextAnnotation("检测结果", 100, 50, "white", 16);

// 箭头标注 | Arrow annotation
widget->addArrowAnnotation(x1, y1, x2, y2, "red", 2.0);

// 尺寸标注 | Dimension annotation
widget->addDimensionAnnotation(x1, y1, x2, y2, "mm", "blue");

// 清除标注 | Clear annotations
widget->clearAllAnnotations();
```

---

## 🎮 交互操作速查 | Interactive Operations Quick Reference

### 🖱️ 鼠标操作 | Mouse Operations
- **左键拖拽**: 平移图像 | Pan image
- **滚轮**: 缩放图像 | Zoom image  
- **双击**: 重置显示 | Reset display

### ⚡ 快捷操作 | Quick Operations
```cpp
widget->zoomToFit();           // 适合窗口 | Fit to window
widget->zoomToActualSize();    // 实际大小 | Actual size
widget->zoomToRatio(2.0);      // 指定比例 | Specific ratio
widget->undoLastOperation();   // 撤销 | Undo
widget->redoOperation();       // 重做 | Redo
```

---

## 🧊 3D功能速查 | 3D Functions Quick Reference

```cpp
// 创建3D模型 | Create 3D model
HTuple model3D = widget->create3DObjectModel(pointCloudFiles, errorMsg);

// 点云配准 | Point cloud registration
HTuple transform = widget->register3DPointClouds(model1, model2, errorMsg);

// 表面缺陷检测 | Surface defect detection
HObject defects = widget->detect3DSurfaceDefects(model3D, tolerance, errorMsg);

// 体积计算 | Volume calculation
double volume = widget->calculate3DObjectVolume(model3D, errorMsg);
```

---

## 🤖 手眼标定速查 | Hand-Eye Calibration Quick Reference

```cpp
// 1. 创建标定数据 | Create calibration data
HTuple calibData = widget->createHandEyeCalibrationData(errorMsg);

// 2. 添加姿态 | Add poses (多次调用 | Multiple calls)
widget->addCalibrationPose(calibData, cameraPose, robotPose, errorMsg);

// 3. 执行标定 | Perform calibration
HTuple transformation = widget->performHandEyeCalibration(calibData, errorMsg);

// 4. 保存结果 | Save results
widget->saveCalibrationResults(transformation, "calib.dat", errorMsg);
```

---

## 📋 ROI管理速查 | ROI Management Quick Reference

```cpp
// 保存ROI | Save ROI
widget->saveROIToFile(region, "roi.hobj", errorMsg);

// 加载ROI | Load ROI
widget->loadROIFromFile("roi.hobj", region, errorMsg);

// 复制ROI | Copy ROI
widget->copyROI(sourceRegion, targetRegion);

// 合并ROI | Merge ROIs
HObject merged = widget->mergeROIs(regionList);

// 偏移ROI | Offset ROI
HObject offset = widget->offsetROI(region, rowOffset, colOffset);
```

---

## 🔧 便捷工具速查 | Utility Functions Quick Reference

### 🛡️ 安全操作 | Safe Operations
```cpp
// 安全读取图像 | Safe image reading
bool success = widget->QtGetLocalImageSafe(image, errorMsg);

// 安全保存图像 | Safe image saving  
bool saved = widget->QtSaveImageSafe(image, errorMsg);
```

### 📊 状态查询 | Status Queries
```cpp
bool loaded = widget->isImageLoaded();           // 图像是否加载 | Image loaded
int count = widget->getDisplayObjectsCount();    // 显示对象数量 | Display count
QString info = widget->getImageInfo();           // 图像信息 | Image info
```

### 🎯 像素信息 | Pixel Information
```cpp
widget->setPixelInfoDisplayEnabled(true);       // 启用像素信息 | Enable pixel info
QString info = widget->getPixelValueAtPosition(x, y);  // 获取像素值 | Get pixel value
```

---

## ⚠️ 常见注意事项 | Common Pitfalls

### 🧵 线程安全 | Thread Safety
```cpp
// ❌ 错误 - 在工作线程中调用
// Wrong - calling in worker thread
workerThread->run([=]() {
    widget->showImage(image); // 不安全 | Unsafe
});

// ✅ 正确 - 在GUI线程中调用  
// Correct - calling in GUI thread
QMetaObject::invokeMethod(widget, [=]() {
    widget->showImage(image); // 安全 | Safe
});
```

### 💾 内存管理 | Memory Management
```cpp
// ✅ 及时清理大对象 | Clean large objects promptly
HObject largeImage;
// ... 使用 | use
largeImage.Clear(); // 手动清理 | Manual cleanup
```

### 🔧 错误处理 | Error Handling
```cpp
// ✅ 使用安全版本的API | Use safe API versions
QString errorMsg;
if (!widget->QtSaveImageSafe(image, errorMsg)) {
    qDebug() << "保存失败 | Save failed:" << errorMsg;
}
```

---

<div align="center">

## 🎉 快速参考卡片使用提示 | Quick Reference Usage Tips

| 🔍 **查找技巧** | 💡 **使用建议** |
|---------------|---------------|
| Ctrl+F 搜索关键词 | 按功能模块查找API |
| 收藏常用代码片段 | 复制粘贴快速开发 |
| 对照完整文档深入学习 | 结合实际项目练习 |

**📚 完整文档链接：**
- [HalconLable API Manual - Part 1](HalconLable_API_Manual.md)
- [HalconLable API Manual - Part 2](HalconLable_API_Manual_Part2.md)
- [HalconLable 使用手册](HalconLable使用手册.md)

**🎯 记住：先查快速参考，再看详细文档！**

---

**版本：v1.0** | **更新：2025-05-31** | **维护：开发团队**

</div> 