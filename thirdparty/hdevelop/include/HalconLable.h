/**
 * @file HalconLable.h
 * @brief Halcon图像显示控件类头文件 | Halcon Image Display Widget Class Header
 * @author Your Name
 * @date 2025-06-01
 * @version 1.0
 * 
 * 该文件定义了HalconLable类，这是一个基于Qt的Halcon图像显示控件。
 * 它提供了图像显示、交互操作、ROI绘制、测量标注等功能，
 * 是机器视觉应用程序中图像处理和分析的核心显示组件。
 * 
 * This file defines the HalconLable class, which is a Qt-based Halcon image display widget.
 * It provides image display, interactive operations, ROI drawing, measurement annotations,
 * and serves as the core display component for image processing and analysis in machine vision applications.
 */

#ifndef HALCONLABLE_H
#define HALCONLABLE_H

// Halcon机器视觉库头文件 | Halcon Machine Vision Library Header
#include "halconcpp/HalconCpp.h"

// Qt基础框架头文件 | Qt Framework Base Headers
#include <QWidget>       // Qt窗口控件基类 | Qt widget base class
#include <QMouseEvent>   // 鼠标事件处理 | Mouse event handling
#include <QWheelEvent>   // 滚轮事件处理 | Wheel event handling
#include <QResizeEvent>  // 窗口大小调整事件 | Resize event handling
#include <QEvent>        // 通用事件基类 | Generic event base class
#include <QContextMenuEvent> // 右键菜单事件 | Context menu event

// Qt工具类头文件 | Qt Utility Headers
#include <QDateTime>     // 日期时间处理 | Date time handling
#include <QDebug>        // 调试输出 | Debug output
#include <QTimer>        // 定时器 | Timer
#include <QtMath>        // 数学函数 | Math functions
#include <QColor>        // 颜色处理 | Color handling

// Qt UI组件头文件 | Qt UI Component Headers
#include <QLabel>        // 标签控件 | Label widget
#include <QLineEdit>     // 行编辑控件 | Line edit widget
#include <QListView>     // 列表视图 | List view
#include <QListWidgetItem> // 列表项 | List widget item
#include <QMenu>         // 菜单控件 | Menu widget
#include <QAction>       // 动作对象 | Action object
#include <QMessageBox>   // 消息框 | Message box
#include <QPushButton>   // 按钮控件 | Push button

// Qt文件和布局相关头文件 | Qt File and Layout Headers
#include <QFileDialog>   // 文件对话框 | File dialog
#include <QHBoxLayout>   // 水平布局 | Horizontal layout
#include <QPainter>      // 绘图工具 | Painter tool

// Qt事件和验证相关头文件 | Qt Event and Validation Headers
#include <QKeyEvent>     // 键盘事件 | Keyboard event
#include <QRegExpValidator> // 正则表达式验证器 | Regular expression validator
#include <QStringListModel> // 字符串列表模型 | String list model

// 使用Halcon命名空间 | Using Halcon Namespace
using namespace HalconCpp;

/* ==================== 数据结构定义 | Data Structure Definitions ==================== */

/**
 * @struct pointStruct
 * @brief 点结构体 | Point Structure
 * 
 * 用于表示2D平面上的一个点，包含坐标和旋转角度信息。
 * 广泛用于图像处理中的特征点、标记点等位置记录。
 * 
 * Used to represent a point on a 2D plane, including coordinate and rotation angle information.
 * Widely used for recording positions of feature points, marker points, etc. in image processing.
 */
struct pointStruct {
  double X = 0;      // ch:X坐标，水平方向位置（像素单位）| en:X coordinate, horizontal position (in pixels)
  double Y = 0;      // ch:Y坐标，垂直方向位置（像素单位）| en:Y coordinate, vertical position (in pixels)
  double Angle = 0;  // ch:角度，点的旋转角度（弧度制）| en:Angle, rotation angle of the point (in radians)
};
Q_DECLARE_METATYPE(pointStruct);

/**
 * @struct ShapeModelData
 * @brief 形状模型数据结构体 | Shape Model Data Structure
 * 
 * 存储Halcon形状匹配模型的相关数据，包括模型位置、角度、ID和区域信息。
 * 用于模板匹配、目标识别等机器视觉应用场景。
 * 
 * Stores data related to Halcon shape matching models, including model position, angle, ID, and region information.
 * Used in template matching, object recognition, and other machine vision applications.
 */
struct ShapeModelData {
  HTuple R;              // ch:行坐标，模型中心点Y坐标（像素）| en:Row coordinate, model center Y-coordinate (pixels)
  HTuple C;              // ch:列坐标，模型中心点X坐标（像素）| en:Column coordinate, model center X-coordinate (pixels)
  HTuple Phi;            // ch:角度，模型旋转角度（弧度制）| en:Angle, model rotation angle (in radians)
  HTuple Score;          // ch:匹配分数，模型匹配的置信度分数 | en:Score, confidence score of the model match
  HTuple ModelID;        // ch:模型ID，模型的唯一标识符 | en:Model ID, unique identifier of the model
  HObject modelregion;   // ch:模型区域，模型的形状区域对象 | en:Model region, shape region object of the model
};
Q_DECLARE_METATYPE(ShapeModelData);

/**
 * @struct LineData
 * @brief 线数据结构体 | Line Data Structure
 * 
 * 存储直线的起点和终点坐标，以及相关的XLD轮廓对象。
 * 主要用于线性测量、边缘检测、直线拟合等图像分析功能。
 * 
 * Stores start and end coordinates of a line, along with related XLD contour objects.
 * Mainly used for linear measurement, edge detection, line fitting, and other image analysis functions.
 */
struct LineData {
  HTuple R1;            // ch:起点行坐标（Y坐标，像素）| en:Start point row coordinate (Y-coordinate, pixels)
  HTuple C1;            // ch:起点列坐标（X坐标，像素）| en:Start point column coordinate (X-coordinate, pixels)
  HTuple R2;            // ch:终点行坐标（Y坐标，像素）| en:End point row coordinate (Y-coordinate, pixels)
  HTuple C2;            // ch:终点列坐标（X坐标，像素）| en:End point column coordinate (X-coordinate, pixels)
  HObject orgXld;       // ch:原始XLD轮廓对象 | en:Original XLD contour object
  HObject LineXld;      // ch:处理后的线XLD对象 | en:Processed line XLD object
};
Q_DECLARE_METATYPE(LineData);

/**
 * @struct CodeData
 * @brief 二维码数据结构体 | 2D Code Data Structure
 * 
 * 存储二维码识别的结果数据，包括解码的字符串和检测到的区域对象。
 * 支持多个二维码同时识别的场景。
 * 
 * Stores result data from 2D code recognition, including decoded strings and detected region objects.
 * Supports scenarios where multiple 2D codes are recognized simultaneously.
 */
struct CodeData {
  QList<QString> codestring;  // ch:二维码识别结果字符串列表 | en:List of decoded 2D code strings
  HObject codeobj;            // ch:二维码区域对象 | en:2D code region object
};
Q_DECLARE_METATYPE(CodeData);

/**
 * @class HalconLable
 * @brief Halcon图像显示控件类 | Halcon Image Display Widget Class
 * 
 * 这是一个功能强大的图像显示和交互控件，基于Qt和Halcon开发。
 * 主要功能包括：
 * - 图像显示和缩放
 * - 鼠标交互操作（拖拽、缩放、绘制）
 * - ROI区域绘制（矩形、圆形、任意形状）
 * - 图像处理和分析功能
 * - 测量和标注功能
 * - 3D点云处理
 * - 手眼标定功能
 * - 上下文菜单和操作历史
 * 
 * This is a powerful image display and interaction widget developed based on Qt and Halcon.
 * Main features include:
 * - Image display and zoom
 * - Mouse interaction operations (drag, zoom, draw)
 * - ROI region drawing (rectangle, circle, arbitrary shape)
 * - Image processing and analysis functions
 * - Measurement and annotation functions
 * - 3D point cloud processing
 * - Hand-eye calibration functionality
 * - Context menu and operation history
 */
class HalconLable : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief 构造函数 | Constructor
   * @param parent 父窗口指针 | Parent widget pointer
   */
  explicit HalconLable(QWidget *parent = nullptr);
  /* ==================== 公有成员变量 | Public Member Variables ==================== */
  bool isMove;                              // ch:是否允许移动标志 | en:Movement allowed flag
  HTuple HColor = "green";                  // ch:默认显示颜色 | en:Default display color
  QList<HObject> showSymbolList;            // ch:显示符号列表，用于保存显示的对象 | en:Display symbol list for saving displayed objects
  
  /* ==================== 基础图像操作接口 | Basic Image Operation Interface ==================== */
  
  /**
   * @brief 图像读取功能 | Image Reading Function
   * @param filePath 图像文件路径 | Image file path
   * @return 读取的图像对象 | Loaded image object
   */
  HObject QtReadImage(const HTuple filePath);
  
  /**
   * @brief 打开相机 | Open Camera
   * @return 是否成功打开相机 | Whether camera opened successfully
   */
  bool QtOpenCam();
  
  /**
   * @brief 相机抓取图像 | Camera Grab Image
   * @return 抓取的图像对象 | Grabbed image object
   */
  HObject QtGrabImg();
  
  /**
   * @brief 关闭相机 | Close Camera
   */
  void QtCloseCam();
  
  /* ==================== 图像显示和标注接口 | Image Display and Annotation Interface ==================== */
  
  /**
   * @brief 显示信息文本 | Display Message Text
   * @param positionX 文本X位置（像素）| Text X position (pixels)
   * @param positionY 文本Y位置（像素）| Text Y position (pixels)
   * @param text 要显示的文本内容 | Text content to display
   * @param color 文本颜色（默认绿色）| Text color (default green)
   */
  void dispHalconMessage(int positionX, int positionY, QString text, QString color = "green");
  
  /**
   * @brief 显示图像 | Show Image
   * @param inputImage 要显示的图像对象 | Image object to display
   */
  void showImage(HObject inputImage);
  
  /**
   * @brief 显示Halcon对象 | Show Halcon Object
   * @param hObject 要显示的Halcon对象 | Halcon object to display
   * @param colorStr 显示颜色字符串 | Display color string
   * @param lineWidth 线宽（默认2.0）| Line width (default 2.0)
   */
  void showHalconObject(HObject hObject, QString colorStr, double lineWidth = 2);
  /* ==================== ROI区域生成接口 | ROI Region Generation Interface ==================== */
  
  /**
   * @brief 生成任意形状区域 | Generate Any Shape Region
   * @return 生成的区域对象 | Generated region object
   */
  HObject GenAnyShapeRegion();
  
  /**
   * @brief 生成带角度的矩形区域 | Generate Angled Rectangle Region
   * @param color 显示颜色 | Display color
   * @param SaveFile 保存文件路径（可选）| Save file path (optional)
   * @return 生成的矩形区域对象 | Generated rectangle region object
   */
  HObject genAngleRec(QString color, QString SaveFile = "");
  
  /**
   * @brief 生成无角度的矩形区域 | Generate Rectangle Region Without Angle
   * @return 生成的矩形区域对象 | Generated rectangle region object
   */
  HObject genNoAngleRec();
  
  /**
   * @brief 获取直线区域 | Get Line Region
   * @return 直线区域的参数 | Line region parameters
   */
  HTuple GetLineRegion();
  
  /**
   * @brief 生成圆形区域 | Generate Circle Region
   * @return 生成的圆形区域对象 | Generated circle region object
   */
  HObject GenCircleRegionOne();
  
  /* ==================== 显示管理接口 | Display Management Interface ==================== */
  
  /**
   * @brief 移除所有显示对象 | Remove All Display Objects
   */
  void RemoveShow();
  
  /**
   * @brief 清除显示对象但保留图像 | Clear Display Objects But Keep Image
   */
  void clearDisplayObjectsOnly();
  
  /**
   * @brief 获取带有叠加层的渲染图像 | Get Rendered Image with Overlays
   * @return 包含所有显示对象的渲染图像 | Rendered image containing all display objects
   */
  HObject GetRenderedImageWithOverlays();
  
  /// ch:显示一个十字 | en:Display cross
  void DispCoorCross(double x, double y, int width, double angle = 0, QString color = "green");
  /// ch:改变图像显示区域，当窗口改变大小时，这时需要重新计算没有显示图像 | en:Change image display region
  void changeShowRegion();
  /// ch:显示图像，执行顺序，先清空窗口，再一个个显示 | en:Show Halcon image
  void showHalconImage();
  
  /* ==================== 视觉算法接口 | Vision Algorithm Interface ==================== */
  
  /**
   * @brief 创建形状模板匹配模型 | Create Shape Template Matching Model
   * @param img 输入图像 | Input image
   * @param region 感兴趣区域，用于定义模板区域 | Region of interest for defining template area
   * @param contrast 对比度参数，控制边缘提取的对比度阈值 | Contrast parameter for edge extraction threshold
   * @param mincontrast 最小对比度阈值，过滤弱边缘 | Minimum contrast threshold for filtering weak edges
   * @param file 模型保存文件路径，为空则不保存 | Model save file path, empty means no saving
   * @return 包含创建模型信息的ShapeModelData结构体 | ShapeModelData structure containing created model information
   * 
   * 该函数用于创建形状模板匹配模型，适用于工业视觉中的目标识别和定位。
   * 创建的模型可用于后续的模板匹配操作，实现快速准确的目标检测。
   * 
   * This function creates a shape template matching model for target recognition and positioning in industrial vision.
   * The created model can be used for subsequent template matching operations to achieve fast and accurate target detection.
   */
  ShapeModelData QtCreateShapeModel(HObject img, HObject region, HTuple contrast, HTuple mincontrast, QString file);
  /**
   * @brief 生成线模型和边缘检测 | Generate Line Model and Edge Detection
   * @param img 输入图像 | Input image
   * @param region 感兴趣区域，定义检测范围 | Region of interest defining detection area
   * @param threshold 边缘提取阈值，控制边缘检测的敏感度 | Edge extraction threshold controlling detection sensitivity
   * @return 包含线数据信息的LineData结构体 | LineData structure containing line information
   * 
   * 该函数用于在指定区域内进行线性边缘检测，提取直线特征。
   * 常用于工业测量中的直线度检测、边缘定位等应用。
   * 
   * This function performs linear edge detection within specified regions to extract line features.
   * Commonly used for straightness detection and edge positioning in industrial measurement applications.
   */
  LineData QtGenLine(HObject img, HObject region, HTuple threshold);
  /**
   * @brief 创建二维码识别模型 | Create 2D Code Recognition Model
   * @param CodeKind 二维码类型，如"QR Code", "Data Matrix", "PDF417"等 | 2D code type, e.g. "QR Code", "Data Matrix", "PDF417", etc.
   * @param polarity 二维码极性，"dark_on_light"（深色码在浅色背景）或"light_on_dark"（浅色码在深色背景）| 2D code polarity, "dark_on_light" or "light_on_dark"
   * @return 二维码处理句柄（模型ID）| 2D code handle (model ID)
   * 
   * 该函数用于创建二维码识别模型，支持多种二维码格式。
   * 创建的模型可用于后续的二维码识别操作。
   * 
   * This function creates a 2D code recognition model supporting various 2D code formats.
   * The created model can be used for subsequent 2D code recognition operations.
   */
  HTuple QtData2d(HTuple CodeKind, HTuple polarity);

  /**
   * @brief 识别二维码内容 | Recognize 2D Code Content
   * @param img 输入图像 | Input image
   * @param codeModel 二维码模型句柄 | 2D code model handle
   * @param num 期望识别的二维码数量，通常设为1表示识别单个二维码 | Expected number of codes to recognize, usually set to 1 for single code
   * @return 包含识别结果的CodeData结构体 | CodeData structure containing recognition results
   * 
   * 该函数使用预先创建的二维码模型对图像中的二维码进行识别和解码。
   * 支持同时识别多个二维码，返回所有识别到的内容。
   * 
   * This function uses a pre-created 2D code model to recognize and decode 2D codes in images.
   * Supports simultaneous recognition of multiple 2D codes, returning all recognized content.
   */
  CodeData QtRecogied(HObject img, HTuple codeModel, HTuple num);
  
  /* ==================== 核心功能说明 | Core Functionality Description ==================== */
  /**
   * @brief 系统核心功能模块 | System Core Functionality Modules
   * 
   * 本控件包含以下主要功能模块：
   * This widget contains the following main functional modules:
   * 
   * 1️⃣ 形状模板匹配 | Shape Template Matching
   *    - 创建和管理形状模板
   *    - 高精度目标识别和定位
   *    - 支持旋转、缩放不变匹配
   * 
   * 2️⃣ 边缘检测与线性测量 | Edge Detection and Linear Measurement  
   *    - 亚像素精度边缘提取
   *    - 直线拟合和角度测量
   *    - 工业零件尺寸检测
   * 
   * 3️⃣ 二维码识别系统 | 2D Code Recognition System
   *    - 支持QR码、Data Matrix等多种格式
   *    - 自适应对比度和灰度调整
   *    - 批量识别和内容解析
   * 
   * 4️⃣ 字符识别OCR | Character Recognition OCR
   *    - 工业字符和数字识别
   *    - 可训练的字符分类器
   *    - 支持多语言字符集
   * 
   * 5️⃣ 颜色分析与分割 | Color Analysis and Segmentation
   *    - RGB/HSV颜色空间分析
   *    - 色彩阈值分割
   *    - 颜色匹配和分类
   * 
   * 6️⃣ 卡尺测量工具 | Caliper Measurement Tools
   *    - 精密尺寸测量
   *    - 多点测量和统计分析
   *    - 测量结果可视化
   */
  
  /* ==================== 几何工具函数 | Geometric Utility Functions ==================== */
  
  /**
   * @brief 点绕中心旋转计算 | Point Rotation Around Center
   * @param centerX 旋转中心X坐标 | Rotation center X coordinate
   * @param centerY 旋转中心Y坐标 | Rotation center Y coordinate
   * @param oldX 原始点X坐标 | Original point X coordinate
   * @param oldY 原始点Y坐标 | Original point Y coordinate
   * @param angle 旋转角度（弧度）| Rotation angle (radians)
   * @return 旋转后的点坐标结构体 | Rotated point coordinate structure
   * 
   * 该函数实现点绕指定中心的旋转变换，常用于坐标系转换和几何变换计算。
   * This function implements point rotation around a specified center, commonly used for coordinate system transformation and geometric calculations.
   */
  pointStruct PointRotateByCenter(double centerX, double centerY, double oldX, double oldY, double angle);
  
  /**
   * @brief 显示Halcon对象 | Display Halcon Object
   * @param MarginFill 边距填充参数 | Margin fill parameter
   * @param color 显示颜色 | Display color
   * @param lineW 线宽 | Line width
   * @param obj 要显示的Halcon对象 | Halcon object to display
   * 
   * 该函数用于在窗口中显示各种Halcon对象，包括区域、轮廓、点等。
   * This function displays various Halcon objects in the window, including regions, contours, points, etc.
   */
  void QtShowObj(HTuple MarginFill, HTuple color, HTuple lineW, HObject obj);
  
  /**
   * @brief 获取最长XLD轮廓 | Get Longest XLD Contour
   * @param Img 输入图像 | Input image
   * @param CheckRegion 检查区域 | Check region
   * @param Thr1 阈值参数 | Threshold parameter
   * @return 最长的XLD轮廓对象 | Longest XLD contour object
   * 
   * 该函数在指定区域内提取边缘轮廓，并返回最长的一条轮廓线。
   * 常用于边缘检测和轮廓分析。
   * 
   * This function extracts edge contours within a specified region and returns the longest contour line.
   * Commonly used for edge detection and contour analysis.
   */
  HObject QtGetLengthMaxXld(HObject Img, HObject CheckRegion, int Thr1);
  /* ==================== 文件操作接口 | File Operation Interface ==================== */
  
  /**
   * @brief 保存图像到文件 | Save Image to File
   * @param mImg 要保存的图像对象 | Image object to save
   * @return 保存是否成功 | Whether saving was successful
   * 
   * 该函数将当前显示的图像保存到用户指定的文件中。
   * 支持多种图像格式，包括BMP、JPEG、PNG、TIFF等。
   * 
   * This function saves the currently displayed image to a user-specified file.
   * Supports various image formats including BMP, JPEG, PNG, TIFF, etc.
   */
  bool QtSaveImage(HObject mImg);
  
  /**
   * @brief 从本地加载图像 | Load Image from Local File
   * @param mImg 输出的图像对象引用 | Reference to output image object
   * @return 加载是否成功 | Whether loading was successful
   * 
   * 该函数从本地文件系统加载图像文件到Halcon图像对象中。
   * 自动识别图像格式并进行适当的转换。
   * 
   * This function loads image files from the local file system into Halcon image objects.
   * Automatically recognizes image formats and performs appropriate conversions.
   */
  bool QtGetLocalImage(HObject& mImg);
  
  /* ==================== 便捷功能方法 | Convenient Utility Methods ==================== */
  
  /**
   * @brief 安全读取本地图像 | Safe Read Local Image
   * @param mImg 输出的图像对象引用 | Reference to output image object
   * @param errorMessage 错误信息输出 | Error message output
   * @return 读取是否成功 | Whether reading was successful
   * 
   * 增强版的图像读取函数，提供详细的错误信息和异常处理。
   * Enhanced image reading function with detailed error information and exception handling.
   */
  bool QtGetLocalImageSafe(HObject& mImg, QString& errorMessage);
  
  /**
   * @brief 安全保存图像 | Safe Save Image
   * @param mImg 要保存的图像对象 | Image object to save
   * @param errorMessage 错误信息输出 | Error message output
   * @return 保存是否成功 | Whether saving was successful
   * 
   * 增强版的图像保存函数，提供详细的错误信息和异常处理。
   * Enhanced image saving function with detailed error information and exception handling.
   */
  bool QtSaveImageSafe(HObject mImg, QString& errorMessage);
  
  /**
   * @brief 添加显示对象到列表 | Add Display Object to List
   * @param obj 要添加的Halcon对象 | Halcon object to add
   * @param color 显示颜色（默认绿色）| Display color (default green)
   * @param lineWidth 线宽（默认2.0）| Line width (default 2.0)
   * 
   * 将对象添加到显示列表中，用于批量管理和显示多个对象。
   * Adds objects to the display list for batch management and display of multiple objects.
   */
  void addDisplayObject(HObject obj, QString color = "green", double lineWidth = 2.0);
  
  /**
   * @brief 获取显示对象数量 | Get Display Objects Count
   * @return 当前显示对象的数量 | Current number of display objects
   */
  int getDisplayObjectsCount() const;
  
  /**
   * @brief 清除指定索引的显示对象 | Clear Display Object by Index
   * @param index 要清除的对象索引 | Index of object to clear
   * @return 清除是否成功 | Whether clearing was successful
   */
  bool removeDisplayObjectByIndex(int index);
  
  /**
   * @brief 检查图像是否已加载 | Check if Image is Loaded
   * @return 图像是否已加载 | Whether image is loaded
   */
  bool isImageLoaded() const;
  
  /**
   * @brief 获取图像尺寸信息 | Get Image Size Info
   * @return 包含图像尺寸信息的字符串 | String containing image size information
   */
  QString getImageInfo() const;
  
  /* ==================== 图像预处理功能 | Image Preprocessing Functions ==================== */
  
  /**
   * @brief 应用高斯滤波 | Apply Gaussian Filter
   * @param image 输入图像 | Input image
   * @param sigma 高斯核标准差，值越大模糊效果越强 | Gaussian kernel standard deviation, higher value means stronger blur
   * @return 滤波处理后的图像 | Filtered image
   * 
   * 高斯滤波常用于图像去噪和平滑处理，能够有效减少图像中的随机噪声。
   * Gaussian filtering is commonly used for image denoising and smoothing, effectively reducing random noise in images.
   */
  HObject applyGaussianFilter(HObject image, double sigma = 1.0);

  /**
   * @brief 应用中值滤波 | Apply Median Filter
   * @param image 输入图像 | Input image
   * @param maskType 滤波掩膜类型，"circle"为圆形，"square"为方形 | Filter mask type, "circle" or "square"
   * @param maskParam 掩膜参数，圆形为半径，方形为边长 | Mask parameter, radius for circle, side length for square
   * @return 滤波处理后的图像 | Filtered image
   * 
   * 中值滤波特别适用于去除椒盐噪声，同时保持边缘信息。
   * Median filtering is particularly suitable for removing salt-and-pepper noise while preserving edge information.
   */
  HObject applyMedianFilter(HObject image, QString maskType = "circle", double maskParam = 5.0);

  /**
   * @brief 应用均值滤波 | Apply Mean Filter
   * @param image 输入图像 | Input image
   * @param maskWidth 掩膜宽度 | Mask width
   * @param maskHeight 掩膜高度 | Mask height
   * @return 滤波处理后的图像 | Filtered image
   * 
   * 均值滤波通过邻域平均实现图像平滑，计算速度快但可能模糊边缘。
   * Mean filtering achieves image smoothing through neighborhood averaging, fast computation but may blur edges.
   */
  HObject applyMeanFilter(HObject image, int maskWidth = 5, int maskHeight = 5);
  
  /* ==================== 图像增强功能 | Image Enhancement Functions ==================== */
  
  /**
   * @brief 调整图像对比度 | Adjust Image Contrast
   * @param image 输入图像 | Input image
   * @param factor 对比度调整因子，大于1增加对比度，小于1降低对比度 | Contrast adjustment factor, >1 increases contrast, <1 reduces contrast
   * @return 调整对比度后的图像 | Image with adjusted contrast
   * 
   * 通过调整像素值的分布范围来改变图像对比度，提升图像的视觉效果。
   * Changes image contrast by adjusting the distribution range of pixel values to improve visual effects.
   */
  HObject adjustImageContrast(HObject image, double factor = 1.2);

  /**
   * @brief 调整图像亮度 | Adjust Image Brightness
   * @param image 输入图像 | Input image
   * @param offset 亮度调整偏移，正值增加亮度，负值降低亮度 | Brightness adjustment offset, positive increases brightness, negative decreases it
   * @return 调整亮度后的图像 | Image with adjusted brightness
   * 
   * 通过在像素值上加减偏移量来调整图像的整体亮度。
   * Adjusts overall image brightness by adding or subtracting offset values to pixel values.
   */
  HObject adjustImageBrightness(HObject image, double offset = 10.0);

  /**
   * @brief 增强图像锐度 | Enhance Image Sharpness
   * @param image 输入图像 | Input image
   * @param factor 锐化因子，值越大锐化效果越强 | Sharpening factor, higher value means stronger sharpening
   * @param threshold 锐化阈值，控制受影响的边缘强度 | Sharpening threshold, controls affected edge intensity
   * @return 锐化处理后的图像 | Sharpened image
   * 
   * 通过增强边缘对比度来提高图像的清晰度和细节表现。
   * Improves image clarity and detail representation by enhancing edge contrast.
   */
  HObject enhanceImageSharpness(HObject image, double factor = 1.0, double threshold = 0.4);
  
  /* ==================== 图像几何变换功能 | Image Geometric Transformation Functions ==================== */
  
  /**
   * @brief 旋转图像 | Rotate Image
   * @param image 输入图像 | Input image
   * @param angle 旋转角度（度）| Rotation angle (degrees)
   * @param interpolation 插值方法（默认双线性）| Interpolation method (default bilinear)
   * @return 旋转后的图像 | Rotated image
   * 
   * 绕图像中心旋转指定角度，常用于图像校正和对齐。
   * Rotates the image around its center by a specified angle, commonly used for image correction and alignment.
   */
  HObject rotateImage(HObject image, double angle, QString interpolation = "bilinear");
  
  /**
   * @brief 缩放图像 | Scale Image
   * @param image 输入图像 | Input image
   * @param scaleX X方向缩放比例 | X-direction scale ratio
   * @param scaleY Y方向缩放比例 | Y-direction scale ratio
   * @param interpolation 插值方法（默认双线性）| Interpolation method (default bilinear)
   * @return 缩放后的图像 | Scaled image
   * 
   * 按指定比例缩放图像，支持非等比例缩放。
   * Scales image by specified ratios, supports non-proportional scaling.
   */
  HObject scaleImage(HObject image, double scaleX, double scaleY, QString interpolation = "bilinear");
  
  /**
   * @brief 裁剪图像区域 | Crop Image Region
   * @param image 输入图像 | Input image
   * @param region 裁剪区域 | Crop region
   * @return 裁剪后的图像 | Cropped image
   * 
   * 根据指定区域裁剪图像，提取感兴趣的部分。
   * Crops image according to specified region, extracting the region of interest.
   */
  HObject cropImageRegion(HObject image, HObject region);
  
  /* ==================== 测量和分析功能 | Measurement and Analysis Functions ==================== */
  
  /**
   * @brief 计算两点间距离 | Calculate Distance Between Two Points
   * @param x1 第一个点的X坐标 | X coordinate of first point
   * @param y1 第一个点的Y坐标 | Y coordinate of first point
   * @param x2 第二个点的X坐标 | X coordinate of second point
   * @param y2 第二个点的Y坐标 | Y coordinate of second point
   * @return 两点间的欧氏距离 | Euclidean distance between two points
   * 
   * 计算二维平面上两点间的直线距离，常用于尺寸测量。
   * Calculates straight-line distance between two points in 2D plane, commonly used for dimension measurement.
   */
  double calculatePointDistance(double x1, double y1, double x2, double y2);
  
  /**
   * @brief 计算三点构成的角度 | Calculate Angle from Three Points
   * @param x1 第一个点的X坐标 | X coordinate of first point
   * @param y1 第一个点的Y坐标 | Y coordinate of first point
   * @param x2 第二个点的X坐标（顶点）| X coordinate of second point (vertex)
   * @param y2 第二个点的Y坐标（顶点）| Y coordinate of second point (vertex)
   * @param x3 第三个点的X坐标 | X coordinate of third point
   * @param y3 第三个点的Y坐标 | Y coordinate of third point
   * @return 角度值（度）| Angle value (degrees)
   * 
   * 计算由三个点构成的角度，第二个点为角的顶点。
   * Calculates the angle formed by three points, with the second point as the vertex.
   */
  double calculateThreePointAngle(double x1, double y1, double x2, double y2, double x3, double y3);
  
  /**
   * @brief 计算区域面积 | Calculate Region Area
   * @param region 输入区域对象 | Input region object
   * @return 区域面积（像素数）| Region area (pixel count)
   * 
   * 计算Halcon区域对象的面积，返回像素数量。
   * Calculates the area of a Halcon region object, returning pixel count.
   */
  double calculateRegionArea(HObject region);
  
  /**
   * @brief 计算区域重心 | Calculate Region Centroid
   * @param region 输入区域对象 | Input region object
   * @return 重心坐标点结构体 | Centroid coordinate point structure
   * 
   * 计算区域的几何重心坐标，用于目标定位和对齐。
   * Calculates geometric centroid coordinates of the region for target positioning and alignment.
   */
  pointStruct calculateRegionCentroid(HObject region);
  
  /**
   * @brief 获取最小外接矩形 | Get Minimum Bounding Rectangle
   * @param region 输入区域对象 | Input region object
   * @return 最小外接矩形 | Minimum bounding rectangle
   * 
   * 获取能够完全包含指定区域的最小矩形边界。
   * Gets the minimum rectangular boundary that completely contains the specified region.
   */
  QRect getMinimumBoundingRect(HObject region);
  
  // 📝 图像标注功能 | Image annotation functions
  // ch:添加文本标注 | en:Add text annotation
  void addTextAnnotation(QString text, double x, double y, QString color = "yellow", int fontSize = 16);
  // ch:添加箭头标注 | en:Add arrow annotation
  void addArrowAnnotation(double startX, double startY, double endX, double endY, QString color = "red", double lineWidth = 2.0);
  // ch:添加尺寸标注 | en:Add dimension annotation
  void addDimensionAnnotation(double x1, double y1, double x2, double y2, QString unit = "px", QString color = "blue");
  // ch:显示坐标系 | en:Display coordinate system
  void showCoordinateSystem(double originX = 50, double originY = 50, double scale = 50, QString color = "white");
  // ch:清除所有标注 | en:Clear all annotations
  void clearAllAnnotations();
  
  // 📋 ROI管理功能 | ROI management functions
  // ch:保存ROI到文件 | en:Save ROI to file
  bool saveROIToFile(HObject region, QString filePath, QString& errorMessage);
  // ch:从文件加载ROI | en:Load ROI from file  
  bool loadROIFromFile(QString filePath, HObject& region, QString& errorMessage);
  // ch:复制ROI | en:Copy ROI
  bool copyROI(HObject sourceRegion, HObject& targetRegion);
  // ch:合并多个ROI | en:Merge multiple ROIs
  HObject mergeROIs(const QList<HObject>& regions);
  // ch:ROI偏移 | en:Offset ROI
  HObject offsetROI(HObject region, double rowOffset, double colOffset);
  
  // 📊 导出功能 | Export functions
  // ch:导出带标注的图像 | en:Export annotated image
  bool exportAnnotatedImage(QString filePath, QString& errorMessage);
  // ch:导出测量结果到CSV | en:Export measurement results to CSV  
  bool exportMeasurementResults(QString filePath, const QMap<QString, QVariant>& results, QString& errorMessage);
  // ch:截图当前显示 | en:Screenshot current display
  bool captureCurrentDisplay(QString filePath, QString& errorMessage);
  
  // ⚡ 快捷操作功能 | Quick operation functions
  // ch:缩放到适合窗口 | en:Zoom to fit window
  void zoomToFit();
  void zoomToActualSize();
  // ch:缩放到实际大小 | en:Zoom to actual size  
  void zoomToRatio(double ratio);
  // ch:撤销上一步操作 | en:Undo last operation
  bool undoLastOperation();
  // ch:重做操作 | en:Redo operation
  bool redoOperation();
  // ch:快速获取鼠标位置的像素值 | en:Quick get pixel value at mouse position
  QString getPixelValueAtPosition(double x, double y);
  
  // 🎨 颜色分析功能 | Color analysis functions
  // ch:获取区域平均颜色 | en:Get region average color
  QColor getRegionAverageColor(HObject region);
  // ch:颜色阈值分割 | en:Color threshold segmentation
  HObject colorThresholdSegmentation(HObject image, int minR, int maxR, int minG, int maxG, int minB, int maxB);
  // ch:HSV颜色分割 | en:HSV color segmentation
  HObject hsvColorSegmentation(HObject image, int minH, int maxH, int minS, int maxS, int minV, int maxV);
  
  // 🔧 高级工具功能 | Advanced tool functions
  // ch:自动对比度调整 | en:Auto contrast adjustment
  HObject autoContrastAdjustment(HObject image);
  // ch:直方图均衡化 | en:Histogram equalization
  HObject histogramEqualization(HObject image);
  // ch:边缘检测 | en:Edge detection
  HObject edgeDetection(HObject image, QString edgeType = "canny", double threshold1 = 10, double threshold2 = 20);
  // ch:形态学操作 | en:Morphological operations
  HObject morphologyOperation(HObject region, QString operation = "opening", QString structElement = "circle", double radius = 3.5);
  
  // 📈 统计分析功能 | Statistical analysis functions
  // ch:获取图像统计信息 | en:Get image statistics
  QMap<QString, double> getImageStatistics(HObject image, HObject region = HObject());
  // ch:获取区域几何特征 | en:Get region geometric features
  QMap<QString, double> getRegionFeatures(HObject region);
  // ch:计算图像质量评分 | en:Calculate image quality score
  double calculateImageQualityScore(HObject image);

  // 🧊 3D检测功能 | 3D Detection functions
  // ch:创建3D对象模型 | en:Create 3D object model
  HTuple create3DObjectModel(const QStringList& pointCloudFiles, QString& errorMessage);
  // ch:3D点云配准 | en:3D point cloud registration
  HTuple register3DPointClouds(HTuple objectModel1, HTuple objectModel2, QString& errorMessage);
  // ch:3D表面缺陷检测 | en:3D surface defect detection
  HObject detect3DSurfaceDefects(HTuple objectModel, double tolerance, QString& errorMessage);
  // ch:计算3D对象体积 | en:Calculate 3D object volume
  double calculate3DObjectVolume(HTuple objectModel, QString& errorMessage);
  // ch:3D点云滤波 | en:3D point cloud filtering
  HTuple filter3DPointCloud(HTuple objectModel, QString filterType, double param, QString& errorMessage);
  // ch:3D测量分析 | en:3D measurement analysis
  QMap<QString, double> analyze3DMeasurement(HTuple objectModel, QString& errorMessage);
  
  // 🤖 手眼标定功能 | Hand-Eye Calibration functions
  // ch:创建手眼标定数据 | en:Create hand-eye calibration data
  HTuple createHandEyeCalibrationData(QString& errorMessage);
  // ch:添加标定姿态 | en:Add calibration pose
  bool addCalibrationPose(HTuple calibData, HTuple cameraPose, HTuple robotPose, QString& errorMessage);
  // ch:执行手眼标定 | en:Perform hand-eye calibration
  HTuple performHandEyeCalibration(HTuple calibData, QString& errorMessage);
  // ch:验证标定精度 | en:Validate calibration accuracy
  QMap<QString, double> validateCalibrationAccuracy(HTuple calibData, HTuple transformation, QString& errorMessage);
  // ch:保存标定结果 | en:Save calibration results
  bool saveCalibrationResults(HTuple transformation, QString filePath, QString& errorMessage);
  // ch:加载标定结果 | en:Load calibration results
  HTuple loadCalibrationResults(QString filePath, QString& errorMessage);
  // ch:坐标系转换 | en:Coordinate system transformation
  QMap<QString, double> transformCoordinates(double x, double y, double z, HTuple transformation, QString& errorMessage);
  
  // 🎮 右键菜单功能 | Context Menu functions
  // ch:创建右键菜单 | en:Create context menu
  void setupContextMenu();
  // ch:显示右键菜单 | en:Show context menu
  void showContextMenu(const QPoint& position);
  // ch:添加菜单项 | en:Add menu item
  void addContextMenuItem(const QString& text, const QString& actionId);
  // void contextMenuEvent(QContextMenuEvent* event);
  // ch:移除菜单项 | en:Remove menu item
  void removeContextMenuItem(const QString& actionId);
  // ch:设置菜单项可见性 | en:Set menu item visibility
  void setContextMenuItemVisible(const QString& actionId, bool visible);
  
  // 🔬 高级分析功能 | Advanced Analysis functions
  // ch:频域分析 | en:Frequency domain analysis
  // HObject performFFTAnalysis(HObject image, QString& errorMessage);
  // ch:小波变换 | en:Wavelet transform
  // HObject performWaveletTransform(HObject image, QString waveletType, int levels, QString& errorMessage);
  // ch:纹理分析 | en:Texture analysis
  // QMap<QString, double> analyzeImageTexture(HObject image, HObject region, QString& errorMessage);
  // ch:直方图分析 | en:Histogram analysis
  // QMap<QString, QVariant> analyzeImageHistogram(HObject image, HObject region, QString& errorMessage);
  
  // 🎯 机器学习功能 | Machine Learning functions
  // ch:创建分类器 | en:Create classifier
  // HTuple createImageClassifier(const QStringList& trainingImages, const QStringList& labels, QString& errorMessage);
  // ch:图像分类 | en:Image classification
  // QMap<QString, double> classifyImage(HObject image, HTuple classifier, QString& errorMessage);
  // ch:异常检测 | en:Anomaly detection
  // HObject detectImageAnomalies(HObject image, HTuple model, double threshold, QString& errorMessage);

  // 🎯 新增：实时像素信息显示功能 | Real-time pixel info display
  /// ch:设置实时像素信息显示开关 | en:Enable/disable real-time pixel info display
  void setPixelInfoDisplayEnabled(bool enabled);
  /// ch:获取实时像素信息显示状态 | en:Get real-time pixel info display status
  bool isPixelInfoDisplayEnabled() const;
  /// ch:更新实时像素信息显示 | en:Update real-time pixel info display
  void updatePixelInfoDisplay(double imageX, double imageY);
  /// ch:获取详细像素信息（支持彩色图像）| en:Get detailed pixel info (support color images)
  QString getDetailedPixelInfo(double x, double y);
  /// ch:清除像素信息显示 | en:Clear pixel info display
  void clearPixelInfoDisplay();

  // 🎯 新增：防闪烁窗口优化功能 | Anti-flicker window optimization
  /// ch:设置平滑调整大小开关 | en:Enable/disable smooth resizing
  void setSmoothResizeEnabled(bool enabled);
  /// ch:获取平滑调整大小状态 | en:Get smooth resizing status
  bool isSmoothResizeEnabled() const;
  /// ch:设置防抖动延迟时间 | en:Set resize debounce delay
  void setResizeDebounceDelay(int milliseconds);
  /// ch:获取防抖动延迟时间 | en:Get resize debounce delay
  int getResizeDebounceDelay() const;
  /// ch:立即应用窗口大小变化 | en:Apply window size changes immediately
  void applyWindowSizeChange();
  /// ch:重置窗口优化状态 | en:Reset window optimization
  void resetWindowOptimization();

public:
  QMap<QString, QVariant> measurementCache;    // 测量结果缓存
  /* ==================== 私有成员变量 | Private Member Variables ==================== */
  
  HObject mShowImage;                           // ch:当前显示的图像对象 | en:Currently displayed image object
  HTuple AcqHandle;                            // ch:图像采集句柄 | en:Image acquisition handle
  QPoint lastMousePos;                         // ch:上次鼠标位置 | en:Last mouse position
  double lastRow1, lastCol1, lastRow2, lastCol2; // ch:上次选择区域的坐标 | en:Last selected region coordinates

  HTuple mHWindowID;                           // ch:Halcon窗口句柄 | en:Halcon window handle
  HTuple mQWindowID;                           // ch:Qt窗口ID | en:Qt window ID
  HObject mHoRegion;                           // ch:感兴趣区域对象 | en:Region of interest object
  double showLineWidth = 1;                   // ch:显示线宽 | en:Display line width
  HTuple draw_Row1, draw_Column1, draw_Phi1, draw_Length11, draw_Length21; // ch:绘制参数 | en:Drawing parameters

  QRect Rect;                                  // ch:矩形区域 | en:Rectangle region
  HTuple mHtWidth;                             // ch:图像宽度 | en:Image width
  HTuple mHtHeight;                            // ch:图像高度 | en:Image height
  double mDLableWidth = 564;                   // ch:标签控件宽度 | en:Label widget width
  double mdLableHeight = 568;                  // ch:标签控件高度 | en:Label widget height
  double mDDispImagePartRow0;                  // ch:显示图像部分起始行 | en:Display image part start row
  double mDDispImagePartCol0;                  // ch:显示图像部分起始列 | en:Display image part start column
  double mDDispImagePartRow1;                  // ch:显示图像部分结束行 | en:Display image part end row
  double mDDispImagePartCol1;                  // ch:显示图像部分结束列 | en:Display image part end column

  bool isDraw = false;                         // ch:是否处于绘制模式 | en:Whether in drawing mode
  
  /* ==================== 扩展私有成员变量 | Extended Private Member Variables ==================== */
  
  QList<HObject> annotationList;               // ch:标注对象列表，存储用户添加的标注 | en:Annotation object list storing user-added annotations
  QStringList operationHistory;                // ch:操作历史记录，用于撤销/重做功能 | en:Operation history for undo/redo functionality
  int currentHistoryIndex = -1;                // ch:当前历史记录索引 | en:Current history index
  HObject backupImage;                         // ch:备份图像，用于撤销操作 | en:Backup image for undo operations
  bool enableOperationHistory = true;          // ch:是否启用操作历史记录 | en:Whether to enable operation history
  
  /* ==================== 右键菜单相关 | Context Menu Related ==================== */
  QMenu* contextMenu;                           // ch:右键上下文菜单对象 | en:Right-click context menu object
  QMap<QString, QAction*> contextActions;      // ch:菜单动作映射表 | en:Menu action mapping table
  
  /* ==================== 3D检测相关 | 3D Detection Related ==================== */
  QList<HTuple> object3DModels;                // ch:3D对象模型列表 | en:3D object model list
  HTuple currentHandEyeCalibData;               // ch:当前手眼标定数据 | en:Current hand-eye calibration data
  HTuple handEyeTransformation;                 // ch:手眼变换矩阵 | en:Hand-eye transformation matrix

  /* ==================== 实时像素信息显示相关 | Real-time Pixel Info Display Related ==================== */
  bool m_pixelInfoDisplayEnabled;              // ch:像素信息显示开关 | en:Pixel info display switch
  QLabel* m_pixelInfoLabel;                    // ch:像素信息显示标签控件 | en:Pixel info display label widget
  QString m_lastPixelInfo;                     // ch:上次的像素信息（避免重复更新）| en:Last pixel info (avoid duplicate updates)
  
  /* ==================== 防闪烁优化相关 | Anti-flicker Optimization Related ==================== */
  QTimer* m_resizeTimer;                       // ch:防抖动定时器 | en:Resize debounce timer
  bool m_needWindowRecreate;                   // ch:是否需要重建窗口 | en:Whether window recreation is needed
  QSize m_lastWindowSize;                      // ch:上次窗口大小 | en:Last window size
  bool m_smoothResizeEnabled;                  // ch:平滑调整大小开关 | en:Smooth resize switch
  int m_resizeDebounceMs;                      // ch:防抖动延迟时间（毫秒）| en:Resize debounce delay (milliseconds)
  
  /* ==================== 私有辅助函数 | Private Helper Functions ==================== */
  
  /**
   * @brief 显示信息文本的私有实现 | Private Implementation for Displaying Message Text
   * @param hv_WindowHandle 窗口句柄 | Window handle
   * @param hv_String 显示字符串 | Display string
   * @param hv_CoordSystem 坐标系统 | Coordinate system
   * @param hv_Row 文本行位置 | Text row position
   * @param hv_Column 文本列位置 | Text column position
   * @param hv_Color 文本颜色 | Text color
   * @param hv_Box 文本框样式 | Text box style
   * 
   * 底层文本显示实现函数，处理字体、颜色、位置等细节。
   * Low-level text display implementation function handling font, color, position details.
   */
  void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem, 
                   HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box);
  
  /**
   * @brief 设置显示字体的私有函数 | Private Function for Setting Display Font
   * @param hv_WindowHandle 窗口句柄 | Window handle
   * @param hv_Size 字体大小 | Font size
   * @param hv_Font 字体类型 | Font type
   * @param hv_Bold 是否粗体 | Whether bold
   * @param hv_Slant 是否斜体 | Whether italic
   * 
   * 配置Halcon窗口的字体显示属性。
   * Configures font display properties for Halcon window.
   */
  void set_display_font(HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font, HTuple hv_Bold, HTuple hv_Slant);
  
  /**
   * @brief 获取显示区域参数 | Get Display Region Parameters
   * @param row1 起始行坐标指针 | Start row coordinate pointer
   * @param col1 起始列坐标指针 | Start column coordinate pointer
   * @param row2 结束行坐标指针 | End row coordinate pointer
   * @param col2 结束列坐标指针 | End column coordinate pointer
   * 
   * 获取当前图像显示区域的浮点坐标参数。
   * Gets floating-point coordinate parameters of current image display region.
   */
  void GetPartFloat(double *row1, double *col1, double *row2, double *col2);
  
  /* ==================== 优化相关私有方法 | Optimization Related Private Methods ==================== */
  
  /**
   * @brief 更新像素信息标签位置 | Update Pixel Info Label Position
   * 
   * 根据当前鼠标位置和窗口大小动态调整像素信息显示标签的位置，
   * 确保标签始终在可见区域内且不遮挡重要内容。
   * 
   * Dynamically adjusts pixel info display label position based on current mouse position and window size,
   * ensuring the label stays within visible area without blocking important content.
   */
  void updatePixelInfoLabelPosition();
  
  /**
   * @brief 右键菜单触发处理槽函数 | Context Menu Trigger Handler Slot
   * 
   * 处理右键菜单中各项操作的响应，根据用户选择执行相应的功能。
   * Handles responses to various operations in the context menu, executing corresponding functions based on user selection.
   */
  void onContextMenuTriggered();

  /**
   * @brief 初始化Halcon窗口 | Initialize Halcon Window
   * 
   * 执行Halcon显示窗口的初始化设置，包括窗口创建、参数配置等。
   * Performs initialization setup for Halcon display window, including window creation and parameter configuration.
   */
  void initHalconWindow();
  
  /**
   * @brief 确保Halcon窗口已初始化 | Ensure Halcon Window is Initialized
   * @return 初始化是否成功 | Whether initialization was successful
   * 
   * 检查并确保Halcon窗口已正确初始化，如未初始化则自动执行初始化。
   * Checks and ensures Halcon window is properly initialized, automatically initializes if not.
   */
  bool ensureHalconWindowInitialized();

/* ==================== 保护成员函数 | Protected Member Functions ==================== */
protected:
  /**
   * @brief 鼠标移动事件处理 | Mouse Move Event Handler
   * @param event 鼠标事件对象 | Mouse event object
   * 
   * 处理鼠标移动事件，用于拖动图像移动、实时像素信息显示等功能。
   * 支持图像拖拽平移和鼠标坐标跟踪。
   * 
   * Handles mouse move events for image dragging, real-time pixel info display, etc.
   * Supports image drag panning and mouse coordinate tracking.
   */
  void mouseMoveEvent(QMouseEvent *event);
  
  /**
   * @brief 鼠标滚轮事件处理 | Mouse Wheel Event Handler
   * @param event 滚轮事件对象 | Wheel event object
   * 
   * 处理鼠标滚轮事件，实现图像的缩放操作。
   * 支持以鼠标位置为中心的平滑缩放。
   * 
   * Handles mouse wheel events for image scaling operations.
   * Supports smooth zooming centered at mouse position.
   */
  void wheelEvent(QWheelEvent *event);
  
  /**
   * @brief 鼠标双击事件处理 | Mouse Double Click Event Handler
   * @param event 鼠标事件对象 | Mouse event object
   * 
   * 处理鼠标双击事件，通常用于图像显示复位到原始大小。
   * Handles mouse double click events, typically used for resetting image display to original size.
   */
  void mouseDoubleClickEvent(QMouseEvent *event);
  
  /**
   * @brief 鼠标按下事件处理 | Mouse Press Event Handler
   * @param event 鼠标事件对象 | Mouse event object
   * 
   * 处理鼠标按下事件，配合移动事件获取窗口内坐标，
   * 开始拖拽操作或ROI绘制。
   * 
   * Handles mouse press events, works with move events to get coordinates within window,
   * starts dragging operations or ROI drawing.
   */
  void mousePressEvent(QMouseEvent *event);
  
  /**
   * @brief 鼠标释放事件处理 | Mouse Release Event Handler
   * @param event 鼠标事件对象 | Mouse event object
   * 
   * 处理鼠标释放事件，配合移动事件完成图像移动效果，
   * 结束拖拽操作或完成ROI绘制。
   * 
   * Handles mouse release events, completes image movement effects with move events,
   * ends dragging operations or completes ROI drawing.
   */
  void mouseReleaseEvent(QMouseEvent *event);
  
  /**
   * @brief 鼠标离开事件处理 | Mouse Leave Event Handler
   * @param event 事件对象 | Event object
   * 
   * 处理鼠标离开控件区域的事件，清除实时像素信息显示。
   * Handles mouse leave events from widget area, clears real-time pixel info display.
   */
  void leaveEvent(QEvent *event);
  
  /**
   * @brief 窗口大小改变事件处理 | Resize Event Handler
   * @param event 大小改变事件对象 | Resize event object
   * 
   * 处理窗口大小改变事件，重新计算图像显示区域，
   * 确保图像正确适应新的窗口尺寸。
   * 
   * Handles window resize events, recalculates image display region,
   * ensures image properly adapts to new window size.
   */
  void resizeEvent(QResizeEvent *event);

/* ==================== 信号定义 | Signal Definitions ==================== */
signals:
  /**
   * @brief 右键菜单动作触发信号 | Context Menu Action Triggered Signal
   * @param actionId 触发的动作ID | Triggered action ID
   * 
   * 当用户点击右键菜单中的某个选项时发出此信号。
   * 外部可以连接此信号来处理特定的菜单操作。
   * 
   * Emitted when user clicks an option in the context menu.
   * External code can connect to this signal to handle specific menu operations.
   */
  void contextMenuActionTriggered(const QString& actionId);
  
  /**
   * @brief 3D检测完成信号 | 3D Detection Completed Signal
   * @param results 检测结果数据映射 | Detection result data mapping
   * 
   * 当3D检测算法完成时发出此信号，包含检测到的所有结果数据。
   * 结果包括体积、表面缺陷、测量数据等信息。
   * 
   * Emitted when 3D detection algorithm completes, containing all detected result data.
   * Results include volume, surface defects, measurement data, and other information.
   */
  void detection3DCompleted(const QMap<QString, QVariant>& results);
  
  /**
   * @brief 手眼标定完成信号 | Hand-Eye Calibration Completed Signal
   * @param success 标定是否成功 | Whether calibration was successful
   * @param message 结果消息或错误信息 | Result message or error information
   * 
   * 当手眼标定过程完成时发出此信号，无论成功还是失败。
   * 提供标定结果状态和相关消息信息。
   * 
   * Emitted when hand-eye calibration process completes, whether successful or not.
   * Provides calibration result status and related message information.
   */
  void handEyeCalibrationCompleted(bool success, const QString& message);

/* ==================== 公有槽函数 | Public Slot Functions ==================== */
public slots:
  // ch:预留槽函数接口，可根据需要添加具体实现 | en:Reserved slot function interface, specific implementation can be added as needed

};

/**
 * @brief 文件结束标记 | End of File Marker
 * 
 * HalconLable.h - Halcon图像显示控件头文件
 * 版本: 2.0
 * 最后更新: 2025年6月1日
 * 
 * 本文件定义了功能完整的Halcon图像显示和处理控件类，
 * 集成了图像显示、ROI操作、测量分析、3D检测、手眼标定等
 * 多种机器视觉功能，为工业视觉应用提供全面的解决方案。
 * 
 * HalconLable.h - Halcon Image Display Widget Header File
 * Version: 2.0
 * Last Updated: June 1, 2025
 * 
 * This file defines a fully functional Halcon image display and processing widget class,
 * integrating multiple machine vision functions including image display, ROI operations,
 * measurement analysis, 3D detection, hand-eye calibration, etc., providing comprehensive
 * solutions for industrial vision applications.
 */

#endif // HALCONLABLE_H
