# 🔬 HalconLable API 使用手册 | HalconLable API Manual

<div align="center">

![HalconLable API](https://img.shields.io/badge/HalconLable-API_Manual-4CAF50?style=for-the-badge&logo=code&logoColor=white)
![C++](https://img.shields.io/badge/C++-API-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Halcon](https://img.shields.io/badge/Halcon-20.11-FF6B35?style=for-the-badge&logo=camera&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-5.14+-41CD52?style=for-the-badge&logo=qt&logoColor=white)

### 🎯 基于 Qt + Halcon 的机器视觉控件完整API参考 | Complete API Reference for Qt + Halcon Vision Widget

**专业 • 全面 • 易用 | Professional • Comprehensive • User-friendly**

</div>

---

## 📋 目录 | Table of Contents

- [🏗️ 数据结构](#-数据结构--data-structures)
- [🔧 构造函数](#-构造函数--constructor)
- [📷 相机操作](#-相机操作--camera-operations)
- [🖼️ 图像处理](#-图像处理--image-processing)
- [🔍 形状检测](#-形状检测--shape-detection)
- [🎨 显示功能](#-显示功能--display-functions)
- [📊 模板匹配](#-模板匹配--template-matching)
- [🔲 二维码识别](#-二维码识别--qr-code-recognition)
- [📏 测量分析](#-测量分析--measurement-analysis)
- [📝 图像标注](#-图像标注--image-annotation)
- [📋 ROI管理](#-roi管理--roi-management)
- [🧊 3D检测](#-3d检测--3d-detection)
- [🤖 手眼标定](#-手眼标定--hand-eye-calibration)
- [🎮 交互功能](#-交互功能--interactive-features)
- [🔧 高级工具](#-高级工具--advanced-tools)
- [⚡ 快捷操作](#-快捷操作--quick-operations)
- [🎯 优化功能](#-优化功能--optimization-features)

---

## 🏗️ 数据结构 | Data Structures

### 📍 pointStruct - 点结构体 | Point Structure

```cpp
struct pointStruct {
    double X = 0;        // X坐标 | X coordinate
    double Y = 0;        // Y坐标 | Y coordinate  
    double Angle = 0;    // 角度 | Angle (degrees)
};
```

**用途 | Purpose**: 表示二维平面上的一个点，包含位置和角度信息 | Represents a point in 2D plane with position and angle

**示例 | Example**:
```cpp
pointStruct centerPoint;
centerPoint.X = 320.5;
centerPoint.Y = 240.7;
centerPoint.Angle = 45.0;
```

### 🔷 ShapeModelData - 形状模板数据 | Shape Model Data

```cpp
struct ShapeModelData {
    HTuple R;               // 行坐标 | Row coordinate
    HTuple C;               // 列坐标 | Column coordinate
    HTuple Phi;             // 角度 | Angle
    HTuple ModelID;         // 模板ID | Model ID
    HObject modelregion;    // 模板区域 | Model region
};
```

**用途 | Purpose**: 存储形状模板匹配的结果数据 | Stores shape template matching result data

### 📏 LineData - 直线数据 | Line Data

```cpp
struct LineData {
    HTuple R1, C1;      // 起点坐标 | Start point coordinates
    HTuple R2, C2;      // 终点坐标 | End point coordinates
    HObject orgXld;     // 原始轮廓 | Original contour
    HObject LineXld;    // 直线轮廓 | Line contour
};
```

**用途 | Purpose**: 存储直线检测的结果数据 | Stores line detection result data

### 🔲 CodeData - 二维码数据 | Code Data

```cpp
struct CodeData {
    QList<QString> codestring;  // 解码字符串列表 | Decoded string list
    HObject codeobj;            // 二维码对象 | Code object
};
```

**用途 | Purpose**: 存储二维码识别的结果数据 | Stores QR code recognition result data

---

## 🔧 构造函数 | Constructor

### HalconLable(QWidget *parent = nullptr)

```cpp
HalconLable(QWidget *parent = nullptr);
```

**功能 | Function**: 创建 HalconLable 控件实例 | Creates HalconLable widget instance

**参数 | Parameters**:
- `parent`: 父窗口指针 | Parent widget pointer

**示例 | Example**:
```cpp
// 创建控件 | Create widget
HalconLable *visionWidget = new HalconLable(this);

// 设置属性 | Set properties
visionWidget->setGeometry(10, 10, 800, 600);
visionWidget->setMouseTracking(true);
visionWidget->setFocusPolicy(Qt::StrongFocus);
```

---

## 📷 相机操作 | Camera Operations

### 🔛 QtOpenCam - 开启相机 | Open Camera

```cpp
bool QtOpenCam();
```

**功能 | Function**: 开启工业相机并开始采集 | Opens industrial camera and starts acquisition

**返回值 | Return**: `bool` - 成功返回true | Returns true on success

**支持相机类型 | Supported Camera Types**:
- DirectShow cameras
- GigE Vision cameras
- USB3 Vision cameras
- GenICam compatible cameras

**示例 | Example**:
```cpp
if (halconWidget->QtOpenCam()) {
    qDebug() << "相机开启成功 | Camera opened successfully";
    grabTimer->start(33); // 30 FPS
} else {
    qDebug() << "相机开启失败 | Camera failed to open";
}
```

### 📸 QtGrabImg - 抓取图像 | Grab Image

```cpp
HObject QtGrabImg();
```

**功能 | Function**: 从相机抓取一帧图像 | Grabs one frame from camera

**返回值 | Return**: `HObject` - 抓取的图像对象 | Grabbed image object

**示例 | Example**:
```cpp
HObject liveImage = halconWidget->QtGrabImg();
if (liveImage.IsInitialized()) {
    halconWidget->showImage(liveImage);
}
```

### 🔚 QtCloseCam - 关闭相机 | Close Camera

```cpp
void QtCloseCam();
```

**功能 | Function**: 关闭相机并释放资源 | Closes camera and releases resources

**示例 | Example**:
```cpp
halconWidget->QtCloseCam();
qDebug() << "相机已关闭 | Camera closed";
```

---

## 🖼️ 图像处理 | Image Processing

### 📂 QtReadImage - 读取图像 | Read Image

```cpp
HObject QtReadImage(const HTuple filePath);
```

**功能 | Function**: 从文件路径读取图像 | Reads image from file path

**参数 | Parameters**:
- `filePath`: 图像文件路径 | Image file path

**支持格式 | Supported Formats**: BMP, JPG, PNG, TIFF, HDR

**示例 | Example**:
```cpp
HTuple imagePath = "D:/images/test.bmp";
HObject image = halconWidget->QtReadImage(imagePath);
```

### 🖥️ showImage - 显示图像 | Show Image

```cpp
void showImage(HObject inputImage);
```

**功能 | Function**: 在控件中显示指定的图像 | Displays specified image in widget

**参数 | Parameters**:
- `inputImage`: 要显示的图像对象 | Image object to display

**示例 | Example**:
```cpp
HObject myImage;
ReadImage(&myImage, "test.jpg");
halconWidget->showImage(myImage);
```

### 🎨 showHalconObject - 显示Halcon对象 | Show Halcon Object

```cpp
void showHalconObject(HObject hObject, QString colorStr, double lineWidth = 2);
```

**功能 | Function**: 在图像上叠加显示Halcon对象 | Overlays Halcon objects on image

**参数 | Parameters**:
- `hObject`: 要显示的Halcon对象 | Halcon object to display
- `colorStr`: 显示颜色 | Display color
- `lineWidth`: 线条宽度 | Line width (default: 2)

**示例 | Example**:
```cpp
HObject region;
GenRectangle1(&region, 100, 100, 200, 200);
halconWidget->showHalconObject(region, "red", 3);
```

### 💾 QtSaveImage - 保存图像 | Save Image

```cpp
bool QtSaveImage(HObject mImg);
```

**功能 | Function**: 保存图像到文件 | Saves image to file

**参数 | Parameters**:
- `mImg`: 要保存的图像对象 | Image object to save

**返回值 | Return**: `bool` - 保存成功返回true | Returns true on success

### 📥 QtGetLocalImage - 读取本地图像 | Get Local Image

```cpp
bool QtGetLocalImage(HObject& mImg);
```

**功能 | Function**: 从本地文件读取图像 | Reads image from local file

**参数 | Parameters**:
- `mImg`: 输出图像对象 | Output image object

**返回值 | Return**: `bool` - 读取成功返回true | Returns true on success

---

## 🔍 形状检测 | Shape Detection

### 📐 genAngleRec - 生成有角度矩形 | Generate Angled Rectangle

```cpp
HObject genAngleRec(QString color, QString SaveFile = "");
```

**功能 | Function**: 交互式绘制带角度的矩形区域 | Interactively draws angled rectangle region

**参数 | Parameters**:
- `color`: 显示颜色 | Display color
- `SaveFile`: 保存文件路径(可选) | Save file path (optional)

**返回值 | Return**: `HObject` - 生成的矩形区域 | Generated rectangle region

**示例 | Example**:
```cpp
HObject angleRect = halconWidget->genAngleRec("yellow", "angle_rect.hobj");
```

### ⬜ genNoAngleRec - 生成无角度矩形 | Generate Rectangle

```cpp
HObject genNoAngleRec();
```

**功能 | Function**: 交互式绘制水平矩形区域 | Interactively draws horizontal rectangle region

**返回值 | Return**: `HObject` - 生成的矩形区域 | Generated rectangle region

### ⭕ GenCircleRegionOne - 生成圆形区域 | Generate Circle Region

```cpp
HObject GenCircleRegionOne();
```

**功能 | Function**: 交互式绘制圆形区域 | Interactively draws circle region

**返回值 | Return**: `HObject` - 生成的圆形区域 | Generated circle region

### 🎨 GenAnyShapeRegion - 生成任意形状区域 | Generate Any Shape Region

```cpp
HObject GenAnyShapeRegion();
```

**功能 | Function**: 交互式绘制任意形状区域 | Interactively draws any shape region

**返回值 | Return**: `HObject` - 生成的任意形状区域 | Generated any shape region

### 📏 GetLineRegion - 获取直线区域 | Get Line Region

```cpp
HTuple GetLineRegion();
```

**功能 | Function**: 交互式绘制直线 | Interactively draws line

**返回值 | Return**: `HTuple` - 直线参数 | Line parameters

---

## 🎨 显示功能 | Display Functions

### ➕ DispCoorCross - 显示十字线 | Display Cross

```cpp
void DispCoorCross(double x, double y, int width, double angle = 0, QString color = "green");
```

**功能 | Function**: 在指定位置显示十字线标记 | Displays cross marker at specified position

**参数 | Parameters**:
- `x, y`: 十字线中心坐标 | Cross center coordinates
- `width`: 十字线大小 | Cross size
- `angle`: 旋转角度 | Rotation angle (default: 0)
- `color`: 颜色 | Color (default: "green")

**示例 | Example**:
```cpp
halconWidget->DispCoorCross(400, 300, 20, 0, "red");
```

### 📝 dispHalconMessage - 显示文本消息 | Display Message

```cpp
void dispHalconMessage(int positionX, int positionY, QString text, QString color = "green");
```

**功能 | Function**: 在图像上显示文本信息 | Displays text information on image

**参数 | Parameters**:
- `positionX, positionY`: 文本显示位置 | Text display position
- `text`: 要显示的文本内容 | Text content to display
- `color`: 文本颜色 | Text color (default: "green")

### 🖼️ QtShowObj - 显示对象 | Show Object

```cpp
void QtShowObj(HTuple MarginFill, HTuple color, HTuple lineW, HObject obj);
```

**功能 | Function**: 以指定样式显示Halcon对象 | Displays Halcon object with specified style

**参数 | Parameters**:
- `MarginFill`: 填充模式 | Fill mode
- `color`: 显示颜色 | Display color
- `lineW`: 线条宽度 | Line width
- `obj`: 要显示的对象 | Object to display

### 🗑️ RemoveShow - 清除显示 | Remove Display

```cpp
void RemoveShow();
```

**功能 | Function**: 清除所有叠加显示的内容 | Clears all overlay display content

### 🧹 clearDisplayObjectsOnly - 清除显示对象 | Clear Display Objects Only

```cpp
void clearDisplayObjectsOnly();
```

**功能 | Function**: 清除显示对象但保留图像 | Clears display objects but keeps image

---

## 📊 模板匹配 | Template Matching

### 🎯 QtCreateShapeModel - 创建形状模板 | Create Shape Model

```cpp
ShapeModelData QtCreateShapeModel(HObject img, HObject region, HTuple contrast, HTuple mincontrast, QString file);
```

**功能 | Function**: 基于图像和区域创建形状匹配模板 | Creates shape matching template based on image and region

**参数 | Parameters**:
- `img`: 模板图像 | Template image
- `region`: 模板区域 | Template region
- `contrast`: 对比度阈值 | Contrast threshold
- `mincontrast`: 最小对比度 | Minimum contrast
- `file`: 模板保存文件路径 | Template save file path

**返回值 | Return**: `ShapeModelData` - 包含模板信息的结构体 | Structure containing template information

**示例 | Example**:
```cpp
HObject templateImage, templateRegion;
ReadImage(&templateImage, "template.bmp");
GenRectangle1(&templateRegion, 100, 100, 200, 200);

ShapeModelData model = halconWidget->QtCreateShapeModel(
    templateImage, templateRegion, 30, 10, "model.shm");
```

### 📏 QtGenLine - 生成直线模型 | Generate Line Model

```cpp
LineData QtGenLine(HObject img, HObject region, HTuple threshold);
```

**功能 | Function**: 在指定区域内检测直线 | Detects lines within specified region

**参数 | Parameters**:
- `img`: 输入图像 | Input image
- `region`: 检测区域 | Detection region
- `threshold`: 阈值 | Threshold

**返回值 | Return**: `LineData` - 包含直线信息的结构体 | Structure containing line information

### 📐 QtGetLengthMaxXld - 获取最长轮廓 | Get Longest Contour

```cpp
HObject QtGetLengthMaxXld(HObject Img, HObject CheckRegion, int Thr1);
```

**功能 | Function**: 在指定区域内找到最长的轮廓 | Finds longest contour within specified region

**参数 | Parameters**:
- `Img`: 输入图像 | Input image
- `CheckRegion`: 检测区域 | Check region
- `Thr1`: 阈值 | Threshold

**返回值 | Return**: `HObject` - 最长的轮廓对象 | Longest contour object

---

## 🔲 二维码识别 | QR Code Recognition

### 🔧 QtData2d - 创建二维码模型 | Create 2D Code Model

```cpp
HTuple QtData2d(HTuple CodeKind, HTuple polarity);
```

**功能 | Function**: 创建二维码检测模型 | Creates 2D code detection model

**参数 | Parameters**:
- `CodeKind`: 二维码类型 | Code type ("QR Code", "Data Matrix", etc.)
- `polarity`: 极性设置 | Polarity setting

**返回值 | Return**: `HTuple` - 二维码模型句柄 | 2D code model handle

### 🔍 QtRecogied - 识别二维码 | Recognize 2D Code

```cpp
CodeData QtRecogied(HObject img, HTuple codeModel, HTuple num);
```

**功能 | Function**: 使用模型识别图像中的二维码 | Recognizes 2D codes in image using model

**参数 | Parameters**:
- `img`: 输入图像 | Input image
- `codeModel`: 二维码模型 | Code model
- `num`: 最大识别数量 | Maximum number to recognize

**返回值 | Return**: `CodeData` - 包含识别结果的结构体 | Structure containing recognition results

**示例 | Example**:
```cpp
HTuple qrModel = halconWidget->QtData2d("QR Code", "auto");
CodeData result = halconWidget->QtRecogied(qrImage, qrModel, 10);

for (const QString &code : result.codestring) {
    qDebug() << "识别到二维码 | Detected QR code:" << code;
}
```

---

## 📏 测量分析 | Measurement Analysis

### 📐 calculatePointDistance - 计算两点距离 | Calculate Point Distance

```cpp
double calculatePointDistance(double x1, double y1, double x2, double y2);
```

**功能 | Function**: 计算两点之间的距离 | Calculates distance between two points

**参数 | Parameters**:
- `x1, y1`: 第一个点坐标 | First point coordinates
- `x2, y2`: 第二个点坐标 | Second point coordinates

**返回值 | Return**: `double` - 两点间距离 | Distance between points

### 📐 calculateThreePointAngle - 计算三点角度 | Calculate Three Point Angle

```cpp
double calculateThreePointAngle(double x1, double y1, double x2, double y2, double x3, double y3);
```

**功能 | Function**: 计算三点构成的角度 | Calculates angle formed by three points

**参数 | Parameters**:
- `x1, y1`: 第一个点坐标 | First point coordinates
- `x2, y2`: 顶点坐标 | Vertex coordinates
- `x3, y3`: 第三个点坐标 | Third point coordinates

**返回值 | Return**: `double` - 角度值(度) | Angle value in degrees

### 📊 calculateRegionArea - 计算区域面积 | Calculate Region Area

```cpp
double calculateRegionArea(HObject region);
```

**功能 | Function**: 计算区域面积 | Calculates region area

**参数 | Parameters**:
- `region`: 区域对象 | Region object

**返回值 | Return**: `double` - 区域面积 | Region area

### 🎯 calculateRegionCentroid - 计算区域重心 | Calculate Region Centroid

```cpp
pointStruct calculateRegionCentroid(HObject region);
```

**功能 | Function**: 计算区域重心 | Calculates region centroid

**参数 | Parameters**:
- `region`: 区域对象 | Region object

**返回值 | Return**: `pointStruct` - 重心坐标 | Centroid coordinates

### 📦 getMinimumBoundingRect - 获取最小外接矩形 | Get Minimum Bounding Rectangle

```cpp
QRect getMinimumBoundingRect(HObject region);
```

**功能 | Function**: 获取最小外接矩形 | Gets minimum bounding rectangle

**参数 | Parameters**:
- `region`: 区域对象 | Region object

**返回值 | Return**: `QRect` - 外接矩形 | Bounding rectangle

---

**⏳ 文档分段显示中... | Document segmented display in progress...**

**📝 这是第一部分，包含了核心API。完整文档将继续在下一部分展示 | This is Part 1 containing core APIs. Complete documentation continues in next part**

---

文档版本：**v1.0** | 更新日期：**2025-05-31** | 维护者：开发团队 