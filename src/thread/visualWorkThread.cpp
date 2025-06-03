//
// Created by jinxi on 25-5-30.
//

#include "../inc/thread/visualWorkThread.h"
#include "../thirdparty/log_manager/inc/simplecategorylogger.h"
#include "../thirdparty/hdevelop/include/HalconLable.h"

#include <QDebug>
#include <QApplication>
#include <QMutexLocker>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QThread>
#include <QMetaType>  // 添加 QMetaType 头文件
#include <QMap>       // 添加 QMap 头文件用于文件分组

#define SYSTEM "VisualWorkThread"

// 日志重定义
// 定义优化版的日志宏，基于编译模式和构建设置自动调整行为
#ifdef _DEBUG // 调试模式
// 调试模式下，日志同时输出到控制台和文件，更详细
#define LOG_DEBUG(message) SIMPLE_DEBUG_LOG_DEBUG(SYSTEM, message)
#define LOG_INFO(message) SIMPLE_DEBUG_LOG_INFO(SYSTEM, message)
#define LOG_WARNING(message) SIMPLE_DEBUG_LOG_WARNING(SYSTEM, message)
#define LOG_ERROR(message) SIMPLE_DEBUG_LOG_ERROR(SYSTEM, message)
#define LOG_ASSERT(condition, message) SIMPLE_ASSERT_CATEGORY((condition), (message), (SYSTEM))
// 添加对TRACE日志的支持
#define LOG_TRACE(message) SIMPLE_LOG_TRACE(SYSTEM, message)

#else // 发布模式
// 其他级别的日志使用配置控制的版本
#define LOG_INFO(message) SIMPLE_LOG_INFO_CONFIG(SYSTEM, message, SHOW_IN_CONSOLE, WRITE_TO_FILE)
#define LOG_WARNING(message) SIMPLE_LOG_WARNING_CONFIG(SYSTEM, message, SHOW_IN_CONSOLE, WRITE_TO_FILE)
#define LOG_ERROR(message) SIMPLE_LOG_ERROR_CONFIG(SYSTEM, message, SHOW_IN_CONSOLE, WRITE_TO_FILE)
#define LOG_ASSERT(condition, message)                                                                                 \
  do                                                                                                                   \
  {                                                                                                                    \
    if (!(condition))                                                                                                  \
    {                                                                                                                  \
      LOG_ERROR("断言失败: " + QString(message));                                                                      \
    }                                                                                                                  \
  }                                                                                                                    \
  while (0)

#endif

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
visualWorkThread::visualWorkThread(QObject* parent) :
  QObject(parent)
  , m_running(false)
{
  // 注册 HObject 类型到 Qt 元对象系统，以支持信号槽跨线程传递
  qRegisterMetaType<HObject>("HObject");

  initLog();
  initHalcon();
  initPath();
}

/**
 * @brief 析构函数
 */
visualWorkThread::~visualWorkThread()
{
  if (workThreadHalcon != nullptr)
  {
    delete workThreadHalcon; // 清理Halcon对象
    workThreadHalcon = nullptr; // 清空指针
  }
}

/**
 * @brief 初始化日志系统
 */
void visualWorkThread::initLog()
{
  auto& logger = SimpleCategoryLogger::instance();
  // 初始化日志系统 | en : Initialize the logging system
  QString logFilePath = QApplication::applicationDirPath() + "/logs/" + SYSTEM;
  logger.initCategory(SYSTEM, logFilePath);
  // 设置日志大小 | en : Set the log file size
  logger.setDefaultMaxLogFileSize(10 * 1024 * 1024); // 10MB
  // 设置历史文件数量 | en : Set the number of historical log files
  logger.setMaxHistoryFileCount(30); // 保留30个历史日志文件
  // 配置调试选项 | en : Configure debug options
  logger.setDebugConfig(true, true, true); // 显示在控制台、写入文件、启用调试日志
  // 设置定期清理日志 | en : Set up periodic log cleanup
  logger.setPeriodicCleanup(true); // 默认7天清理一次, 日志天数超过30天自动清理
}

/**
 * @brief 检查线程是否正在运行
 * @return true表示正在运行，false表示未运行
 */
bool visualWorkThread::isRunning()
{
  QMutexLocker locker(&m_mutex);
  return m_running;
}

/**
 * @brief 设置线程运行状态
 * @param running 运行状态
 */
void visualWorkThread::setRunning(bool running)
{
  QMutexLocker locker(&m_mutex);
  m_running = running;
}

/**
 * @brief 初始化Halcon环境
 * @return 初始化是否成功
 */
bool visualWorkThread::initHalcon()
{
  try
  {
    workThreadHalcon = new HalconLable();
    if (workThreadHalcon == nullptr)
    {
      LOG_ERROR("Halcon对象初始化失败");
      return false;
    }
    else
    {
      LOG_INFO("Halcon对象初始化成功");
      return true;
    }
  }
  catch (const std::exception& e)
  {
    LOG_ERROR(QString("Halcon初始化异常: %1").arg(QString::fromStdString(e.what())));
    return false;
  }
}

/**
 * @brief 初始化路径配置
 */
void visualWorkThread::initPath()
{
  try
  {
    // 初始化基础路径配置
    HalconPramFilePath = QApplication::applicationDirPath() + "/config/halconParams/";

    // 设置各种参数路径
    QRcodeReadPath = HalconPramFilePath + "QRCode";
    MeasureReadPath = HalconPramFilePath + "Measure";
    CheckReadPath = HalconPramFilePath + "Check";
    m_modelReadPath = QApplication::applicationDirPath() + "/config/models/";

    LOG_INFO("路径配置初始化完成");
    LOG_INFO(QString("参数文件路径: %1").arg(HalconPramFilePath));
    LOG_INFO(QString("二维码参数路径: %1").arg(QRcodeReadPath));
    LOG_INFO(QString("测量参数路径: %1").arg(MeasureReadPath));
    LOG_INFO(QString("检测参数路径: %1").arg(CheckReadPath));

  }
  catch (const std::exception& e)
  {
    LOG_ERROR(QString("路径初始化异常: %1").arg(QString::fromStdString(e.what())));
  }
}


/**
 * @brief 主要的图像处理方法
 * @details 这是视觉工作线程的核心处理方法，您可以在此基础上实现具体的图像处理逻辑
 */
void visualWorkThread::process()
{
  LOG_INFO("🚀 开始基础视觉处理任务");

  // 发送开始信号
  emit started();

  try
  {
    // 检查运行状态
    if (!isRunning())
    {
      LOG_WARNING("线程未运行，取消处理");
      emit error("线程未运行");
      return;
    }

    // 检查Halcon对象是否可用
    if (workThreadHalcon == nullptr)
    {
      QString errorMsg = "Halcon对象未初始化";
      LOG_ERROR(errorMsg);
      emit error(errorMsg);
      return;
    }

    QString imagePtah = QApplication::applicationDirPath() + "/img";

    m_modelReadPath += "DetectionModel/";
    LOG_INFO(tr("m_modelReadPath : %1").arg(m_modelReadPath));

    visualWorkThreadReadImage(imagePtah);

  }
  catch (const HalconCpp::HException& e)
  {
    QString errorMsg = QString("视觉处理过程中发生Halcon异常: %1").arg(QString(e.ErrorMessage()));
    LOG_ERROR(errorMsg);
    emit error(errorMsg);
  } catch (const std::exception& e)
  {
    QString errorMsg = QString("视觉处理过程中发生异常: %1").arg(QString::fromStdString(e.what()));
    LOG_ERROR(errorMsg);
    emit error(errorMsg);
  } catch (...)
  {
    QString errorMsg = "视觉处理过程中发生未知异常";
    LOG_ERROR(errorMsg);
    emit error(errorMsg);
  }

  // 发送完成信号
  emit finished();
  LOG_INFO("🏁 基础视觉处理任务结束");
}

// 批量读取图像文件
void visualWorkThread::visualWorkThreadReadImage(const QString& imagePath)
{
  // 🎯 基础图像处理示例
  QDir dir(imagePath);

  if (!dir.exists())
  {
    QString errorMsg = QString("图像文件夹不存在：%1").arg(imagePath);
    LOG_ERROR(errorMsg);
    emit error(errorMsg);
    return;
  }

  // 设置图像文件过滤器
  QStringList filters;
  filters << "*.bmp" << "*.jpg" << "*.jpeg" << "*.png" << "*.tiff" << "*.tif";
  dir.setNameFilters(filters);
  dir.setSorting(QDir::Name);

  QFileInfoList fileList = dir.entryInfoList(QDir::Files);

  if (fileList.isEmpty())
  {
    QString warningMsg = QString("在文件夹 %1 中未找到图像文件").arg(imagePath);
    LOG_WARNING(warningMsg);
    emit error(warningMsg);
    return;
  }

  LOG_INFO(QString("📁 找到 %1 个图像文件").arg(fileList.size()));

  // 🎯 简单的图像处理循环示例
  for (int i = 0; i < fileList.size() && isRunning(); ++i)
  {
    const QFileInfo& fileInfo = fileList[i];
    QString filePath = fileInfo.absoluteFilePath();
    QString fileName = fileInfo.fileName();

    try
    {
      LOG_INFO(QString("🖼️ 正在处理图像 [%1/%2]: %3")
          .arg(i + 1).arg(fileList.size()).arg(fileName));

      // 读取图像
      HObject image;
      ReadImage(&image, filePath.toStdString().c_str());

      // 🎯 在这里添加您的具体图像处理逻辑
      // 例如：图像增强、特征提取、模式识别等

      if (image.IsInitialized() == false)
      {
        QString errorMsg = QString("图像 %1 读取失败").arg(fileName);
        LOG_ERROR(errorMsg);
        emit error(errorMsg);
        continue; // 跳过当前图像，继续处理下一个
      }
      LOG_INFO(QString("✅ 图像处理完成：%1").arg(fileName));
      connect(this, &visualWorkThread::imageFinished, this, &visualWorkThread::onProcessImage);
      emit imageFinished(image);
    }
    catch (const HalconCpp::HException& e)
    {
      QString errorMsg = QString("处理图像 %1 时发生Halcon异常: %2")
                         .arg(fileName).arg(QString(e.ErrorMessage()));
      LOG_ERROR(errorMsg);
      // 继续处理下一张图像，而不是中断整个处理流程
      continue;

    } catch (const std::exception& e)
    {
      QString errorMsg = QString("处理图像 %1 时发生异常: %2")
                         .arg(fileName).arg(QString::fromStdString(e.what()));
      LOG_ERROR(errorMsg);
      continue;
    }
  }
  LOG_INFO(QString("🎉 图像处理任务完成，共处理 %1 个文件").arg(fileList.size()));
}

void visualWorkThread::onProcessImage(const HObject& processedImage)
{
  if (m_modelReadPath.isEmpty())
  {
    LOG_ERROR("模型读取路径未设置，无法处理图像");
    return;
  }
  LOG_INFO(tr("m_modelReadPath : %1").arg(m_modelReadPath));
  if (processedImage.IsInitialized() == false)
  {
    LOG_WARNING("图像初始化失败，无法处理图像");
    return;
  }
  const HObject image = processedImage;
  processModelParam(); // 处理模型参数
  
  // 检查模板是否已正确加载
  if (visual_modelId.Length() == 0)
  {
    LOG_ERROR("❌ 模板未正确加载，无法进行匹配");
    return;
  }
  
  if (image.IsInitialized() == true)
  {
    HTuple Crow, Ccol, Cangle, Cscore, AffHomMat2D;
    
    LOG_INFO("🔍 开始进行模板匹配...");
    LOG_INFO(QString("📊 模板参数: Row=%1, Column=%2, Angle=%3")
        .arg(visual_Row.D()).arg(visual_Column.D()).arg(visual_Angle.D()));
    
    // 检查图像和模板的基本信息
    HTuple ImageWidth, ImageHeight, ImageChannels;
    GetImageSize(image, &ImageWidth, &ImageHeight);
    CountChannels(image, &ImageChannels);
    LOG_INFO(QString("📷 图像信息: 宽度=%1, 高度=%2, 通道数=%3").arg(ImageWidth[0].I()).arg(ImageHeight[0].I()).arg(ImageChannels[0].I()));
    
    // 预处理图像以提高匹配成功率
    HObject processedImage = image;
    try 
    {
      // 如果是彩色图像，转换为灰度图像
      if (ImageChannels[0].I() > 1) 
      {
        LOG_INFO("🔄 转换彩色图像为灰度图像...");
        Rgb1ToGray(image, &processedImage);
      }
      
      // 可选：增强对比度
      // HObject enhancedImage;
      // Emphasize(processedImage, &enhancedImage, 7, 7, 1.0);
      // processedImage = enhancedImage;
    }
    catch (const HalconCpp::HException& except)
    {
      LOG_WARNING(QString("⚠️ 图像预处理失败，使用原图像: %1").arg(except.ErrorMessage().Text()));
      processedImage = image;
    }
    
    try
    {
      // 查找模板 - 使用更保守的参数避免挂起
      LOG_INFO("🎯 正在执行 FindShapeModel...");
      
      // 首先尝试快速匹配（高greediness，低精度）
      LOG_INFO("⚡ 尝试快速匹配模式...");
      FindShapeModel(processedImage, visual_modelId, 
                     -0.39, 0.78,    // 角度范围: ±22.5度
                     0.3,            // 最小分数 (降低要求)
                     1,              // 最大匹配数
                     0.5,            // 最大重叠
                     "least_squares", // 子像素精度
                     3,              // 金字塔层数 (减少层数加快速度)
                     0.9,            // 贪婪度 (提高速度)
                     &Crow, &Ccol, &Cangle, &Cscore);
      
      LOG_INFO(QString("✅ FindShapeModel 执行完成，找到 %1 个匹配").arg(Crow.Length()));
      
      // 检查是否找到模板
      if (Crow.Length() == 0)
      {
        LOG_WARNING("⚠️ 快速匹配未找到结果，尝试精确匹配模式...");
        
        // 如果快速匹配失败，尝试更精确的匹配
        try 
        {
          FindShapeModel(processedImage, visual_modelId, 
                         -0.79, 1.57,    // 更大角度范围: ±45度 到 90度
                         0.2,            // 更低分数阈值
                         3,              // 更多匹配候选
                         0.7,            // 允许更多重叠
                         "least_squares",
                         4,              // 增加金字塔层数
                         0.7,            // 降低贪婪度获得更好精度
                         &Crow, &Ccol, &Cangle, &Cscore);
          
          LOG_INFO(QString("🔍 精确匹配完成，找到 %1 个匹配").arg(Crow.Length()));
        }
        catch (const HalconCpp::HException& except)
        {
          LOG_ERROR(QString("❌ 精确匹配也失败: %1").arg(except.ErrorMessage().Text()));
        }
      }
      
      if (Crow.Length() == 0)
      {
        LOG_WARNING("❌ 未找到匹配的模板");
        // 创建空的显示对象列表
        QList<DisplayObjectInfo> emptyDisplayObjects;
        emit sendImageWithDisplayObjects(image, emptyDisplayObjects);
        return;
      }
      
      LOG_INFO(QString("✅ 找到模板匹配: Row=%1, Col=%2, Angle=%3, Score=%4")
          .arg(Crow[0].D()).arg(Ccol[0].D()).arg(Cangle[0].D()).arg(Cscore[0].D()));
      
      // 计算仿射变换矩阵
      LOG_INFO("🔄 正在计算仿射变换矩阵...");
      VectorAngleToRigid(visual_Row, visual_Column, visual_Angle, Crow[0], Ccol[0], Cangle[0], &AffHomMat2D);
      
      LOG_INFO("✅ 仿射变换矩阵计算成功");

      // 读取原始测量区域
      HObject Measyre_Rect1, Measyre_Rect2;
      QString Measyre_Rect1_Path = MeasureReadPath + "/m_Measyre_Rect1.hobj";
      QString Measyre_Rect2_Path = MeasureReadPath + "/m_Measyre_Rect2.hobj";
      
      try
      {
        ReadRegion(&Measyre_Rect1, Measyre_Rect1_Path.toLatin1().data());
        ReadRegion(&Measyre_Rect2, Measyre_Rect2_Path.toLatin1().data());
        LOG_INFO("✅ 成功读取原始测量区域");
      }
      catch (const HalconCpp::HException& e)
      {
        LOG_ERROR(QString("读取测量区域文件失败: %1").arg(QString(e.ErrorMessage())));
        return;
      }

      // 🎯 将测量区域通过仿射变换映射到找到的模板位置
      HObject TransformedRect1, TransformedRect2;
      AffineTransRegion(Measyre_Rect1, &TransformedRect1, AffHomMat2D, "nearest_neighbor");
      AffineTransRegion(Measyre_Rect2, &TransformedRect2, AffHomMat2D, "nearest_neighbor");
      
      LOG_INFO("🎯 测量区域已成功映射到模板位置");

      // 🎯 创建显示对象列表
      QList<DisplayObjectInfo> displayObjects;
      
      // 添加变换后的区域（绿色显示）
      displayObjects.append(DisplayObjectInfo(TransformedRect1, "green", 2.0));
      displayObjects.append(DisplayObjectInfo(TransformedRect2, "green", 2.0));

      // 在变换后的区域中提取轮廓
      HObject Xld1 = workThreadHalcon->QtGetLengthMaxXld(image, TransformedRect1, 100);

      // 检查第一个轮廓是否提取成功
      if (!Xld1.IsInitialized())
      {
        LOG_INFO(tr("❌ 警告：第一个映射区域没有找到有效轮廓"));
      }
      else
      {
        LOG_INFO(tr("✅ 第一个映射区域轮廓提取成功"));
        // 🎨 添加轮廓到显示对象列表（红色显示）
        displayObjects.append(DisplayObjectInfo(Xld1, "red", 3.0));
      }

      LOG_INFO(tr("正在提取第二个映射区域的轮廓..."));
      HObject Xld2 = workThreadHalcon->QtGetLengthMaxXld(image, TransformedRect2, 100);

      // 检查第二个轮廓是否提取成功
      if (!Xld2.IsInitialized())
      {
        LOG_INFO(tr("❌ 警告：第二个映射区域没有找到有效轮廓"));
      }
      else
      {
        LOG_INFO(tr("✅ 第二个映射区域轮廓提取成功"));
        // 🎨 添加轮廓到显示对象列表（蓝色显示）
        displayObjects.append(DisplayObjectInfo(Xld2, "blue", 3.0));
      }

      // 只有两个轮廓都成功提取才进行距离计算
      if (Xld1.IsInitialized() && Xld2.IsInitialized())
      {
        LOG_INFO(tr("正在计算两个轮廓之间的距离..."));

        try
        {
          HTuple DisMin, DisMax;
          DistanceCc(Xld1, Xld2, "point_to_point", &DisMin, &DisMax); // 计算两点之间的距离

          // 🎯 使用新的测量分析功能获取额外信息
          double area1 = workThreadHalcon->calculateRegionArea(TransformedRect1);
          double area2 = workThreadHalcon->calculateRegionArea(TransformedRect2);
          pointStruct centroid1 = workThreadHalcon->calculateRegionCentroid(TransformedRect1);
          pointStruct centroid2 = workThreadHalcon->calculateRegionCentroid(TransformedRect2);

          // 计算重心之间的距离
          double centroidDistance = workThreadHalcon->calculatePointDistance(
              centroid1.X, centroid1.Y, centroid2.X, centroid2.Y);

          QString msg = QString("🎯 映射区域测量结果:\n最小距离: %1px\n最大距离: %2px\n重心距离: %3px\n区域1面积: %4px²\n区域2面积: %5px²\n模板匹配得分: %6")
                        .arg(QString::number(DisMin.D(), 'f', 2))
                        .arg(QString::number(DisMax.D(), 'f', 2))
                        .arg(QString::number(centroidDistance, 'f', 2))
                        .arg(QString::number(area1, 'f', 1))
                        .arg(QString::number(area2, 'f', 1))
                        .arg(QString::number(Cscore[0].D(), 'f', 3));

          // 发送图像和所有显示对象（原子操作）
          emit sendImageWithDisplayObjects(image, displayObjects);
          emit sendMainWinddowMsg(msg);

          // 🎯 可选：导出测量结果到缓存
          QMap<QString, QVariant> measurementResults;
          measurementResults["最小距离"] = DisMin.D();
          measurementResults["最大距离"] = DisMax.D();
          measurementResults["重心距离"] = centroidDistance;
          measurementResults["区域1面积"] = area1;
          measurementResults["区域2面积"] = area2;
          measurementResults["重心1_X"] = centroid1.X;
          measurementResults["重心1_Y"] = centroid1.Y;
          measurementResults["重心2_X"] = centroid2.X;
          measurementResults["重心2_Y"] = centroid2.Y;
          measurementResults["模板匹配得分"] = Cscore[0].D();
          measurementResults["模板位置_Row"] = Crow[0].D();
          measurementResults["模板位置_Col"] = Ccol[0].D();
          measurementResults["模板角度"] = Cangle[0].D();

          // 保存到HalconLable的缓存中
          for (auto it = measurementResults.begin(); it != measurementResults.end(); ++it)
          {
            workThreadHalcon->measurementCache[it.key()] = it.value();
          }

          LOG_INFO(tr("📊 测量结果已保存到缓存，共 %1 项数据").arg(measurementResults.size()));

        }
        catch (HalconCpp::HException& e)
        {
          QString errorMsg = tr("❌ 距离计算失败：%1").arg(QString(e.ErrorMessage()));
          LOG_INFO(errorMsg);
        } catch (...)
        {
          LOG_INFO(tr("❌ 距离计算时发生未知错误"));
        }
      }
      else
      {
        LOG_INFO(tr("❌ 无法进行距离测量：轮廓提取失败"));
        // 即使轮廓提取失败，也显示区域
        emit sendImageWithDisplayObjects(image, displayObjects);
      }
    }
    catch (const HalconCpp::HException& e)
    {
      QString errorMsg = tr("❌ 模板匹配或变换失败：%1").arg(QString(e.ErrorMessage()));
      LOG_ERROR(errorMsg);
      // 创建空的显示对象列表
      QList<DisplayObjectInfo> emptyDisplayObjects;
      emit sendImageWithDisplayObjects(image, emptyDisplayObjects);
    }
  }
  else
  {
    LOG_INFO(tr("❌ 图像未初始化，无法进行处理"));
  }
}

// 处理模型参数 - 读取模板文件和参数文件
void visualWorkThread::processModelParam()
{
  // 获取指定目录 , 文件类型， 模板路径， 模版参数
  QStringList Type;
  QString modelPath = "";
  QString modelHTuplePath = "";

  // 🎯 支持多种文件扩展名 - 您可以根据需要添加或删除
  Type << "*.shm" << "*.tup";

  LOG_INFO(QString("📋 设置文件类型过滤器: %1").arg(Type.join(", ")));

  // 现在返回的是按文件类型分组的映射
  QMap<QString, QStringList> fileTypeGroups = getCurrentToDayDate(m_modelReadPath, Type);

  LOG_INFO(QString("🔍 共找到 %1 种文件类型").arg(fileTypeGroups.size()));

  // 分别处理每种文件类型
  for (auto it = fileTypeGroups.begin(); it != fileTypeGroups.end(); ++it)
  {
    QString extension = it.key();
    QStringList files = it.value();

    LOG_INFO(QString("📁 处理 %1 文件类型，共 %2 个文件:").arg(extension.toUpper()).arg(files.size()));

    for (int i = 0; i < files.size(); ++i)
    {
      LOG_INFO(QString("  [%1] %2").arg(i + 1).arg(QFileInfo(files[i]).fileName()));
    }

    // 根据文件扩展名执行不同的处理逻辑
    if (extension == "shm")
    {
      LOG_INFO(QString("🎯 专门处理 %1 个 .shm 模型文件").arg(files.size()));
      if (!files.isEmpty())
      {
        modelPath = files[0]; // 只取最新的一个.shm文件
        LOG_INFO("modelPath : " + modelPath);
        
        try
        {
          // 读取模板文件
          ReadShapeModel(modelPath.toLatin1().data(), &visual_modelId);
          LOG_INFO(QString("✅ 成功读取模板文件: %1, ModelId: %2")
              .arg(QFileInfo(modelPath).fileName())
              .arg(visual_modelId.ToString().Text()));
        }
        catch (const HalconCpp::HException& e)
        {
          LOG_ERROR(QString("读取模板文件 %1 时发生Halcon异常: %2")
              .arg(QFileInfo(modelPath).fileName()).arg(QString(e.ErrorMessage())));
        }
      }
    }
  }

  if (fileTypeGroups.contains("tup"))
  {
    QStringList tupFiles = fileTypeGroups["tup"];
    LOG_INFO(QString("💡 可以直接访问 .tup 文件列表，共 %1 个文件").arg(tupFiles.size()));

    // 过滤出文件名以"data"结尾的.tup文件
    QStringList dataTupFiles;
    for (const QString& filePath : tupFiles)
    {
      QFileInfo fileInfo(filePath);
      QString baseName = fileInfo.baseName(); // 获取不带扩展名的文件名
      if (baseName.endsWith("data", Qt::CaseInsensitive))
      {
        dataTupFiles.append(filePath);
        LOG_INFO(QString("✅ 找到符合条件的文件: %1").arg(fileInfo.fileName()));
        modelHTuplePath = filePath;
      }
    }

    if (!dataTupFiles.isEmpty())
    {
      LOG_INFO(QString("🎯 找到 %1 个文件名以'data'结尾的.tup文件").arg(dataTupFiles.size()));

      // 对符合条件的.tup文件进行特定处理
      for (const QString& dataFilePath : dataTupFiles)
      {
        QFileInfo fileInfo(dataFilePath);
        LOG_INFO(QString("🔧 正在处理data类型的.tup文件: %1").arg(fileInfo.fileName()));

        try
        {
          // 读取模板参数文件
          HTuple DataTuple;
          ReadTuple(dataFilePath.toLatin1().data(), &DataTuple);
          LOG_INFO(QString("✅ 成功读取data文件: %1，数据长度: %2")
              .arg(fileInfo.fileName()).arg(DataTuple.Length()));
          
          // 验证数据长度并解析参数
          if (DataTuple.Length() >= 3)
          {
            visual_Row = DataTuple[0];
            visual_Column = DataTuple[1]; 
            visual_Angle = DataTuple[2];
            
            LOG_INFO(QString("📊 模板参数解析成功: Row=%1, Column=%2, Angle=%3")
                .arg(visual_Row.D()).arg(visual_Column.D()).arg(visual_Angle.D()));
          }
          else
          {
            LOG_ERROR(QString("❌ 模板参数文件数据不足: 期望>=3，实际=%1").arg(DataTuple.Length()));
            // 设置默认值
            visual_Row = 0;
            visual_Column = 0;
            visual_Angle = 0;
          }
        }
        catch (const HalconCpp::HException& e)
        {
          LOG_ERROR(QString("读取data文件 %1 时发生Halcon异常: %2")
              .arg(fileInfo.fileName()).arg(QString(e.ErrorMessage())));
          // 设置默认值
          visual_Row = 0;
          visual_Column = 0;
          visual_Angle = 0;
        }
      }
    }
    else
    {
      LOG_WARNING("未找到文件名以'data'结尾的.tup文件，使用默认参数");
      // 设置默认值
      visual_Row = 0;
      visual_Column = 0;
      visual_Angle = 0;
    }
  }
}

// 获取文件列表 - 按文件类型分组返回（混合策略：shm只返回最新，其他返回全部）
QMap<QString, QStringList> visualWorkThread::getCurrentToDayDate(const QString Path, const QStringList& fileType) const
{
  // 获取指定目录
  QString dirPath = Path;
  QDir dir(dirPath);

  // 用于按文件类型分组存储 - 这将是返回值
  QMap<QString, QStringList> fileTypeGroups;

  // 跟踪.shm文件类型的最新文件和修改时间
  QMap<QString, QPair<QString, QDateTime>> latestShmFileForType;

  if (dir.exists())
  {
    for (const QString& type : fileType)
    {
      // 设置文件类型过滤器
      QStringList filters;
      filters << type;
      dir.setNameFilters(filters);
      dir.setSorting(QDir::Time); // 按时间排序

      // 获取过滤后的文件列表
      QFileInfoList allFiles = dir.entryInfoList(QDir::Files);

      LOG_INFO(QString("文件类型 %1 过滤后找到 %2 个文件").arg(type).arg(allFiles.size()));

      // 处理目录中的所有文件
      for (const QFileInfo& fileInfo : allFiles)
      {
        QDateTime fileDateTime = fileInfo.lastModified(); // 获取文件的最后修改时间
        QString filePath = fileInfo.absoluteFilePath();
        QString fileExtension = fileInfo.suffix().toLower();

        LOG_INFO(QString("检查文件: %1 (扩展名: %2, 修改时间: %3)")
            .arg(fileInfo.fileName())
            .arg(fileExtension)
            .arg(fileDateTime.toString("yyyy-MM-dd hh:mm:ss")));

        // 根据文件类型采用不同策略
        if (fileExtension == "shm")
        {
          // 对于.shm文件，只保留最新的一个
          if (!latestShmFileForType.contains(fileExtension) ||
            fileDateTime > latestShmFileForType[fileExtension].second)
          {
            latestShmFileForType[fileExtension] = qMakePair(filePath, fileDateTime);
            LOG_INFO(QString("🔄 更新最新.shm文件: %1 -> %2")
                .arg(fileExtension.toUpper())
                .arg(fileInfo.fileName()));
          }
        }
        else
        {
          // 对于其他文件类型（如.tup），添加所有文件
          fileTypeGroups[fileExtension].append(filePath);
          LOG_INFO(QString("➕ 添加文件到 %1 组: %2")
              .arg(fileExtension.toUpper())
              .arg(fileInfo.fileName()));
        }
      }
    }

    // 将.shm文件的最新文件添加到结果中
    for (auto it = latestShmFileForType.begin(); it != latestShmFileForType.end(); ++it)
    {
      QString extension = it.key();
      QString filePath = it.value().first;
      QDateTime lastModified = it.value().second;

      fileTypeGroups[extension].append(filePath);

      QFileInfo fileInfo(filePath);
      LOG_INFO(QString("✅ 选定最新.shm文件: %1 (类型: %2, 修改时间: %3)")
          .arg(fileInfo.fileName())
          .arg(extension.toUpper())
          .arg(lastModified.toString("yyyy-MM-dd hh:mm:ss")));
    }

    // 输出按文件类型分组的结果（混合策略：shm最新文件，其他全部文件）
    LOG_INFO("=== 文件类型分组结果（混合策略） ===");
    LOG_INFO(QString("🎯 共处理 %1 种文件类型").arg(fileType.size()));

    for (auto it = fileTypeGroups.begin(); it != fileTypeGroups.end(); ++it)
    {
      QString extension = it.key();
      QStringList files = it.value();

      if (extension == "shm")
      {
        LOG_INFO(QString("📁 %1 文件类型 (最新文件): %2 个").arg(extension.toUpper()).arg(files.size()));
      }
      else
      {
        LOG_INFO(QString("📁 %1 文件类型 (全部文件): %2 个").arg(extension.toUpper()).arg(files.size()));
      }

      for (const QString& filePath : files)
      {
        QFileInfo info(filePath);
        QDateTime lastModified = info.lastModified();
        LOG_INFO(QString("  📄 %1 (修改时间: %2)")
            .arg(info.fileName())
            .arg(lastModified.toString("yyyy-MM-dd hh:mm:ss")));
      }

      // 根据不同文件扩展名执行不同的处理逻辑
      if (extension == "shm")
      {
        LOG_INFO(QString("🎯 开始处理最新的 .shm 模型文件"));
        for (const QString& filePath : files)
        {
          QFileInfo fileInfo(filePath);
          HTuple ModelId;
          LOG_INFO(QString("正在读取最新的 .shm 模型文件: %1").arg(fileInfo.fileName()));

          try
          {
            // 读取模型参数
            ReadShapeModel(filePath.toLatin1().data(), &ModelId);
            if (ModelId.Length() == 0)
            {
              LOG_ERROR(QString("模型 %1 读取失败: ModelId为空").arg(fileInfo.fileName()));
              continue;
            }
            LOG_INFO(QString("✅ 最新模型 %1 读取成功，ModelId: %2").arg(fileInfo.fileName()).arg(ModelId.ToString().Text()));
          }
          catch (const HalconCpp::HException& e)
          {
            LOG_ERROR(QString("读取最新模型 %1 时发生Halcon异常: %2").arg(fileInfo.fileName()).arg(QString(e.ErrorMessage())));
            continue;
          }
        }
      }
      else
      {
        LOG_INFO(QString("ℹ️  %1 文件类型暂不执行特定处理逻辑").arg(extension.toUpper()));
      }
    }
    return fileTypeGroups; // 返回分组的文件映射（混合策略：shm最新，其他全部）
  }
  else
  {
    LOG_WARNING(QString("目录不存在: %1").arg(dirPath));
  }
  return QMap<QString, QStringList>(); // 返回空映射表示目录不存在
}
