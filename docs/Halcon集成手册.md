# MyOperation Halcon集成手册

## 概述

本手册详细介绍了MyOperation系统中Halcon机器视觉库的集成方式、使用方法和开发指南。Halcon是MVTec公司开发的专业机器视觉软件包，提供了丰富的图像处理和分析算法。

## Halcon环境配置

### 系统要求
- Halcon版本: 20.11 或更高版本
- 操作系统: Windows 10/11 64位
- Qt版本: 5.12 或更高版本
- 编译器: MSVC 2019 或更高版本

### 安装配置

#### 1. Halcon安装
```bash
# 安装Halcon到默认目录
C:\Program Files\MVTec\HALCON-20.11-Steady
```

#### 2. 环境变量设置
```bash
HALCONROOT=C:\Program Files\MVTec\HALCON-20.11-Steady
HALCONARCH=x64-win64
PATH=%PATH%;%HALCONROOT%\bin\%HALCONARCH%
```

#### 3. CMake配置
```cmake
# CMakeLists.txt中的Halcon配置
set(HALCON_ROOT "C:/Program Files/MVTec/HALCON-20.11-Steady")
set(HALCON_ARCH "x64-win64")

# 包含目录
include_directories(${HALCON_ROOT}/include)
include_directories(${HALCON_ROOT}/include/halconcpp)

# 库目录
link_directories(${HALCON_ROOT}/lib/${HALCON_ARCH})

# 链接库
target_link_libraries(MyOperation
    halcon.lib
    halconcpp.lib
)
```

## 核心组件

### 1. HalconLable (图像显示组件)

#### 类定义
```cpp
class HalconLable : public QLabel
{
    Q_OBJECT
public:
    explicit HalconLable(QWidget *parent = nullptr);
    ~HalconLable();
```

#### 主要功能
- Halcon图像显示
- 鼠标交互处理
- 图像缩放和平移
- ROI区域选择
- 测量结果叠加显示

#### 关键方法

##### 图像操作
```cpp
// 设置显示图像
void setHalconImage(const HalconCpp::HImage& image);
void setHalconImage(const QString& imagePath);

// 清除图像
void clearImage();

// 获取当前图像
HalconCpp::HImage getCurrentImage() const;
```

##### 显示控制
```cpp
// 缩放控制
void zoomIn();
void zoomOut();
void zoomFit();
void setZoomFactor(double factor);
double getZoomFactor() const;

// 平移控制
void panImage(int dx, int dy);
void centerImage();
```

##### ROI操作
```cpp
// 设置ROI
void setROI(const QRect& roi);
void setROI(const HalconCpp::HRegion& region);

// 获取ROI
QRect getROI() const;
HalconCpp::HRegion getHalconROI() const;

// ROI绘制模式
void setROIDrawingMode(ROIDrawingMode mode);
void enableROIDrawing(bool enable);
```

##### 结果显示
```cpp
// 显示测量结果
void drawMeasurementResults(const QList<QPointF>& points);
void drawBoundingBox(const QRectF& box);
void drawCircle(const QPointF& center, double radius);
void drawLine(const QLineF& line);

// 显示文本信息
void drawText(const QString& text, const QPointF& position);
void clearOverlay();
```

#### 信号
```cpp
signals:
    void imageClicked(const QPointF& position);
    void roiChanged(const QRect& roi);
    void zoomChanged(double factor);
    void mouseMoved(const QPointF& position);
```

#### 使用示例
```cpp
#include "HalconLable.h"
#include <HalconCpp.h>

// 创建图像显示组件
HalconLable* halconLabel = new HalconLable(this);
halconLabel->resize(640, 480);

// 连接信号槽
connect(halconLabel, &HalconLable::imageClicked,
        this, &MainWindow::onImageClicked);
connect(halconLabel, &HalconLable::roiChanged,
        this, &MainWindow::onROIChanged);

// 加载并显示图像
try {
    HalconCpp::HImage image;
    image.ReadImage("test_image.bmp");
    halconLabel->setHalconImage(image);
    halconLabel->zoomFit();
} catch (const HalconCpp::HException& e) {
    qWarning() << "Halcon error:" << e.ErrorMessage().Text();
}
```

### 2. HalconFileManager (文件管理器)

#### 类定义
```cpp
class HalconFileManager : public QObject
{
    Q_OBJECT
public:
    explicit HalconFileManager(QObject *parent = nullptr);
```

#### 主要功能
- 图像文件管理
- 格式转换
- 批处理支持
- 元数据管理

#### 关键方法

##### 文件操作
```cpp
// 加载图像
HalconCpp::HImage loadImage(const QString& filePath);
bool loadImageSequence(const QString& pattern, QList<HalconCpp::HImage>& images);

// 保存图像
bool saveImage(const HalconCpp::HImage& image, const QString& filePath, const QString& format = "bmp");
bool saveImageSequence(const QList<HalconCpp::HImage>& images, const QString& pattern);

// 格式转换
bool convertImageFormat(const QString& inputPath, const QString& outputPath, const QString& format);
```

##### 文件信息
```cpp
// 获取图像信息
QSize getImageSize(const QString& filePath);
QString getImageFormat(const QString& filePath);
QDateTime getImageTimestamp(const QString& filePath);

// 支持的格式
QStringList getSupportedFormats() const;
bool isFormatSupported(const QString& format) const;
```

#### 使用示例
```cpp
#include "HalconFileManager.h"

HalconFileManager* fileManager = new HalconFileManager(this);

// 加载图像
HalconCpp::HImage image = fileManager->loadImage("input.jpg");

// 保存为不同格式
fileManager->saveImage(image, "output.bmp", "bmp");
fileManager->saveImage(image, "output.png", "png");

// 批量处理
QList<HalconCpp::HImage> images;
fileManager->loadImageSequence("images/*.bmp", images);
```

## 图像处理算法

### 1. 基础图像处理

#### 图像增强
```cpp
// 直方图均衡化
HalconCpp::HImage enhanceImage(const HalconCpp::HImage& inputImage) {
    HalconCpp::HImage outputImage;
    try {
        // 转换为灰度图像
        HalconCpp::HImage grayImage = inputImage.Rgb1ToGray();
        
        // 直方图均衡化
        outputImage = grayImage.EqualizeHist();
        
    } catch (const HalconCpp::HException& e) {
        qWarning() << "Image enhancement failed:" << e.ErrorMessage().Text();
    }
    return outputImage;
}

// 噪声滤波
HalconCpp::HImage filterNoise(const HalconCpp::HImage& inputImage, const QString& filterType) {
    HalconCpp::HImage outputImage;
    try {
        if (filterType == "median") {
            outputImage = inputImage.MedianImage("circle", 3, "mirrored");
        } else if (filterType == "gauss") {
            outputImage = inputImage.GaussFilter(1.5);
        } else if (filterType == "mean") {
            outputImage = inputImage.MeanImage("circle", 3, "mirrored");
        }
    } catch (const HalconCpp::HException& e) {
        qWarning() << "Noise filtering failed:" << e.ErrorMessage().Text();
    }
    return outputImage;
}
```

#### 边缘检测
```cpp
// Canny边缘检测
HalconCpp::HRegion detectEdges(const HalconCpp::HImage& inputImage, double lowThreshold, double highThreshold) {
    HalconCpp::HRegion edges;
    try {
        // 转换为灰度图像
        HalconCpp::HImage grayImage = inputImage.Rgb1ToGray();
        
        // Canny边缘检测
        HalconCpp::HImage edgeImage = grayImage.EdgesCannyAlpha(lowThreshold, highThreshold, "thin", "nms", 0.7, "both");
        
        // 转换为区域
        edges = edgeImage.Threshold(128, 255);
        
    } catch (const HalconCpp::HException& e) {
        qWarning() << "Edge detection failed:" << e.ErrorMessage().Text();
    }
    return edges;
}

// Sobel边缘检测
HalconCpp::HImage detectEdgesSobel(const HalconCpp::HImage& inputImage) {
    HalconCpp::HImage edges;
    try {
        HalconCpp::HImage grayImage = inputImage.Rgb1ToGray();
        edges = grayImage.SobelAmp("sum_abs", 3);
    } catch (const HalconCpp::HException& e) {
        qWarning() << "Sobel edge detection failed:" << e.ErrorMessage().Text();
    }
    return edges;
}
```

### 2. 形状检测和测量

#### 圆形检测
```cpp
struct CircleResult {
    QPointF center;
    double radius;
    double score;
};

QList<CircleResult> detectCircles(const HalconCpp::HImage& inputImage, double minRadius, double maxRadius) {
    QList<CircleResult> results;
    try {
        // 转换为灰度图像
        HalconCpp::HImage grayImage = inputImage.Rgb1ToGray();
        
        // 边缘检测
        HalconCpp::HRegion edges = grayImage.EdgesSubPix("canny", 0.5, 20, 40);
        
        // Hough圆变换
        HalconCpp::HTuple rows, cols, radii, scores;
        edges.HoughCircles(grayImage, minRadius, maxRadius, 0.1, 2, 1, "true", 3, rows, cols, radii, scores);
        
        // 解析结果
        for (int i = 0; i < rows.Length(); i++) {
            CircleResult result;
            result.center = QPointF(cols[i].D(), rows[i].D());
            result.radius = radii[i].D();
            result.score = scores[i].D();
            results.append(result);
        }
        
    } catch (const HalconCpp::HException& e) {
        qWarning() << "Circle detection failed:" << e.ErrorMessage().Text();
    }
    return results;
}
```

#### 直线检测
```cpp
struct LineResult {
    QPointF startPoint;
    QPointF endPoint;
    double angle;
    double length;
};

QList<LineResult> detectLines(const HalconCpp::HImage& inputImage, double minLength) {
    QList<LineResult> results;
    try {
        HalconCpp::HImage grayImage = inputImage.Rgb1ToGray();
        
        // 边缘检测
        HalconCpp::HRegion edges = grayImage.EdgesSubPix("canny", 0.5, 20, 40);
        
        // 直线检测
        HalconCpp::HTuple rowBegin, colBegin, rowEnd, colEnd;
        edges.SelectShapeXld("contlength", "and", minLength, 99999);
        edges.FitLineContourXld("tukey", -1, 0, 5, 2, rowBegin, colBegin, rowEnd, colEnd);
        
        // 解析结果
        for (int i = 0; i < rowBegin.Length(); i++) {
            LineResult result;
            result.startPoint = QPointF(colBegin[i].D(), rowBegin[i].D());
            result.endPoint = QPointF(colEnd[i].D(), rowEnd[i].D());
            
            double dx = result.endPoint.x() - result.startPoint.x();
            double dy = result.endPoint.y() - result.startPoint.y();
            result.length = sqrt(dx*dx + dy*dy);
            result.angle = atan2(dy, dx) * 180.0 / M_PI;
            
            results.append(result);
        }
        
    } catch (const HalconCpp::HException& e) {
        qWarning() << "Line detection failed:" << e.ErrorMessage().Text();
    }
    return results;
}
```

### 3. 模板匹配

#### 形状模板匹配
```cpp
class HalconShapeModel {
private:
    HalconCpp::HShapeModel model;
    bool modelCreated;
    
public:
    HalconShapeModel() : modelCreated(false) {}
    
    // 创建模板
    bool createTemplate(const HalconCpp::HImage& templateImage, const HalconCpp::HRegion& roi) {
        try {
            HalconCpp::HImage grayTemplate = templateImage.Rgb1ToGray();
            model = grayTemplate.CreateShapeModel(1, 0, 6.28, "auto", "none", "use_polarity", 30, 10);
            modelCreated = true;
            return true;
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Template creation failed:" << e.ErrorMessage().Text();
            return false;
        }
    }
    
    // 模板匹配
    struct MatchResult {
        QPointF position;
        double angle;
        double score;
    };
    
    QList<MatchResult> findMatches(const HalconCpp::HImage& searchImage, double minScore = 0.7) {
        QList<MatchResult> results;
        if (!modelCreated) return results;
        
        try {
            HalconCpp::HImage graySearch = searchImage.Rgb1ToGray();
            
            HalconCpp::HTuple rows, cols, angles, scores;
            graySearch.FindShapeModel(model, 0, 6.28, minScore, 0, 0.5, "least_squares", 0, 0.9, rows, cols, angles, scores);
            
            for (int i = 0; i < rows.Length(); i++) {
                MatchResult result;
                result.position = QPointF(cols[i].D(), rows[i].D());
                result.angle = angles[i].D();
                result.score = scores[i].D();
                results.append(result);
            }
            
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Template matching failed:" << e.ErrorMessage().Text();
        }
        return results;
    }
};
```

### 4. 标定和测量

#### 相机标定
```cpp
class HalconCalibration {
private:
    HalconCpp::HCamPar cameraParameters;
    bool calibrated;
    
public:
    HalconCalibration() : calibrated(false) {}
    
    // 标定
    bool calibrateCamera(const QList<HalconCpp::HImage>& calibrationImages) {
        try {
            // 创建标定数据模型
            HalconCpp::HCalibData calibData;
            calibData.CreateCalibData("calibration_object", 1, 1);
            
            // 设置标定板参数
            calibData.SetCalibDataCalibObject(0, "caltab_30mm.descr");
            
            // 添加标定图像
            for (int i = 0; i < calibrationImages.size(); i++) {
                HalconCpp::HImage grayImage = calibrationImages[i].Rgb1ToGray();
                calibData.SetCalibDataObservPoints(grayImage, 0, 0, i);
            }
            
            // 执行标定
            HalconCpp::HTuple error;
            calibData.CalibrateCamera(error);
            calibData.GetCalibData("camera", 0, "params", cameraParameters);
            
            calibrated = true;
            return true;
            
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Camera calibration failed:" << e.ErrorMessage().Text();
            return false;
        }
    }
    
    // 像素到世界坐标转换
    QPointF pixelToWorld(const QPointF& pixelPoint) {
        if (!calibrated) return QPointF();
        
        try {
            HalconCpp::HTuple worldX, worldY;
            HalconCpp::HTuple pixelX = pixelPoint.x();
            HalconCpp::HTuple pixelY = pixelPoint.y();
            
            cameraParameters.ImagePointsToWorldPlane(pixelX, pixelY, "m", 1, worldX, worldY);
            
            return QPointF(worldX[0].D(), worldY[0].D());
            
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Coordinate conversion failed:" << e.ErrorMessage().Text();
            return QPointF();
        }
    }
};
```

## 性能优化

### 1. 内存管理
```cpp
// 使用智能指针管理Halcon对象
class HalconImageProcessor {
private:
    std::unique_ptr<HalconCpp::HImage> currentImage;
    
public:
    void processImage(const QString& imagePath) {
        try {
            // 释放之前的图像
            currentImage.reset();
            
            // 加载新图像
            currentImage = std::make_unique<HalconCpp::HImage>();
            currentImage->ReadImage(imagePath.toStdString().c_str());
            
            // 处理图像...
            
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Processing failed:" << e.ErrorMessage().Text();
        }
    }
};
```

### 2. 多线程处理
```cpp
// 异步图像处理
class HalconAsyncProcessor : public QObject {
    Q_OBJECT
    
public slots:
    void processImageAsync(const QString& imagePath) {
        QtConcurrent::run([this, imagePath]() {
            try {
                HalconCpp::HImage image;
                image.ReadImage(imagePath.toStdString().c_str());
                
                // 执行处理...
                HalconCpp::HImage processed = image.MedianImage("circle", 3, "mirrored");
                
                emit processingCompleted(processed);
                
            } catch (const HalconCpp::HException& e) {
                emit processingFailed(QString::fromStdString(e.ErrorMessage()));
            }
        });
    }
    
signals:
    void processingCompleted(const HalconCpp::HImage& result);
    void processingFailed(const QString& error);
};
```

## 调试和错误处理

### 1. 异常处理
```cpp
// 标准异常处理模式
void safeHalconOperation() {
    try {
        HalconCpp::HImage image;
        image.ReadImage("test.bmp");
        
        // 其他Halcon操作...
        
    } catch (const HalconCpp::HException& e) {
        // 获取详细错误信息
        QString errorCode = QString::number(e.ErrorCode());
        QString errorMessage = QString::fromStdString(e.ErrorMessage());
        QString procedure = QString::fromStdString(e.ProcName());
        QString parameter = QString::fromStdString(e.ParName());
        
        qCritical() << "Halcon Error:"
                   << "Code:" << errorCode
                   << "Message:" << errorMessage
                   << "Procedure:" << procedure
                   << "Parameter:" << parameter;
    }
}
```

### 2. 调试工具
```cpp
// Halcon变量查看器
class HalconDebugger {
public:
    static void dumpImageInfo(const HalconCpp::HImage& image, const QString& name) {
        try {
            HalconCpp::HTuple width, height, channels;
            image.GetImageSize(width, height);
            channels = image.CountChannels();
            
            qDebug() << "Image Info:" << name
                    << "Size:" << width[0].I() << "x" << height[0].I()
                    << "Channels:" << channels[0].I();
                    
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Cannot get image info:" << e.ErrorMessage().Text();
        }
    }
    
    static void dumpRegionInfo(const HalconCpp::HRegion& region, const QString& name) {
        try {
            HalconCpp::HTuple area, row, col;
            region.AreaCenter(area, row, col);
            
            qDebug() << "Region Info:" << name
                    << "Area:" << area[0].D()
                    << "Center:" << col[0].D() << "," << row[0].D();
                    
        } catch (const HalconCpp::HException& e) {
            qWarning() << "Cannot get region info:" << e.ErrorMessage().Text();
        }
    }
};
```

## 最佳实践

### 1. 图像处理流程
1. **图像预处理**: 噪声滤波、增强对比度
2. **特征提取**: 边缘检测、区域分割
3. **特征分析**: 形状测量、模板匹配
4. **结果验证**: 质量检查、异常检测

### 2. 性能优化建议
- 使用合适的图像分辨率
- 优化ROI区域设置
- 合理选择算法参数
- 使用多线程处理

### 3. 错误处理策略
- 始终使用try-catch捕获异常
- 记录详细的错误日志
- 提供用户友好的错误提示
- 实现错误恢复机制

## 常见问题解答

### Q1: 如何解决图像加载失败？
**A**: 检查图像路径、格式支持、文件权限等因素。

### Q2: 内存占用过高怎么办？
**A**: 及时释放不用的图像对象，使用合适的图像分辨率。

### Q3: 处理速度慢如何优化？
**A**: 使用多线程、优化算法参数、减少不必要的操作。

---

*本文档最后更新时间: 2025年6月5日*
