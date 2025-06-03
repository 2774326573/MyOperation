# 🔬 HalconLable API 使用手册 - 第二部分 | HalconLable API Manual - Part 2

## 📝 图像标注 | Image Annotation

### 📝 addTextAnnotation - 添加文本标注 | Add Text Annotation

```cpp
void addTextAnnotation(QString text, double x, double y, QString color = "yellow", int fontSize = 16);
```

**功能 | Function**: 添加文本标注到图像 | Adds text annotation to image

**参数 | Parameters**:
- `text`: 标注文本 | Annotation text
- `x, y`: 文本位置 | Text position
- `color`: 文本颜色 | Text color (default: "yellow")
- `fontSize`: 字体大小 | Font size (default: 16)

**示例 | Example**:
```cpp
halconWidget->addTextAnnotation("检测结果 | Detection Result", 100, 50, "white", 20);
```

### ➡️ addArrowAnnotation - 添加箭头标注 | Add Arrow Annotation

```cpp
void addArrowAnnotation(double startX, double startY, double endX, double endY, QString color = "red", double lineWidth = 2.0);
```

**功能 | Function**: 添加箭头标注 | Adds arrow annotation

**参数 | Parameters**:
- `startX, startY`: 箭头起点 | Arrow start point
- `endX, endY`: 箭头终点 | Arrow end point
- `color`: 箭头颜色 | Arrow color (default: "red")
- `lineWidth`: 线条宽度 | Line width (default: 2.0)

### 📏 addDimensionAnnotation - 添加尺寸标注 | Add Dimension Annotation

```cpp
void addDimensionAnnotation(double x1, double y1, double x2, double y2, QString unit = "px", QString color = "blue");
```

**功能 | Function**: 添加尺寸标注 | Adds dimension annotation

**参数 | Parameters**:
- `x1, y1`: 起点坐标 | Start point coordinates
- `x2, y2`: 终点坐标 | End point coordinates
- `unit`: 单位 | Unit (default: "px")
- `color`: 颜色 | Color (default: "blue")

### 🗃️ showCoordinateSystem - 显示坐标系 | Show Coordinate System

```cpp
void showCoordinateSystem(double originX = 50, double originY = 50, double scale = 50, QString color = "white");
```

**功能 | Function**: 显示坐标系 | Displays coordinate system

**参数 | Parameters**:
- `originX, originY`: 原点位置 | Origin position (default: 50, 50)
- `scale`: 坐标轴长度 | Axis length (default: 50)
- `color`: 颜色 | Color (default: "white")

### 🧹 clearAllAnnotations - 清除所有标注 | Clear All Annotations

```cpp
void clearAllAnnotations();
```

**功能 | Function**: 清除所有标注 | Clears all annotations

---

## 📋 ROI管理 | ROI Management

### 💾 saveROIToFile - 保存ROI到文件 | Save ROI to File

```cpp
bool saveROIToFile(HObject region, QString filePath, QString& errorMessage);
```

**功能 | Function**: 保存ROI到文件 | Saves ROI to file

**参数 | Parameters**:
- `region`: ROI区域对象 | ROI region object
- `filePath`: 保存文件路径 | Save file path
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `bool` - 保存成功返回true | Returns true on success

### 📂 loadROIFromFile - 从文件加载ROI | Load ROI from File

```cpp
bool loadROIFromFile(QString filePath, HObject& region, QString& errorMessage);
```

**功能 | Function**: 从文件加载ROI | Loads ROI from file

**参数 | Parameters**:
- `filePath`: 文件路径 | File path
- `region`: 输出ROI区域对象 | Output ROI region object
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `bool` - 加载成功返回true | Returns true on success

### 📋 copyROI - 复制ROI | Copy ROI

```cpp
bool copyROI(HObject sourceRegion, HObject& targetRegion);
```

**功能 | Function**: 复制ROI | Copies ROI

**参数 | Parameters**:
- `sourceRegion`: 源ROI区域 | Source ROI region
- `targetRegion`: 目标ROI区域 | Target ROI region

**返回值 | Return**: `bool` - 复制成功返回true | Returns true on success

### 🔗 mergeROIs - 合并多个ROI | Merge Multiple ROIs

```cpp
HObject mergeROIs(const QList<HObject>& regions);
```

**功能 | Function**: 合并多个ROI | Merges multiple ROIs

**参数 | Parameters**:
- `regions`: ROI区域列表 | ROI region list

**返回值 | Return**: `HObject` - 合并后的ROI | Merged ROI

### ⬇️ offsetROI - ROI偏移 | Offset ROI

```cpp
HObject offsetROI(HObject region, double rowOffset, double colOffset);
```

**功能 | Function**: ROI偏移 | Offsets ROI

**参数 | Parameters**:
- `region`: 原始ROI区域 | Original ROI region
- `rowOffset`: 行偏移量 | Row offset
- `colOffset`: 列偏移量 | Column offset

**返回值 | Return**: `HObject` - 偏移后的ROI | Offset ROI

---

## 🧊 3D检测 | 3D Detection

### 🏗️ create3DObjectModel - 创建3D对象模型 | Create 3D Object Model

```cpp
HTuple create3DObjectModel(const QStringList& pointCloudFiles, QString& errorMessage);
```

**功能 | Function**: 创建3D对象模型 | Creates 3D object model

**参数 | Parameters**:
- `pointCloudFiles`: 点云文件列表 | Point cloud file list
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `HTuple` - 3D对象模型句柄 | 3D object model handle

### 🔄 register3DPointClouds - 3D点云配准 | 3D Point Cloud Registration

```cpp
HTuple register3DPointClouds(HTuple objectModel1, HTuple objectModel2, QString& errorMessage);
```

**功能 | Function**: 3D点云配准 | Performs 3D point cloud registration

**参数 | Parameters**:
- `objectModel1`: 第一个对象模型 | First object model
- `objectModel2`: 第二个对象模型 | Second object model
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `HTuple` - 配准变换矩阵 | Registration transformation matrix

### 🔍 detect3DSurfaceDefects - 3D表面缺陷检测 | 3D Surface Defect Detection

```cpp
HObject detect3DSurfaceDefects(HTuple objectModel, double tolerance, QString& errorMessage);
```

**功能 | Function**: 3D表面缺陷检测 | Detects 3D surface defects

**参数 | Parameters**:
- `objectModel`: 3D对象模型 | 3D object model
- `tolerance`: 公差 | Tolerance
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `HObject` - 缺陷区域 | Defect regions

### 📊 calculate3DObjectVolume - 计算3D对象体积 | Calculate 3D Object Volume

```cpp
double calculate3DObjectVolume(HTuple objectModel, QString& errorMessage);
```

**功能 | Function**: 计算3D对象体积 | Calculates 3D object volume

**参数 | Parameters**:
- `objectModel`: 3D对象模型 | 3D object model
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `double` - 体积值 | Volume value

### 🔧 filter3DPointCloud - 3D点云滤波 | 3D Point Cloud Filtering

```cpp
HTuple filter3DPointCloud(HTuple objectModel, QString filterType, double param, QString& errorMessage);
```

**功能 | Function**: 3D点云滤波 | Filters 3D point cloud

**参数 | Parameters**:
- `objectModel`: 3D对象模型 | 3D object model
- `filterType`: 滤波类型 | Filter type
- `param`: 滤波参数 | Filter parameter
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `HTuple` - 滤波后的模型 | Filtered model

### 📈 analyze3DMeasurement - 3D测量分析 | 3D Measurement Analysis

```cpp
QMap<QString, double> analyze3DMeasurement(HTuple objectModel, QString& errorMessage);
```

**功能 | Function**: 3D测量分析 | Performs 3D measurement analysis

**参数 | Parameters**:
- `objectModel`: 3D对象模型 | 3D object model
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `QMap<QString, double>` - 测量结果映射 | Measurement results map

---

## 🤖 手眼标定 | Hand-Eye Calibration

### 🏗️ createHandEyeCalibrationData - 创建手眼标定数据 | Create Hand-Eye Calibration Data

```cpp
HTuple createHandEyeCalibrationData(QString& errorMessage);
```

**功能 | Function**: 创建手眼标定数据 | Creates hand-eye calibration data

**参数 | Parameters**:
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `HTuple` - 标定数据句柄 | Calibration data handle

### ➕ addCalibrationPose - 添加标定姿态 | Add Calibration Pose

```cpp
bool addCalibrationPose(HTuple calibData, HTuple cameraPose, HTuple robotPose, QString& errorMessage);
```

**功能 | Function**: 添加标定姿态 | Adds calibration pose

**参数 | Parameters**:
- `calibData`: 标定数据 | Calibration data
- `cameraPose`: 相机姿态 | Camera pose
- `robotPose`: 机器人姿态 | Robot pose
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `bool` - 添加成功返回true | Returns true on success

### 🎯 performHandEyeCalibration - 执行手眼标定 | Perform Hand-Eye Calibration

```cpp
HTuple performHandEyeCalibration(HTuple calibData, QString& errorMessage);
```

**功能 | Function**: 执行手眼标定 | Performs hand-eye calibration

**参数 | Parameters**:
- `calibData`: 标定数据 | Calibration data
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `HTuple` - 标定变换矩阵 | Calibration transformation matrix

### ✅ validateCalibrationAccuracy - 验证标定精度 | Validate Calibration Accuracy

```cpp
QMap<QString, double> validateCalibrationAccuracy(HTuple calibData, HTuple transformation, QString& errorMessage);
```

**功能 | Function**: 验证标定精度 | Validates calibration accuracy

**参数 | Parameters**:
- `calibData`: 标定数据 | Calibration data
- `transformation`: 变换矩阵 | Transformation matrix
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `QMap<QString, double>` - 精度结果 | Accuracy results

### 💾 saveCalibrationResults - 保存标定结果 | Save Calibration Results

```cpp
bool saveCalibrationResults(HTuple transformation, QString filePath, QString& errorMessage);
```

**功能 | Function**: 保存标定结果 | Saves calibration results

**参数 | Parameters**:
- `transformation`: 变换矩阵 | Transformation matrix
- `filePath`: 保存文件路径 | Save file path
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `bool` - 保存成功返回true | Returns true on success

### 📂 loadCalibrationResults - 加载标定结果 | Load Calibration Results

```cpp
HTuple loadCalibrationResults(QString filePath, QString& errorMessage);
```

**功能 | Function**: 加载标定结果 | Loads calibration results

**参数 | Parameters**:
- `filePath`: 文件路径 | File path
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `HTuple` - 变换矩阵 | Transformation matrix

### 🔄 transformCoordinates - 坐标系转换 | Coordinate System Transformation

```cpp
QMap<QString, double> transformCoordinates(double x, double y, double z, HTuple transformation, QString& errorMessage);
```

**功能 | Function**: 坐标系转换 | Transforms coordinates between coordinate systems

**参数 | Parameters**:
- `x, y, z`: 输入坐标 | Input coordinates
- `transformation`: 变换矩阵 | Transformation matrix
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `QMap<QString, double>` - 转换后坐标 | Transformed coordinates

---

## 🎮 交互功能 | Interactive Features

### 🖱️ 鼠标事件处理 | Mouse Event Handling

控件自动处理以下鼠标事件 | Widget automatically handles the following mouse events:

#### 🔄 图像拖拽 | Image Dragging
- **左键拖拽 | Left-click drag**: 平移图像 | Pan image
- **操作方式 | Operation**: 按住左键并移动鼠标 | Hold left button and move mouse

#### 🔍 图像缩放 | Image Zooming
- **鼠标滚轮 | Mouse wheel**: 缩放图像 | Zoom image
- **向上滚动 | Scroll up**: 放大图像 | Zoom in
- **向下滚动 | Scroll down**: 缩小图像 | Zoom out

#### 🎯 图像复位 | Image Reset
- **双击左键 | Double-click left**: 重置图像显示到原始状态 | Reset image display to original state

### 🎮 右键菜单功能 | Context Menu Functions

### 📋 setupContextMenu - 创建右键菜单 | Setup Context Menu

```cpp
void setupContextMenu();
```

**功能 | Function**: 创建右键菜单 | Creates context menu

### 📋 showContextMenu - 显示右键菜单 | Show Context Menu

```cpp
void showContextMenu(const QPoint& position);
```

**功能 | Function**: 显示右键菜单 | Shows context menu

**参数 | Parameters**:
- `position`: 菜单显示位置 | Menu display position

### ➕ addContextMenuItem - 添加菜单项 | Add Context Menu Item

```cpp
void addContextMenuItem(const QString& text, const QString& actionId);
```

**功能 | Function**: 添加菜单项 | Adds menu item

**参数 | Parameters**:
- `text`: 菜单项文本 | Menu item text
- `actionId`: 动作ID | Action ID

### ➖ removeContextMenuItem - 移除菜单项 | Remove Context Menu Item

```cpp
void removeContextMenuItem(const QString& actionId);
```

**功能 | Function**: 移除菜单项 | Removes menu item

**参数 | Parameters**:
- `actionId`: 动作ID | Action ID

### 👁️ setContextMenuItemVisible - 设置菜单项可见性 | Set Context Menu Item Visibility

```cpp
void setContextMenuItemVisible(const QString& actionId, bool visible);
```

**功能 | Function**: 设置菜单项可见性 | Sets menu item visibility

**参数 | Parameters**:
- `actionId`: 动作ID | Action ID
- `visible`: 是否可见 | Visibility

---

## 🔧 高级工具 | Advanced Tools

### 📊 getImageStatistics - 获取图像统计信息 | Get Image Statistics

```cpp
QMap<QString, double> getImageStatistics(HObject image, HObject region = HObject());
```

**功能 | Function**: 获取图像统计信息 | Gets image statistics

**参数 | Parameters**:
- `image`: 输入图像 | Input image
- `region`: 统计区域(可选) | Statistics region (optional)

**返回值 | Return**: `QMap<QString, double>` - 统计结果 | Statistics results

### 📐 getRegionFeatures - 获取区域几何特征 | Get Region Features

```cpp
QMap<QString, double> getRegionFeatures(HObject region);
```

**功能 | Function**: 获取区域几何特征 | Gets region geometric features

**参数 | Parameters**:
- `region`: 区域对象 | Region object

**返回值 | Return**: `QMap<QString, double>` - 特征结果 | Feature results

### 📊 calculateImageQualityScore - 计算图像质量评分 | Calculate Image Quality Score

```cpp
double calculateImageQualityScore(HObject image);
```

**功能 | Function**: 计算图像质量评分 | Calculates image quality score

**参数 | Parameters**:
- `image`: 输入图像 | Input image

**返回值 | Return**: `double` - 质量评分 | Quality score

---

## ⚡ 快捷操作 | Quick Operations

### 🔍 zoomToFit - 缩放到适合窗口 | Zoom to Fit Window

```cpp
void zoomToFit();
```

**功能 | Function**: 缩放到适合窗口 | Zooms to fit window

### 🔍 zoomToActualSize - 缩放到实际大小 | Zoom to Actual Size

```cpp
void zoomToActualSize();
```

**功能 | Function**: 缩放到实际大小 | Zooms to actual size

### 🔍 zoomToRatio - 缩放到指定比例 | Zoom to Specific Ratio

```cpp
void zoomToRatio(double ratio);
```

**功能 | Function**: 缩放到指定比例 | Zooms to specific ratio

**参数 | Parameters**:
- `ratio`: 缩放比例 | Zoom ratio

### ↶ undoLastOperation - 撤销上一步操作 | Undo Last Operation

```cpp
bool undoLastOperation();
```

**功能 | Function**: 撤销上一步操作 | Undoes last operation

**返回值 | Return**: `bool` - 撤销成功返回true | Returns true on success

### ↷ redoOperation - 重做操作 | Redo Operation

```cpp
bool redoOperation();
```

**功能 | Function**: 重做操作 | Redoes operation

**返回值 | Return**: `bool` - 重做成功返回true | Returns true on success

### 🖱️ getPixelValueAtPosition - 获取鼠标位置像素值 | Get Pixel Value at Mouse Position

```cpp
QString getPixelValueAtPosition(double x, double y);
```

**功能 | Function**: 快速获取鼠标位置的像素值 | Quickly gets pixel value at mouse position

**参数 | Parameters**:
- `x, y`: 位置坐标 | Position coordinates

**返回值 | Return**: `QString` - 像素值信息 | Pixel value information

---

## 🎯 优化功能 | Optimization Features

### 🔄 PointRotateByCenter - 点绕中心旋转 | Point Rotate by Center

```cpp
pointStruct PointRotateByCenter(double centerX, double centerY, double oldX, double oldY, double angle);
```

**功能 | Function**: 计算点绕指定中心旋转后的新坐标 | Calculates new coordinates after rotating point around center

**参数 | Parameters**:
- `centerX, centerY`: 旋转中心坐标 | Rotation center coordinates
- `oldX, oldY`: 原始点坐标 | Original point coordinates
- `angle`: 旋转角度(度) | Rotation angle in degrees

**返回值 | Return**: `pointStruct` - 旋转后的点坐标 | Rotated point coordinates

### 📊 像素信息显示 | Pixel Info Display

### 🔛 setPixelInfoDisplayEnabled - 设置像素信息显示 | Set Pixel Info Display

```cpp
void setPixelInfoDisplayEnabled(bool enabled);
```

**功能 | Function**: 设置实时像素信息显示开关 | Enables/disables real-time pixel info display

**参数 | Parameters**:
- `enabled`: 是否启用 | Whether to enable

### ❓ isPixelInfoDisplayEnabled - 获取像素信息显示状态 | Is Pixel Info Display Enabled

```cpp
bool isPixelInfoDisplayEnabled() const;
```

**功能 | Function**: 获取实时像素信息显示状态 | Gets real-time pixel info display status

**返回值 | Return**: `bool` - 是否启用 | Whether enabled

### 🔄 updatePixelInfoDisplay - 更新像素信息显示 | Update Pixel Info Display

```cpp
void updatePixelInfoDisplay(double imageX, double imageY);
```

**功能 | Function**: 更新实时像素信息显示 | Updates real-time pixel info display

**参数 | Parameters**:
- `imageX, imageY`: 图像坐标 | Image coordinates

### 📋 getDetailedPixelInfo - 获取详细像素信息 | Get Detailed Pixel Info

```cpp
QString getDetailedPixelInfo(double x, double y);
```

**功能 | Function**: 获取详细像素信息(支持彩色图像) | Gets detailed pixel info (supports color images)

**参数 | Parameters**:
- `x, y`: 像素坐标 | Pixel coordinates

**返回值 | Return**: `QString` - 详细像素信息 | Detailed pixel information

---

## 🎨 便捷功能方法 | Convenience Methods

### 🛡️ QtGetLocalImageSafe - 安全读取本地图像 | Safe Read Local Image

```cpp
bool QtGetLocalImageSafe(HObject& mImg, QString& errorMessage);
```

**功能 | Function**: 安全读取本地图像 | Safely reads local image

**参数 | Parameters**:
- `mImg`: 输出图像对象 | Output image object
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `bool` - 读取成功返回true | Returns true on success

### 🛡️ QtSaveImageSafe - 安全保存图像 | Safe Save Image

```cpp
bool QtSaveImageSafe(HObject mImg, QString& errorMessage);
```

**功能 | Function**: 安全保存图像 | Safely saves image

**参数 | Parameters**:
- `mImg`: 要保存的图像对象 | Image object to save
- `errorMessage`: 错误消息输出 | Error message output

**返回值 | Return**: `bool` - 保存成功返回true | Returns true on success

### ➕ addDisplayObject - 添加显示对象到列表 | Add Display Object to List

```cpp
void addDisplayObject(HObject obj, QString color = "green", double lineWidth = 2.0);
```

**功能 | Function**: 添加显示对象到列表 | Adds display object to list

**参数 | Parameters**:
- `obj`: 显示对象 | Display object
- `color`: 颜色 | Color (default: "green")
- `lineWidth`: 线条宽度 | Line width (default: 2.0)

### 📊 getDisplayObjectsCount - 获取显示对象数量 | Get Display Objects Count

```cpp
int getDisplayObjectsCount() const;
```

**功能 | Function**: 获取显示对象数量 | Gets display objects count

**返回值 | Return**: `int` - 显示对象数量 | Number of display objects

### ❌ removeDisplayObjectByIndex - 清除指定索引的显示对象 | Remove Display Object by Index

```cpp
bool removeDisplayObjectByIndex(int index);
```

**功能 | Function**: 清除指定索引的显示对象 | Removes display object by index

**参数 | Parameters**:
- `index`: 对象索引 | Object index

**返回值 | Return**: `bool` - 移除成功返回true | Returns true on success

### ✅ isImageLoaded - 检查图像是否已加载 | Check if Image is Loaded

```cpp
bool isImageLoaded() const;
```

**功能 | Function**: 检查图像是否已加载 | Checks if image is loaded

**返回值 | Return**: `bool` - 图像已加载返回true | Returns true if image is loaded

### ℹ️ getImageInfo - 获取图像尺寸信息 | Get Image Size Info

```cpp
QString getImageInfo() const;
```

**功能 | Function**: 获取图像尺寸信息 | Gets image size info

**返回值 | Return**: `QString` - 图像信息 | Image information

---

## 📈 使用示例 | Usage Examples

### 🎯 完整工作流程示例 | Complete Workflow Example

```cpp
// 创建控件 | Create widget
HalconLable *visionWidget = new HalconLable(this);

// 设置像素信息显示 | Enable pixel info display
visionWidget->setPixelInfoDisplayEnabled(true);

// 加载图像 | Load image
HTuple imagePath = "test_image.bmp";
HObject image = visionWidget->QtReadImage(imagePath);

// 创建检测区域 | Create detection region
HObject region = visionWidget->genAngleRec("yellow");

// 形状模板创建 | Create shape template
ShapeModelData model = visionWidget->QtCreateShapeModel(
    image, region, 30, 10, "template.shm");

// 添加文本标注 | Add text annotation
visionWidget->addTextAnnotation("模板区域 | Template Region", 
    100, 50, "white", 16);

// 测量区域面积 | Measure region area
double area = visionWidget->calculateRegionArea(region);
QString areaText = QString("面积 | Area: %1 pixels²").arg(area);
visionWidget->addTextAnnotation(areaText, 100, 80, "yellow", 14);

// 保存结果 | Save results
QString errorMsg;
bool saved = visionWidget->QtSaveImageSafe(image, errorMsg);
if (saved) {
    qDebug() << "保存成功 | Saved successfully";
} else {
    qDebug() << "保存失败 | Save failed:" << errorMsg;
}
```

### 🤖 手眼标定示例 | Hand-Eye Calibration Example

```cpp
// 创建标定数据 | Create calibration data
QString errorMsg;
HTuple calibData = visionWidget->createHandEyeCalibrationData(errorMsg);

// 添加多个标定姿态 | Add multiple calibration poses
for (int i = 0; i < 10; i++) {
    HTuple cameraPose, robotPose;
    // ... 获取姿态数据 | Get pose data
    visionWidget->addCalibrationPose(calibData, cameraPose, robotPose, errorMsg);
}

// 执行标定 | Perform calibration
HTuple transformation = visionWidget->performHandEyeCalibration(calibData, errorMsg);

// 验证精度 | Validate accuracy
auto accuracy = visionWidget->validateCalibrationAccuracy(
    calibData, transformation, errorMsg);

// 保存结果 | Save results
visionWidget->saveCalibrationResults(transformation, "hand_eye_calib.dat", errorMsg);
```

---

## 🔔 信号槽机制 | Signal-Slot Mechanism

### 📡 可用信号 | Available Signals

```cpp
// 右键菜单动作触发信号 | Context menu action triggered signal
void contextMenuActionTriggered(const QString& actionId);

// 3D检测完成信号 | 3D detection completed signal  
void detection3DCompleted(const QMap<QString, QVariant>& results);

// 手眼标定完成信号 | Hand-eye calibration completed signal
void handEyeCalibrationCompleted(bool success, const QString& message);
```

### 🔗 信号连接示例 | Signal Connection Example

```cpp
// 连接右键菜单信号 | Connect context menu signal
connect(visionWidget, &HalconLable::contextMenuActionTriggered,
        this, [this](const QString& actionId) {
    if (actionId == "save_image") {
        // 保存图像 | Save image
        QString errorMsg;
        visionWidget->QtSaveImageSafe(currentImage, errorMsg);
    } else if (actionId == "zoom_fit") {
        // 缩放适合 | Zoom to fit
        visionWidget->zoomToFit();
    }
});

// 连接3D检测信号 | Connect 3D detection signal
connect(visionWidget, &HalconLable::detection3DCompleted,
        this, [this](const QMap<QString, QVariant>& results) {
    // 处理3D检测结果 | Process 3D detection results
    for (auto it = results.begin(); it != results.end(); ++it) {
        qDebug() << it.key() << ":" << it.value();
    }
});
```

---

## ⚠️ 注意事项 | Important Notes

### 🧵 线程安全 | Thread Safety
- `HalconLable` 不是线程安全的 | `HalconLable` is not thread-safe
- 所有操作应在GUI线程中进行 | All operations should be performed in GUI thread
- 相机操作建议在单独线程处理，结果在GUI线程显示 | Camera operations recommended in separate thread, results displayed in GUI thread

### 💾 内存管理 | Memory Management
```cpp
// Halcon对象会自动管理内存，但大量对象时建议手动清理
// Halcon objects automatically manage memory, but manual cleanup recommended for large objects
HObject largeImage;
// ... 使用后 | After use
largeImage.Clear(); // 手动释放 | Manual release
```

### 🎨 性能优化 | Performance Optimization
```cpp
// 对于高频显示，建议控制帧率 | For high-frequency display, recommend controlling frame rate
QTimer *displayTimer = new QTimer();
displayTimer->setSingleShot(true);
displayTimer->setInterval(33); // 30 FPS
```

---

<div align="center">

**🎉 HalconLable API 手册完整版 | Complete HalconLable API Manual**

**感谢您的使用！如有疑问请参考使用手册或联系技术支持**  
**Thank you for using! Please refer to the user manual or contact technical support for questions**

---

**📝 文档更新记录 | Document Update Log**

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| v1.0 | 2025-05-31 | 🎯 完整API文档创建，包含所有公共方法说明 |
|      |            | 📚 中英双语API参考，详细参数说明和示例 |
|      |            | 🔧 包含3D检测、手眼标定等高级功能 |
|      |            | 💡 提供完整的使用示例和最佳实践 |

文档版本：**v1.0** | 更新日期：**2025-05-31** | 维护者：开发团队

</div> 