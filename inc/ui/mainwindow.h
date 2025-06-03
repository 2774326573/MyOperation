//
// Created by jinxi on 25-5-23.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QEvent>

#include "../inc/thread/visualWorkThread.h"
#include <QToolButton>

class HalconLable;
class VisualProcess;
class SerialDialog;

QT_BEGIN_NAMESPACE

namespace Ui
{
  class Mainwindow;
}

QT_END_NAMESPACE

/**
 * @brief 主窗口类
 * @details 应用程序的主窗口，负责协调各个功能模块
 */
class Mainwindow : public QWidget
{
  Q_OBJECT

  /**
   * @brief 日志级别枚举
   */
  enum Level
  {
    INFO = 0, ///< 信息级别
    WARNING, ///< 警告级别
    ERR, ///< 错误级别
  };

public:
  explicit Mainwindow(QWidget* parent = nullptr);
  ~Mainwindow() override;

private slots:
  /**
   * @brief 视觉配置按钮点击事件
   */
  void on_vision_config_toolBtn_clicked();

  /**
   * @brief 视觉处理按钮点击事件
   */
  void on_vision_process_toolBtn_clicked();

  void on_serialPort_config_toolBtn_clicked();

  /**
   * @brief 开始按钮点击事件
   */
  void on_start_toolBtn_clicked();

  /**
   * @brief 清理按钮点击事件
   */
  void onCleanupButtonClicked();

  /**
   * @brief 视觉处理结果槽函数
   * @param results 处理结果
   */
  void onVisualProcess_results(int results);

  /**
   * @brief 基础的视觉工作线程完成槽函数
   */
  void onWorkThreadFinished();

  /**
   * @brief 基础的视觉工作线程错误槽函数
   * @param error 错误信息
   */
  void onWorkThreadError(const QString& error);

private:
  /**
   * @brief 初始化日志系统
   */
  static void initLog();

  /**
   * @brief 初始化信号和槽连接
   */
  void initConnect();

  /**
   * @brief 初始化视觉处理组件
   */
  void initVisionProcess();

  /**
   * @brief 初始化工作线程
   */
  void initThread();

  /**
   * @brief 应用程序日志信息输出
   * @param message 日志信息
   * @param level 日志级别
   */
  void appLogInfo(const QString& message, Level level = INFO);

  /**
   * @brief 设置工具按钮动画效果
   */
  void setupToolButtonAnimations();

  /**
   * @brief 为单个按钮设置颜色样式
   */
  void setupButtonStyle(QToolButton* button, const QString& normalColor, const QString& hoverColor,
                        const QString& pressedColor);

  /**
   * @brief 保留按钮原始样式但添加悬停效果
   */
  void preserveOriginalButtonStyle(QToolButton* button);

  /**
   * @brief 事件过滤器，用于实现按钮动画效果
   */
  bool eventFilter(QObject* watched, QEvent* event) override;

  /**
   * @brief 调整子窗口大小
   */
  void resizeChildWindows(double scale, double scale_x, double scale_y);

  /**
   * @brief 主窗口大小改变事件处理
   */
  void resizeEvent(QResizeEvent* event) override;

  /**
   * @brief 根据主窗口状态计算最优子窗口尺寸
   * @param preferredWidth 首选宽度
   * @param preferredHeight 首选高度
   * @return 计算后的最优尺寸
   */
  QSize getOptimalChildWindowSize(int preferredWidth, int preferredHeight) const;

  /**
   * @brief 设置文件清理功能
   */
  static void setupFileCleanup();

  /**
   * @brief 清理布局
   * @param layout 布局指针
   */
  void clearDisplayLayout(QLayout* layout);

private:
  // UI状态管理
  bool m_isVisionConfigOpen = false; ///< 视觉处理窗口是否打开
  bool m_isVisionConfig = false; ///< 视觉配置状态
  bool m_isSerialPortConfigOpen = false;
  bool m_isSerialPortConfig = false;

  // UI组件
  Ui::Mainwindow* ui; ///< UI界面对象
  SerialDialog* m_serialDialog = nullptr; ///< 串口配置对话框对象

  // 线程管理
  QThread* m_visualProcessThread = nullptr; ///< 视觉处理线程对象
  visualWorkThread* m_visualWorkThread = nullptr; ///< 视觉工作线程对象

  // 功能组件
  HalconLable* leftHal = nullptr; ///< 左侧图像显示对象
  HalconLable* rightHal = nullptr; ///< 右侧图像显示对象
  VisualProcess* m_visualProcess = nullptr; ///< 视觉处理配置对象
};

#endif //MAINWINDOW_H
