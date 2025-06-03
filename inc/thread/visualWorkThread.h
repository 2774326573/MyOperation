//
// Created by jinxi on 25-5-30.
//

#ifndef VISUALWORKTHREAD_H
#define VISUALWORKTHREAD_H

#include <QObject>
#include <QMutex>
#include <QStringList>
#include <QMap>  // 添加 QMap 头文件

// Halcon相关头文件
#include "../thirdparty/hdevelop/include/halconcpp/HalconCpp.h"

using namespace HalconCpp;

class HalconLable;

/**
 * @brief 显示对象信息结构体
 */
struct DisplayObjectInfo {
    HObject object;     // Halcon对象
    QString color;      // 显示颜色
    double lineWidth;   // 线宽
    
    DisplayObjectInfo() : lineWidth(1.0) {}
    DisplayObjectInfo(const HObject& obj, const QString& col, double width)
        : object(obj), color(col), lineWidth(width) {}
};

/**
 * @brief 视觉处理工作线程类
 * @details 提供基础的Halcon图像处理功能和线程管理
 */
class visualWorkThread : public QObject
{
  Q_OBJECT

public:
  explicit visualWorkThread(QObject* parent = nullptr);
  ~visualWorkThread() override;

  HTuple visual_modelId;  // Halcon模型ID
  HObject visual_Image;  // 当前处理的图像对象
  HObject visual_modelImage;  // 模型图像对象
  HObject visual_modelRegion;  // 模型区域对象
  HTuple visual_Region; // 模型区域参数
  HTuple visual_Row;  // 模型区域参数
  HTuple visual_Column;  // 模型区域参数
  HTuple visual_Angle;  // 模型区域参数
  HTuple visual_Score;  // 模型区域参数

  
  /**
   * @brief 初始化日志系统
   */
  static void initLog();

  /**
   * @brief 检查线程是否正在运行
   * @return true表示正在运行，false表示未运行
   */
  bool isRunning();
  
  /**
   * @brief 设置线程运行状态
   * @param running 运行状态
   */
  void setRunning(bool running);

signals:
  /**
   * @brief 工作线程启动信号
   */
  void started();
  
  /**
   * @brief 工作线程完成信号
   */
  void finished();
  
  /**
   * @brief 错误信号
   * @param error 错误信息
   */
  void error(const QString& error);
  
  /**
   * @brief 图像处理完成信号
   * @param processedImage 处理后的图像对象
   */
  void imageFinished(const HObject& processedImage);
  /**
   * @brief 发送主窗口图像信号
   * @param processedImage 处理后的图像对象
   */
  void sendMainWinddowImage(const HObject& processedImage);  /**
   * @brief 发送主窗口消息
   * @param Msg 消息内容
   */
  void sendMainWinddowMsg(QString Msg);
  
  /**
   * @brief 发送显示对象到主窗口
   * @param displayObject 要显示的Halcon对象
   * @param color 显示颜色
   * @param lineWidth 线宽
   */
  void sendDisplayObject(const HObject& displayObject, const QString& color, double lineWidth);
  
  /**
   * @brief 发送图像和显示对象列表到主窗口（原子操作）
   * @param image 处理后的图像对象
   * @param displayObjects 显示对象列表
   */
  void sendImageWithDisplayObjects(const HObject& image, const QList<DisplayObjectInfo>& displayObjects);
public slots:
  /**
   * @brief 主要工作处理槽函数
   */  void process();
  void onProcessImage(const HObject& processedImage);
    /**
   * @brief 获取文件列表，按文件类型分组（混合策略：shm只返回最新，其他返回全部）
   * @param Path 文件路径
   * @param fileType 文件类型过滤器列表
   * @return 文件类型到文件路径列表的映射（shm类型只包含最新的一个文件，其他类型包含所有文件）
   */
  QMap<QString, QStringList> getCurrentToDayDate(const QString Path, const QStringList& fileType) const;

private:
  /**
   * @brief 初始化Halcon环境
   * @return 初始化是否成功
   */
  bool initHalcon();
  
  /**
   * @brief 初始化路径配置
   */
  void initPath();

  void visualWorkThreadReadImage(const QString& imagePath);

  void processModelParam();

private:
  bool m_running;                    // 线程运行状态
  mutable QMutex m_mutex;           // 线程安全互斥锁
  
  // Halcon基础对象
  HObject m_Image;                  // 当前处理的图像对象
  
  // 工作线程的Halcon处理对象
  HalconLable* workThreadHalcon = nullptr;
  
  // 基础路径配置
  QString HalconPramFilePath = "";  // Halcon参数文件路径
  QString QRcodeReadPath = "";      // 二维码读取路径
  QString MeasureReadPath = "";     // 测量读取路径
  QString CheckReadPath = "";       // 检测读取路径
  QString m_modelReadPath = "";     // 模型读取路径
};

#endif //VISUALWORKTHREAD_H
