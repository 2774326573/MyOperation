/**
 * @file visualprocess.h
 * @brief 视觉处理主控制类头文件 | Visual Processing Main Control Class Header
 * @author Your Name
 * @date 2025-06-01
 * @version 1.0
 * 
 * 该文件包含视觉处理系统的核心类定义，支持二维码识别、测量和检测功能
 * This file contains the core class definition for the visual processing system,
 * supporting QR code recognition, measurement, and detection functions.
 */

#ifndef VISUALPROCESS_H
#define VISUALPROCESS_H

// Qt框架相关头文件 | Qt Framework Headers
#include <QWidget>
#include <qboxlayout.h>
#include <QObject>
#include <QStandardItemModel>
#include <QTimer>
#include <QVariantMap>
#include <QDateTime>

// Halcon机器视觉库头文件 | Halcon Machine Vision Library Header
#include "halconcpp/HalconCpp.h"

// 前向声明 | Forward Declarations
class QToolButton;     // Qt工具按钮
class QLabel;          // Qt标签控件
class HalconLable;     // Halcon显示控件
class HalconFileManager;  // Halcon文件管理器
class DynamicUIBuilder;   // 动态UI构建器
class ConfigManager;  // 配置管理器

// Qt命名空间声明 | Qt Namespace Declaration
QT_BEGIN_NAMESPACE

namespace Ui
{
  class VisualProcess;  // UI界面类前向声明 | UI Class Forward Declaration
}

QT_END_NAMESPACE

// 使用Halcon命名空间 | Using Halcon Namespace
using namespace HalconCpp;

/**
 * @struct VisualTaskParams
 * @brief 视觉处理任务参数结构体 | Visual Processing Task Parameters Structure
 * 
 * 该结构体定义了视觉处理任务所需的所有参数，包括通用参数和各种任务类型的特定参数
 * This structure defines all parameters required for visual processing tasks,
 * including general parameters and specific parameters for various task types.
 */
struct VisualTaskParams {
    QString taskType;           // 任务类型：QRCode, Measure, Detection | Task type: QRCode, Measure, Detection
    QString templateName;       // 模板名称 | Template name
    QVariantMap parameters;     // 任务特定参数 | Task-specific parameters
    QString regionPath;         // 区域文件路径（HOBJ格式）| Region file path (HOBJ format)
    QString modelPath;          // 模型文件路径 | Model file path
    bool isValid = false;       // 参数是否有效 | Whether parameters are valid
    
    // 二维码特定参数 | QR Code Specific Parameters
    QString qrCodeType;         // 二维码类型 | QR code type
    QString qrPolarity;         // 极性设置 | Polarity setting
    
    // 测量特定参数 | Measurement Specific Parameters  
    QString measureType;        // 测量类型 | Measurement type
    int edgeThreshold;          // 边缘阈值 | Edge threshold
    double measurePrecision;    // 测量精度 | Measurement precision
    
    // 检测特定参数 | Detection Specific Parameters
    int maxContrast;            // 最大对比度 | Maximum contrast
    int minContrast;            // 最小对比度 | Minimum contrast
    double matchThreshold;      // 匹配阈值 | Match threshold
    QString pyramidLevels;      // 金字塔层数 | Pyramid levels
};

/**
 * @struct VisualProcessResult
 * @brief 处理结果结构体 | Processing Result Structure
 * 
 * 该结构体用于存储视觉处理任务的执行结果，包括成功状态、错误信息和各种类型的结果数据
 * This structure stores the execution results of visual processing tasks,
 * including success status, error messages, and various types of result data.
 */
struct VisualProcessResult {
    QString taskType;           // 任务类型 | Task type
    bool success;               // 处理是否成功 | Whether processing was successful
    QString errorMessage;       // 错误信息 | Error message
    QVariantMap resultData;     // 结果数据 | Result data
    QString imagePath;          // 处理的图像路径 | Processed image path
    QDateTime processTime;      // 处理时间 | Processing time
    
    // 测量结果特定数据 | Measurement Result Specific Data
    double minDistance = 0.0;      // 最小距离 | Minimum distance
    double maxDistance = 0.0;      // 最大距离 | Maximum distance
    double centroidDistance = 0.0; // 质心距离 | Centroid distance
    
    // 二维码结果特定数据 | QR Code Result Specific Data
    QStringList decodedTexts;      // 解码文本 | Decoded texts
    QList<QPointF> codePositions;  // 二维码位置 | QR code positions
    
    // 检测结果特定数据 | Detection Result Specific Data
    int matchCount = 0;            // 匹配数量 | Match count
    QList<QPointF> matchPositions; // 匹配位置 | Match positions
    QList<double> matchScores;     // 匹配分数 | Match scores
};

/**
 * @class VisualProcess
 * @brief 视觉处理主控制类 | Visual Processing Main Control Class
 * 
 * 该类是视觉处理系统的核心控制类，负责管理二维码识别、测量和检测等功能。
 * 提供统一的接口来处理不同类型的视觉任务，包括参数配置、任务执行和结果管理。
 * 
 * This class is the core control class of the visual processing system, responsible for
 * managing QR code recognition, measurement, and detection functions. It provides a
 * unified interface to handle different types of visual tasks, including parameter
 * configuration, task execution, and result management.
 */
class VisualProcess : public QWidget
{
  Q_OBJECT

  /**
   * @enum Level
   * @brief 日志级别枚举 | Log Level Enumeration
   */
  enum Level
  {
    DEBUG = 0,    // 调试级别 | Debug level
    INFO,         // 信息级别 | Information level
    WARNING,      // 警告级别 | Warning level
    ERR           // 错误级别 | Error level
  };

  /**
   * @enum ComboBoxType
   * @brief 模式选择类型枚举 | Mode Selection Type Enumeration
   */
  enum ComboBoxType
  {
    UNKNOWN = 0,  // 未知类型 | Unknown type
    QRCODE,       // 二维码模式 | QR code mode
    MEASURE,      // 测量模式 | Measurement mode
    CHECK         // 检测模式 | Detection mode
  };
  /* ============================== 公有接口 | Public Interface ==============================*/
public:
  /**
   * @brief 构造函数 | Constructor
   * @param parent 父窗口指针 | Parent widget pointer
   */
  explicit VisualProcess(QWidget* parent = nullptr);
  
  /**
   * @brief 析构函数 | Destructor
   */
  ~VisualProcess() override;

  /**
   * @brief 获取当前配置的任务参数 | Get Current Configured Task Parameters
   * @return 当前任务参数结构体 | Current task parameters structure
   */
  VisualTaskParams getCurrentTaskParams() const;
  
  /**
   * @brief 验证任务参数是否完整 | Validate Task Parameters Completeness
   * @param params 要验证的任务参数 | Task parameters to validate
   * @param errorMessage 错误信息输出参数 | Error message output parameter
   * @return 参数是否有效 | Whether parameters are valid
   */
  bool validateTaskParams(const VisualTaskParams& params, QString& errorMessage) const;
  
  /**
   * @brief 启动批处理任务 | Start Batch Processing Task
   */
  void startBatchProcessing();
  
  /**
   * @brief 停止批处理任务 | Stop Batch Processing Task
   */
  void stopBatchProcessing();

private:
  /* ============================== 配置管理器 | configmanager ==============================*/
  ConfigManager* m_configManager; // 配置管理器 | Configuration manager
  /// 当前时间
  QString getCurrentTime() const;
  /* ============================== 相机状态标志 | Camera Status Flags ==============================*/
  bool m_QCodeCam_flag;          // 二维码相机标志 | QR code camera flag
  bool m_isOpenQCodeCam_flag;    // 二维码相机是否打开标志 | QR code camera open flag
  bool m_MeasureCam_flag;        // 测量相机标志 | Measurement camera flag
  bool m_isOpenMeasureCam_flag;  // 测量相机是否打开标志 | Measurement camera open flag
  bool m_CheckCam_flag;          // 检测相机标志 | Detection camera flag
  bool m_isOpenCheckCam_flag;    // 检测相机是否打开标志 | Detection camera open flag

  /* ============================== UI组件管理 | UI Component Management ==============================*/
  QWidget* m_Test_Result_Widget = nullptr;  // 测试结果窗口 | Test result window
  QWidget* m_Create_Model_Widget = nullptr; // 创建模板窗口 | Create template window
  QWidget* m_Create_Shape_Model_Widget = nullptr;
  QWidget* m_Create_Result_Widget = nullptr; // 创建结果窗口 | Create result window

  QLabel* gifTip;    // GIF提示标签 | GIF tip label
  QMovie* movie;     // 动画对象 | Animation object
  QMap<QToolButton*, QPair<QLabel*, QMovie*>> m_buttonTips; // 按钮提示映射 | Button tips mapping

  /* ============================== UI动态构建 | Dynamic UI Building ==============================*/
  DynamicUIBuilder* m_uiBuilder; // 动态UI构建器 | Dynamic UI builder  

  /* ============================== 文件路径配置 | File Path Configuration ==============================*/
  QString m_halconParamPath; // Halcon参数路径 | Halcon parameters path
  QString m_modelSavePath;   // 模板保存路径 | Template save path
  QString QRcodeSavePath;   // 二维码保存路径 | QR code save path
  QString MeasureSavePath; // 测量结果保存路径 | Measurement result save path
  QString CheckSavePath;  // 检测结果保存路径 | Detection result save path
  
  /* ============================== 视觉处理组件 | Visual Processing Components ==============================*/
  HalconLable* halWin;              // Halcon显示窗口 | Halcon display window
  HalconFileManager* m_halFileManager; // Halcon文件管理器 | Halcon file manager
  HObject m_Img;                    // 当前图像对象 | Current image object
  HObject m_Read_code_first, m_Read_code_tail; // 二维码读取对象 | QR code reading objects
  HObject m_Measyre_Rect1, m_Measyre_Rect2;   // 测量矩形对象 | Measurement rectangle objects
  QString m_region_Kind = "";       // 区域种类标识 | Region type identifier
  HObject m_CheckRegion;            // 检测区域对象 | Detection region object
  
  /* ============================== 视觉算法参数 | Vision Algorithm Parameters ==============================*/
  HTuple m_Data_code_handle;        // 数据码处理句柄 | Data code handle
  HTuple m_Decode_data_strings;     // 解码数据字符串 | Decoded data strings
  HObject m_SymbolXLDs;             // 符号XLD对象 | Symbol XLD objects
  HObject m_OKRegion, m_NGRegion;   // OK区域和NG区域 | OK and NG regions

signals:
  /* ============================== 任务管理信号 | Task Management Signals ==============================*/
  /**
   * @brief 任务参数准备完成信号 | Task Parameters Ready Signal
   * @param params 准备好的任务参数 | Prepared task parameters
   */
  void taskParamsReady(const VisualTaskParams& params);
  
  /**
   * @brief 处理请求信号 | Processing Request Signal
   * @param params 处理任务参数 | Processing task parameters
   */
  void processingRequested(const VisualTaskParams& params);
  
  /**
   * @brief 处理停止请求信号 | Processing Stop Request Signal
   */
  void processingStopRequested();
  
  /**
   * @brief 结果生成信号 | Result Generated Signal
   * @param result 生成的处理结果 | Generated processing result
   */
  void resultGenerated(const VisualProcessResult& result);
  
  /* ============================== UI交互信号 | UI Interaction Signals ==============================*/
  /**
   * @brief 工具按钮点击信号 | Tool Button Clicked Signal
   * @param key 按钮标识键 | Button identification key
   */
  void toolButtonClicked(const QString& key);
  /* ============================== 公有槽函数 | Public Slots ==============================*/
public slots:
  /**
   * @brief 下拉框选择槽函数 | ComboBox Selection Slot
   * @param index 选择的索引 | Selected index
   */
  void onComboBoxAction(int index);
  
  /**
   * @brief 二维码处理槽函数 | QR Code Processing Slot
   */
  void onQCodeTabWidget();
  
  /**
   * @brief 测量相机处理槽函数 | Measurement Camera Processing Slot
   */
  void onMeasureCam();
  
  /**
   * @brief 检测相机处理槽函数 | Detection Camera Processing Slot
   */
  void onCheckCam();
  
  /**
   * @brief 工具按钮点击事件处理槽函数 | Tool Button Click Event Handler Slot
   * @param key 按钮标识键 | Button identification key
   */
  void handleToolButtonClicked(const QString& key);

  /**
   * @brief 创建模板槽函数(使用动态UI构建器) | Create Template Slot (Using Dynamic UI Builder)
   */
  void onCreateModelWithBuilder();

  /* ============================== 工作线程结果处理槽函数 | Work Thread Result Processing Slots ==============================*/
  /**
   * @brief 处理结果接收槽函数 | Processing Result Reception Slot
   * @param result 处理结果 | Processing result
   */
  void onProcessingResult(const VisualProcessResult& result);
  
  /**
   * @brief 批处理进度更新槽函数 | Batch Processing Progress Update Slot
   * @param current 当前处理数量 | Current processing count
   * @param total 总处理数量 | Total processing count
   * @param currentFile 当前处理文件 | Current processing file
   */
  void onProcessingProgress(int current, int total, const QString& currentFile);
  
  /**
   * @brief 处理错误槽函数 | Processing Error Slot
   * @param error 错误信息 | Error message
   */
  void onProcessingError(const QString& error);
  /* ==================== 私有成员函数 | Private Member Functions ==================== */
private:
  /* ==================== 初始化函数组 | Initialization Function Group ==================== */
  /**
   * @brief 初始化日志系统 | Initialize Log System
   */
  static void initLog();
  void initConfig();

  /**
   * @brief 初始化视觉处理系统 | Initialize Vision Processing System
   */
  void initVisionProcess();
  
  /**
   * @brief 初始化信号槽连接 | Initialize Signal-Slot Connections
   */
  void initConnect();

  /* ==================== 模板创建逻辑组 | Template Creation Logic Group ==================== */
  /**
   * @brief 处理创建模板的核心逻辑 | Handle Template Creation Core Logic
   */
  void handleCreateTemplate();
  
  /**
   * @brief 处理二维码模板创建 | Handle QR Code Template Creation
   * @param fileName 文件名 | File name
   */
  void handleQRCodeTemplateCreation(const QString& fileName);
  
  /**
   * @brief 处理测量模板创建 | Handle Measurement Template Creation
   * @param fileName 文件名 | File name
   */
  void handleMeasureTemplateCreation(const QString& fileName);
  
  /**
   * @brief 处理检测模板创建 | Handle Detection Template Creation
   * @param fileName 文件名 | File name
   */
  void handleDetectionTemplateCreation(const QString& fileName);
  void onCreateShapeModelWithBuilder(const QString& fileName);

  void handleShapeTemplateCreation(const QString& fileName);

  /* ==================== 模板创建辅助函数组 | Template Creation Helper Function Group ==================== */
  /**
   * @brief 关闭模板创建窗口 | Close Template Creation Window
   */
  void closeTemplateWindow();
  
  /**
   * @brief 保存二维码模板信息 | Save QR Code Template Information
   * @param filePath 文件路径 | File path
   * @param fileName 文件名 | File name
   * @param modelType 模型类型 | Model type
   * @param polarity 极性设置 | Polarity setting
   * @param customParams 自定义参数 | Custom parameters
   */
  void saveQRCodeTemplateInfo(const QString& filePath, const QString& fileName, 
                              const QString& modelType, const QString& polarity, const QString& customParams = "");
  
  /**
   * @brief 保存测量模板信息 | Save Measurement Template Information
   * @param filePath 文件路径 | File path
   * @param fileName 文件名 | File name
   * @param measureType 测量类型 | Measurement type
   * @param edgeThreshold 边缘阈值 | Edge threshold
   * @param measurePrecision 测量精度 | Measurement precision
   */
  void saveMeasureTemplateInfo(const QString& filePath, const QString& fileName, 
                               const QString& measureType, int edgeThreshold, double measurePrecision = 0.01);
  
  /**
   * @brief 保存检测模板信息 | Save Detection Template Information
   * @param filePath 文件路径 | File path
   * @param fileName 文件名 | File name
   * @param modelId 模型ID | Model ID
   * @param maxContrast 最大对比度 | Maximum contrast
   * @param minContrast 最小对比度 | Minimum contrast
   * @param matchThreshold
   * @param pyramidLevels
   */
  void saveDetectionTemplateInfo(const QString& filePath, const QString& fileName,
                                 QString modelId, int maxContrast, int minContrast, double matchThreshold, const QString& pyramidLevels);
  /* ==================== 通用辅助工具函数组 | General Utility Function Group ==================== */
  /**
   * @brief 应用程序日志记录 | Application Log Recording
   * @param message 日志消息 | Log message
   * @param level 日志级别 | Log level
   */
  void appLog(QString message, Level level = INFO);
  
  /**
   * @brief 清理布局中的所有控件 | Clear All Widgets in Layout
   * @param layout 要清理的布局 | Layout to be cleared
   */
  void clearLaout(QLayout* layout);
  
  /**
   * @brief 创建工具按钮组 | Create Tool Button Group
   * @param layout 布局对象 | Layout object
   * @param buttonConfigs 按钮配置列表 | Button configuration list
   */
  void createToolButtons(QVBoxLayout* layout, const QVector<QPair<QString, QString>>& buttonConfigs);
  
  /**
   * @brief 事件过滤器重写 | Event Filter Override
   * @param watched 被监视的对象 | Watched object
   * @param event 事件对象 | Event object
   * @return 是否处理该事件 | Whether to handle the event
   */
  bool eventFilter(QObject* watched, QEvent* event) override;
  
  /**
   * @brief 仅清理显示对象 | Clear Display Objects Only
   */
  void clearDisplayObjectsOnly();
  /* ==================== 任务管理成员变量组 | Task Management Member Variables Group ==================== */
  VisualTaskParams m_currentTaskParams;        // 当前任务参数缓存 | Current task parameters cache
  QList<VisualProcessResult> m_resultHistory;  // 结果历史记录 | Result history records
  bool m_batchProcessing = false;              // 批处理状态标志 | Batch processing status flag
  
  /* ==================== 任务参数提取方法组 | Task Parameter Extraction Method Group ==================== */
  /**
   * @brief 提取二维码任务参数 | Extract QR Code Task Parameters
   * @return 二维码任务参数 | QR code task parameters
   */
  VisualTaskParams extractQRCodeParams() const;
  
  /**
   * @brief 提取测量任务参数 | Extract Measurement Task Parameters
   * @return 测量任务参数 | Measurement task parameters
   */
  VisualTaskParams extractMeasureParams() const;
  
  /**
   * @brief 提取检测任务参数 | Extract Detection Task Parameters
   * @return 检测任务参数 | Detection task parameters
   */
  VisualTaskParams extractDetectionParams() const;
  
  /* ==================== 结果显示方法组 | Result Display Method Group ==================== */
  /**
   * @brief 显示处理结果 | Display Processing Result
   * @param result 处理结果 | Processing result
   */
  void displayProcessingResult(const VisualProcessResult& result);
  
  /**
   * @brief 更新结果历史记录 | Update Result History
   * @param result 处理结果 | Processing result
   */
  void updateResultHistory(const VisualProcessResult& result);  
  
  /* ==================== 参数配置窗口创建方法组 | Parameter Configuration Window Creation Method Group ==================== */
  
  /* -------------------- 📷 相机参数配置窗口 | Camera Parameter Configuration Windows -------------------- */
  /**
   * @brief 连接相机参数按钮 | Connect Camera Parameter Buttons
   * @param window 窗口对象 | Window object
   * @param action 操作类型 | Action type
   */
  void connectCameraParamButtons(QWidget* window, const QString& action);
  
  /* -------------------- 💾 文件操作参数窗口 | File Operation Parameter Windows -------------------- */
  /**
   * @brief 创建图像保存参数窗口 | Create Image Save Parameter Window
   */
  void createImageSaveParamWindow();
  
  /**
   * @brief 创建图像加载参数窗口 | Create Image Load Parameter Window
   */
  void createImageLoadParamWindow();
  
  /**
   * @brief 创建图像清除参数窗口 | Create Image Clear Parameter Window
   */
  void createImageClearParamWindow();
  
  /**
   * @brief 连接图像保存参数按钮 | Connect Image Save Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectImageSaveParamButtons(QWidget* window);
  
  /**
   * @brief 连接图像加载参数按钮 | Connect Image Load Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectImageLoadParamButtons(QWidget* window);
  
  /**
   * @brief 连接图像清除参数按钮 | Connect Image Clear Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectImageClearParamButtons(QWidget* window);  
  /* -------------------- 🔍 二维码功能参数窗口 | QR Code Function Parameter Windows -------------------- */
  /**
   * @brief 创建二维码区域参数窗口 | Create QR Code Region Parameter Window
   */
  void createQRCodeRegionParamWindow();
  
  /**
   * @brief 创建二维码批处理参数窗口 | Create QR Code Batch Parameter Window
   */
  void createQRCodeBatchParamWindow();
  
  /**
   * @brief 创建二维码模板参数窗口 | Create QR Code Template Parameter Window
   */
  void createQRCodeTemplateParamWindow();
  
  /**
   * @brief 连接二维码区域参数按钮 | Connect QR Code Region Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectQRCodeRegionParamButtons(QWidget* window);
  
  /**
   * @brief 连接二维码批处理参数按钮 | Connect QR Code Batch Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectQRCodeBatchParamButtons(QWidget* window);
  
  /**
   * @brief 连接二维码模板参数按钮 | Connect QR Code Template Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectQRCodeTemplateParamButtons(QWidget* window);  

  /* -------------------- 📏 测量功能参数窗口 | Measurement Function Parameter Windows -------------------- */

  /**
   * @brief 创建距离测量参数窗口 | Create Distance Measurement Parameter Window
   */
  void createDistanceMeasureParamWindow();
  
  /**
   * @brief 创建角度测量参数窗口 | Create Angle Measurement Parameter Window
   */
  void createAngleMeasureParamWindow();
  
  /**
   * @brief 创建圆形测量参数窗口 | Create Circle Measurement Parameter Window
   */
  void createCircleMeasureParamWindow();
  
  /**
   * @brief 创建轮廓分析参数窗口 | Create Contour Analysis Parameter Window
   */
  void createContourAnalysisParamWindow();
  
  /**
   * @brief 创建测量模板参数窗口 | Create Measurement Template Parameter Window
   */
  void createMeasureTemplateParamWindow();
  
  /**
   * @brief 连接距离测量参数按钮 | Connect Distance Measurement Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectDistanceMeasureParamButtons(QWidget* window);
  
  /**
   * @brief 连接角度测量参数按钮 | Connect Angle Measurement Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectAngleMeasureParamButtons(QWidget* window);
  
  /**
   * @brief 连接圆形测量参数按钮 | Connect Circle Measurement Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectCircleMeasureParamButtons(QWidget* window);
  
  /**
   * @brief 连接轮廓分析参数按钮 | Connect Contour Analysis Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectContourAnalysisParamButtons(QWidget* window);
  
  /**
   * @brief 连接测量模板参数按钮 | Connect Measurement Template Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectMeasureTemplateParamButtons(QWidget* window);  

  /* -------------------- 🔍 检测功能参数窗口 | Detection Function Parameter Windows -------------------- */

  /**
   * @brief 创建特征匹配参数窗口 | Create Feature Match Parameter Window
   */
  void createFeatureMatchParamWindow();
  
  /**
   * @brief 创建缺陷检测参数窗口 | Create Defect Detection Parameter Window
   */
  void createDefectDetectionParamWindow();
  
  /**
   * @brief 创建尺寸检测参数窗口 | Create Size Detection Parameter Window
   */
  void createSizeDetectionParamWindow();
  
  /**
   * @brief 创建对齐参数窗口 | Create Alignment Parameter Window
   */
  void createAlignmentParamWindow();
  
  /**
   * @brief 创建检测模板参数窗口 | Create Detection Template Parameter Window
   */
  void createDetectionTemplateParamWindow();
  
  /**
   * @brief 连接特征匹配参数按钮 | Connect Feature Match Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectFeatureMatchParamButtons(QWidget* window);
  
  /**
   * @brief 连接缺陷检测参数按钮 | Connect Defect Detection Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectDefectDetectionParamButtons(QWidget* window);
  
  /**
   * @brief 连接尺寸检测参数按钮 | Connect Size Detection Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectSizeDetectionParamButtons(QWidget* window);
  
  /**
   * @brief 连接对齐参数按钮 | Connect Alignment Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectAlignmentParamButtons(QWidget* window);
  
  /**
   * @brief 连接检测模板参数按钮 | Connect Detection Template Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectDetectionTemplateParamButtons(QWidget* window);  

  /* -------------------- 🎯 图像处理功能参数窗口 | Image Processing Function Parameter Windows -------------------- */
  /**
   * @brief 创建图像预处理参数窗口 | Create Image Preprocessing Parameter Window
   */
  void createImagePreprocessParamWindow();
  
  /**
   * @brief 创建快速操作参数窗口 | Create Quick Operation Parameter Window
   */
  void createQuickOperationParamWindow();
  
  /**
   * @brief 创建导出结果参数窗口 | Create Export Result Parameter Window
   */
  void createExportResultParamWindow();
  
  /**
   * @brief 连接图像预处理参数按钮 | Connect Image Preprocessing Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectImagePreprocessParamButtons(QWidget* window);
  
  /**
   * @brief 连接快速操作参数按钮 | Connect Quick Operation Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectQuickOperationParamButtons(QWidget* window);
  
  /**
   * @brief 连接导出结果参数按钮 | Connect Export Result Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectExportResultParamButtons(QWidget* window);  

  /* -------------------- 🚀 批处理功能参数窗口 | Batch Processing Function Parameter Windows -------------------- */
  /**
   * @brief 创建批处理参数窗口 | Create Batch Processing Parameter Window
   */
  void createBatchProcessParamWindow();
  
  /**
   * @brief 创建停止批处理参数窗口 | Create Stop Batch Processing Parameter Window
   */
  void createStopBatchParamWindow();
  
  /**
   * @brief 创建结果历史参数窗口 | Create Result History Parameter Window
   */
  void createResultHistoryParamWindow();
  
  /**
   * @brief 连接批处理参数按钮 | Connect Batch Processing Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectBatchProcessParamButtons(QWidget* window);
  
  /**
   * @brief 连接停止批处理参数按钮 | Connect Stop Batch Processing Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectStopBatchParamButtons(QWidget* window);
  
  /**
   * @brief 连接结果历史参数按钮 | Connect Result History Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectResultHistoryParamButtons(QWidget* window);  

  /* -------------------- ⚙️ 通用设置参数窗口 | General Settings Parameter Windows -------------------- */

  /**
   * @brief 创建通用设置参数窗口 | Create General Settings Parameter Window
   */
  void createGeneralSettingsParamWindow();
  
  /**
   * @brief 创建结果视图参数窗口 | Create Result View Parameter Window
   * @param resultType 结果类型 | Result type
   */
  void createResultViewParamWindow(const QString& resultType);
  
  /**
   * @brief 连接通用设置参数按钮 | Connect General Settings Parameter Buttons
   * @param window 窗口对象 | Window object
   */
  void connectGeneralSettingsParamButtons(QWidget* window);
  
  /**
   * @brief 连接结果视图参数按钮 | Connect Result View Parameter Buttons
   * @param window 窗口对象 | Window object
   * @param resultType 结果类型 | Result type
   */
  void connectResultViewParamButtons(QWidget* window, const QString& resultType);  

  /* -------------------- 🎛️ 通用窗口管理方法 | General Window Management Methods -------------------- */

  /**
   * @brief 在鼠标附近显示参数窗口 | Show Parameter Window Near Mouse
   * @param window 要显示的窗口 | Window to show
   * @param title 窗口标题 | Window title
   */
  void showParamWindowNearMouse(QWidget* window, const QString& title);
  
  /**
   * @brief 关闭参数窗口 | Close Parameter Window
   * @param window 要关闭的窗口 | Window to close
   */
  void closeParamWindow(QWidget* window);

private:
  /* ==================== UI界面管理 | UI Interface Management ==================== */
  Ui::VisualProcess* ui; // UI指针 | UI pointer
  
  /* ==================== 参数配置窗口管理系统 | Parameter Configuration Window Management System ==================== */
  QHash<QString, QWidget*> m_paramWindows; // 参数窗口管理器 | Parameter windows manager
  QHash<QString, QVariant> m_currentParams; // 当前参数缓存 | Current parameters cache
};

/**
 * @brief 文件结束标记 | End of File Marker
 * 
 * 该头文件定义了视觉处理系统的核心类VisualProcess，包含了：
 * - 任务参数管理结构体
 * - 处理结果管理结构体  
 * - 视觉处理主控制类
 * - 完整的参数配置窗口管理系统
 * - 信号槽机制用于组件间通信
 * 
 * This header file defines the core class VisualProcess of the visual processing system, including:
 * - Task parameter management structures
 * - Processing result management structures
 * - Visual processing main control class
 * - Complete parameter configuration window management system
 * - Signal-slot mechanism for inter-component communication
 */

#endif // VISUALPROCESS_H
