//
// Created by jinxi on 25-5-23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_Mainwindow.h" resolved

#include "../inc/ui/mainwindow.h"
#include "../thirdparty/log_manager/inc/simplecategorylogger.h"
#include "../thirdparty/hdevelop/include/HalconLable.h"
#include "../inc/ui/VisualProcess.h"
#include "../inc/ui/serialdialog.h"

#include <QWidget>
#include <QMessageBox>
#include <QSettings>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QDateTime>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QInputDialog>
#include <QMetaMethod>
#include <QMetaType>

#include "ui_Mainwindow.h"

#define APPNAME "test"
#define SYSTEM "System"
#define APPLICATION "Application"

// 日志重定义
// 定义优化版的日志宏，基于编译模式和构建设置自动调整行为
#ifdef _DEBUG // 调试模式
// 调试模式下，日志同时输出到控制台和文件，更详细
#define LOG_DEBUG(category, message) SIMPLE_DEBUG_LOG_DEBUG(category, message)
#define LOG_INFO(category, message) SIMPLE_DEBUG_LOG_INFO(category, message)
#define LOG_WARNING(category, message) SIMPLE_DEBUG_LOG_WARNING(category, message)
#define LOG_ERROR(category, message) SIMPLE_DEBUG_LOG_ERROR(category, message)
#define LOG_ASSERT(condition, message) SIMPLE_ASSERT((condition), (message))
// 添加对TRACE日志的支持
#define LOG_TRACE(category, message) SIMPLE_LOG_TRACE(category, message)

#else // 发布模式
// 发布模式下，根据CMake配置决定日志行为

// 其他级别的日志使用配置控制的版本
#define LOG_INFO(category, message) SIMPLE_LOG_INFO_CONFIG(category, message, false, WRITE_TO_FILE)
#define LOG_WARNING(category, message) SIMPLE_LOG_WARNING_CONFIG(category, message, false, WRITE_TO_FILE)
#define LOG_ERROR(category, message) SIMPLE_LOG_ERROR_CONFIG(category, message, false, WRITE_TO_FILE)
#define LOG_ASSERT(condition, message, category)                                                                       \
  do                                                                                                                   \
  {                                                                                                                    \
    if (!(condition))                                                                                                  \
    {                                                                                                                  \
      LOG_ERROR(category, "断言失败: " + QString(message));                                                            \
    }                                                                                                                  \
  }                                                                                                                    \
  while (0)

#endif


Mainwindow::Mainwindow(QWidget* parent) :
  QWidget(parent), ui(new Ui::Mainwindow)
{
  ui->setupUi(this);
  // 初始化各个组件
  initLog(); // 先初始化日志系统
  initVisionProcess(); // 初始化视觉处理
  // 设置文件清理功能
  setupFileCleanup();
  // 设置工具按钮动画效果
  setupToolButtonAnimations();
  // 初始化线程
  initThread();
  // 初始化信号和槽连接
  initConnect();

  LOG_INFO(SYSTEM, "应用程序初始化完成");
}

Mainwindow::~Mainwindow()
{
  // 清理视觉工作线程
  if (m_visualProcessThread != nullptr)
  {
    m_visualWorkThread->setRunning(false); // 停止工作线程
    m_visualProcessThread->quit(); // 请求线程退出
    m_visualProcessThread->wait(); // 等待线程结束
    delete m_visualProcessThread; // 删除线程对象
    m_visualProcessThread = nullptr; // 清空指针
    LOG_INFO(SYSTEM, "视觉工作线程已停止并清理完成");
  }
  delete ui;
}

void Mainwindow::initLog()
{
  // 初始化SimpleCategoryLogger系统
#ifdef _DEBUG // 调试模式
  // 确保使用绝对路径而不是相对路径
  QString logPath = QDir::currentPath() + "/logs/"; // 日志文件存放的目录

  // 创建目录结构 - 调试日志目录
  QString debug_log_path = logPath + "debug_log"; // 日志文件路径
  QDir debug_log_Dir(debug_log_path);
  // 初始化调试模式下的日志系统
  bool sysInitOk = SIMPLE_LOG_INIT(SYSTEM, debug_log_path);
  bool appInitOk = SIMPLE_LOG_INIT(APPLICATION, debug_log_path);
  if (sysInitOk && appInitOk)
  {
    SIMPLE_DEBUG_LOG_INFO(SYSTEM, "SimpleCategoryLogger日志系统初始化完成");
  }
  else
  {
    LOG_ERROR(SYSTEM, QString("日志系统初始化失败: 系统: %1 应用: %2").arg(sysInitOk).arg(appInitOk));
  }
  SIMPLE_LOG_INFO(SYSTEM, "SimpleCategoryLogger日志系统初始化完成");
  QString currentTime = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
  QString logFileName = QString("debug_log_%1.txt").arg(currentTime);

  // 检查目录是否存在
  if (!debug_log_Dir.exists())
  {
    LOG_DEBUG(SYSTEM, "调试日志目录不存在，正在创建...");
    if (debug_log_Dir.mkpath("."))
    {
      LOG_DEBUG(SYSTEM, "成功创建调试日志目录");
    }
    else
    {
      LOG_DEBUG(SYSTEM, "创建调试日志目录失败");
      // 尝试使用备用路径
      debug_log_path = logPath;
      LOG_DEBUG(SYSTEM, "使用备用调试日志路径: " + debug_log_path);
    }
  } // 设置日志文件路径
  QString fullLogPath = debug_log_path + "/" + logFileName;
  LOG_DEBUG(SYSTEM, "设置调试日志文件路径: " + fullLogPath);

#else
  // 使用当前目录作为日志根目录，避免权限问题
  QString logBaseDir = QDir::currentPath() + "/logs";
  // 创建日志目录
  QDir logDir(logBaseDir);
  if (!logDir.exists())
  {
    bool created = logDir.mkpath(".");
    LOG_INFO(SYSTEM, QString("创建日志根目录: %1 %2").arg(logBaseDir).arg(created ? "成功" : "失败"));
  }

  // 初始化分类
  bool sysInitOk = SIMPLE_LOG_INIT(SYSTEM, logBaseDir);
  bool appInitOk = SIMPLE_LOG_INIT(APPLICATION, logBaseDir);
  LOG_INFO(SYSTEM, QString("SimpleCategoryLogger初始化结果 - 系统: %1 应用: %2").arg(sysInitOk).arg(appInitOk));

  if (sysInitOk && appInitOk)
  {
    LOG_INFO(SYSTEM, "分类日志系统初始化成功，路径: " + logBaseDir);

    // 现在可以安全地使用日志系统了
  }
#endif

  LOG_INFO(SYSTEM, "日志系统初始化完成");
}

void Mainwindow::initConnect()
{
  // 基础的工作线程信号连接
  if (m_visualWorkThread)
  {
    connect(m_visualWorkThread, &visualWorkThread::finished,
            this, &Mainwindow::onWorkThreadFinished);
    connect(m_visualWorkThread, &visualWorkThread::error,
            this, &Mainwindow::onWorkThreadError); // 如果信号未连接，则建立连接
    LOG_INFO(SYSTEM, "建立图像显示信号连接");
    connect(m_visualWorkThread, &visualWorkThread::sendMainWinddowMsg, this, [this](QString Msg)
    {
      rightHal->dispHalconMessage(20, 20, Msg, "green");
    });

    // 连接图像和显示对象组合信号（原子操作）
    connect(m_visualWorkThread, &visualWorkThread::sendImageWithDisplayObjects, this,
            [this](const HObject& image, const QList<DisplayObjectInfo>& displayObjects)
            {
              rightHal->clearDisplayObjectsOnly();
              LOG_INFO(SYSTEM, QString("收到图像和 %1 个显示对象").arg(displayObjects.size()));

              // 首先显示图像
              if (image.IsInitialized())
              {
                rightHal->showImage(image);
                LOG_INFO(SYSTEM, "图像已显示在右侧Halcon标签");
              }
              else
              {
                LOG_WARNING(SYSTEM, "收到未初始化的图像，无法显示");
              } // 然后添加所有显示对象
              for (const auto& dispObj : displayObjects)
              {
                if (dispObj.object.IsInitialized())
                {
                  rightHal->addDisplayObject(dispObj.object, dispObj.color, dispObj.lineWidth);
                  LOG_DEBUG(SYSTEM, QString("添加显示对象: 颜色=%1, 线宽=%2").arg(dispObj.color).arg(dispObj.lineWidth));
                }
              }

              LOG_INFO(SYSTEM, QString("成功处理图像和 %1 个显示对象").arg(displayObjects.size()));
            });

    LOG_INFO(SYSTEM, "视觉工作线程基础信号连接完成");
  }
}

void Mainwindow::setupFileCleanup()
{
  LOG_INFO(SYSTEM, "初始化文件清理功能");

  // 设置默认的清理过滤器
  QStringList filters;
  filters << "*.log" << "*.txt" << "*.tmp";
  SIMPLE_LOGGER.setCleanupFilters(filters);

  // 配置定期清理 - 默认每7天清理一次，保留30天内的文件
  SIMPLE_LOGGER.setPeriodicCleanup(true, 7, 30);

  LOG_INFO(SYSTEM, "文件清理功能已启用，间隔: 7天，保留期: 30天");
}

void Mainwindow::onCleanupButtonClicked()
{
  LOG_INFO(SYSTEM, "触发手动文件清理");

  // 对话框获取清理天数
  bool ok;
  int days = QInputDialog::getInt(this, tr("文件清理"), tr("请输入要保留的最大天数:"), 30, 1, 365, 1, &ok);
  if (!ok)
  {
    LOG_INFO(SYSTEM, "用户取消了文件清理");
    return;
  }

  // 执行清理
  int count = SIMPLE_LOGGER.cleanupFiles(QString(), days);

  // 显示结果
  QMessageBox::information(this, tr("清理完成"), tr("已清理 %1 个过期文件。").arg(count));

  LOG_INFO(SYSTEM, QString("手动清理完成，删除了 %1 个文件").arg(count));
}

void Mainwindow::onVisualProcess_results(int results)
{
  ui->le_OKNum->setText(QString::number(results));
}

/********************************************* 事件管理器 ****************************************************/

void Mainwindow::clearDisplayLayout(QLayout* layout)
{
  if (!layout)
    return;
  QLayoutItem* item;
  while ((item = layout->takeAt(0)))
  {
    if (item->layout())
    {
      clearDisplayLayout(item->layout());
    }
    else if (item->widget())
    {
      delete item->widget();
    }
    delete item;
  }
}

/**
 * @brief Mainwindow::initVisionProcess 初始化视觉处理对象
 * 该函数创建视觉处理对象，并将其添加到主窗口的布局中。
 */
void Mainwindow::initVisionProcess()
{
  try
  {
    leftHal = new HalconLable();
    rightHal = new HalconLable();

    if (!leftHal && !rightHal)
    {
      LOG_ERROR(SYSTEM, "图像处理对象初始化失败");
      return;
    }
    else
    {
      LOG_INFO(SYSTEM, "图像处理对象初始化成功");
    }
    // 初始化图像处理对象
    clearDisplayLayout(ui->mainDisply_gridLayout);
    ui->mainDisply_gridLayout->addWidget(leftHal, 0, 0);
    ui->mainDisply_gridLayout->addWidget(rightHal, 0, 1);
  }
  catch (...)
  {
    LOG_ERROR(SYSTEM, "initVisionProcess初始化失败");
  }
}

void Mainwindow::initThread()
{
  // 注册自定义类型，用于信号槽传递
  qRegisterMetaType<DisplayObjectInfo>("DisplayObjectInfo");
  qRegisterMetaType<QList<DisplayObjectInfo>>("QList<DisplayObjectInfo>");

  if (m_visualWorkThread == nullptr)
  {
    // 创建新的线程对象
    m_visualProcessThread = new QThread();
    // 创建视觉工作线程对象
    m_visualWorkThread = new visualWorkThread();
    // 将视觉工作线程移动到新线程中
    m_visualWorkThread->moveToThread(m_visualProcessThread);

    // 设置线程运行状态
    m_visualWorkThread->setRunning(true);
    // 启动线程
    m_visualProcessThread->start();

    LOG_INFO(SYSTEM, "视觉工作线程初始化并启动完成");
  }
}

void Mainwindow::appLogInfo(const QString& message, Level level)
{
  // 创建新的列表项
  QListWidgetItem* item = new QListWidgetItem(message);

  // 根据日志级别设置不同的颜色
  switch (level)
  {
  case Level::INFO:
  {
    LOG_INFO(APPLICATION, message);
    item->setForeground(QBrush(QColor(0, 220, 0))); // 绿色文本
    break;
  }
  case Level::WARNING:
  {
    LOG_WARNING(APPLICATION, message);
    item->setForeground(QBrush(QColor(255, 165, 0))); // 橙色文本
    break;
  }
  case Level::ERR:
  {
    LOG_ERROR(APPLICATION, message);
    item->setForeground(QBrush(QColor(255, 0, 0))); // 红色文本
    break;
  }
  }

  // 添加到列表控件
  ui->lwLogInfo->addItem(item);
}

void Mainwindow::setupToolButtonAnimations()
{
  LOG_INFO(SYSTEM, "设置工具按钮独立样式效果");
  // 为确保获取正确的几何信息，延迟设置效果
  QTimer::singleShot(100, this,
                     [this]()
                     {
                       // 为每个按钮设置独立的样式和悬浮效果
                       setupButtonStyle(ui->login_config_ToolBtn, "#1976D2", "#2196F3", "#1565C0"); // 蓝色
                       setupButtonStyle(ui->serialPort_config_toolBtn, "#388E3C", "#4CAF50", "#2E7D32"); // 绿色
                       setupButtonStyle(ui->motion_config_toolBtn, "#F57C00", "#FF9800", "#EF6C00"); // 橙色
                       setupButtonStyle(ui->IO_config_toolBtn, "#7B1FA2", "#9C27B0", "#6A1B9A"); // 紫色
                       setupButtonStyle(ui->mes_config_toolBtn, "#D32F2F", "#F44336", "#C62828"); // 红色
                       setupButtonStyle(ui->offerings_config_toolBtn, "#F9A825", "#FFC107", "#F57F17"); // 黄色
                       setupButtonStyle(ui->data_chart_config_toolBtn, "#0097A7", "#00BCD4", "#00838F"); // 青色
                       setupButtonStyle(ui->vision_config_toolBtn, "#C2185B", "#E91E63", "#AD1457"); // 粉色
                       setupButtonStyle(ui->vision_process_toolBtn, "#00695C", "#009688", "#00574B"); // 蓝绿色

                       // 为控制按钮保留原有样式，只添加悬停效果
                       if (ui->start_toolBtn)
                         preserveOriginalButtonStyle(ui->start_toolBtn);
                       if (ui->pause_toolBtn)
                         preserveOriginalButtonStyle(ui->pause_toolBtn);
                       if (ui->stop_toolBtn)
                         preserveOriginalButtonStyle(ui->stop_toolBtn);

                       LOG_INFO(SYSTEM, "所有工具按钮独立样式效果设置完成");
                     });
}

void Mainwindow::setupButtonStyle(QToolButton* button, const QString& normalColor, const QString& hoverColor,
                                  const QString& pressedColor)
{
  if (!button)
    return;

  // Qt样式表，移除不支持的CSS3属性
  QString styleSheet = QString(
      "QToolButton {"
      "  background-color: white;"
      "  color: black;"
      "  border: 2px solid #E0E0E0;"
      "  border-radius: 8px;"
      "  padding: 8px 16px;"
      "  font-weight: bold;"
      "  font-size: 12px;"
      "  min-width: 80px;"
      "  min-height: 32px;"
      "}"
      "QToolButton:hover {"
      "  background-color: %2;"
      "  border: 2px solid %2;"
      "  color: white;"
      "}"
      "QToolButton:pressed {"
      "  background-color: %3;"
      "  border: 2px solid %3;"
      "  color: white;"
      "}"
      "QToolButton:disabled {"
      "  background-color: #E0E0E0;"
      "  border: 2px solid #D0D0D0;"
      "  color: #808080;"
      "}"
      ).arg(normalColor, hoverColor, pressedColor);

  button->setStyleSheet(styleSheet);

  // 启用鼠标跟踪以支持悬浮效果
  button->setAttribute(Qt::WA_Hover, true);
  button->setMouseTracking(true);

  // 为按钮安装事件过滤器以实现C++级别的动效
  button->installEventFilter(this);

  LOG_DEBUG(SYSTEM, QString("为按钮 %1 设置样式: 正常=%2, 悬浮=%3, 按下=%4")
            .arg(button->objectName(), normalColor, hoverColor, pressedColor));
}

void Mainwindow::preserveOriginalButtonStyle(QToolButton* button)
{
  if (!button)
    return;

  // 保留原有样式，只添加鼠标跟踪和hover效果
  button->setAttribute(Qt::WA_Hover, true);
  button->setMouseTracking(true);

  // 为按钮安装事件过滤器以实现C++级别的动效，但不改变原有样式
  button->installEventFilter(this);

  LOG_DEBUG(SYSTEM, QString("为控制按钮 %1 保留原有样式并添加悬停效果").arg(button->objectName()));
}

bool Mainwindow::eventFilter(QObject* watched, QEvent* event)
{
  QToolButton* btn = qobject_cast<QToolButton*>(watched);
  if (!btn)
    return QWidget::eventFilter(watched, event);

  // 检查是否是控制按钮（start, pause, stop）
  bool isControlButton = (btn == ui->start_toolBtn || btn == ui->pause_toolBtn || btn == ui->stop_toolBtn);

  // 为控制按钮使用轻微的缩放效果，保留原有样式
  if (isControlButton)
  {
    switch (event->type())
    {
    case QEvent::Enter:
    {
      // 鼠标进入按钮时的轻微放大效果
      QPropertyAnimation* scaleAnimation = new QPropertyAnimation(btn, "geometry");
      scaleAnimation->setDuration(150);

      QRect originalRect = btn->geometry();
      QRect enlargedRect = originalRect;

      // 控制按钮仅轻微放大（增加2%）
      int widthIncrease = originalRect.width() * 0.01;
      int heightIncrease = originalRect.height() * 0.01;

      enlargedRect.setWidth(originalRect.width() + widthIncrease * 2);
      enlargedRect.setHeight(originalRect.height() + heightIncrease * 2);
      enlargedRect.moveCenter(originalRect.center());

      scaleAnimation->setStartValue(originalRect);
      scaleAnimation->setEndValue(enlargedRect);
      scaleAnimation->start(QAbstractAnimation::DeleteWhenStopped);

      // 存储原始尺寸供离开时恢复
      btn->setProperty("originalGeometry", originalRect);
      break;
    }
    case QEvent::Leave:
    case QEvent::MouseButtonRelease:
    {
      // 鼠标离开或释放时恢复原始尺寸
      QPropertyAnimation* restoreAnimation = new QPropertyAnimation(btn, "geometry");
      restoreAnimation->setDuration(150);

      QRect currentRect = btn->geometry();
      QRect originalRect = btn->property("originalGeometry").toRect();

      if (originalRect.isValid())
      {
        restoreAnimation->setStartValue(currentRect);
        restoreAnimation->setEndValue(originalRect);
        restoreAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      }
      break;
    }
    default:
      break;
    }
  }
  else // 非控制按钮使用原有的完整动画效果
  {
    switch (event->type())
    {
    case QEvent::Enter:
    {
      // 鼠标进入按钮时的动画效果
      QPropertyAnimation* scaleAnimation = new QPropertyAnimation(btn, "geometry");
      scaleAnimation->setDuration(150);

      QRect originalRect = btn->geometry();
      QRect enlargedRect = originalRect;

      // 计算放大后的尺寸（增加5%）
      int widthIncrease = originalRect.width() * 0.025;
      int heightIncrease = originalRect.height() * 0.025;

      enlargedRect.setWidth(originalRect.width() + widthIncrease * 2);
      enlargedRect.setHeight(originalRect.height() + heightIncrease * 2);
      enlargedRect.moveCenter(originalRect.center());

      scaleAnimation->setStartValue(originalRect);
      scaleAnimation->setEndValue(enlargedRect);
      scaleAnimation->start(QAbstractAnimation::DeleteWhenStopped);

      // 存储原始尺寸供离开时恢复
      btn->setProperty("originalGeometry", originalRect);

      break;
    }
    case QEvent::Leave:
    {
      // 鼠标离开按钮时恢复原始尺寸
      QPropertyAnimation* restoreAnimation = new QPropertyAnimation(btn, "geometry");
      restoreAnimation->setDuration(150);

      QRect currentRect = btn->geometry();
      QRect originalRect = btn->property("originalGeometry").toRect();

      if (originalRect.isValid())
      {
        restoreAnimation->setStartValue(currentRect);
        restoreAnimation->setEndValue(originalRect);
        restoreAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      }

      break;
    }
    case QEvent::MouseButtonPress:
    {
      // 鼠标按下时的收缩效果
      QPropertyAnimation* pressAnimation = new QPropertyAnimation(btn, "geometry");
      pressAnimation->setDuration(100);

      QRect currentRect = btn->geometry();
      QRect shrunkRect = currentRect;

      // 计算收缩后的尺寸（减少3%）
      int widthDecrease = currentRect.width() * 0.015;
      int heightDecrease = currentRect.height() * 0.015;

      shrunkRect.setWidth(currentRect.width() - widthDecrease * 2);
      shrunkRect.setHeight(currentRect.height() - heightDecrease * 2);
      shrunkRect.moveCenter(currentRect.center());

      pressAnimation->setStartValue(currentRect);
      pressAnimation->setEndValue(shrunkRect);
      pressAnimation->start(QAbstractAnimation::DeleteWhenStopped);

      break;
    }
    case QEvent::MouseButtonRelease:
    {
      // 鼠标释放时恢复悬浮状态尺寸
      QPropertyAnimation* releaseAnimation = new QPropertyAnimation(btn, "geometry");
      releaseAnimation->setDuration(100);

      QRect currentRect = btn->geometry();
      QRect originalRect = btn->property("originalGeometry").toRect();

      if (originalRect.isValid())
      {
        // 恢复到悬浮状态的放大尺寸
        QRect hoverRect = originalRect;
        int widthIncrease = originalRect.width() * 0.025;
        int heightIncrease = originalRect.height() * 0.025;

        hoverRect.setWidth(originalRect.width() + widthIncrease * 2);
        hoverRect.setHeight(originalRect.height() + heightIncrease * 2);
        hoverRect.moveCenter(originalRect.center());

        releaseAnimation->setStartValue(currentRect);
        releaseAnimation->setEndValue(hoverRect);
        releaseAnimation->start(QAbstractAnimation::DeleteWhenStopped);
      }

      break;
    }
    default:
      break;
    }
  }

  return QWidget::eventFilter(watched, event);
}

/**
 * @brief Mainwindow::resizeEvent 主窗口大小改变事件处理
 * @param event 窗口大小改变事件
 */
void Mainwindow::resizeEvent(QResizeEvent* event)
{
  // 调用基类的resizeEvent
  QWidget::resizeEvent(event);

  // 如果是初始化阶段或者没有可用的子窗口，直接返回
  if (!event || event->oldSize().isEmpty())
  {
    return;
  }

  // 计算缩放比例
  QSize oldSize = event->oldSize();
  QSize newSize = event->size();

  if (oldSize.width() <= 0 || oldSize.height() <= 0)
  {
    return;
  }

  double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
  double scaleY = static_cast<double>(newSize.height()) / oldSize.height();

  // 为了保持比例一致，使用较小的缩放比例
  double scale = qMin(scaleX, scaleY);

  // 调整子窗口大小
  resizeChildWindows(scale, scaleX, scaleY);

  // 记录日志
  LOG_INFO(SYSTEM, QString("主窗口大小改变: %1x%2 -> %3x%4, 缩放比例: %5")
           .arg(oldSize.width()).arg(oldSize.height())
           .arg(newSize.width()).arg(newSize.height())
           .arg(scale).toStdString().c_str());
}

/**
 * @brief Mainwindow::resizeChildWindows 调整子窗口大小
 * @param scale 统一缩放比例
 * @param scaleX X轴缩放比例
 * @param scaleY Y轴缩放比例
 */
void Mainwindow::resizeChildWindows(double scale, double scaleX, double scaleY)
{
  // 调整视觉处理窗口
  if (m_visualProcess && m_visualProcess->isVisible())
  {
    QSize currentSize = m_visualProcess->size();
    QSize newSize(static_cast<int>(currentSize.width() * scaleX),
                  static_cast<int>(currentSize.height() * scaleY));
    m_visualProcess->resize(newSize);
  }
}

QSize Mainwindow::getOptimalChildWindowSize(int preferredWidth, int preferredHeight) const
{
  // 获取主窗口当前尺寸
  QSize mainWindowSize = this->size();

  // 根据主窗口状态调整子窗口尺寸
  int targetWidth = preferredWidth;
  int targetHeight = preferredHeight;

  // 如果主窗口是最大化状态，增大子窗口尺寸
  if (this->isMaximized() || mainWindowSize.width() > 1600 || mainWindowSize.height() > 1000)
  {
    // 对于大屏幕，使用更大的子窗口
    targetWidth = qMax(preferredWidth, static_cast<int>(mainWindowSize.width() * 0.7));
    targetHeight = qMax(preferredHeight, static_cast<int>(mainWindowSize.height() * 0.8));
  }
  else if (mainWindowSize.width() > 1200 || mainWindowSize.height() > 800)
  {
    // 对于中等屏幕，适度增大
    targetWidth = qMax(preferredWidth, static_cast<int>(mainWindowSize.width() * 0.6));
    targetHeight = qMax(preferredHeight, static_cast<int>(mainWindowSize.height() * 0.7));
  }
  else
  {
    // 对于小屏幕，保持较小尺寸但不小于最小值
    targetWidth = qMax(600, qMin(preferredWidth, static_cast<int>(mainWindowSize.width() * 0.8)));
    targetHeight = qMax(400, qMin(preferredHeight, static_cast<int>(mainWindowSize.height() * 0.8)));
  }

  // 确保子窗口不会超过主窗口
  targetWidth = qMin(targetWidth, mainWindowSize.width() - 100);
  targetHeight = qMin(targetHeight, mainWindowSize.height() - 100);

  return QSize(targetWidth, targetHeight);
}

/********************************************* 视觉配置按钮点击事件
 * ****************************************************/

void Mainwindow::on_vision_config_toolBtn_clicked()
{
  try
  {
    QMessageBox::information(this, tr("视觉配置"), tr("相机配置设置功能即将实现!"));
  }
  catch (const std::exception& e)
  {
    LOG_ERROR(SYSTEM, "打开视觉配置窗口时发生异常: " + QString::fromStdString(e.what()));
  }
}

/********************************************* 视觉处理按钮点击事件
 * ****************************************************/

void Mainwindow::on_vision_process_toolBtn_clicked()
{
  try
  {
    if (m_isVisionConfigOpen == false)
    {
      if (m_visualProcess != nullptr)
      {
        m_isVisionConfigOpen = true;
        return;
      }
      if (m_visualProcess == nullptr)
      {
        m_isVisionConfigOpen = true;
        m_visualProcess = new VisualProcess();
        m_visualProcess->setWindowTitle(tr("视觉处理配置"));
        m_visualProcess->setWindowFlag(Qt::WindowStaysOnTopHint,
                                       true); // 置顶
        m_visualProcess->setWindowModality(
            Qt::NonModal); // 设置为非模态
        m_visualProcess->setParent(this); // 设置为父窗口

        // 根据主窗口状态设置合适的尺寸
        QSize optimalSize = getOptimalChildWindowSize(1200, 800);
        m_visualProcess->resize(optimalSize);

        m_visualProcess->move(
            (this->width() - m_visualProcess->width()) / 2,
            (this->height() - m_visualProcess->height()) / 2 + 50);
        m_visualProcess->show(); // 显示窗口
        m_visualProcess->raise(); // 确保窗口在最上层
        m_visualProcess->activateWindow(); // 激活窗口
        qDebug() << "m_isVisionConfigOpen" << m_isVisionConfigOpen;
      }
    }
    else if (m_isVisionConfigOpen == true) // 窗口关闭触发
      if (m_visualProcess != nullptr)
      {
        m_isVisionConfigOpen = false;
        m_visualProcess->close();
        delete m_visualProcess;
        m_visualProcess =
            nullptr; // 重要：删除后设置为nullptr避免悬空指针
        qDebug() << "m_isVisionConfigOpen" << m_isVisionConfigOpen;
      }
  }
  catch (...)
  {
    LOG_ERROR(SYSTEM, "打开视觉处理窗口时发生异常: ");
  }
}

void Mainwindow::on_serialPort_config_toolBtn_clicked()
{
  try
  {
    if (m_isSerialPortConfigOpen == false)
    {
      if (m_serialDialog != nullptr)
      {
        m_isSerialPortConfigOpen = true;
        return;
      }
      if (m_serialDialog == nullptr)
      {
        m_isSerialPortConfigOpen = true;
        m_serialDialog = new SerialDialog(this);
        m_serialDialog->setWindowTitle(tr("串口配置"));
        m_serialDialog->setWindowFlag(Qt::WindowStaysOnTopHint,
                                      true); // 置顶
        m_serialDialog->setWindowModality(Qt::NonModal); // 设置为非模态
        m_serialDialog->setParent(this); // 设置为父窗口

        // 根据主窗口状态设置合适的尺寸
        QSize optimalSize = getOptimalChildWindowSize(800, 600);
        m_serialDialog->resize(optimalSize);

        m_serialDialog->move(
            (this->width() - m_serialDialog->width()) / 2,
            (this->height() - m_serialDialog->height()) / 2 + 50);
        m_serialDialog->show(); // 显示窗口
        m_serialDialog->raise(); // 确保窗口在最上层
        m_serialDialog->activateWindow(); // 激活窗口
      }
    }
    else if (m_isSerialPortConfigOpen == true) // 窗口关闭触发
    {
      // 如果对话框已经打开，直接关闭
      if (m_serialDialog == nullptr)
      {
        m_isSerialPortConfigOpen = false;
        return;
      }
      // 如果对话框不为空，则关闭并删除
      if (m_serialDialog != nullptr)
      {
        m_isSerialPortConfigOpen = false;
        m_serialDialog->close();
        delete m_serialDialog; // 删除对话框对象
        qDebug() << "m_isSerialPortConfigOpen" << m_isSerialPortConfigOpen;
        // 重要：删除后设置为nullptr避免悬空指针
        m_serialDialog = nullptr;
      }
    }
  }
  catch (...)
  {
    LOG_ERROR(SYSTEM, "发生异常无法打开通信窗口");
  }
}

void Mainwindow::on_start_toolBtn_clicked()
{
  try
  {
    // 然后启动基础的视觉处理
    QMetaObject::invokeMethod(m_visualWorkThread, "process", Qt::QueuedConnection);

    appLogInfo("✅ 已启动视觉处理任务");
  }
  catch (const std::exception& e)
  {
    appLogInfo(QString("❌ 启动处理时发生异常：%1").arg(QString::fromStdString(e.what())), ERR);
  }
}

/* ============================== 基础的视觉工作线程槽函数 ============================== */

void Mainwindow::onWorkThreadFinished()
{
  try
  {
    appLogInfo("🎉 视觉处理任务完成");

    // 更新UI状态
    ui->start_toolBtn->setEnabled(true);
    ui->start_toolBtn->setText("开始");

    // 在显示区域显示完成信息
    if (rightHal)
    {
      rightHal->dispHalconMessage(20, 20, "处理任务完成!", "green");
    }

    // 更新结果计数（示例）
    onVisualProcess_results(1);

  }
  catch (const std::exception& e)
  {
    appLogInfo(QString("❌ 处理完成事件时发生异常：%1").arg(QString::fromStdString(e.what())), ERR);
  }
}

void Mainwindow::onWorkThreadError(const QString& error)
{
  try
  {
    appLogInfo(QString("❌ 视觉处理错误：%1").arg(error), ERR);

    // 在右侧显示区域显示错误信息
    if (rightHal)
    {
      rightHal->dispHalconMessage(20, 20, QString("错误: %1").arg(error), "red");
    }

    // 恢复UI状态
    ui->start_toolBtn->setEnabled(true);
    ui->start_toolBtn->setText("开始");

  }
  catch (const std::exception& e)
  {
    appLogInfo(QString("❌ 处理错误事件时发生异常：%1").arg(QString::fromStdString(e.what())), ERR);
  }
}
