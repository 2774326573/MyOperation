# 🔬 HDevelop API 使用手册 / HDevelop API Manual

<div align="center">

![HDevelop](https://img.shields.io/badge/HDevelop-Machine%20Vision-blue?style=for-the-badge&logo=data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMjQiIGhlaWdodD0iMjQiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTEyIDJMMTMuMDkgOC4yNkwyMCA5TDEzLjA5IDE1Ljc0TDEyIDIyTDEwLjkxIDE1Ljc0TDQgOUwxMC45MSA4LjI2TDEyIDJaIiBmaWxsPSJ3aGl0ZSIvPgo8L3N2Zz4K)
![HALCON](https://img.shields.io/badge/HALCON-Computer%20Vision-green?style=for-the-badge)
![C++](https://img.shields.io/badge/C++-17-red?style=for-the-badge&logo=cplusplus)
![Python](https://img.shields.io/badge/Python-3.8+-yellow?style=for-the-badge&logo=python)
![Qt](https://img.shields.io/badge/Qt-6.0+-brightgreen?style=for-the-badge&logo=qt)

**🚀 专业机器视觉开发库 | Professional Machine Vision Development Library**

*为工业检测、质量控制和自动化视觉系统提供强大支持*  
*Powerful support for industrial inspection, quality control and automated vision systems*

</div>

---

## 📚 目录 / Table of Contents

<table>
<tr>
<td width="50%">

### 🏁 基础入门 / Getting Started
- [📖 简介 / Introduction](#简介)
- [🧠 核心概念 / Core Concepts](#核心概念)
- [⚡ 基础API / Basic API](#基础api)

### 🔧 核心功能 / Core Features
- [🖼️ 图像处理操作 / Image Processing](#图像处理操作)
- [🔄 形态学操作 / Morphology](#形态学操作)
- [🎯 特征提取 / Feature Extraction](#特征提取)
- [🤖 模式识别 / Pattern Recognition](#模式识别)

</td>
<td width="50%">

### 🌟 高级功能 / Advanced Features
- [📐 3D视觉 / 3D Vision](#3d视觉)
- [📝 OCR文字识别 / OCR](#ocr文字识别)
- [💻 代码示例 / Code Examples](#代码示例)

### 🔗 集成指南 / Integration Guide
- [🐍 Python和Qt集成 / Python & Qt](#python和qt集成)
- [📋 详细使用教程 / Detailed Tutorial](#详细使用教程)
- [🛠️ 项目集成指南 / Project Integration](#项目集成指南)
- [🎯 实际应用场景 / Real Applications](#实际应用场景)
- [✨ 最佳实践 / Best Practices](#最佳实践)
- [❓ 常见问题解答 / FAQ](#常见问题解答)

</td>
</tr>
</table>

## 📖 简介 / Introduction

<div align="center">
<img src="https://img.shields.io/badge/Version-Latest-brightgreen?style=flat-square" alt="Version">
<img src="https://img.shields.io/badge/License-Commercial-blue?style=flat-square" alt="License">
<img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square" alt="Platform">
</div>

**HDevelop** 是 MVTec 公司开发的一套功能强大的机器视觉开发环境和库。本手册旨在提供 HDevelop 库 API 的中文参考，帮助开发者更高效地使用 HDevelop 进行机器视觉应用开发。

**HDevelop** is a powerful machine vision development environment and library developed by MVTec. This manual aims to provide a Chinese reference for the HDevelop library API to help developers use HDevelop more efficiently for machine vision application development.

### 🌟 核心特性 / Key Features

<table>
<tr>
<td width="50%">

#### 🔍 图像处理 / Image Processing
- 高性能图像滤波和增强
- 多种图像格式支持
- 实时图像处理能力

#### 🎯 模式识别 / Pattern Recognition  
- 形状匹配和模板匹配
- 机器学习算法集成
- 高精度识别能力

</td>
<td width="50%">

#### 📐 测量分析 / Measurement & Analysis
- 精确的几何测量
- 统计分析功能
- 质量检测工具

#### 🔗 集成能力 / Integration
- C/C++ API 支持
- Python 绑定
- Qt 框架集成

</td>
</tr>
</table>

---

## 🧠 核心概念 / Core Concepts

### 📊 HALCON 数据类型 / HALCON Data Types

<div align="center">
<img src="https://img.shields.io/badge/Data%20Types-6%20Main%20Types-blue?style=flat-square" alt="Data Types">
<img src="https://img.shields.io/badge/Architecture-Object%20Oriented-green?style=flat-square" alt="Architecture">
</div>

HDevelop/HALCON 中的主要数据类型包括 / Main data types in HDevelop/HALCON include:

<table>
<tr>
<td width="50%">

#### 🖼️ 视觉对象 / Visual Objects
- **`Hobject`** - 包含图像、区域、轮廓等视觉对象  
  Contains images, regions, contours and other visual objects
- **`HImage`** - 图像数据类型  
  Image data type
- **`HRegion`** - 区域数据类型  
  Region data type

</td>
<td width="50%">

#### 🔧 工具类型 / Utility Types
- **`HTuple`** - 通用数据类型，存储数值、字符串等  
  Universal data type for numbers, strings, etc.
- **`HXLDCont`** - 轮廓数据类型  
  Contour data type
- **`HWindow`** - 显示窗口  
  Display window

</td>
</tr>
</table>

### ⚙️ 操作符和过程 / Operators and Procedures

<div align="center">
<img src="https://img.shields.io/badge/Operators-1000+-orange?style=flat-square" alt="Operators">
<img src="https://img.shields.io/badge/Processing-Pipeline%20Based-purple?style=flat-square" alt="Processing">
</div>

在 HALCON 中，所有的图像处理功能都被称为"操作符"。每个操作符都有输入和输出参数。

In HALCON, all image processing functions are called "operators". Each operator has input and output parameters.

#### 🔄 操作符特点 / Operator Characteristics
- **输入参数** / Input Parameters: 图像、区域、参数值等
- **输出参数** / Output Parameters: 处理结果、测量值等
- **管道处理** / Pipeline Processing: 支持链式操作

## 基础API

### 系统初始化和设置

```c
// 初始化HALCON
void HalconInit();

// 获取系统信息
void GetSystem(const HTuple& Information, HTuple* SystemValue);

// 设置系统参数
void SetSystem(const HTuple& SystemParameter, const HTuple& SystemValue);

// 获取HALCON版本
void GetHalconVersion(HTuple* MajorVersion, HTuple* MinorVersion);

// 获取错误描述
HTuple HetErrorText(const HTuple& ErrorCode);
```

### 内存管理

```c
// 清空对象
void ClearObject(const Hobject& Objects);

// 清空所有对象
void ClearAllObjects();
```

### 窗口显示

```c
// 打开窗口
void OpenWindow(const HTuple& Row, const HTuple& Column, const HTuple& Width, 
                const HTuple& Height, const HTuple& FatherWindow, 
                const HTuple& Mode, const HTuple& Machine, HTuple* WindowHandle);

// 设置窗口参数
void SetWindowParam(const HTuple& WindowHandle, const HTuple& Param, const HTuple& Value);

// 显示图像
void DispImage(const HObject& Image, const HTuple& WindowHandle);

// 显示区域
void DispRegion(const HObject& Region, const HTuple& WindowHandle);

// 显示轮廓
void DispXld(const HObject& XLD, const HTuple& WindowHandle);

// 设置显示颜色
void SetColor(const HTuple& WindowHandle, const HTuple& Color);

// 设置线宽
void SetLineWidth(const HTuple& WindowHandle, const HTuple& Width);

// 设置绘图模式
void SetDraw(const HTuple& WindowHandle, const HTuple& Mode);

// 关闭窗口
void CloseWindow(const HTuple& WindowHandle);
```

## 图像处理操作

### 图像读写

```c
// 读取图像
void ReadImage(HObject* Image, const HTuple& FileName);

// 写入图像
void WriteImage(const HObject& Image, const HTuple& Format, 
                const HTuple& FillColor, const HTuple& FileName);
```

### 图像转换

```c
// 图像格式转换
void ConvertImageType(const HObject& Image, HObject* ConverImage, 
                      const HTuple& NewType);

// 彩色转灰度
void Rgb1ToGray(const HObject& Image, HObject* GrayImage);

// 灰度转彩色
void GrayToRgb1(const HObject& GrayImage, HObject* RGBImage);

// 图像翻转
void MirrorImage(const HObject& Image, HObject* MirroredImage,
                const HTuple& Mode);

// 图像缩放
void ZoomImageFactor(const HObject& Image, HObject* ZoomedImage,
                    const HTuple& ScaleWidth, const HTuple& ScaleHeight,
                    const HTuple& Interpolation);
```

### 图像滤波

```c
// 高斯滤波
void GaussFilter(const HObject& Image, HObject* ImageGauss,
                const HTuple& Size);

// 中值滤波
void MedianFilter(const HObject& Image, HObject* ImageMedian,
                 const HTuple& MaskType, const HTuple& Radius, 
                 const HTuple& Margin);

// 平滑滤波
void SmoothImage(const HObject& Image, HObject* ImageSmooth,
                const HTuple& Filter);

// 图像增强
void Emphasize(const HObject& Image, HObject* EmphasizedImage,
              const HTuple& MaskWidth, const HTuple& MaskHeight,
              const HTuple& Factor);
```

### 图像分割

```c
// 阈值分割
void Threshold(const HObject& Image, HObject* Region, 
              const HTuple& MinGray, const HTuple& MaxGray);

// 自动阈值
void AutoThreshold(const HObject& Image, HObject* Region,
                  const HTuple& Sigma);

// 区域生长
void RegionGrowing(const HObject& Image, HObject* Regions,
                  const HTuple& Row, const HTuple& Column,
                  const HTuple& Tolerance, const HTuple& MinSize);

// 分水岭变换
void Watersheds(const HObject& Image, HObject* Basins);
```

## 形态学操作

```c
// 腐蚀操作
void ErodeImage(const HObject& Image, HObject* ImageEroded,
               const HTuple& SE);

// 膨胀操作
void DilateImage(const HObject& Image, HObject* ImageDilated,
                const HTuple& SE);

// 开运算
void OpeningImage(const HObject& Image, HObject* ImageOpening,
                 const HTuple& SE);

// 闭运算
void ClosingImage(const HObject& Image, HObject* ImageClosing,
                 const HTuple& SE);

// 区域填充
void FillUp(const HObject& Region, HObject* RegionFillUp);

// 区域连接
void Connection(const HObject& Region, HObject* ConnectedRegions);

// 选择区域
void SelectShape(const HObject& Regions, HObject* SelectedRegions,
                const HTuple& Features, const HTuple& Operation,
                const HTuple& Min, const HTuple& Max);
```

## 特征提取

```c
// 边缘检测
void EdgesImage(const HObject& Image, HObject* EdgeImage,
               const HTuple& Filter, const HTuple& Alpha,
               const HTuple& Low, const HTuple& High);

// 线检测
void LinesGauss(const HObject& Image, HObject* Lines,
               const HTuple& Sigma, const HTuple& Low,
               const HTuple& High, const HTuple& LightDark);

// 圆检测
void FindCircle(const HObject& Regions, const HObject& Image,
               const HTuple& MinRadius, const HTuple& MaxRadius,
               const HTuple& MinScore, HTuple* Row, HTuple* Column,
               HTuple* Radius);

// 区域特征计算
void RegionFeatures(const HObject& Regions, const HTuple& Features,
                   HTuple* Values);

// 计算区域中心点
void AreaCenter(const HObject& Regions, HTuple* Area, 
               HTuple* Row, HTuple* Column);

// 计算区域方向
void OrientationRegion(const HObject& Regions, HTuple* Phi);

// 计算区域长宽比
void EllipticAxis(const HObject& Regions, HTuple* Ra, 
                 HTuple* Rb, HTuple* Phi);
```

## 模式识别

### 模板匹配

```c
// 创建模板
void CreateTemplate(const HObject& Template, HTuple* TemplateID);

// 匹配模板
void BestMatchTemplate(const HObject& Image, const HTuple& TemplateID,
                      const HTuple& MaxError, HTuple* Row, HTuple* Column,
                      HTuple* Error);

// 释放模板
void ClearTemplate(const HTuple& TemplateID);
```

### 形状匹配

```c
// 创建形状模型
void CreateShapeModel(const HObject& Template, const HTuple& NumLevels,
                     const HTuple& AngleStart, const HTuple& AngleExtent,
                     const HTuple& AngleStep, const HTuple& Optimization,
                     const HTuple& Metric, const HTuple& Contrast,
                     const HTuple& MinContrast, HTuple* ModelID);

// 查找形状模型
void FindShapeModel(const HObject& Image, const HTuple& ModelID,
                   const HTuple& AngleStart, const HTuple& AngleExtent,
                   const HTuple& MinScore, const HTuple& NumMatches,
                   const HTuple& MaxOverlap, const HTuple& SubPixel,
                   const HTuple& NumLevels, const HTuple& Greediness,
                   HTuple* Row, HTuple* Column, HTuple* Angle, HTuple* Score);

// 释放形状模型
void ClearShapeModel(const HTuple& ModelID);
```

## 3D视觉

```c
// 立体视觉校准
void StereoCalibration(const HTuple& NX, const HTuple& NY, const HTuple& NZ,
                      const HTuple& NRow1, const HTuple& NCol1,
                      const HTuple& NRow2, const HTuple& NCol2,
                      HTuple* CameraMatrix1, HTuple* CameraMatrix2,
                      HTuple* RotationMatrix, HTuple* TranslationMatrix);

// 视差图计算
void DisparityImage(const HObject& Image1, const HObject& Image2,
                   const HTuple& CameraMatrix1, const HTuple& CameraMatrix2,
                   const HTuple& RotationMatrix, const HTuple& TranslationMatrix,
                   HObject* DisparityMap);

// 3D点云重建
void DisparityToPointMap(const HObject& DisparityMap,
                        const HTuple& CameraMatrix1, const HTuple& CameraMatrix2,
                        const HTuple& RotationMatrix, const HTuple& TranslationMatrix,
                        HObject* PointMap);
```

## OCR文字识别

```c
// 创建OCR分类器
void CreateOcrClassifier(const HTuple& FeatureList, const HTuple& Characters,
                        HTuple* OCRHandle);

// 训练OCR分类器
void TrainfOcrClassifier(const HTuple& OCRHandle, const HObject& TrainingImages,
                       const HTuple& CharacterNames);

// 使用OCR分类器
void DoOcrMultiClass(const HObject& Character, const HObject& Image,
                   const HTuple& OCRHandle, HTuple* Class, HTuple* Confidence);

// 释放OCR分类器
void ClearOcrClassifier(const HTuple& OCRHandle);
```

## 代码示例

### 基础图像处理示例

```c
#include "HalconC.h"

int main()
{
    // 初始化
    HTuple hv_WindowHandle;
    HObject ho_Image, ho_ImageSmooth, ho_Region, ho_SelectedRegions;
    
    // 打开窗口
    OpenWindow(0, 0, 512, 512, 0, "visible", "", &hv_WindowHandle);
    
    // 读取图像
    ReadImage(&ho_Image, "example.jpg");
    
    // 显示原始图像
    DispImage(ho_Image, hv_WindowHandle);
    
    // 图像平滑
    SmoothImage(ho_Image, &ho_ImageSmooth, "gaussian", 5);
    
    // 阈值分割
    Threshold(ho_ImageSmooth, &ho_Region, 50, 200);
    
    // 选择大的区域
    SelectShape(ho_Region, &ho_SelectedRegions, "area", "and", 100, 99999);
    
    // 设置显示颜色和线宽
    SetColor(hv_WindowHandle, "green");
    SetLineWidth(hv_WindowHandle, 2);
    
    // 显示结果区域
    DispRegion(ho_SelectedRegions, hv_WindowHandle);
    
    // 等待按键并关闭窗口
    WaitSeconds(10);
    CloseWindow(hv_WindowHandle);
    
    return 0;
}
```

### 模板匹配示例

```c
#include "HalconC.h"

int main()
{
    // 初始化
    HTuple hv_WindowHandle, hv_TemplateID;
    HTuple hv_Row, hv_Column, hv_Error;
    HObject ho_Image, ho_Template;
    
    // 打开窗口
    OpenWindow(0, 0, 800, 600, 0, "visible", "", &hv_WindowHandle);
    
    // 读取图像和模板
    ReadImage(&ho_Image, "scene.jpg");
    ReadImage(&ho_Template, "template.jpg");
    
    // 显示原始图像
    DispImage(ho_Image, hv_WindowHandle);
    
    // 创建模板
    CreateTemplate(ho_Template, &hv_TemplateID);
    
    // 匹配模板
    BestMatchTemplate(ho_Image, hv_TemplateID, 0.3, &hv_Row, &hv_Column, &hv_Error);
    
    // 显示匹配结果
    SetColor(hv_WindowHandle, "red");
    SetLineWidth(hv_WindowHandle, 2);
    
    // 绘制匹配位置
    DispCross(hv_WindowHandle, hv_Row, hv_Column, 20, 0.0);
    
    // 释放模板
    ClearTemplate(hv_TemplateID);
    
    // 等待并关闭窗口
    WaitSeconds(10);
    CloseWindow(hv_WindowHandle);
    
    return 0;
}
```

## Python和Qt集成

### Python集成示例

通过PyHalcon（HALCON的Python包装器）可以在Python环境中使用HDevelop/HALCON的功能。下面是一些基本使用示例：

#### 基本图像处理

```python
import halcon as ha
import numpy as np
import cv2

def process_image_with_halcon(image_path):
    # 读取图像
    img = ha.read_image(image_path)
    
    # 获取图像尺寸
    width, height = ha.get_image_size(img)
    print(f"图像尺寸: {width} x {height}")
    
    # 转为灰度图像
    if ha.count_channels(img) > 1:
        gray_img = ha.rgb1_to_gray(img)
    else:
        gray_img = img
    
    # 平滑处理
    smoothed = ha.smooth_image(gray_img, "gaussian", 5)
    
    # 阈值分割
    region = ha.threshold(smoothed, 0, 128)
    
    # 连通区域分析
    connected = ha.connection(region)
    
    # 选择区域
    selected = ha.select_shape(connected, "area", "and", 100, 99999)
    
    # 计算区域特征
    area, row, col = ha.area_center(selected)
    
    # 保存结果
    output_img = ha.region_to_bin(selected, 255, 0, width, height)
    ha.write_image(output_img, "png", 0, "result.png")
    
    return selected, (row, col)

# 与OpenCV集成
def halcon_to_opencv(halcon_image):
    # 从HALCON图像转为NumPy数组
    width, height = ha.get_image_size(halcon_image)
    pointer, _, _ = ha.get_image_pointer1(halcon_image)
    opencv_img = np.array(pointer).reshape(height, width)
    return opencv_img

def opencv_to_halcon(opencv_image):
    # 从OpenCV图像转为HALCON图像
    height, width = opencv_image.shape[:2]
    if len(opencv_image.shape) == 3:
        # 彩色图像
        r = opencv_image[:, :, 0].flatten()
        g = opencv_image[:, :, 1].flatten()
        b = opencv_image[:, :, 2].flatten()
        halcon_img = ha.gen_image_3("byte", width, height, r, g, b)
    else:
        # 灰度图像
        halcon_img = ha.gen_image_1("byte", width, height, opencv_image.flatten())
    return halcon_img

if __name__ == "__main__":
    # 使用HALCON处理图像
    regions, center = process_image_with_halcon("test.jpg")
    
    # 读取原始图像用OpenCV显示结果
    cv_img = cv2.imread("test.jpg")
    
    # 在OpenCV图像上标记HALCON找到的中心点
    cv2.circle(cv_img, (int(center[1]), int(center[0])), 5, (0, 255, 0), -1)
    
    # 显示结果
    cv2.imshow("Result", cv_img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
```

#### 条码识别示例

```python
import halcon as ha
import cv2
import numpy as np

def barcode_recognition(image_path):
    # 读取图像
    img = ha.read_image(image_path)
    
    # 预处理图像
    if ha.count_channels(img) > 1:
        gray_img = ha.rgb1_to_gray(img)
    else:
        gray_img = img
        
    # 增强图像对比度
    enhanced = ha.emphasize(gray_img, 5, 5, 1.5)
    
    # 创建条码模型
    barcode_handle = ha.create_bar_code_model(["EAN-13", "Code 128", "QR Code"], [])
    
    try:
        # 查找条码
        decoded_data, code_type = ha.find_bar_code(enhanced, barcode_handle)
        
        if len(decoded_data) > 0:
            print(f"条码类型: {code_type}")
            print(f"解码数据: {decoded_data}")
            return decoded_data, code_type
        else:
            print("未找到条码")
            return None, None
    finally:
        # 释放资源
        ha.clear_bar_code_model(barcode_handle)

if __name__ == "__main__":
    result, code_type = barcode_recognition("barcode.jpg")
```

### Qt集成示例

通过HALCON的C++接口，可以将其与Qt框架结合，创建功能强大的机器视觉应用程序。

#### 基本集成 (Qt Widgets)

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>

// HALCON头文件
#include "HalconCpp.h"
#include "HalconC.h"

using namespace HalconCpp;

class HalconQtWindow : public QMainWindow
{
    Q_OBJECT

public:
    HalconQtWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("HALCON Qt集成示例");
        setMinimumSize(800, 600);
        
        // 创建中央部件和布局
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        
        // 图像显示标签
        m_imageLabel = new QLabel(this);
        m_imageLabel->setAlignment(Qt::AlignCenter);
        m_imageLabel->setMinimumSize(640, 480);
        m_imageLabel->setText("未加载图像");
        
        // 按钮
        QPushButton *loadButton = new QPushButton("加载图像", this);
        QPushButton *processButton = new QPushButton("处理图像", this);
        processButton->setEnabled(false);
        
        // 添加部件到布局
        layout->addWidget(m_imageLabel);
        layout->addWidget(loadButton);
        layout->addWidget(processButton);
        
        setCentralWidget(centralWidget);
        
        // 连接信号和槽
        connect(loadButton, &QPushButton::clicked, this, &HalconQtWindow::loadImage);
        connect(processButton, &QPushButton::clicked, this, &HalconQtWindow::processImage);
        
        m_processButton = processButton;
    }

private slots:
    void loadImage()
    {
        try {
            QString fileName = QFileDialog::getOpenFileName(this, 
                "打开图像", "", "图像文件 (*.png *.jpg *.bmp)");
                
            if (fileName.isEmpty())
                return;
                
            // 读取图像
            m_hoImage.ReadImage(fileName.toStdString().c_str());
            
            // 转换HALCON图像为Qt图像
            HTuple width, height;
            GetImageSize(m_hoImage, &width, &height);
            
            HImage grayImage;
            if (CountChannels(m_hoImage) == 3) {
                // 仅用于显示
                Rgb1ToGray(m_hoImage, &grayImage);
            } else {
                grayImage = m_hoImage;
            }
            
            // 将HALCON图像转换为Qt图像
            HTuple imagePointer, type, width2, height2;
            GetImagePointer1(grayImage, &imagePointer, &type, &width2, &height2);
            
            // 创建QImage
            QImage qImage((const uchar*)imagePointer.L(), 
                width.I(), height.I(), width.I(), QImage::Format_Grayscale8);
            
            // 显示图像
            m_imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(
                m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                
            // 启用处理按钮
            m_processButton->setEnabled(true);
        }
        catch (HException &exception) {
            QMessageBox::critical(this, "错误", 
                QString("HALCON异常: %1").arg(exception.ErrorMessage().Text()));
        }
    }
    
    void processImage()
    {
        try {
            if (m_hoImage.IsInitialized()) {
                // 图像处理操作
                HImage smoothedImage;
                SmoothImage(m_hoImage, &smoothedImage, "gaussian", 5);
                
                HRegion region;
                Threshold(smoothedImage, &region, 0, 128);
                
                HRegion connectedRegions, selectedRegions;
                Connection(region, &connectedRegions);
                SelectShape(connectedRegions, &selectedRegions, "area", "and", 100, 99999);
                
                // 计算区域特征
                HTuple area, row, column;
                AreaCenter(selectedRegions, &area, &row, &column);
                
                // 创建显示图像
                HImage resultImage = m_hoImage.CopyImage();
                
                // 在图像上绘制区域
                HTuple windowHandle;
                try {
                    SetOutputUtilPreference("graphics", "on");
                    SetOutputUtilPreference("filename", "window");
                    OpenWindow(0, 0, 640, 480, 0, "visible", "", &windowHandle);
                    DispObj(resultImage, windowHandle);
                    SetColor(windowHandle, "green");
                    DispObj(selectedRegions, windowHandle);
                    
                    // 绘制中心点
                    SetColor(windowHandle, "red");
                    for (int i = 0; i < row.Length(); i++) {
                        DispCross(windowHandle, row[i], column[i], 20, 0);
                    }
                    
                    // 捕获窗口内容为图像
                    HImage displayedImage;
                    DumpWindowImage(&displayedImage, windowHandle);
                    
                    // 关闭HALCON窗口
                    CloseWindow(windowHandle);
                    
                    // 显示结果
                    HTuple imagePointer, type, width, height;
                    GetImagePointer1(displayedImage, &imagePointer, &type, &width, &height);
                    
                    // 创建QImage
                    QImage qImage((const uchar*)imagePointer.L(), 
                        width.I(), height.I(), width.I() * 4, QImage::Format_RGB32);
                    
                    // 显示图像
                    m_imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(
                        m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
                catch (...) {
                    if (windowHandle.Length() > 0)
                        CloseWindow(windowHandle);
                    throw;
                }
            }
        }
        catch (HException &exception) {
            QMessageBox::critical(this, "错误", 
                QString("HALCON异常: %1").arg(exception.ErrorMessage().Text()));
        }
    }

private:
    QLabel *m_imageLabel;
    QPushButton *m_processButton;
    HImage m_hoImage;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HalconQtWindow window;
    window.show();
    return app.exec();
}
```

#### QML集成

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "HalconProcessor.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 注册自定义类型
    qmlRegisterType<HalconProcessor>("HalconModule", 1, 0, "HalconProcessor");
    
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    return app.exec();
}
```

```cpp
// HalconProcessor.h
#pragma once

#include <QObject>
#include <QString>
#include <QImage>

// HALCON头文件
#include "HalconCpp.h"

class HalconProcessor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QImage processedImage READ processedImage NOTIFY processedImageChanged)
    Q_PROPERTY(QString processingStatus READ processingStatus NOTIFY processingStatusChanged)
    
public:
    explicit HalconProcessor(QObject *parent = nullptr);
    ~HalconProcessor();
    
    QImage processedImage() const { return m_processedImage; }
    QString processingStatus() const { return m_status; }
    
public slots:
    bool loadImage(const QString &path);
    bool processImage();
    bool detectObjects();
    bool measureObjects();
    void saveProcessedImage(const QString &path);
    
signals:
    void processedImageChanged();
    void processingStatusChanged();
    void processingError(const QString &message);
    
private:
    QImage halconImageToQImage(const HalconCpp::HImage &image);
    
    HalconCpp::HImage m_inputImage;
    QImage m_processedImage;
    QString m_status;
};
```

```qml
// main.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import HalconModule 1.0

ApplicationWindow {
    title: "HALCON QML集成示例"
    width: 1024
    height: 768
    visible: true
    
    HalconProcessor {
        id: halconProcessor
        
        onProcessingStatusChanged: {
            statusLabel.text = processingStatus
        }
        
        onProcessingError: {
            errorDialog.text = message
            errorDialog.open()
        }
    }
    
    Dialog {
        id: errorDialog
        title: "错误"
        property string text: ""
        
        Label {
            text: errorDialog.text
        }
        
        standardButtons: Dialog.Ok
    }
    
    FileDialog {
        id: openFileDialog
        title: "选择图像"
        folder: shortcuts.home
        nameFilters: ["图像文件 (*.png *.jpg *.bmp)"]
        onAccepted: {
            halconProcessor.loadImage(fileUrl)
        }
    }
    
    FileDialog {
        id: saveFileDialog
        title: "保存图像"
        folder: shortcuts.home
        nameFilters: ["PNG图像 (*.png)", "JPEG图像 (*.jpg)"]
        selectExisting: false
        onAccepted: {
            halconProcessor.saveProcessedImage(fileUrl)
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // 工具栏
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                text: "打开图像"
                onClicked: openFileDialog.open()
            }
            
            Button {
                text: "处理图像"
                onClicked: halconProcessor.processImage()
            }
            
            Button {
                text: "检测对象"
                onClicked: halconProcessor.detectObjects()
            }
            
            Button {
                text: "测量对象"
                onClicked: halconProcessor.measureObjects()
            }
            
            Button {
                text: "保存结果"
                onClicked: saveFileDialog.open()
            }
            
            Label {
                id: statusLabel
                text: "就绪"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
            }
        }
        
        // 图像显示区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "black"
            
            Image {
                anchors.fill: parent
                source: halconProcessor.processedImage
                fillMode: Image.PreserveAspectFit
            }
        }
    }
}
```

#### 实时视频处理

```cpp
// VideoProcessor.h
#pragma once

#include <QObject>
#include <QImage>
#include <QTimer>
#include <QCamera>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include "HalconCpp.h"

class VideoProcessor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QImage processedFrame READ processedFrame NOTIFY processedFrameChanged)
    Q_PROPERTY(bool isProcessing READ isProcessing WRITE setIsProcessing NOTIFY isProcessingChanged)
    
public:
    explicit VideoProcessor(QObject *parent = nullptr);
    ~VideoProcessor();
    
    QImage processedFrame() const { return m_processedFrame; }
    bool isProcessing() const { return m_isProcessing; }
    void setIsProcessing(bool processing);
    
public slots:
    void startCamera();
    void stopCamera();
    void processFrame(const QVideoFrame &frame);
    void setProcessingMode(int mode);
    
signals:
    void processedFrameChanged();
    void isProcessingChanged();
    void error(const QString &message);
    
private:
    QImage halconImageToQImage(const HalconCpp::HImage &image);
    HalconCpp::HImage qImageToHalconImage(const QImage &image);
    
    QCamera *m_camera;
    QAbstractVideoSurface *m_surface;
    QImage m_processedFrame;
    bool m_isProcessing;
    int m_processingMode;
    HalconCpp::HImage m_lastFrame;
    
    // HALCON模型句柄
    HalconCpp::HTuple m_shapeModel;
    bool m_modelInitialized;
    
    void initializeModels();
    void cleanupModels();
};

---

## 12. 详细使用教程 (Detailed Usage Tutorial)

### 12.1 环境配置 (Environment Setup)

#### 12.1.1 CMake 集成 (CMake Integration)

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(HalconExample)

# 设置 Qt 版本 (Set Qt version)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 设置 HALCON 路径 (Set HALCON path)
set(HALCON_ROOT "C:/Program Files/MVTec/HALCON-20.11-Progress")
set(HALCON_ARCH "x64-win64")

# 包含 HALCON 头文件 (Include HALCON headers)
include_directories(${HALCON_ROOT}/include)
include_directories(${HALCON_ROOT}/include/halconcpp)

# 查找 HALCON 库 (Find HALCON libraries)
find_library(HALCON_LIBRARY
    NAMES halcon
    PATHS ${HALCON_ROOT}/lib/${HALCON_ARCH}
    NO_DEFAULT_PATH
)

find_library(HALCONCPP_LIBRARY
    NAMES halconcpp
    PATHS ${HALCON_ROOT}/lib/${HALCON_ARCH}
    NO_DEFAULT_PATH
)

# 创建可执行文件 (Create executable)
add_executable(${PROJECT_NAME}
    main.cpp
    halconmanager.cpp
    halconmanager.h
    imageprocessor.cpp
    imageprocessor.h
)

# 链接库 (Link libraries)
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    ${HALCON_LIBRARY}
    ${HALCONCPP_LIBRARY}
)

# 设置 C++ 标准 (Set C++ standard)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
```

### 12.2 HALCON管理器类 (HALCON Manager Class)

```cpp
// halconmanager.h
#ifndef HALCONMANAGER_H
#define HALCONMANAGER_H

#include <QObject>
#include <QImage>
#include <QDebug>
#include "HalconCpp.h"

class HalconManager : public QObject
{
    Q_OBJECT

public:
    explicit HalconManager(QObject *parent = nullptr);
    ~HalconManager();

    // 图像处理 (Image Processing)
    bool loadImage(const QString &path);
    bool saveImage(const QString &path);
    QImage getCurrentImage() const;

    // 基础操作 (Basic Operations)
    bool convertToGray();
    bool applyGaussianFilter(double sigma);
    bool threshold(int minGray, int maxGray);
    bool detectEdges();

    // 形状匹配 (Shape Matching)
    bool createShapeModel(const QRect &roi);
    QList<QPointF> findShapeModel();

    // 测量功能 (Measurement Functions)
    double measureDistance(const QPointF &p1, const QPointF &p2);
    double measureAngle(const QPointF &p1, const QPointF &p2, const QPointF &p3);

signals:
    void imageProcessed();
    void errorOccurred(const QString &error);

private:
    QImage halconToQImage(const HalconCpp::HImage &halconImage);
    HalconCpp::HImage qImageToHalcon(const QImage &qImage);

    HalconCpp::HImage m_currentImage;
    HalconCpp::HTuple m_shapeModelID;
    QString m_lastError;
};

#endif // HALCONMANAGER_H
```

### 12.3 实际应用场景 (Real-world Application Scenarios)

#### 12.3.1 质量检测系统 (Quality Inspection System)

```cpp
class QualityInspector : public QObject
{
    Q_OBJECT

public:
    struct DefectInfo {
        QString type;           // 缺陷类型 (Defect type)
        QPointF position;       // 位置 (Position)
        double area;            // 面积 (Area)
        double severity;        // 严重程度 (Severity)
    };

    explicit QualityInspector(QObject *parent = nullptr);

    // 检测功能 (Detection Functions)
    QList<DefectInfo> detectDefects(const QImage &image);
    bool calibrateSystem(const QImage &calibrationImage);
    double measureDimension(const QImage &image, const QString &feature);

signals:
    void inspectionCompleted(const QList<DefectInfo> &defects);
    void calibrationCompleted(bool success);

private:
    HalconManager *m_halconManager;
    bool m_isCalibrated;
};
```

## 13. 最佳实践 (Best Practices)

### 13.1 性能优化 (Performance Optimization)

```cpp
// 内存管理最佳实践 (Memory Management Best Practices)
class HalconOptimizer
{
public:
    // 批量处理优化 (Batch Processing Optimization)
    static void processBatch(const QStringList &imagePaths) {
        HalconCpp::HImage image;
        for (const QString &path : imagePaths) {
            try {
                image.ReadImage(path.toStdString().c_str());
                // 处理图像 (Process image)
                processImage(image);
                // 清理内存 (Clean memory)
                image.Clear();
            } catch (const HalconCpp::HException &e) {
                qDebug() << tr("处理失败 (Processing failed):") << e.ErrorMessage().Text();
            }
        }
    }

private:
    static void processImage(HalconCpp::HImage &image) {
        // 图像处理逻辑 (Image processing logic)
        HalconCpp::HImage grayImage;
        image.Rgb1ToGray(&grayImage);
        
        HalconCpp::HRegion region;
        grayImage.Threshold(&region, 0, 128);
        
        // 清理临时对象 (Clean temporary objects)
        grayImage.Clear();
        region.Clear();
    }
};
```

### 13.2 错误处理 (Error Handling)

```cpp
class HalconErrorHandler
{
public:
    static bool safeExecute(std::function<void()> operation, QString &errorMessage) {
        try {
            operation();
            return true;
        } catch (const HalconCpp::HException &e) {
            errorMessage = tr("HALCON错误 (HALCON Error): %1 (代码 Code: %2)")
                          .arg(e.ErrorMessage().Text())
                          .arg(e.ErrorCode());
            return false;
        } catch (const std::exception &e) {
            errorMessage = tr("标准异常 (Standard Exception): %1").arg(e.what());
            return false;
        } catch (...) {
            errorMessage = tr("未知错误 (Unknown Error)");
            return false;
        }
    }
};
```

## 14. 常见问题解答 (FAQ)

**Q1: 如何处理大图像的内存问题？ (How to handle memory issues with large images?)**

A1: 使用图像分块处理和及时清理内存：

```cpp
void processLargeImage(const QString &imagePath) {
    HalconCpp::HImage image;
    image.ReadImage(imagePath.toStdString().c_str());
    
    // 获取图像尺寸 (Get image dimensions)
    HalconCpp::HTuple width, height;
    image.GetImageSize(&width, &height);
    
    // 分块处理 (Process in blocks)
    int blockSize = 512;
    for (int row = 0; row < height[0].I(); row += blockSize) {
        for (int col = 0; col < width[0].I(); col += blockSize) {
            HalconCpp::HImage block;
            image.CropPart(&block, row, col, blockSize, blockSize);
            
            // 处理块 (Process block)
            processImageBlock(block);
            
            // 清理块 (Clean block)
            block.Clear();
        }
    }
    
    image.Clear();
}
```

**Q2: 如何在Qt中显示HALCON图像？ (How to display HALCON images in Qt?)**

A2: 转换HALCON图像为QImage：

```cpp
QImage HalconManager::halconToQImage(const HalconCpp::HImage &halconImage) {
    try {
        HalconCpp::HTuple width, height, type;
        halconImage.GetImageSize(&width, &height);
        halconImage.GetImageType(&type);
        
        if (type[0].S() == "byte") {
            // 灰度图像 (Grayscale image)
            HalconCpp::HTuple pointer;
            halconImage.GetImagePointer1(&pointer, &width, &height);
            
            uchar *data = reinterpret_cast<uchar*>(pointer[0].L());
            QImage qImage(data, width[0].I(), height[0].I(), QImage::Format_Grayscale8);
            return qImage.copy(); // 创建副本 (Create copy)
        }
        
    } catch (const HalconCpp::HException &e) {
        qDebug() << tr("图像转换失败 (Image conversion failed):") << e.ErrorMessage().Text();
    }
    
    return QImage();
}
```

---

<div align="center">

## 🎉 总结 / Summary

**通过以上详细的使用教程，您可以充分利用 HALCON 库的强大功能来进行机器视觉应用开发。HALCON 特别适合于工业检测、质量控制和自动化视觉系统。**

**Through the detailed tutorial above, you can fully utilize the powerful features of the HALCON library for machine vision application development. HALCON is particularly suitable for industrial inspection, quality control and automated vision systems.**

### 🚀 下一步 / Next Steps

<table>
<tr>
<td width="33%">

#### 📚 深入学习
- 阅读官方文档
- 参加培训课程
- 实践项目开发

</td>
<td width="33%">

#### 🛠️ 项目实践
- 创建示例项目
- 集成到现有系统
- 性能优化调试

</td>
<td width="33%">

#### 🤝 社区支持
- 加入开发者社区
- 分享经验心得
- 获取技术支持

</td>
</tr>
</table>

---

<img src="https://img.shields.io/badge/Status-Document%20Complete-success?style=for-the-badge" alt="Complete">
<img src="https://img.shields.io/badge/Quality-Professional-blue?style=for-the-badge" alt="Quality">
<img src="https://img.shields.io/badge/Language-中英双语-orange?style=for-the-badge" alt="Bilingual">

**📝 文档版本 / Document Version:** v2.0 Enhanced  
**📅 最后更新 / Last Updated:** 2024  
**👨‍💻 维护者 / Maintainer:** TestModbus Team

</div>
``` 