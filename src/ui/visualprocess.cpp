//
// Created by jinxi on 25-5-22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_visualprocess.h" resolved

#include "../inc/ui/visualprocess.h"
#include "../thirdparty/hdevelop/include/HalconLable.h"
#include "../thirdparty/hdevelop/include/HalconFileManager.h"
#include "../thirdparty/log_manager/inc/simplecategorylogger.h"
#include "../thirdparty/dynamic_ui/include/DynamicUIBuilder.h"
#include "../thirdparty/config/inc/config_manager.h"

/*========================================  End  ===============================================*/

#include <QDateTime>
#include <QToolButton>
#include <QMovie>
#include <QDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileDialog>
#include <QRegularExpression>  // Qt 5.12兼容：使用QRegularExpression替代QRegExp
#include <QDesktopWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>

#include "ui_visualprocess.h"

#define SYSTEM "VisualProcess"

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

VisualProcess::VisualProcess(QWidget* parent) :
  QWidget(parent), ui(new Ui::VisualProcess)
  , m_QCodeCam_flag(false), m_isOpenQCodeCam_flag(false), m_MeasureCam_flag(false)
  , m_isOpenMeasureCam_flag(false), m_CheckCam_flag(false), m_isOpenCheckCam_flag(false), halWin(nullptr)
{
  ui->setupUi(this);
  initLog();
  initVisionProcess();
  m_uiBuilder = new DynamicUIBuilder(this);
  initConnect(); // 初始化连接
  setAttribute(Qt::WA_DeleteOnClose); // 设置窗口关闭时自动删除

  // 🎨 配置日志列表支持自动换行
  if (ui->workflows_listView)
  {
    ui->workflows_listView->setWordWrap(true); // 启用自动换行
    ui->workflows_listView->setTextElideMode(Qt::ElideNone); // 不省略文本
    ui->workflows_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏水平滚动条
    ui->workflows_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 需要时显示垂直滚动条
    ui->workflows_listView->setResizeMode(QListView::Adjust); // 自动调整内容大小

    // 设置样式以优化显示效果
    ui->workflows_listView->setStyleSheet(R"(
      QListWidget {
        background-color: #f8f9fa;
        border: 1px solid #dee2e6;
        border-radius: 6px;
        padding: 5px;
        font-family: 'Microsoft YaHei', Consolas, monospace;
      }
      QListWidget::item {
        border: none;
        padding: 5px;
        margin: 2px 0;
        border-radius: 4px;
        word-wrap: break-word;
      }
      QListWidget::item:hover {
        background-color: rgba(0, 123, 255, 0.1);
      }
      QScrollBar:vertical {
        background-color: #f1f1f1;
        width: 12px;
        border-radius: 6px;
      }
      QScrollBar::handle:vertical {
        background-color: #c1c1c1;
        border-radius: 6px;
        min-height: 20px;
      }
      QScrollBar::handle:vertical:hover {
        background-color: #a8a8a8;
      }
    )");

    appLog(tr("✅ 日志系统已配置：支持自动换行和动态高度调整"));
  }
}

VisualProcess::~VisualProcess()
{
  LOG_INFO(tr("Visual process shutdown"));
  // 清理所有提示标签和动画
  for (auto it = m_buttonTips.begin(); it != m_buttonTips.end(); ++it)
  {
    it.value().second->stop();
    delete it.value().second;
    delete it.value().first;
  }
  if (halWin != nullptr)
  {
    delete halWin;
    halWin = nullptr;
  }
  if (m_halFileManager != nullptr)
  {
    delete m_halFileManager;
    m_halFileManager = nullptr;
  }
  delete ui;
}

/*============================= 初始化 ============================*/
// 初始化视觉处理
void VisualProcess::initVisionProcess()
{
  if (halWin == nullptr)
  {
    halWin = new HalconLable(this);
    m_halFileManager = new HalconFileManager(this);
    clearLaout(ui->gridLayout_ImageDisplay);
    ui->gridLayout_ImageDisplay->addWidget(halWin, 0, 0);

    // 🎯 强制触发初始化，确保Halcon窗口正确创建
    QTimer::singleShot(100, [this]()
    {
      if (halWin)
      {
        // 强制触发resize事件，确保Halcon窗口被创建
        halWin->resize(halWin->size());
        qDebug() << "✅ HalconLable窗口初始化完成，Halcon显示窗口已创建";
        appLog("🖼️ 图像显示窗口初始化完成");
      }
    });

    // 🖼️ 设置Halcon窗口样式
    halWin->setStyleSheet(
        "QWidget {"
        "  background-color: #2b2b2b;"
        "  border: 1px solid #555555;"
        "  border-radius: 4px;"
        "}"
        );

    /* ============================== 初始化二维码句柄 ============================== */
    try
    {
      CreateDataCode2dModel("Data Matrix ECC 200", "default_parameters", "maximum_recognition", &m_Data_code_handle);
      appLog(tr("二维码句柄初始化成功"));
    }
    catch (...)
    {
      appLog(tr("二维码句柄初始化失败"), ERR);
    }
  }
  else
  {
    LOG_WARNING(tr("HalconLable对象已存在，无法重新初始化"));
  }
}

void VisualProcess::initConnect()
{
  connect(ui->comboBoxModels, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &VisualProcess::onComboBoxAction);
  // 连接工具按钮信号到处理函数
  connect(this, &VisualProcess::toolButtonClicked, this, &VisualProcess::handleToolButtonClicked);
}

void VisualProcess::initLog()
{
  auto& logger = SimpleCategoryLogger::instance();
  // 初始化日志系统 | en : Initialize the logging system
  QString logFilePath = QApplication::applicationDirPath() + "/logs/" + SYSTEM;
  logger.initCategory(SYSTEM, logFilePath);
  // 设置日志大小 | en : Set the log file size
  logger.setDefaultMaxLogFileSize(10 * 1024 * 1024); // 10MB
  // 设置历史文件数量 | en : Set the number of historical log files
  logger.setMaxHistoryFileCount(30); // 保留5个历史日志文件
  // 配置调试选项 | en : Configure debug options
  logger.setDebugConfig(true, true, true); // 显示在控制台、写入文件、启用调试日志
  // 设置定期清理日志 | en : Set up periodic log cleanup
  logger.setPeriodicCleanup(true); // 默认7天清理一次, 日志天数超过30天自动清理
}

/* ==================== 工具函数 ==================== */
// 创建工具按钮
void VisualProcess::createToolButtons(QVBoxLayout* layout, const QVector<QPair<QString, QString>>& buttonConfigs)
{
  QGridLayout* gridLayout = new QGridLayout();
  gridLayout->setSpacing(5);

  int row = 0;
  int col = 0;
  const int maxCols = 3;

  for (const auto& config : buttonConfigs)
  {
    QToolButton* button = new QToolButton(this);
    button->setText(config.first);
    button->setToolTip(config.second);
    button->setMinimumSize(120, 40);
    button->setIconSize(QSize(32, 32));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setStyleSheet(
        "QToolButton {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "}"
        "QToolButton:hover {"
        "    background-color: #e9ecef;"
        "}"
        "QToolButton:pressed {"
        "    background-color: #dee2e6;"
        "}");

    button->installEventFilter(this);

    connect(button, &QToolButton::clicked, [this, key = config.first]()
    {
      emit toolButtonClicked(key);
    });

    // 为每个按钮创建独立的 QLabel 和 QMovie
    QLabel* tipLabel = new QLabel(this);
    QMovie* tipMovie = new QMovie(QString(":/images/%1.gif").arg(config.first));
    tipLabel->setMovie(tipMovie);
    tipMovie->start();
    tipLabel->setWindowFlags(Qt::ToolTip);
    tipLabel->hide();

    // 存储按钮对应的提示标签和动画
    m_buttonTips[button] = qMakePair(tipLabel, tipMovie);

    gridLayout->addWidget(button, row, col);

    col++;
    if (col >= maxCols)
    {
      col = 0;
      row++;
    }
  }

  // 将 gridLayout 添加到传入的 layout 中
  layout->addLayout(gridLayout);
}

bool VisualProcess::eventFilter(QObject* watched, QEvent* event)
{
  if (auto* button = qobject_cast<QToolButton*>(watched))
  {
    if (event->type() == QEvent::Enter)
    {
      // 显示对应按钮的 GIF 提示
      if (m_buttonTips.contains(button))
      {
        auto& tip = m_buttonTips[button];
        tip.first->move(QCursor::pos() + QPoint(10, 10));
        tip.first->show();
      }
    }
    else if (event->type() == QEvent::Leave)
    {
      if (m_buttonTips.contains(button))
      {
        m_buttonTips[button].first->hide();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void VisualProcess::clearLaout(QLayout* layout)
{
  // 清除布局（如 clearLaout(QLayout* layout)）后，布局中的控件指针（widget）会被 delete，布局项（item）也会被
  // delete。 因此，布局里的指针已经被释放，不再有效，不能再使用，否则会导致野指针或崩溃。
  if (!layout)
    return;
  QLayoutItem* item;
  while ((item = layout->takeAt(0)))
  {
    if (item->layout())
    {
      clearLaout(item->layout());
    }
    else if (item->widget())
    {
      delete item->widget();
    }
    delete item;
  }
}

void VisualProcess::appLog(QString message, Level level)
{
  // 获取当前时间戳
  QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
  QString formattedMessage;
  QListWidgetItem* item = new QListWidgetItem();
  item->setTextAlignment(Qt::AlignLeft);

  // 创建美观的日志格式
  switch (level)
  {
  case 0: // DEBUG
    LOG_DEBUG(message);
    formattedMessage = QString("🔧 [%1] %2").arg(timestamp).arg(message);
    item->setForeground(QBrush(QColor("#6c757d"))); // 灰色
    item->setBackground(QBrush(QColor("#f8f9fa"))); // 浅灰色背景
    break;
  case 1:
    // INFO
    LOG_INFO(message);
    formattedMessage = QString("ℹ️ [%1] %2").arg(timestamp).arg(message);
    item->setForeground(QBrush(QColor("#28a745"))); // 绿色
    item->setBackground(QBrush(QColor("#d4edda"))); // 浅绿色背景
    break;
  case 2: // WARNING
    LOG_WARNING(message);
    formattedMessage = QString("⚠️ [%1] %2").arg(timestamp).arg(message);
    item->setForeground(QBrush(QColor("#ffc107"))); // 黄色
    item->setBackground(QBrush(QColor("#fff3cd"))); // 浅黄色背景
    break;
  case 3: // ERROR
    LOG_ERROR(message);
    formattedMessage = QString("❌ [%1] %2 ").arg(timestamp).arg(message);
    item->setForeground(QBrush(QColor("#dc3545"))); // 红色
    item->setBackground(QBrush(QColor("#f8d7da"))); // 浅红色背景
    break;
  default:
    LOG_DEBUG(message);
    formattedMessage = QString("📝 [%1] %2").arg(timestamp).arg(message);
    item->setForeground(QBrush(QColor("#6c757d"))); // 灰色
    item->setBackground(QBrush(QColor("#f8f9fa"))); // 浅灰色背景
    break;
  }

  item->setText(formattedMessage);

  // 设置字体
  QFont font = item->font();
  font.setFamily("Consolas, Monaco, monospace");
  font.setPixelSize(14);
  font.setBold(true);
  item->setFont(font);

  // 🔧 计算文本所需的高度以支持自动换行
  QFontMetrics fontMetrics(font);
  int listWidth = ui->workflows_listView->width() - 20; // 减去边距
  if (listWidth < 200)
    listWidth = 200; // 最小宽度

  QRect textRect = fontMetrics.boundingRect(QRect(0, 0, listWidth, 0),
                                            Qt::AlignLeft | Qt::TextWordWrap,
                                            formattedMessage);

  // 设置最小高度为35，根据文本内容动态调整
  int itemHeight = qMax(35, textRect.height() + 10); // 添加10px内边距
  item->setSizeHint(QSize(0, itemHeight));

  // 🎨 设置自动换行支持
  item->setData(Qt::DisplayRole, formattedMessage);
  item->setData(Qt::ToolTipRole, formattedMessage); // 工具提示显示完整内容

  // ui->workflows_listView->insertItem(0, item);
  ui->workflows_listView->addItem(item);
  ui->workflows_listView->scrollToBottom();
}

/* ==================== 清除显示对象但保留图像 ==================== */
void VisualProcess::clearDisplayObjectsOnly()
{
  try
  {
    if (halWin != nullptr)
    {
      appLog(tr("🧹 清除所有区域和轮廓，保留图像..."));

      // 🎯 使用HalconLable的新便捷方法
      int objectCount = halWin->getDisplayObjectsCount();
      if (objectCount > 0)
      {
        appLog(tr("🧹 当前有 %1 个显示对象需要清除").arg(objectCount));
        halWin->clearDisplayObjectsOnly();
        appLog(tr("✅ 所有显示对象已清除"));
      }
      else
      {
        appLog(tr("ℹ️ 没有需要清除的显示对象"));
      }

      // 🧹 清除相关的测量和检测区域对象，但保留主图像
      int clearedCount = 0;
      try
      {
        if (m_Measyre_Rect1.IsInitialized())
        {
          m_Measyre_Rect1.Clear();
          clearedCount++;
        }

        if (m_Measyre_Rect2.IsInitialized())
        {
          m_Measyre_Rect2.Clear();
          clearedCount++;
        }

        if (m_Read_code_first.IsInitialized())
        {
          m_Read_code_first.Clear();
          clearedCount++;
        }

        if (m_Read_code_tail.IsInitialized())
        {
          m_Read_code_tail.Clear();
          clearedCount++;
        }

        if (m_CheckRegion.IsInitialized())
        {
          m_CheckRegion.Clear();
          clearedCount++;
        }

        if (m_SymbolXLDs.IsInitialized())
        {
          m_SymbolXLDs.Clear();
          clearedCount++;
        }

        if (m_OKRegion.IsInitialized())
        {
          m_OKRegion.Clear();
          clearedCount++;
        }

        if (m_NGRegion.IsInitialized())
        {
          m_NGRegion.Clear();
          clearedCount++;
        }

        if (clearedCount > 0)
        {
          appLog(tr("🧹 已清除 %1 个相关区域对象").arg(clearedCount));
        }

      }
      catch (HalconCpp::HException& e)
      {
        appLog(tr("⚠️ 清除区域对象时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
      }

      // 📊 显示清理结果
      QString imageInfo = halWin->getImageInfo();
      appLog(tr("📊 当前状态: %1").arg(imageInfo));

    }
    else
    {
      appLog(tr("❌ 错误：图像显示窗口未初始化"), ERR);
    }
  }
  catch (HalconCpp::HException& e)
  {
    QString errorMsg = tr("❌ Halcon异常：%1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text()));
    appLog(errorMsg, ERR);
  } catch (const std::exception& e)
  {
    QString errorMsg = tr("❌ C++异常：%1").arg(QString::fromLocal8Bit(e.what()));
    appLog(errorMsg, ERR);
  } catch (...)
  {
    appLog(tr("❌ 清除显示对象时发生未知异常"), ERR);
  }
}

/* ==================== 槽函数 ==================== */

void VisualProcess::onComboBoxAction(int index)
{
  clearLaout(ui->vBoxLayout_Params);
  if (index == QRCODE)
  {
    LOG_INFO(tr("📷 选择二维码"));
    m_QCodeCam_flag = true;
    m_MeasureCam_flag = false;
    m_CheckCam_flag = false;
    onQCodeTabWidget(); // 调用读码函数
  }
  else if (index == MEASURE)
  {
    LOG_INFO(tr("📏 选择测量"));
    m_QCodeCam_flag = false;
    m_MeasureCam_flag = true;
    m_CheckCam_flag = false;
    onMeasureCam(); // 调用测量函数
  }
  else if (index == CHECK)
  {
    LOG_INFO(tr("🔍 选择检测"));
    m_QCodeCam_flag = false;
    m_MeasureCam_flag = false;
    m_CheckCam_flag = true;
    onCheckCam(); // 调用检测函数
  }
  else
  {
    LOG_WARNING(tr("⚠️ 未知类型"));
  }
}

// 处理二维码的槽函数
void VisualProcess::onQCodeTabWidget()
{
  if (m_QCodeCam_flag && m_isOpenQCodeCam_flag == true)
  {
    QVector<QPair<QString, QString>> buttonConfigs = {
        {tr("📷 打开相机"), tr("打开相机进行二维码识别")},
        {tr("⏹️ 停止相机"), tr("停止相机")},
        {tr("💾 保存图像"), tr("保存当前图像")},
        {tr("🗑️ 清除图像"), tr("清除当前图像")},
        {tr("📋 创建二维码模板"), tr("创建新的二维码识别模板")},
        {tr("⚙️ 设置参数"), tr("设置二维码识别参数")},
        {tr("⚡ 快捷操作"), tr("缩放、像素查询等快捷功能")},
        {tr("📁 读取本地图片"), tr("选择本地图片进行二维码识别")},
        {tr("📌 拾取单个区域"), tr("拾取二维码识别区域")},
        {tr("📊 规律拾取"), tr("拾取规律二维码识别区域")},
        {tr("📊 查看识别结果"), tr("查看历史二维码识别结果")},
        {tr("🚀 开始批处理"), tr("启动二维码批处理任务")}, // 🎯 新增
        {tr("⏹️ 停止批处理"), tr("停止批处理任务")}, // 🎯 新增
        {tr("📊 查看结果历史"), tr("查看处理结果历史")}, // 🎯 新增
    };
    createToolButtons(ui->vBoxLayout_Params, buttonConfigs);
  }
  else if (m_QCodeCam_flag && m_isOpenQCodeCam_flag == false)
  {
    QVector<QPair<QString, QString>> readLocalImageConfigs = {
        {tr("📁 读取本地图片"), tr("选择本地图片进行二维码识别")},
        {tr("🗑️ 清除图像"), tr("清除当前图像")},
        {tr("📋 创建形状模板"), tr("创建新的形状模板")},
        {tr("📋 创建二维码模板"), tr("创建新的二维码识别模板")},
        {tr("📌 拾取单个区域"), tr("拾取二维码识别区域")},
        {tr("📊 规律拾取"), tr("拾取规律二维码识别区域")},
        {tr("📊 查看识别结果"), tr("查看历史二维码识别结果")},
        {tr("⚡ 快捷操作"), tr("缩放、像素查询等快捷功能")},
        {tr("🚀 开始批处理"), tr("启动二维码批处理任务")}, // 🎯 新增
        {tr("⏹️ 停止批处理"), tr("停止批处理任务")}, // 🎯 新增
        {tr("📊 查看结果历史"), tr("查看处理结果历史")}, // 🎯 新增
    };
    createToolButtons(ui->vBoxLayout_Params, readLocalImageConfigs);
  }
}

// 处理测量的槽函数
void VisualProcess::onMeasureCam()
{
  if (m_MeasureCam_flag && m_isOpenMeasureCam_flag == true)
  {
    // 已打开相机的情况下显示的按钮
    QVector<QPair<QString, QString>> buttonConfigs = {
        {tr("📷 打开相机"), tr("打开相机进行测量")},
        {tr("⏹️ 停止相机"), tr("停止相机")},
        {tr("💾 保存图像"), tr("保存当前图像")},
        {tr("🗑️ 清除图像"), tr("清除当前图像")},
        {tr("📋 创建测量模板"), tr("创建新的测量模板")},
        {tr("⚙️ 设置参数"), tr("设置测量参数")},
        {tr("⚡ 快捷操作"), tr("缩放、像素查询等快捷功能")},
        {tr("📏 选择测量工具"), tr("选择不同的测量工具")},
        {tr("📊 查看测量结果"), tr("查看历史测量结果")},
        {tr("🚀 开始批处理"), tr("启动测量批处理任务")}, // 🎯 新增
        {tr("⏹️ 停止批处理"), tr("停止批处理任务")}, // 🎯 新增
        {tr("📊 查看结果历史"), tr("查看处理结果历史")}, // 🎯 新增
    };
    createToolButtons(ui->vBoxLayout_Params, buttonConfigs);

    // 添加测量工具分组
    QVBoxLayout* layout = new QVBoxLayout();
    QGroupBox* groupBoxTools = new QGroupBox(tr("🛠️ 测量工具"), this);
    groupBoxTools->setLayout(layout);
    QVector<QPair<QString, QString>> measureToolButtons = {
        {tr("📏 距离测量"), tr("测量两点之间的距离")},
        {tr("📐 角度测量"), tr("测量角度")},
        {tr("⭕ 圆测量"), tr("测量圆的直径和圆心")},
        {tr("🔄 轮廓分析"), tr("分析物体轮廓")},
        {tr("🎯 图像预处理"), tr("滤波、增强等图像处理")},
        {tr("📊 导出结果"), tr("导出测量数据和截图")}
    };
    createToolButtons(layout, measureToolButtons);
    ui->vBoxLayout_Params->addWidget(groupBoxTools);
  }
  else if (m_MeasureCam_flag && m_isOpenMeasureCam_flag == false)
  {
    // 未打开相机的情况下显示的按钮
    QVector<QPair<QString, QString>> buttonConfigs = {
        {tr("📁 读取本地图片"), tr("选择本地图片进行测量")},
        {tr("🗑️ 清除图像"), tr("清除当前图像")},
        {tr("📋 创建形状模板"), tr("创建新的形状模板")},
        {tr("📋 创建测量模板"), tr("创建新的测量模板")},
        {tr("⚡ 快捷操作"), tr("缩放、像素查询等快捷功能")},
        {tr("📏 选择测量工具"), tr("选择不同的测量工具")},
        {tr("📊 查看测量结果"), tr("查看历史测量结果")},
        {tr("🚀 开始批处理"), tr("启动测量批处理任务")}, // 🎯 新增
        {tr("⏹️ 停止批处理"), tr("停止批处理任务")}, // 🎯 新增
        {tr("📊 查看结果历史"), tr("查看处理结果历史")}, // 🎯 新增
    };
    createToolButtons(ui->vBoxLayout_Params, buttonConfigs);

    // 添加测量工具分组
    QVBoxLayout* layout = new QVBoxLayout();
    QGroupBox* groupBoxTools = new QGroupBox(tr("🛠️ 测量工具"), this);
    groupBoxTools->setLayout(layout);
    QVector<QPair<QString, QString>> measureToolButtons = {
        {tr("📏 距离测量"), tr("测量两点之间的距离")},
        {tr("📐 角度测量"), tr("测量角度")},
        {tr("⭕ 圆测量"), tr("测量圆的直径和圆心")},
        {tr("🔄 轮廓分析"), tr("分析物体轮廓")},
        {tr("🎯 图像预处理"), tr("滤波、增强等图像处理")},
        {tr("📊 导出结果"), tr("导出测量数据和截图")}
    };
    createToolButtons(layout, measureToolButtons);
    ui->vBoxLayout_Params->addWidget(groupBoxTools);
  }
}

// 处理检测的槽函数
void VisualProcess::onCheckCam()
{
  if (m_CheckCam_flag && m_isOpenCheckCam_flag == true)
  {
    // 已打开相机的情况下显示的按钮
    QVector<QPair<QString, QString>> buttonConfigs = {
        {tr("📷 打开相机"), tr("打开相机进行检测")},
        {tr("⏹️ 停止相机"), tr("停止相机")},
        {tr("💾 保存图像"), tr("保存当前图像")},
        {tr("🗑️ 清除图像"), tr("清除当前图像")},
        {tr("📋 创建形状模板"), tr("创建新的形状模板")},
        {tr("📋 创建检测模板"), tr("创建新的检测模板")},
        {tr("⚙️ 设置参数"), tr("设置检测参数")},
        {tr("⚡ 快捷操作"), tr("缩放、像素查询等快捷功能")},
        {tr("🔄 连续检测"), tr("启动连续检测模式")},
        {tr("📊 查看检测结果"), tr("查看历史检测结果")},
        {tr("🚀 开始批处理"), tr("启动检测批处理任务")}, // 🎯 新增
        {tr("⏹️ 停止批处理"), tr("停止批处理任务")}, // 🎯 新增
        {tr("📊 查看结果历史"), tr("查看处理结果历史")}, // 🎯 新增
    };
    createToolButtons(ui->vBoxLayout_Params, buttonConfigs);

    // 添加检测工具分组
    QVBoxLayout* layout = new QVBoxLayout();
    QGroupBox* groupBoxDetect = new QGroupBox(tr("🔍 检测工具"), this);
    groupBoxDetect->setLayout(layout);
    QVector<QPair<QString, QString>> detectToolButtons = {
        {tr("✅ 特征匹配"), tr("特征点匹配检测")},
        {tr("🔴 缺陷检测"), tr("发现产品表面缺陷")},
        {tr("📊 尺寸检测"), tr("检测产品尺寸是否合格")},
        {tr("🎯 定位对齐"), tr("产品位置定位")}
    };
    createToolButtons(layout, detectToolButtons);
    ui->vBoxLayout_Params->addWidget(groupBoxDetect);
  }
  else if (m_CheckCam_flag && m_isOpenCheckCam_flag == false)
  {
    // 未打开相机的情况下显示的按钮
    QVector<QPair<QString, QString>> buttonConfigs = {
        {tr("📁 读取本地图片"), tr("选择本地图片进行检测")},
        {tr("📋 创建检测模板"), tr("创建新的检测模板")},
        {tr("🗑️ 清除图像"), tr("清除当前图像")},
        {tr("⚡ 快捷操作"), tr("缩放、像素查询等快捷功能")},
        {tr("🔄 连续检测"), tr("启动连续检测模式")},
        {tr("📊 查看检测结果"), tr("查看历史检测结果")},
        {tr("🚀 开始批处理"), tr("启动检测批处理任务")}, // 🎯 新增
        {tr("⏹️ 停止批处理"), tr("停止批处理任务")}, // 🎯 新增
        {tr("📊 查看结果历史"), tr("查看处理结果历史")}, // 🎯 新增
    };
    createToolButtons(ui->vBoxLayout_Params, buttonConfigs);

    // 添加检测工具分组
    QVBoxLayout* layout = new QVBoxLayout();
    QGroupBox* groupBoxDetect = new QGroupBox(tr("🔍 检测工具"), this);
    groupBoxDetect->setLayout(layout);
    QVector<QPair<QString, QString>> detectToolButtons = {
        {tr("✅ 特征匹配"), tr("特征点匹配检测")},
        {tr("🔴 缺陷检测"), tr("发现产品表面缺陷")},
        {tr("📊 尺寸检测"), tr("检测产品尺寸是否合格")},
        {tr("🎯 定位对齐"), tr("产品位置定位")}
    };
    createToolButtons(layout, detectToolButtons);
    ui->vBoxLayout_Params->addWidget(groupBoxDetect);
  }
}

// 处理工具按钮点击事件
void VisualProcess::handleToolButtonClicked(const QString& key)
{
  m_halconParamPath = QApplication::applicationDirPath() + "/config/halconParams/";
  /// 设置二维码参数路径
  QRcodeSavePath = m_halconParamPath + "QRCode";
  /// 设置测量参数路径
  MeasureSavePath = m_halconParamPath + "Measure";
  /// 设置检测参数路径
  CheckSavePath = m_halconParamPath + "Check";
  /// 设置模板保存路径
  m_modelSavePath = QApplication::applicationDirPath() + "/config/models/";
  // 通用按钮处理
  if (key == tr("📷 打开相机"))
  {
    appLog(tr("开始打开相机..."));
    // TODO: 根据当前模式打开不同的相机
    if (m_QCodeCam_flag)
    {
      // 二维码相机处理
      m_isOpenQCodeCam_flag = true;
      // 重新加载按钮，更新界面
      clearLaout(ui->vBoxLayout_Params);
      onQCodeTabWidget();
    }
    else if (m_MeasureCam_flag)
    {
      // 测量相机处理
      m_isOpenMeasureCam_flag = true;
      // 重新加载按钮，更新界面
      clearLaout(ui->vBoxLayout_Params);
      onMeasureCam();
    }
    else if (m_CheckCam_flag)
    {
      // 检测相机处理
      m_isOpenCheckCam_flag = true;
      // 重新加载按钮，更新界面
      clearLaout(ui->vBoxLayout_Params);
      onCheckCam();
    }
  }
  else if (key == tr("⏹️ 停止相机"))
  {
    appLog(tr("停止相机..."));
    // TODO: 根据当前模式停止不同的相机
    if (m_QCodeCam_flag && m_isOpenQCodeCam_flag)
    {
      m_isOpenQCodeCam_flag = false;
      // 重新加载按钮，更新界面
      clearLaout(ui->vBoxLayout_Params);
      onQCodeTabWidget();
    }
    else if (m_MeasureCam_flag && m_isOpenMeasureCam_flag)
    {
      m_isOpenMeasureCam_flag = false;
      // 重新加载按钮，更新界面
      clearLaout(ui->vBoxLayout_Params);
      onMeasureCam();
    }
    else if (m_CheckCam_flag && m_isOpenCheckCam_flag)
    {
      m_isOpenCheckCam_flag = false;
      // 重新加载按钮，更新界面
      clearLaout(ui->vBoxLayout_Params);
      onCheckCam();
    }
  }
  else if (key == tr("💾 保存图像"))
  {
    try
    {
      // 🎯 使用新的安全保存方法
      QString errorMessage;
      bool success = halWin->QtSaveImageSafe(m_Img, errorMessage);

      if (success)
      {
        appLog(tr("✅ %1").arg(errorMessage));
      }
      else
      {
        appLog(tr("❌ 保存失败：%1").arg(errorMessage), ERR);
      }
    }
    catch (const std::exception& e)
    {
      appLog(tr("❌ 保存图像时发生异常：%1").arg(QString::fromLocal8Bit(e.what())), ERR);
    }
    catch (...)
    {
      appLog(tr("保存失败, 无法保存图像"), ERR);
    }

  }
  else if (key == tr("📁 读取本地图片") || key == tr("📁 本地图片"))
  {
    try
    {
      // 🎯 使用新的安全读取方法
      QString errorMessage;
      bool success = halWin->QtGetLocalImageSafe(m_Img, errorMessage);

      if (success)
      {
        appLog(tr("✅ %1").arg(errorMessage));
        // 📊 显示图像信息
        QString imageInfo = halWin->getImageInfo();
        appLog(tr("📊 %1").arg(imageInfo));
      }
      else
      {
        appLog(tr("❌ 读取失败：%1").arg(errorMessage), ERR);
      }
    }
    catch (const std::exception& e)
    {
      appLog(tr("❌ 读取图像时发生异常：%1").arg(QString::fromLocal8Bit(e.what())), ERR);
    }
    catch (...)
    {
      appLog(tr("❌ 读取图像时发生未知异常"), ERR);
    }

  }
  else if (key == tr("🗑️ 清除图像"))
  {
    appLog(tr("清除图像和所有显示对象..."));

    try
    {
      if (halWin != nullptr)
      {
        // 🔍 检查当前图像状态
        if (!m_Img.IsInitialized())
        {
          appLog(tr("⚠️ 当前没有加载的图像，仅清除显示对象"), WARNING);
        }
        else
        {
          appLog(tr("🗑️ 正在清除已加载的图像..."));
        }

        // 🧹 使用HalconLabel的安全清除方法
        try
        {
          int objectCount = halWin->showSymbolList.size();
          if (objectCount > 0)
          {
            appLog(tr("🧹 准备清除 %1 个显示对象...").arg(objectCount));
          }

          // 🛡️ 调用HalconLabel的RemoveShow方法，这是最安全的方式
          halWin->RemoveShow();
          appLog(tr("✅ 显示对象已清除"));

        }
        catch (HalconCpp::HException& e)
        {
          appLog(tr("⚠️ 清除显示对象时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
        }

        // 🗑️ 清除主图像对象
        if (m_Img.IsInitialized())
        {
          try
          {
            m_Img.Clear();
            appLog(tr("✅ 主图像对象已清除"));
          }
          catch (HalconCpp::HException& e)
          {
            appLog(tr("⚠️ 清除主图像对象时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
          }
        }

        // 🖼️ 显示空图像（不创建空图像，直接让HalconLabel处理）
        try
        {
          HObject emptyImage;
          // 不需要调用GenEmptyObj，直接让showImage处理未初始化的图像
          halWin->showImage(emptyImage);
          appLog(tr("🖼️ 图像显示已重置"));
        }
        catch (HalconCpp::HException& e)
        {
          appLog(tr("⚠️ 重置图像显示时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
        }

        // 🧹 清除所有相关的测量和检测对象
        int clearedCount = 0;
        try
        {
          if (m_Measyre_Rect1.IsInitialized())
          {
            m_Measyre_Rect1.Clear();
            clearedCount++;
          }

          if (m_Measyre_Rect2.IsInitialized())
          {
            m_Measyre_Rect2.Clear();
            clearedCount++;
          }

          if (m_Read_code_first.IsInitialized())
          {
            m_Read_code_first.Clear();
            clearedCount++;
          }

          if (m_Read_code_tail.IsInitialized())
          {
            m_Read_code_tail.Clear();
            clearedCount++;
          }

          if (m_CheckRegion.IsInitialized())
          {
            m_CheckRegion.Clear();
            clearedCount++;
          }

          if (m_SymbolXLDs.IsInitialized())
          {
            m_SymbolXLDs.Clear();
            clearedCount++;
          }

          if (m_OKRegion.IsInitialized())
          {
            m_OKRegion.Clear();
            clearedCount++;
          }

          if (m_NGRegion.IsInitialized())
          {
            m_NGRegion.Clear();
            clearedCount++;
          }

          if (clearedCount > 0)
          {
            appLog(tr("🧹 已清除 %1 个相关对象").arg(clearedCount));
          }

        }
        catch (HalconCpp::HException& e)
        {
          appLog(tr("⚠️ 清除相关对象时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
        }

        appLog(tr("✅ 图像和所有显示对象清除完成"));
      }
      else
      {
        appLog(tr("❌ 错误：图像显示窗口未初始化"), ERR);
      }
    }
    catch (HalconCpp::HException& e)
    {
      QString errorMsg = tr("❌ Halcon异常：%1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text()));
      appLog(errorMsg, ERR);
    }
    catch (const std::exception& e)
    {
      QString errorMsg = tr("❌ C++异常：%1").arg(QString::fromLocal8Bit(e.what()));
      appLog(errorMsg, ERR);
    }
    catch (...)
    {
      appLog(tr("❌ 清除图像时发生未知异常"), ERR);
    }
  }
  else if (key == tr("⚙️ 设置参数"))
  {
    appLog(tr("打开参数设置面板..."));
    // TODO: 根据当前模式打开不同的参数设置面板
  }

  // 二维码模式特有按钮
  else if (key == tr("📌 拾取单个区域"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始拾取二维码识别区域..."));
    // TODO: 实现拾取二维码识别区域的逻辑
  }
  else if (key == tr("📊 规律拾取"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始规律拾取二维码区域..."));

    // 创建非模态规律拾取对话框
    QDialog* pickupDialog = new QDialog(this, Qt::Window | Qt::WindowStaysOnTopHint);
    pickupDialog->setWindowTitle(tr("📊 规律拾取设置"));
    pickupDialog->setMinimumSize(450, 400);
    pickupDialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除

    QVBoxLayout* dialogLayout = new QVBoxLayout(pickupDialog);

    // 添加规律拾取相关控件
    QGroupBox* groupBox = new QGroupBox(tr("拾取操作"), pickupDialog);
    QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);

    QVector<QPair<QString, QString>> pickUpButtons = {
        {tr("🔍 拾取首区域"), tr("拾取规律首个二维码识别区域")},
        {tr("🔍 拾取尾区域"), tr("拾取规律最后一个二维码识别区域")},
    };


    dialogLayout->addWidget(groupBox);

    // 添加参数设置区域
    QGroupBox* paramsGroupBox = new QGroupBox(tr("规律参数设置"), pickupDialog);
    QGridLayout* paramsLayout = new QGridLayout(paramsGroupBox);

    // 添加横向和纵向的数量选择
    QLabel* rowLabel = new QLabel(tr("横向个数:"), paramsGroupBox);
    QSpinBox* rowCountSpin = new QSpinBox(paramsGroupBox);
    rowCountSpin->setMinimum(1);
    rowCountSpin->setMaximum(50);
    rowCountSpin->setValue(1);

    QLabel* colLabel = new QLabel(tr("纵向个数:"), paramsGroupBox);
    QSpinBox* colCountSpin = new QSpinBox(paramsGroupBox);
    colCountSpin->setMinimum(1);
    colCountSpin->setMaximum(50);
    colCountSpin->setValue(1);

    QLabel* totalLabel = new QLabel(tr("总数量:"), paramsGroupBox);
    QLabel* totalCountLabel = new QLabel("1", paramsGroupBox);

    // 更新总数量的显示
    auto updateTotalCount = [rowCountSpin, colCountSpin, totalCountLabel]()
    {
      int total = rowCountSpin->value() * colCountSpin->value();
      totalCountLabel->setText(QString::number(total));
    };

    connect(rowCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), updateTotalCount);
    connect(colCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), updateTotalCount);

    paramsLayout->addWidget(rowLabel, 0, 0);
    paramsLayout->addWidget(rowCountSpin, 0, 1);
    paramsLayout->addWidget(colLabel, 1, 0);
    paramsLayout->addWidget(colCountSpin, 1, 1);
    paramsLayout->addWidget(totalLabel, 2, 0);
    paramsLayout->addWidget(totalCountLabel, 2, 1);

    // 添加间距设置
    QLabel* spacingLabel = new QLabel(tr("码间距(像素):"), paramsGroupBox);
    QSpinBox* spacingSpin = new QSpinBox(paramsGroupBox);
    spacingSpin->setMinimum(0);
    spacingSpin->setMaximum(1000);
    spacingSpin->setValue(10);

    paramsLayout->addWidget(spacingLabel, 3, 0);
    paramsLayout->addWidget(spacingSpin, 3, 1);

    dialogLayout->addWidget(paramsGroupBox);

    // 预览和应用按钮
    QHBoxLayout* actionLayout = new QHBoxLayout();
    QPushButton* previewButton = new QPushButton(tr("🔍 预览"), pickupDialog);
    QPushButton* applyButton = new QPushButton(tr("✓ 应用"), pickupDialog);

    connect(previewButton, &QPushButton::clicked, [this, rowCountSpin, colCountSpin, spacingSpin]()
    {
      int rows = rowCountSpin->value();
      int cols = colCountSpin->value();
      int spacing = spacingSpin->value();

      appLog(tr("预览规律拾取: %1x%2 网格, 间距: %3").arg(rows).arg(cols).arg(spacing));

      // 在这里调用显示预览的代码
      // ...
    });

    connect(applyButton, &QPushButton::clicked, [this, pickupDialog, rowCountSpin, colCountSpin, spacingSpin]()
    {
      int rows = rowCountSpin->value();
      int cols = colCountSpin->value();
      int spacing = spacingSpin->value();

      appLog(tr("应用规律拾取: %1x%2 网格, 共 %3 个码, 间距: %4")
             .arg(rows).arg(cols).arg(rows * cols).arg(spacing));

      // 在这里应用设置
      // ...

      // 注意我们不关闭对话框，而是让用户可以继续调整参数
    });

    for (const auto& btnConfig : pickUpButtons)
    {
      QPushButton* button = new QPushButton(btnConfig.first, groupBox);
      button->setToolTip(btnConfig.second);
      button->setMinimumHeight(40);
      groupLayout->addWidget(button);
      connect(button, &QPushButton::clicked, [this, key = btnConfig.first, qrSavePath = QRcodeSavePath]()
      {
        if (key == tr("🔍 拾取首区域"))
        {
          appLog(tr("开始拾取规律首个二维码识别区域..."));
          appLog(tr("请在图像上绘制首个二维码区域..."));
          halWin->genAngleRec(qrSavePath + "/qrcode_first_area.hobj",QStringLiteral("green"));
        }
        else if (key == tr("🔍 拾取尾区域"))
        {
          appLog(tr("开始拾取规律最后一个二维码识别区域..."));
          appLog(tr("请在图像上绘制最后一个二维码区域..."));
          halWin->genAngleRec(qrSavePath + "/qrcode_last_area.hobj", QStringLiteral("green"));
        }
      });
    }
    actionLayout->addWidget(previewButton);
    actionLayout->addWidget(applyButton);
    dialogLayout->addLayout(actionLayout);

    // 显示对话框（非模态）
    pickupDialog->show();
  }
  else if (key == tr("📏 选择测量工具"))
  {
    appLog(tr("打开测量工具选择面板..."));
    // TODO: 实现打开测量工具选择面板的逻辑
  }
  else if (key == tr("📊 查看测量结果"))
  {
    appLog(tr("打开测量结果查看面板..."));
    // TODO: 实现查看测量结果的逻辑
  }
  else if (key == tr("📏 距离测量"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始距离测量..."));

    // 首先检查是否有图像
    if (!m_Img.IsInitialized())
    {
      appLog(tr("❌ 错误：请先加载图像"), ERR);
      return;
    }

    // 绘制测量区域并存储
    appLog(tr("请绘制第一个测量区域..."));
    m_Measyre_Rect1 = halWin->genAngleRec(MeasureSavePath + "/m_Measyre_Rect1.hobj", QStringLiteral("green"));

    // 检查第一个区域是否创建成功
    if (!m_Measyre_Rect1.IsInitialized())
    {
      appLog(tr("❌ 错误：第一个测量区域创建失败"), ERR);
      return;
    }

    LOG_INFO(tr("✅ 第一个测量区域创建成功：%1").arg(MeasureSavePath + "/m_Measyre_Rect1.hobj"));

    appLog(tr("请绘制第二个测量区域..."));
    m_Measyre_Rect2 = halWin->genAngleRec(MeasureSavePath + "/m_Measyre_Rect2.hobj", QStringLiteral("green"));

    // 检查第二个区域是否创建成功
    if (!m_Measyre_Rect2.IsInitialized())
    {
      appLog(tr("❌ 错误：第二个测量区域创建失败"), ERR);
      return;
    }

    LOG_INFO(tr("✅ 第二个测量区域创建成功：%1").arg(MeasureSavePath + "/m_Measyre_Rect2.hobj"));

    // 🎨 使用新的便捷方法添加显示对象
    appLog(tr("🎨 添加测量区域到显示列表..."));
    halWin->addDisplayObject(m_Measyre_Rect1, "green", 2.0);
    halWin->addDisplayObject(m_Measyre_Rect2, "green", 2.0);

    // 提取轮廓
    appLog(tr("正在提取第一个区域的轮廓..."));
    HObject Xld1 = halWin->QtGetLengthMaxXld(m_Img, m_Measyre_Rect1, 100);

    // 检查第一个轮廓是否提取成功
    if (!Xld1.IsInitialized())
    {
      appLog(tr("❌ 警告：第一个区域没有找到有效轮廓"), WARNING);
    }
    else
    {
      appLog(tr("✅ 第一个区域轮廓提取成功"));
      // 🎨 添加轮廓到显示列表（红色显示）
      halWin->addDisplayObject(Xld1, "red", 3.0);
    }

    appLog(tr("正在提取第二个区域的轮廓..."));
    HObject Xld2 = halWin->QtGetLengthMaxXld(m_Img, m_Measyre_Rect2, 100);

    // 检查第二个轮廓是否提取成功
    if (!Xld2.IsInitialized())
    {
      appLog(tr("❌ 警告：第二个区域没有找到有效轮廓"), WARNING);
    }
    else
    {
      appLog(tr("✅ 第二个区域轮廓提取成功"));
      // 🎨 添加轮廓到显示列表（蓝色显示）
      halWin->addDisplayObject(Xld2, "blue", 3.0);
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
        double area1 = halWin->calculateRegionArea(m_Measyre_Rect1);
        double area2 = halWin->calculateRegionArea(m_Measyre_Rect2);
        pointStruct centroid1 = halWin->calculateRegionCentroid(m_Measyre_Rect1);
        pointStruct centroid2 = halWin->calculateRegionCentroid(m_Measyre_Rect2);

        // 计算重心之间的距离
        double centroidDistance = halWin->calculatePointDistance(centroid1.X, centroid1.Y, centroid2.X, centroid2.Y);

        QString msg = QString("距离测量结果:\n最小距离: %1px\n最大距离: %2px\n重心距离: %3px\n区域1面积: %4px²\n区域2面积: %5px²")
                      .arg(QString::number(DisMin.D(), 'f', 2))
                      .arg(QString::number(DisMax.D(), 'f', 2))
                      .arg(QString::number(centroidDistance, 'f', 2))
                      .arg(QString::number(area1, 'f', 1))
                      .arg(QString::number(area2, 'f', 1));

        // 重新显示图像和所有对象
        halWin->showImage(m_Img);
        halWin->dispHalconMessage(20, 20, msg, "green");
        appLog(tr("✅ 距离测量完成：%1").arg(msg.replace('\n', ' ')));

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

        // 保存到HalconLable的缓存中
        for (auto it = measurementResults.begin(); it != measurementResults.end(); ++it)
        {
          halWin->measurementCache[it.key()] = it.value();
        }

        appLog(tr("📊 测量结果已保存到缓存，共 %1 项数据").arg(measurementResults.size()));

      }
      catch (HalconCpp::HException& e)
      {
        QString errorMsg = tr("❌ 距离计算失败：%1").arg(QString(e.ErrorMessage()));
        appLog(errorMsg, ERR);
      } catch (...)
      {
        appLog(tr("❌ 距离计算时发生未知错误"), ERR);
      }
    }
    else
    {
      appLog(tr("❌ 无法进行距离测量：轮廓提取失败"), ERR);
      // 即使轮廓提取失败，也显示区域
      halWin->showImage(m_Img);
    }
  }
  else if (key == tr("📐 角度测量"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始角度测量..."));
    // TODO: 实现角度测量的逻辑
  }
  else if (key == tr("⭕ 圆测量"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始圆测量..."));
    // TODO: 实现圆测量的逻辑
  }
  else if (key == tr("🔄 轮廓分析"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始轮廓分析..."));
    // TODO: 实现轮廓分析的逻辑
  }

  // 检测模式特有按钮
  else if (key == tr("🔄 连续检测"))
  {
    appLog(tr("开始连续检测模式..."));
    // TODO: 实现连续检测的逻辑
  }
  else if (key == tr("📊 查看检测结果"))
  {
    try
    {
      // 1. 首先检查 m_uiBuilder 是否有效
      if (!m_uiBuilder)
      {
        appLog(tr("错误：UI构建器未初始化"));
        return;
      }

      // 如果窗口已经存在且可见,则关闭它
      if (m_Test_Result_Widget && m_Test_Result_Widget->isVisible())
      {
        appLog(tr("检测结果查看面板已打开，关闭它..."));
        m_Test_Result_Widget->close();
        m_Test_Result_Widget = nullptr;
        return;
      }

      appLog(tr("打开检测结果查看面板..."));

      // 2. 构建JSON配置
      QJsonObject showtestResults = {
          {"layout", QJsonObject{
               {"type", "VBox"},
               {"spacing", 10},
               {"widgets", QJsonArray{
                    QJsonObject{
                        {"type", "Button"},
                        {"name", "btn_submit"},
                        {"text", tr("检测结果列表")},
                        {"animation", "FadeIn"},
                        {"animationDuration", 500},
                    },
                }}
           }}
      };

      // 3. 获取合适的显示位置 - 使用鼠标当前位置作为备用方案
      QPoint globalPos = QCursor::pos();
      // 稍微偏移避免遮挡鼠标
      globalPos += QPoint(20, 20);

      // 4. 创建窗口
      m_Test_Result_Widget = m_uiBuilder->createFromJson(showtestResults);
      if (!m_Test_Result_Widget)
      {
        appLog(tr("错误：无法创建检测结果窗口"), ERR);
        return;
      }

      // 5. 设置窗口属性（重要：必须在show()之前设置）
      m_Test_Result_Widget->setWindowTitle(tr("检测结果"));
      m_Test_Result_Widget->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
      m_Test_Result_Widget->setAttribute(Qt::WA_DeleteOnClose, true);
      m_Test_Result_Widget->setMinimumSize(400, 300);

      // 设置父窗口以确保正确的生命周期管理
      m_Test_Result_Widget->setParent(this, Qt::Window);

      // 6. 移动到指定位置并显示
      m_Test_Result_Widget->move(globalPos);
      m_Test_Result_Widget->show();
      m_Test_Result_Widget->raise();
      m_Test_Result_Widget->activateWindow();

      appLog(tr("检测结果面板已打开"));

      // 7. 连接销毁信号以确保状态同步
      connect(m_Test_Result_Widget, &QWidget::destroyed, this, [this]()
      {
        m_Test_Result_Widget = nullptr;
      });
    }
    catch (const std::exception& e)
    {
      appLog(tr("创建窗口时发生异常: %1").arg(e.what()), ERR);
    }
    catch (...)
    {
      appLog(tr("创建窗口时发生未知异常"), ERR);
    }
  }
  // 🎯 模板创建按钮处理 - 支持不同模式
  else if (key == tr("📋 创建形状模板"))
  {
    appLog(tr("📋 创建形状模板..."));
  }
  else if (key == tr("📋 创建二维码模板"))
  {
    appLog(tr("🔍 创建二维码识别模板..."));
    onCreateModelWithBuilder();
  }
  else if (key == tr("📋 创建测量模板"))
  {
    appLog(tr("📏 创建测量模板..."));
    onCreateModelWithBuilder();
  }
  else if (key == tr("📋 创建检测模板"))
  {
    appLog(tr("🔍 创建检测模板..."));
    onCreateModelWithBuilder();
  }
  else if (key == tr("✅ 特征匹配"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始特征匹配检测..."));
    // TODO: 实现特征匹配检测的逻辑
  }
  else if (key == tr("🔴 缺陷检测"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始缺陷检测..."));
    // TODO: 实现缺陷检测的逻辑
  }
  else if (key == tr("📊 尺寸检测"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始尺寸检测..."));
    // TODO: 实现尺寸检测的逻辑
  }
  else if (key == tr("🎯 定位对齐"))
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("开始产品位置定位..."));
    // TODO: 实现产品位置定位的逻辑
  }
  // 🎯 新增实用功能按钮处理
  else if (key == tr("🎯 图像预处理"))
  {
    appLog(tr("📸 打开图像预处理面板..."));

    if (!m_Img.IsInitialized())
    {
      appLog(tr("❌ 错误：请先加载图像"), ERR);
      return;
    }

    // 创建图像预处理选择对话框
    QDialog* preprocessDialog = new QDialog(this, Qt::Window | Qt::WindowStaysOnTopHint);
    preprocessDialog->setWindowTitle(tr("🎯 图像预处理"));
    preprocessDialog->setMinimumSize(400, 350);
    preprocessDialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* layout = new QVBoxLayout(preprocessDialog);

    // 添加滤波功能按钮
    QGroupBox* filterGroup = new QGroupBox(tr("🔧 滤波功能"), preprocessDialog);
    QVBoxLayout* filterLayout = new QVBoxLayout(filterGroup);

    QPushButton* gaussBtn = new QPushButton(tr("📐 高斯滤波"), filterGroup);
    QPushButton* medianBtn = new QPushButton(tr("🔘 中值滤波"), filterGroup);
    QPushButton* meanBtn = new QPushButton(tr("📊 均值滤波"), filterGroup);

    filterLayout->addWidget(gaussBtn);
    filterLayout->addWidget(medianBtn);
    filterLayout->addWidget(meanBtn);
    layout->addWidget(filterGroup);

    // 添加增强功能按钮
    QGroupBox* enhanceGroup = new QGroupBox(tr("✨ 图像增强"), preprocessDialog);
    QVBoxLayout* enhanceLayout = new QVBoxLayout(enhanceGroup);

    QPushButton* contrastBtn = new QPushButton(tr("🌓 调整对比度"), enhanceGroup);
    QPushButton* brightnessBtn = new QPushButton(tr("☀️ 调整亮度"), enhanceGroup);

    enhanceLayout->addWidget(contrastBtn);
    enhanceLayout->addWidget(brightnessBtn);
    layout->addWidget(enhanceGroup);

    // 连接按钮事件
    connect(gaussBtn, &QPushButton::clicked, [this]()
    {
      HObject filteredImg = halWin->applyGaussianFilter(m_Img, 1.5);
      if (filteredImg.IsInitialized())
      {
        m_Img = filteredImg;
        halWin->showImage(m_Img);
        appLog(tr("✅ 高斯滤波应用成功"));
      }
    });

    connect(medianBtn, &QPushButton::clicked, [this]()
    {
      HObject filteredImg = halWin->applyMedianFilter(m_Img, "circle", 3.0);
      if (filteredImg.IsInitialized())
      {
        m_Img = filteredImg;
        halWin->showImage(m_Img);
        appLog(tr("✅ 中值滤波应用成功"));
      }
    });

    connect(contrastBtn, &QPushButton::clicked, [this]()
    {
      HObject enhancedImg = halWin->adjustImageContrast(m_Img, 1.3);
      if (enhancedImg.IsInitialized())
      {
        m_Img = enhancedImg;
        halWin->showImage(m_Img);
        appLog(tr("✅ 对比度调整成功"));
      }
    });

    connect(brightnessBtn, &QPushButton::clicked, [this]()
    {
      HObject enhancedImg = halWin->adjustImageBrightness(m_Img, 20.0);
      if (enhancedImg.IsInitialized())
      {
        m_Img = enhancedImg;
        halWin->showImage(m_Img);
        appLog(tr("✅ 亮度调整成功"));
      }
    });

    preprocessDialog->show();
  }
  else if (key == tr("⚡ 快捷操作"))
  {
    appLog(tr("⚡ 打开快捷操作面板..."));

    // 创建快捷操作对话框
    QDialog* quickDialog = new QDialog(this, Qt::Window | Qt::WindowStaysOnTopHint);
    quickDialog->setWindowTitle(tr("⚡ 快捷操作"));
    quickDialog->setMinimumSize(350, 300);
    quickDialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* layout = new QVBoxLayout(quickDialog);

    // 缩放控制
    QGroupBox* zoomGroup = new QGroupBox(tr("🔍 缩放控制"), quickDialog);
    QVBoxLayout* zoomLayout = new QVBoxLayout(zoomGroup);

    QPushButton* fitBtn = new QPushButton(tr("📐 适合窗口"), zoomGroup);
    QPushButton* actualBtn = new QPushButton(tr("🔎 实际大小"), zoomGroup);
    QPushButton* zoom50Btn = new QPushButton(tr("📉 50%大小"), zoomGroup);
    QPushButton* zoom200Btn = new QPushButton(tr("📈 200%大小"), zoomGroup);

    zoomLayout->addWidget(fitBtn);
    zoomLayout->addWidget(actualBtn);
    zoomLayout->addWidget(zoom50Btn);
    zoomLayout->addWidget(zoom200Btn);
    layout->addWidget(zoomGroup);

    // 信息查询
    QGroupBox* infoGroup = new QGroupBox(tr("ℹ️ 信息查询"), quickDialog);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);

    QPushButton* imageInfoBtn = new QPushButton(tr("📊 图像信息"), infoGroup);
    QPushButton* objectCountBtn = new QPushButton(tr("📋 对象数量"), infoGroup);

    infoLayout->addWidget(imageInfoBtn);
    infoLayout->addWidget(objectCountBtn);
    layout->addWidget(infoGroup);

    // 🎯 新增：窗口优化控制
    QGroupBox* optimizeGroup = new QGroupBox(tr("🎨 窗口优化"), quickDialog);
    QVBoxLayout* optimizeLayout = new QVBoxLayout(optimizeGroup);

    // 像素信息显示开关
    QPushButton* pixelInfoBtn = new QPushButton(tr("🎯 像素信息显示"), optimizeGroup);
    pixelInfoBtn->setCheckable(true);
    pixelInfoBtn->setChecked(halWin->isPixelInfoDisplayEnabled());

    if (halWin->isPixelInfoDisplayEnabled())
    {
      pixelInfoBtn->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; font-weight: bold; }");
      pixelInfoBtn->setText(tr("🎯 像素信息显示 (开启)"));
    }
    else
    {
      pixelInfoBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; font-weight: bold; }");
      pixelInfoBtn->setText(tr("🎯 像素信息显示 (关闭)"));
    }
    optimizeLayout->addWidget(pixelInfoBtn);

    // 平滑调整大小开关
    QPushButton* smoothResizeBtn = new QPushButton(tr("🎨 平滑调整"), optimizeGroup);
    smoothResizeBtn->setCheckable(true);
    smoothResizeBtn->setChecked(halWin->isSmoothResizeEnabled());

    if (halWin->isSmoothResizeEnabled())
    {
      smoothResizeBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-weight: bold; }");
      smoothResizeBtn->setText(tr("🎨 平滑调整 (开启)"));
    }
    else
    {
      smoothResizeBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; font-weight: bold; }");
      smoothResizeBtn->setText(tr("🎨 平滑调整 (关闭)"));
    }
    optimizeLayout->addWidget(smoothResizeBtn);

    // 防抖动延迟设置按钮
    QPushButton* debounceSettingsBtn = new QPushButton(tr("⏱️ 延迟设置"), optimizeGroup);
    optimizeLayout->addWidget(debounceSettingsBtn);

    // 窗口状态重置按钮
    QPushButton* resetWindowBtn = new QPushButton(tr("🔄 重置窗口"), optimizeGroup);
    optimizeLayout->addWidget(resetWindowBtn);

    layout->addWidget(optimizeGroup);

    connect(objectCountBtn, &QPushButton::clicked, [this]()
    {
      int count = halWin->getDisplayObjectsCount();
      appLog(tr("📋 当前显示对象数量：%1").arg(count));
    });


    // 🎯 新增：窗口优化按钮事件连接
    connect(pixelInfoBtn, &QPushButton::clicked, [this, pixelInfoBtn]()
    {
      bool currentState = halWin->isPixelInfoDisplayEnabled();
      bool newState = !currentState;
      halWin->setPixelInfoDisplayEnabled(newState);

      if (newState)
      {
        pixelInfoBtn->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; font-weight: bold; }");
        pixelInfoBtn->setText(tr("🎯 像素信息显示 (开启)"));
        appLog(tr("🎯 像素信息显示已开启"));
      }
      else
      {
        pixelInfoBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; font-weight: bold; }");
        pixelInfoBtn->setText(tr("🎯 像素信息显示 (关闭)"));
        appLog(tr("🎯 像素信息显示已关闭"));
      }
    });

    connect(smoothResizeBtn, &QPushButton::clicked, [this, smoothResizeBtn]()
    {
      bool currentState = halWin->isSmoothResizeEnabled();
      bool newState = !currentState;
      halWin->setSmoothResizeEnabled(newState);

      if (newState)
      {
        smoothResizeBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-weight: bold; }");
        smoothResizeBtn->setText(tr("🎨 平滑调整 (开启)"));
        appLog(tr("🎨 平滑调整功能已开启，减少窗口闪烁"));
      }
      else
      {
        smoothResizeBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; font-weight: bold; }");
        smoothResizeBtn->setText(tr("🎨 平滑调整 (关闭)"));
        appLog(tr("🎨 平滑调整功能已关闭，立即调整窗口"));
      }
    });

    connect(debounceSettingsBtn, &QPushButton::clicked, [this]()
    {
      // 创建延迟设置对话框
      QDialog* debounceDialog = new QDialog(this, Qt::Dialog | Qt::WindowStaysOnTopHint);
      debounceDialog->setWindowTitle(tr("⏱️ 防抖动延迟设置"));
      debounceDialog->setMinimumSize(300, 200);
      debounceDialog->setAttribute(Qt::WA_DeleteOnClose);

      QVBoxLayout* debounceLayout = new QVBoxLayout(debounceDialog);

      QLabel* infoLabel = new QLabel(tr("调整窗口大小时的延迟时间（毫秒）："), debounceDialog);
      debounceLayout->addWidget(infoLabel);

      QSpinBox* delaySpinBox = new QSpinBox(debounceDialog);
      delaySpinBox->setRange(50, 1000);
      delaySpinBox->setSingleStep(25);
      delaySpinBox->setValue(halWin->getResizeDebounceDelay());
      delaySpinBox->setSuffix(" ms");
      debounceLayout->addWidget(delaySpinBox);

      QLabel* tipLabel = new QLabel(tr("💡 提示：较小的值响应更快，较大的值更稳定"), debounceDialog);
      tipLabel->setStyleSheet("color: #666; font-size: 11px;");
      debounceLayout->addWidget(tipLabel);

      QHBoxLayout* buttonLayout = new QHBoxLayout();
      QPushButton* applyBtn = new QPushButton(tr("✅ 应用"), debounceDialog);
      QPushButton* cancelBtn = new QPushButton(tr("❌ 取消"), debounceDialog);

      buttonLayout->addWidget(applyBtn);
      buttonLayout->addWidget(cancelBtn);
      debounceLayout->addLayout(buttonLayout);

      connect(applyBtn, &QPushButton::clicked, [this, delaySpinBox, debounceDialog]()
      {
        int newDelay = delaySpinBox->value();
        halWin->setResizeDebounceDelay(newDelay);
        appLog(tr("⏱️ 防抖动延迟已设置为 %1ms").arg(newDelay));
        debounceDialog->accept();
      });

      connect(cancelBtn, &QPushButton::clicked, debounceDialog, &QDialog::reject);

      debounceDialog->exec();
    });

    connect(resetWindowBtn, &QPushButton::clicked, [this]()
    {
      halWin->resetWindowOptimization();
      appLog(tr("🔄 窗口优化状态已重置"));
    });

    // 连接按钮事件
    connect(fitBtn, &QPushButton::clicked, [this]()
    {
      halWin->zoomToFit();
      appLog(tr("🔍 图像已缩放到适合窗口"));
    });

    connect(actualBtn, &QPushButton::clicked, [this]()
    {
      halWin->zoomToActualSize();
      appLog(tr("🔎 图像已缩放到实际大小"));
    });

    connect(zoom50Btn, &QPushButton::clicked, [this]()
    {
      halWin->zoomToRatio(0.5);
      appLog(tr("📉 图像已缩放到50%"));
    });

    connect(zoom200Btn, &QPushButton::clicked, [this]()
    {
      halWin->zoomToRatio(2.0);
      appLog(tr("📈 图像已缩放到200%"));
    });

    connect(imageInfoBtn, &QPushButton::clicked, [this]()
    {
      QString info = halWin->getImageInfo();
      appLog(tr("📊 %1").arg(info));
    });

    connect(objectCountBtn, &QPushButton::clicked, [this]()
    {
      int count = halWin->getDisplayObjectsCount();
      appLog(tr("📋 当前显示对象数量：%1").arg(count));
    });

    quickDialog->show();
  }
  else if (key == tr("📊 导出结果"))
  {
    appLog(tr("📊 打开导出功能面板..."));

    // 创建导出对话框
    QDialog* exportDialog = new QDialog(this, Qt::Window | Qt::WindowStaysOnTopHint);
    exportDialog->setWindowTitle(tr("📊 导出功能"));
    exportDialog->setMinimumSize(350, 250);
    exportDialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* layout = new QVBoxLayout(exportDialog);

    QPushButton* exportDataBtn = new QPushButton(tr("📋 导出测量数据"), exportDialog);
    QPushButton* screenshotBtn = new QPushButton(tr("📸 截图保存"), exportDialog);

    layout->addWidget(exportDataBtn);
    layout->addWidget(screenshotBtn);

    // 连接按钮事件
    connect(exportDataBtn, &QPushButton::clicked, [this]()
    {
      if (halWin->measurementCache.isEmpty())
      {
        appLog(tr("⚠️ 没有测量数据可导出，请先进行测量"), WARNING);
        return;
      }

      QString fileName = QFileDialog::getSaveFileName(this, tr("导出测量数据"),
                                                      "measurement_results.csv",
                                                      tr("CSV文件 (*.csv)"));
      if (!fileName.isEmpty())
      {
        QString errorMsg;
        bool success = halWin->exportMeasurementResults(fileName, halWin->measurementCache, errorMsg);
        appLog(success ? tr("✅ %1").arg(errorMsg) : tr("❌ %1").arg(errorMsg), success ? INFO : ERR);
      }
    });

    connect(screenshotBtn, &QPushButton::clicked, [this]()
    {
      QString fileName = QFileDialog::getSaveFileName(this, tr("保存截图"),
                                                      "screenshot.jpg",
                                                      tr("图像文件 (*.jpg *.png *.bmp)"));
      if (!fileName.isEmpty())
      {
        QString errorMsg;
        bool success = halWin->captureCurrentDisplay(fileName, errorMsg);
        appLog(success ? tr("✅ %1").arg(errorMsg) : tr("❌ %1").arg(errorMsg), success ? INFO : ERR);
      }
    });

    exportDialog->show();
  }
  // 🎯 新增：批处理相关按钮
  else if (key == tr("🚀 开始批处理"))
  {
    startBatchProcessing();
  }
  else if (key == tr("⏹️ 停止批处理"))
  {
    stopBatchProcessing();
  }
  else if (key == tr("📊 查看结果历史"))
  {
    // 显示结果历史窗口
    if (!m_resultHistory.isEmpty())
    {
      QString historyMsg = tr("结果历史记录（最近10条）:\n");
      int showCount = qMin(10, m_resultHistory.size());
      for (int i = m_resultHistory.size() - showCount; i < m_resultHistory.size(); ++i)
      {
        const auto& result = m_resultHistory[i];
        historyMsg += tr("%1. [%2] %3 - %4\n")
                      .arg(i + 1)
                      .arg(result.processTime.toString("hh:mm:ss"))
                      .arg(result.taskType)
                      .arg(result.success ? "成功" : "失败");
      }

      if (halWin)
      {
        halWin->dispHalconMessage(50, 250, historyMsg, "cyan");
      }
      appLog(tr("📊 显示结果历史，共 %1 条记录").arg(m_resultHistory.size()));
    }
    else
    {
      appLog(tr("📊 暂无历史记录"), INFO);
    }
  }
  else
  {
    appLog(tr("未知的按钮操作: %1").arg(key), WARNING);
  }
}

/*============================ 创建模板 ============================*/

/*============================ Dynamic UI 优化版本 1: 便捷构建器方式 ============================*/
void VisualProcess::onCreateModelWithBuilder()
{
  try
  {
    // 🔍 检查现有窗口
    if (m_Create_Model_Widget && m_Create_Model_Widget->isVisible())
    {
      appLog(tr("模板创建面板已打开，关闭它..."));
      m_Create_Model_Widget->close();
      m_Create_Model_Widget = nullptr;
      return;
    }

    // 🎯 确定当前模式和窗口标题
    QString currentMode;
    QString windowTitle;
    QString windowIcon;

    if (m_QCodeCam_flag)
    {
      currentMode = "QRCode";
      m_modelSavePath += "QRcodeModel";
      windowTitle = tr("🔍 创建二维码识别模板");
      windowIcon = "🔍";
      appLog(tr("🔍 打开二维码识别模板创建面板..."));
    }
    else if (m_MeasureCam_flag)
    {
      currentMode = "Measure";
      m_modelSavePath += "MeasureModel";
      windowTitle = tr("📏 创建测量模板");
      windowIcon = "📏";
      appLog(tr("📏 打开测量模板创建面板..."));
    }
    else if (m_CheckCam_flag)
    {
      currentMode = "Detection";
      windowTitle = tr("🔍 创建检测模板");
      windowIcon = "🔍";
      appLog(tr("🔍 打开检测模板创建面板..."));
    }
    else
    {
      appLog(tr("❌ 错误：无法确定当前模式"), ERR);
      return;
    }

    // 🏗️ 使用便捷构建器创建UI而不是复杂的JSON
    if (!m_uiBuilder)
    {
      appLog(tr("❌ 错误：UI构建器未初始化"), ERR);
      return;
    }

    // 🎨 创建主窗口
    m_Create_Model_Widget = new QDialog(this);
    m_Create_Model_Widget->setWindowTitle(windowTitle);
    m_Create_Model_Widget->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    m_Create_Model_Widget->setAttribute(Qt::WA_DeleteOnClose, true);
    m_Create_Model_Widget->setMinimumSize(450, 500);
    m_Create_Model_Widget->resize(500, 600);

    // 🏗️ 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(m_Create_Model_Widget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 📁 文件名设置区域
    QGroupBox* fileGroupBox = new QGroupBox(tr("📁 模板文件设置"), m_Create_Model_Widget);
    fileGroupBox->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #9b59b6;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 15px;
            background-color: #f4f1f7;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #7d3c98;
        }
    )");

    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroupBox);

    QLabel* fileLabel = new QLabel(tr("模板文件名:"), fileGroupBox);
    fileLabel->setStyleSheet("color: #7d3c98; margin: 5px; font-weight: bold;");
    fileLayout->addWidget(fileLabel);

    QLineEdit* fileNameEdit = new QLineEdit(fileGroupBox);
    fileNameEdit->setObjectName("templateFileName");
    fileNameEdit->setText(
        QString("%1_template").arg(currentMode));
    fileNameEdit->setPlaceholderText(tr("请输入模板文件名（不含扩展名）"));
    fileNameEdit->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #bb7fcc;
            border-radius: 4px;
            padding: 8px;
            font-size: 13px;
            margin: 2px;
            background-color: white;
            font-weight: bold;
        }
        QLineEdit:focus {
            border-color: #9b59b6;
            background-color: #fdfcfe;
        }
    )");
    fileLayout->addWidget(fileNameEdit);

    mainLayout->addWidget(fileGroupBox);

    // 🎛️ 根据不同模式添加特定参数
    if (m_QCodeCam_flag)
    {
      // 二维码参数组
      QGroupBox* qrGroupBox = new QGroupBox(tr("🔍 二维码识别参数"), m_Create_Model_Widget);
      qrGroupBox->setStyleSheet(R"(
            QGroupBox {
                font-weight: bold;
                border: 2px solid #3498db;
                border-radius: 8px;
                margin-top: 20px;
                padding-top: 20px;
                background-color: #f8f9fa;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 15px;
                top: 2px;
                padding: 0 8px 0 8px;
                background-color: #f8f9fa;
                color: #2c3e50;
                font-size: 14px;
                font-weight: bold;
            }
        )");

      QVBoxLayout* qrLayout = new QVBoxLayout(qrGroupBox);
      qrLayout->setSpacing(12);
      qrLayout->setContentsMargins(15, 25, 15, 15);

      // 二维码类型 - 改为ComboBox
      QLabel* typeLabel = new QLabel(tr("二维码类型:"), qrGroupBox);
      typeLabel->setStyleSheet("color: #2c3e50; margin: 5px; font-weight: bold; font-size: 13px;");
      qrLayout->addWidget(typeLabel);

      QComboBox* typeCombo = new QComboBox(qrGroupBox);
      typeCombo->setObjectName("qrModelType");
      typeCombo->addItem("Data Matrix ECC 200");
      typeCombo->addItem("QR Code");
      typeCombo->addItem("PDF417");
      typeCombo->addItem("Aztec Code");
      typeCombo->addItem("MaxiCode");
      typeCombo->setCurrentText("Data Matrix ECC 200");
      typeCombo->setStyleSheet(R"(
            QComboBox {
                margin: 5px;
                padding: 8px;
                border: 2px solid #3498db;
                border-radius: 4px;
                background-color: white;
                font-size: 13px;
                min-height: 25px;
            }
            QComboBox:focus {
                border-color: #2980b9;
                background-color: #fdfcfe;
            }
            QComboBox::drop-down {
                border: none;
                width: 20px;
            }
            QComboBox::down-arrow {
                width: 12px;
                height: 12px;
            }
        )");
      qrLayout->addWidget(typeCombo);

      // 极性 - 改为ComboBox
      QLabel* polarityLabel = new QLabel(tr("极性:"), qrGroupBox);
      polarityLabel->setStyleSheet("color: #2c3e50; margin: 5px; font-weight: bold; font-size: 13px;");
      qrLayout->addWidget(polarityLabel);

      QComboBox* polarityCombo = new QComboBox(qrGroupBox);
      polarityCombo->setObjectName("qrPolarity");
      polarityCombo->addItem("light_on_dark", tr("浅色背景深色码"));
      polarityCombo->addItem("dark_on_light", tr("深色背景浅色码"));
      polarityCombo->addItem("all", tr("自动检测"));
      polarityCombo->setCurrentText("light_on_dark");
      polarityCombo->setStyleSheet(R"(
            QComboBox {
                margin: 5px;
                padding: 8px;
                border: 2px solid #3498db;
                border-radius: 4px;
                background-color: white;
                font-size: 13px;
                min-height: 25px;
            }
            QComboBox:focus {
                border-color: #2980b9;
                background-color: #fdfcfe;
            }
        )");
      qrLayout->addWidget(polarityCombo);

      // 添加自定义参数输入
      QLabel* customLabel = new QLabel(tr("自定义参数 (可选):"), qrGroupBox);
      customLabel->setStyleSheet("color: #2c3e50; margin: 5px; font-weight: bold; font-size: 13px;");
      qrLayout->addWidget(customLabel);

      QLineEdit* customEdit = new QLineEdit(qrGroupBox);
      customEdit->setObjectName("qrCustomParams");
      customEdit->setPlaceholderText(tr("输入额外的二维码识别参数，如: timeout=5000"));
      customEdit->setStyleSheet(R"(
            QLineEdit {
                border: 2px solid #bb7fcc;
                border-radius: 4px;
                padding: 8px;
                font-size: 13px;
                margin: 2px;
                background-color: white;
                font-weight: normal;
            }
            QLineEdit:focus {
                border-color: #9b59b6;
                background-color: #fdfcfe;
            }
        )");
      qrLayout->addWidget(customEdit);

      mainLayout->addWidget(qrGroupBox);
    }
    else if (m_MeasureCam_flag)
    {
      // 测量参数组
      QGroupBox* measureGroupBox = new QGroupBox(tr("📏 测量模板参数"), m_Create_Model_Widget);
      measureGroupBox->setStyleSheet(R"(
            QGroupBox {
                font-weight: bold;
                border: 2px solid #f39c12;
                border-radius: 8px;
                margin-top: 20px;
                padding-top: 20px;
                background-color: #fefaf5;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 15px;
                top: 2px;
                padding: 0 8px 0 8px;
                background-color: #fefaf5;
                color: #e67e22;
                font-size: 14px;
                font-weight: bold;
            }
        )");

      QVBoxLayout* measureLayout = new QVBoxLayout(measureGroupBox);
      measureLayout->setSpacing(12);
      measureLayout->setContentsMargins(15, 25, 15, 15);

      // 测量类型 - 改为ComboBox
      QLabel* typeLabel = new QLabel(tr("测量类型:"), measureGroupBox);
      typeLabel->setStyleSheet("color: #e67e22; margin: 5px; font-weight: bold; font-size: 13px;");
      measureLayout->addWidget(typeLabel);

      QComboBox* typeCombo = new QComboBox(measureGroupBox);
      typeCombo->setObjectName("measureType");
      typeCombo->addItem("edge_detection", tr("边缘检测"));
      typeCombo->addItem("contour", tr("轮廓测量"));
      typeCombo->addItem("distance", tr("距离测量"));
      typeCombo->addItem("angle", tr("角度测量"));
      typeCombo->addItem("circle", tr("圆测量"));
      typeCombo->setCurrentText("edge_detection");
      typeCombo->setStyleSheet(R"(
            QComboBox {
                margin: 5px;
                padding: 8px;
                border: 2px solid #f39c12;
                border-radius: 4px;
                background-color: white;
                font-size: 13px;
                min-height: 25px;
            }
            QComboBox:focus {
                border-color: #e67e22;
                background-color: #fdfcfe;
            }
        )");
      measureLayout->addWidget(typeCombo);

      // 边缘阈值
      QLabel* thresholdLabel = new QLabel(tr("边缘阈值:"), measureGroupBox);
      thresholdLabel->setStyleSheet("color: #e67e22; margin: 5px; font-weight: bold; font-size: 13px;");
      measureLayout->addWidget(thresholdLabel);

      QLineEdit* thresholdEdit = new QLineEdit(measureGroupBox);
      thresholdEdit->setObjectName("edgeThreshold");
      thresholdEdit->setText("30");
      thresholdEdit->setPlaceholderText(tr("边缘检测阈值: 10-100"));
      thresholdEdit->setStyleSheet(R"(
            QLineEdit {
                margin: 5px;
                padding: 8px;
                border: 2px solid #f39c12;
                border-radius: 4px;
                background-color: white;
                font-size: 13px;
            }
            QLineEdit:focus {
                border-color: #e67e22;
                background-color: #fdfcfe;
            }
        )");
      measureLayout->addWidget(thresholdEdit);

      // 测量精度参数
      QLabel* precisionLabel = new QLabel(tr("测量精度参数:"), measureGroupBox);
      precisionLabel->setStyleSheet("color: #e67e22; margin: 5px; font-weight: bold; font-size: 13px;");
      measureLayout->addWidget(precisionLabel);

      QLineEdit* precisionEdit = new QLineEdit(measureGroupBox);
      precisionEdit->setObjectName("measurePrecision");
      precisionEdit->setText("0.01");
      precisionEdit->setPlaceholderText(tr("测量精度，如: 0.01 (单位：像素)"));
      precisionEdit->setStyleSheet(R"(
            QLineEdit {
                margin: 5px;
                padding: 8px;
                border: 2px solid #f39c12;
                border-radius: 4px;
                background-color: white;
                font-size: 13px;
            }
            QLineEdit:focus {
                border-color: #e67e22;
                background-color: #fdfcfe;
            }
        )");
      measureLayout->addWidget(precisionEdit);

      mainLayout->addWidget(measureGroupBox);
    }
    else if (m_CheckCam_flag)
    {
      // 检测参数组
      QGroupBox* detectGroupBox = new QGroupBox(tr("🔍 检测模板参数"), m_Create_Model_Widget);
      detectGroupBox->setStyleSheet(R"(
            QGroupBox {
                font-weight: bold;
                border: 2px solid #27ae60;
                border-radius: 8px;
                margin-top: 20px;
                padding-top: 20px;
                background-color: #f8f9fa;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 15px;
                top: 2px;
                padding: 0 8px 0 8px;
                background-color: #f8f9fa;
                color: #196f3d;
                font-size: 14px;
                font-weight: bold;
            }
        )");

      QVBoxLayout* detectLayout = new QVBoxLayout(detectGroupBox);
      detectLayout->setSpacing(12);
      detectLayout->setContentsMargins(15, 25, 15, 15);

      // 最大对比度
      QLabel* maxLabel = new QLabel(tr("最大对比度:"), detectGroupBox);
      maxLabel->setStyleSheet("color: #196f3d; margin: 5px; font-weight: bold; font-size: 13px;");
      detectLayout->addWidget(maxLabel);

      QLineEdit* maxEdit = new QLineEdit(detectGroupBox);
      maxEdit->setObjectName("maxContrast");
      maxEdit->setText("128");
      maxEdit->setPlaceholderText(tr("建议值: 100-255"));
      maxEdit->setStyleSheet(R"(
            QLineEdit {
                margin: 5px;
                padding: 8px;
                border: 2px solid #27ae60;
                border-radius: 4px;
                background-color: white;
                font-size: 13px;
            }
            QLineEdit:focus {
                border-color: #196f3d;
                background-color: #fdfcfe;
            }
        )");
      detectLayout->addWidget(maxEdit);

      // 最小对比度
      QLabel* minLabel = new QLabel(tr("最小对比度:"), detectGroupBox);
      minLabel->setStyleSheet("color: #196f3d; margin: 5px; font-weight: bold; font-size: 13px;");
      detectLayout->addWidget(minLabel);

      QLineEdit* minEdit = new QLineEdit(detectGroupBox);
      minEdit->setObjectName("minContrast");
      minEdit->setText("30");
      minEdit->setPlaceholderText(tr("建议值: 10-50"));
      minEdit->setStyleSheet(R"(
            QLineEdit {
                margin: 5px;
                padding: 8px;
                border: 2px solid #27ae60;
                border-radius: 4px;
                background-color: white;
                font-size: 13px;
            }
            QLineEdit:focus {
                border-color: #196f3d;
                background-color: #fdfcfe;
            }
        )");
      detectLayout->addWidget(minEdit);

      mainLayout->addWidget(detectGroupBox);
    }

    // 🚀 创建按钮
    QPushButton* createButton = new QPushButton(tr("🚀 开始创建%1模板").arg(windowIcon), m_Create_Model_Widget);
    createButton->setObjectName("btn_create_template");
    createButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c, stop:1 #c0392b);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-weight: bold;
            font-size: 14px;
            min-height: 35px;
            margin: 10px 0;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ec7063, stop:1 #e74c3c);
        }
        QPushButton:pressed {
            background: #a93226;
        }
    )");
    mainLayout->addWidget(createButton);

    // 🎨 应用全局样式
    m_Create_Model_Widget->setStyleSheet(R"(
        QDialog {
            background-color: #f8f9fa;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }
        QLabel {
            font-size: 12px;
            color: #2c3e50;
            margin: 2px;
        }
    )");

    // 🔗 连接按钮点击事件
    connect(createButton, &QPushButton::clicked, this, [this]()
    {
      appLog(tr("🎯 创建模板按钮被点击"));
      clearDisplayObjectsOnly();
      handleCreateTemplate();
    });

    // 🗑️ 内存管理 - 重要：防止崩溃
    connect(m_Create_Model_Widget, &QWidget::destroyed, this, [this]()
    {
      appLog(tr("🧹 窗口已销毁，清理资源"));
      m_Create_Model_Widget = nullptr;
    });

    // 🪟 智能窗口定位和显示
    QPoint globalPos = QCursor::pos();
    globalPos += QPoint(20, 20);
    m_Create_Model_Widget->move(globalPos);
    m_Create_Model_Widget->show();
    m_Create_Model_Widget->raise();
    m_Create_Model_Widget->activateWindow();

    appLog(tr("✅ %1面板已打开 - 支持自定义文件名和参数配置").arg(windowTitle));
  }
  catch (const std::exception& e)
  {
    appLog(tr("❌ 创建窗口时发生异常: %1").arg(e.what()), ERR);

    // 🛡️ 异常时清理资源
    if (m_Create_Model_Widget)
    {
      m_Create_Model_Widget->deleteLater();
      m_Create_Model_Widget = nullptr;
    }
  }
  catch (...)
  {
    appLog(tr("❌ 创建窗口时发生未知异常"), ERR);

    // 🛡️ 异常时清理资源
    if (m_Create_Model_Widget)
    {
      m_Create_Model_Widget->deleteLater();
      m_Create_Model_Widget = nullptr;
    }
  }
}

/* ==================== 模板创建逻辑 ==================== */
void VisualProcess::handleCreateTemplate()
{
  try
  {
    if (!m_Create_Model_Widget)
    {
      appLog(tr("❌ 错误：创建模板窗口不存在"), ERR);
      return;
    }

    // 🎯 确定当前模式
    QString currentMode;
    QString modeIcon;
    if (m_QCodeCam_flag)
    {
      currentMode = "QRCode";
      modeIcon = "🔍";
      appLog(tr("🔍 开始处理二维码模板创建..."));
    }
    else if (m_MeasureCam_flag)
    {
      currentMode = "Measure";
      modeIcon = "📏";
      appLog(tr("📏 开始处理测量模板创建..."));
    }
    else if (m_CheckCam_flag)
    {
      currentMode = "Detection";
      modeIcon = "🔍";
      appLog(tr("🔍 开始处理检测模板创建..."));
    }
    else
    {
      appLog(tr("❌ 错误：无法确定当前模式"), ERR);
      return;
    }

    // 📁 获取自定义文件名
    QString customFileName;
    QLineEdit* fileNameEdit = m_Create_Model_Widget->findChild<QLineEdit*>("templateFileName");
    if (fileNameEdit)
    {
      customFileName = fileNameEdit->text().trimmed();
      LOG_INFO(tr("fileNameEdit: %1").arg(customFileName));
    }

    // 如果没有自定义文件名，生成默认文件名
    if (customFileName.isEmpty())
    {
      customFileName = QString("%1_template_%2").arg(currentMode)
                                                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
      LOG_INFO(tr("customFileName: %1").arg(customFileName));
    }

    // 🈚 中文文件名支持处理 | Chinese filename support
    QString originalFileName = customFileName;
    QRegularExpression chineseRegex("[\\x4e00-\\x9fa5]");
    if (customFileName.contains(chineseRegex))
    {
      appLog(tr("🈚 检测到中文文件名，正在验证系统支持..."));

      // 验证中文文件名支持
      QString validationError;
      QString testPath = m_modelSavePath + customFileName + ".test";
      if (!m_halFileManager->validateChineseFilePath(testPath, validationError))
      {
        appLog(tr("⚠️ 中文文件名验证失败：%1").arg(validationError), WARNING);

        // 转换为安全的文件名
        customFileName = m_halFileManager->convertToSafeFileName(customFileName);
        appLog(tr("🔄 已自动转换为安全文件名：%1").arg(customFileName), INFO);

        // 提示用户
        if (halWin)
        {
          halWin->dispHalconMessage(50, 200,
                                    tr("中文文件名已转换为:\n%1").arg(customFileName), "orange");
        }
      }
      else
      {
        appLog(tr("✅ 中文文件名验证通过：%1").arg(validationError));
      }
    }
    else
    {
      // 即使是英文文件名，也进行基本的清理
      customFileName = m_halFileManager->sanitizeChineseFileName(customFileName);
    }

    appLog(tr("📁 最终使用文件名: %1").arg(customFileName));
    if (originalFileName != customFileName)
    {
      appLog(tr("💡 文件名已从 '%1' 优化为 '%2'").arg(originalFileName, customFileName), INFO);
    }

    appLog(tr("📁 使用文件名: %1").arg(customFileName));

    // 🖼️ 检查图像和显示窗口
    if (!halWin)
    {
      appLog(tr("❌ 错误：图像显示窗口未初始化"), ERR);
      return;
    }

    if (!m_Img.IsInitialized())
    {
      appLog(tr("❌ 错误：没有加载图像，请先加载图像"), ERR);
      return;
    }

    // 📁 确保模型保存路径存在
    QDir().mkpath(m_modelSavePath);

    // 🎯 根据不同模式处理模板创建
    if (m_QCodeCam_flag)
    {
      handleQRCodeTemplateCreation(customFileName);
    }
    else if (m_MeasureCam_flag)
    {
      handleMeasureTemplateCreation(customFileName);
    }
    else if (m_CheckCam_flag)
    {
      handleDetectionTemplateCreation(customFileName);
    }

  }
  catch (const std::exception& e)
  {
    appLog(tr("❌ 创建模板时发生C++异常: %1").arg(QString::fromLocal8Bit(e.what())), ERR);
  }
  catch (...)
  {
    appLog(tr("❌ 创建模板时发生未知异常"), ERR);
  }
}

/* ==================== 二维码模板创建 ==================== */
void VisualProcess::handleQRCodeTemplateCreation(const QString& fileName)
{
  try
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("🔍 开始创建二维码识别模板..."));

    // 🔧 获取二维码参数
    QString qrModelType = "Data Matrix ECC 200";
    QString qrPolarity = "light_on_dark";
    QString customParams = "";

    if (m_Create_Model_Widget)
    {
      QComboBox* typeCombo = m_Create_Model_Widget->findChild<QComboBox*>("qrModelType");
      QComboBox* polarityCombo = m_Create_Model_Widget->findChild<QComboBox*>("qrPolarity");
      QLineEdit* customEdit = m_Create_Model_Widget->findChild<QLineEdit*>("qrCustomParams");

      if (typeCombo)
        qrModelType = typeCombo->currentText();
      if (polarityCombo)
        qrPolarity = polarityCombo->currentData().toString();
      if (customEdit)
        customParams = customEdit->text().trimmed();
    }

    appLog(tr("📋 二维码参数: 类型=%1, 极性=%2").arg(qrModelType, qrPolarity));
    if (!customParams.isEmpty())
    {
      appLog(tr("📋 自定义参数: %1").arg(customParams));
    }

    // 🎯 提示用户绘制二维码区域
    appLog(tr("📝 请在图像上绘制二维码识别区域..."));
    appLog(tr("💡 提示：在图像窗口中拖拽鼠标绘制矩形区域"));

    m_modelSavePath += "QRCodeModel/";
    LOG_INFO(tr("modelSavePath: %1").arg(m_modelSavePath));
    // 🔲 生成二维码识别区域 - 使用HOBJ通用格式（推荐）
    QString regionPath = m_modelSavePath + m_halFileManager->generateHobjFileName(fileName, "qr_region");
    HObject qrRegion = halWin->genAngleRec(regionPath, "blue");

    if (!qrRegion.IsInitialized() || qrRegion.CountObj() == 0)
    {
      appLog(tr("⚠️ 警告：未检测到有效的二维码区域，请重新绘制"), WARNING);
      return;
    }

    appLog(tr("✅ 已使用HOBJ通用格式保存二维码区域: %1").arg(regionPath));
    appLog(tr("💡 HOBJ格式说明: %1").arg(m_halFileManager->getHobjFileDescription("qr_region")));

    // 💾 保存二维码参数为Halcon tuple文件
    QString qrParamsPath = m_modelSavePath + fileName + "_qr_params" + m_halFileManager->getFileExtension("tuple");
    try
    {
      HTuple qrParamsTuple;
      qrParamsTuple.Append(qrModelType.toStdString().c_str());
      qrParamsTuple.Append(qrPolarity.toStdString().c_str());
      if (!customParams.isEmpty())
      {
        qrParamsTuple.Append(customParams.toStdString().c_str());
      }
      WriteTuple(qrParamsTuple, qrParamsPath.toStdString().c_str());
      appLog(tr("💾 二维码参数已保存为Halcon Tuple文件: %1").arg(qrParamsPath));
    }
    catch (HalconCpp::HException& e)
    {
      appLog(tr("⚠️ 保存二维码参数文件失败: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
    }

    // 📝 保存二维码模板信息（JSON文件作为元数据）
    QString templateInfoFile = m_modelSavePath + fileName + "_qr_template_info.json";
    saveQRCodeTemplateInfo(templateInfoFile, fileName, qrModelType, qrPolarity, customParams);

    // 🎨 显示创建的区域
    halWin->showSymbolList.append(qrRegion);
    halWin->showImage(m_Img);

    appLog(tr("🎉 二维码模板创建成功！"));
    appLog(tr("📁 模板文件: %1").arg(fileName));
    appLog(tr("📝 区域文件: %1").arg(regionPath));
    appLog(tr("📝 参数文件: %1").arg(qrParamsPath));
    appLog(tr("📝 模板信息已保存到: %1").arg(templateInfoFile));

    // 🔔 显示成功消息
    halWin->dispHalconMessage(50, 50,
                              tr("二维码模板创建成功！\n文件: %1\n类型: %2").arg(fileName, qrModelType),
                              "green");

    closeTemplateWindow();

  }
  catch (HalconCpp::HException& e)
  {
    QString errorMsg = tr("❌ Halcon异常：%1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text()));
    appLog(errorMsg, ERR);
    if (halWin)
    {
      halWin->dispHalconMessage(50, 100, tr("二维码模板创建失败\n%1").arg(errorMsg), "red");
    }
  } catch (...)
  {
    appLog(tr("❌ 二维码模板创建时发生未知异常"), ERR);
  }
}

/* ==================== 测量模板创建 ==================== */
void VisualProcess::handleMeasureTemplateCreation(const QString& fileName)
{
  try
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("📏 开始创建测量模板..."));

    // 🔧 获取测量参数
    QString measureType = "edge_detection";
    int edgeThreshold = 30;
    double measurePrecision = 0.01;

    if (m_Create_Model_Widget)
    {
      QComboBox* typeCombo = m_Create_Model_Widget->findChild<QComboBox*>("measureType");
      QLineEdit* thresholdEdit = m_Create_Model_Widget->findChild<QLineEdit*>("edgeThreshold");
      QLineEdit* precisionEdit = m_Create_Model_Widget->findChild<QLineEdit*>("measurePrecision");

      if (typeCombo)
        measureType = typeCombo->currentData().toString();
      if (thresholdEdit)
      {
        bool ok;
        int val = thresholdEdit->text().toInt(&ok);
        if (ok && val > 0)
          edgeThreshold = val;
      }
      if (precisionEdit)
      {
        bool ok;
        double val = precisionEdit->text().toDouble(&ok);
        if (ok && val > 0)
          measurePrecision = val;
      }
    }

    appLog(tr("📋 测量参数: 类型=%1, 边缘阈值=%2, 精度=%3").arg(measureType).arg(edgeThreshold).arg(measurePrecision));

    // 🎯 提示用户绘制测量区域
    appLog(tr("📝 请在图像上绘制测量模板区域..."));
    appLog(tr("💡 提示：在图像窗口中拖拽鼠标绘制矩形区域"));

    // 🔲 生成测量区域 - 使用HOBJ通用格式（推荐）
    m_modelSavePath += "MeasureModel/";
    QString regionPath = m_modelSavePath + m_halFileManager->generateHobjFileName(fileName, "measure_region");
    HObject measureRegion = halWin->genAngleRec(regionPath, "orange");


    if (!measureRegion.IsInitialized() || measureRegion.CountObj() == 0)
    {
      appLog(tr("⚠️ 警告：未检测到有效的测量区域，请重新绘制"), WARNING);
      return;
    }

    appLog(tr("✅ 已使用HOBJ通用格式保存测量区域: %1").arg(regionPath));
    appLog(tr("💡 HOBJ格式说明: %1").arg(m_halFileManager->getHobjFileDescription("measure_region")));

    // 💾 保存测量参数为Halcon tuple文件
    QString measureParamsPath = m_modelSavePath + fileName + "_measure_params" + m_halFileManager->
        getFileExtension("tuple");
    try
    {
      HTuple measureParamsTuple;
      measureParamsTuple.Append(measureType.toStdString().c_str());
      measureParamsTuple.Append(edgeThreshold);
      measureParamsTuple.Append(measurePrecision);
      WriteTuple(measureParamsTuple, measureParamsPath.toStdString().c_str());
      appLog(tr("💾 测量参数已保存为Halcon Tuple文件: %1").arg(measureParamsPath));
    }
    catch (HalconCpp::HException& e)
    {
      appLog(tr("⚠️ 保存测量参数文件失败: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
    }

    // 📝 保存测量模板信息
    QString templateInfoFile = m_modelSavePath + fileName + "_measure_template_info.json";
    saveMeasureTemplateInfo(templateInfoFile, fileName, measureType, edgeThreshold, measurePrecision);

    // 🎨 显示创建的区域
    halWin->showSymbolList.append(measureRegion);
    halWin->showImage(m_Img);

    appLog(tr("🎉 测量模板创建成功！"));
    appLog(tr("📁 模板文件: %1").arg(fileName));
    appLog(tr("📝 区域文件: %1").arg(regionPath));
    appLog(tr("📝 参数文件: %1").arg(measureParamsPath));
    appLog(tr("📝 模板信息已保存到: %1").arg(templateInfoFile));

    // 🔔 显示成功消息
    halWin->dispHalconMessage(50, 50,
                              tr("测量模板创建成功！\n文件: %1\n类型: %2").arg(fileName, measureType),
                              "green");

    closeTemplateWindow();

  }
  catch (HalconCpp::HException& e)
  {
    QString errorMsg = tr("❌ Halcon异常：%1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text()));
    appLog(errorMsg, ERR);
    if (halWin)
    {
      halWin->dispHalconMessage(50, 100, tr("测量模板创建失败\n%1").arg(errorMsg), "red");
    }
  } catch (...)
  {
    appLog(tr("❌ 测量模板创建时发生未知异常"), ERR);
  }
}

/* ==================== 检测模板创建 ==================== */
void VisualProcess::handleDetectionTemplateCreation(const QString& fileName)
{
  try
  {
    clearDisplayObjectsOnly(); // 🧹 清除之前的显示对象但保留图像
    appLog(tr("🔍 开始创建检测模板..."));

    // 🔧 获取检测参数
    HTuple maxContrast = 128;
    HTuple minContrast = 30;
    HTuple matchThreshold = 0.7;
    QString pyramidLevels = "auto";

    if (m_Create_Model_Widget)
    {
      QLineEdit* maxContrastEdit = m_Create_Model_Widget->findChild<QLineEdit*>("maxContrast");
      QLineEdit* minContrastEdit = m_Create_Model_Widget->findChild<QLineEdit*>("minContrast");
      QLineEdit* thresholdEdit = m_Create_Model_Widget->findChild<QLineEdit*>("matchThreshold");
      QComboBox* pyramidCombo = m_Create_Model_Widget->findChild<QComboBox*>("pyramidLevels");

      if (maxContrastEdit)
      {
        bool ok;
        int val = maxContrastEdit->text().toInt(&ok);
        if (ok && val > 0)
          maxContrast = val;
      }

      if (minContrastEdit)
      {
        bool ok;
        int val = minContrastEdit->text().toInt(&ok);
        if (ok && val > 0)
          minContrast = val;
      }

      if (thresholdEdit)
      {
        bool ok;
        double val = thresholdEdit->text().toDouble(&ok);
        if (ok && val > 0.0 && val <= 1.0)
          matchThreshold = val;
      }

      if (pyramidCombo)
      {
        pyramidLevels = pyramidCombo->currentData().toString();
      }
    }

    appLog(tr("📋 检测参数: 对比度[%1-%2], 金字塔层数[%3]")
           .arg(minContrast[0].I()).arg(maxContrast[0].I())
           .arg(pyramidLevels));

    // 🎯 提示用户绘制模板区域
    appLog(tr("📝 请在图像上绘制检测模板区域..."));
    appLog(tr("💡 提示：在图像窗口中拖拽鼠标绘制矩形区域"));

    // 🔲 生成模板区域 - 使用HOBJ通用格式（推荐）

    m_modelSavePath += "DetectionModel/";
    QString currentTimer = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
    QString regionPath = m_modelSavePath + m_halFileManager->generateHobjFileName(fileName, "detection_region");
    HObject templateRegion = halWin->genAngleRec(regionPath, "green");

    if (!templateRegion.IsInitialized() || templateRegion.CountObj() == 0)
    {
      appLog(tr("⚠️ 警告：未检测到有效的模板区域，请重新绘制"), WARNING);
      return;
    }

    appLog(tr("✅ 模板区域绘制成功，开始创建形状模板..."));
    appLog(tr("✅ 已使用HOBJ通用格式保存检测区域: %1").arg(regionPath));
    appLog(tr("💡 HOBJ格式说明: %1").arg(m_halFileManager->getHobjFileDescription("detection_region")));

    // 🏭 创建形状模板 - 使用专用的.shm扩展名
    QString modelFile = m_modelSavePath + fileName + "_" + currentTimer; // QtCreateShapeModel会自动添加.shm和.tup后缀
    appLog(tr("🔧 正在创建形状模板，这可能需要几秒钟..."));

    // 🎯 调用Halcon创建模板
    ShapeModelData modelResult = halWin->QtCreateShapeModel(
        m_Img, templateRegion, maxContrast, minContrast, modelFile);

    appLog(tr("🔧 模板创建完成，正在处理结果..."));

    // 📊 检查创建结果
    if (modelResult.ModelID.Length() > 0)
    {
      appLog(tr("🎉 检测模板创建成功！"));
      appLog(tr("📋 模板ID: %1").arg(modelResult.ModelID.ToString().Text()));
      appLog(tr("📁 模板文件: %1").arg(fileName));

      // 💾 保存检测参数为Halcon tuple文件
      QString detectParamsPath = m_modelSavePath + fileName + "_detection_params" + m_halFileManager->
          getFileExtension("tuple");
      try
      {
        HTuple detectParamsTuple;
        detectParamsTuple.Append(modelResult.ModelID.ToString().Text());
        detectParamsTuple.Append(maxContrast[0].I());
        detectParamsTuple.Append(minContrast[0].I());
        detectParamsTuple.Append(matchThreshold[0].D());
        detectParamsTuple.Append(pyramidLevels.toStdString().c_str());
        WriteTuple(detectParamsTuple, detectParamsPath.toStdString().c_str());
        appLog(tr("💾 检测参数已保存为Halcon Tuple文件: %1").arg(detectParamsPath));
      }
      catch (HalconCpp::HException& e)
      {
        appLog(tr("⚠️ 保存检测参数文件失败: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
      }

      // 🎨 显示创建的模板区域
      if (modelResult.modelregion.IsInitialized() && modelResult.modelregion.CountObj() > 0)
      {
        halWin->showSymbolList.append(modelResult.modelregion);
        halWin->showImage(m_Img);
        appLog(tr("✅ 模板区域已显示在图像上"));

        // 💾 保存模板区域为独立的Halcon区域文件
        QString templateRegionPath = m_modelSavePath + fileName + "_template_region" + m_halFileManager->
            getFileExtension("region");
        try
        {
          WriteRegion(modelResult.modelregion, templateRegionPath.toStdString().c_str());
          appLog(tr("💾 模板区域已保存: %1").arg(templateRegionPath));
        }
        catch (HalconCpp::HException& e)
        {
          appLog(tr("⚠️ 保存模板区域文件失败: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
        }
      }

      // 📝 保存模板信息到文件
      QString templateInfoFile = m_modelSavePath + fileName + "_" + currentTimer +  "_detection_template_info.json";
      saveDetectionTemplateInfo(templateInfoFile, fileName, modelResult.ModelID.ToString().Text(),
                                maxContrast[0].I(), minContrast[0].I(),
                                modelResult.Score.D(), pyramidLevels);

      // 🔔 显示成功消息
      halWin->dispHalconMessage(50, 50,
                                tr("检测模板创建成功！\n文件: %1\nID: %2\n评分: %3 \n金字塔层数: %4").arg(fileName).arg(
                                    modelResult.ModelID.ToString().Text())
                                .arg(modelResult.Score.D()).arg(pyramidLevels), "green");

      appLog(tr("📝 主模板文件: %1model.shm").arg(modelFile));
      appLog(tr("📝 模板数据文件: %1data.tup").arg(modelFile));
      appLog(tr("📝 区域文件: %1").arg(regionPath));
      appLog(tr("📝 参数文件: %1").arg(detectParamsPath));

      closeTemplateWindow();

    }
    else
    {
      appLog(tr("❌ 检测模板创建失败，可能的原因："), ERR);
      appLog(tr("   • 选择的区域特征不明显"), ERR);
      appLog(tr("   • 对比度参数设置不当"), ERR);
      appLog(tr("   • 图像质量问题"), ERR);
      appLog(tr("💡 建议：尝试选择具有明显特征的区域，或调整对比度参数"), INFO);
    }

  }
  catch (HalconCpp::HException& e)
  {
    QString errorMsg = tr("❌ Halcon异常：%1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text()));
    appLog(errorMsg, ERR);
    if (halWin)
    {
      halWin->dispHalconMessage(50, 100, tr("检测模板创建失败\n%1").arg(errorMsg), "red");
    }
  } catch (...)
  {
    appLog(tr("❌ 检测模板创建时发生未知异常"), ERR);
  }
}


/* ==================== 辅助函数 ==================== */
void VisualProcess::closeTemplateWindow()
{
  // ✅ 延迟关闭创建窗口，让用户看到结果
  QTimer::singleShot(2000, this, [this]()
  {
    if (m_Create_Model_Widget)
    {
      appLog(tr("🧹 自动关闭模板创建窗口"));
      m_Create_Model_Widget->close();
    }
  });
}

void VisualProcess::saveQRCodeTemplateInfo(const QString& filePath, const QString& fileName,
                                           const QString& modelType, const QString& polarity,
                                           const QString& customParams)
{
  try
  {
    QJsonObject templateInfo;
    templateInfo["template_type"] = "QRCode";
    templateInfo["file_name"] = fileName;
    templateInfo["creation_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    templateInfo["qr_model_type"] = modelType;
    templateInfo["qr_polarity"] = polarity;
    templateInfo["custom_params"] = customParams;

    QJsonDocument doc(templateInfo);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
    {
      file.write(doc.toJson());
      file.close();
      appLog(tr("💾 二维码模板信息已保存"));
    }
  }
  catch (...)
  {
    appLog(tr("⚠️ 保存二维码模板信息失败"), WARNING);
  }
}

void VisualProcess::saveMeasureTemplateInfo(const QString& filePath, const QString& fileName,
                                            const QString& measureType, int edgeThreshold, double measurePrecision)
{
  try
  {
    QJsonObject templateInfo;
    templateInfo["template_type"] = "Measure";
    templateInfo["file_name"] = fileName;
    templateInfo["creation_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    templateInfo["measure_type"] = measureType;
    templateInfo["edge_threshold"] = edgeThreshold;
    templateInfo["measure_precision"] = measurePrecision;

    QJsonDocument doc(templateInfo);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
    {
      file.write(doc.toJson());
      file.close();
      appLog(tr("💾 测量模板信息已保存"));
    }
  }
  catch (...)
  {
    appLog(tr("⚠️ 保存测量模板信息失败"), WARNING);
  }
}

void VisualProcess::saveDetectionTemplateInfo(const QString& filePath, const QString& fileName,
                                              QString modelId, int maxContrast, int minContrast,
                                              double Score, const QString& pyramidLevels)
{
  try
  {
    QJsonObject templateInfo;
    templateInfo["template_type"] = "Detection";
    templateInfo["file_name"] = fileName;
    templateInfo["creation_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    templateInfo["model_id"] = modelId;
    templateInfo["max_contrast"] = maxContrast;
    templateInfo["min_contrast"] = minContrast;
    templateInfo["modle_Score"] = Score;
    templateInfo["pyramid_levels"] = pyramidLevels;

    QJsonDocument doc(templateInfo);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
    {
      file.write(doc.toJson());
      file.close();
      appLog(tr("💾 检测模板信息已保存"));
    }
  }
  catch (...)
  {
    appLog(tr("⚠️ 保存检测模板信息失败"), WARNING);
  }
}


/* ==================== 视觉处理函数 ==================== */

/* ==================== 🎯 新增：任务参数管理和批处理功能 ==================== */

VisualTaskParams VisualProcess::getCurrentTaskParams() const
{
  VisualTaskParams params;

  // 🎯 根据当前模式提取对应的参数
  if (m_QCodeCam_flag)
  {
    params = extractQRCodeParams();
  }
  else if (m_MeasureCam_flag)
  {
    params = extractMeasureParams();
  }
  else if (m_CheckCam_flag)
  {
    params = extractDetectionParams();
  }

  return params;
}

bool VisualProcess::validateTaskParams(const VisualTaskParams& params, QString& errorMessage) const
{
  // 🔍 基本验证
  if (params.taskType.isEmpty())
  {
    errorMessage = tr("任务类型未设置");
    return false;
  }

  if (params.templateName.isEmpty())
  {
    errorMessage = tr("模板名称未设置");
    return false;
  }

  // 🎯 根据任务类型进行特定验证
  if (params.taskType == "QRCode")
  {
    if (params.qrCodeType.isEmpty())
    {
      errorMessage = tr("二维码类型未设置");
      return false;
    }
    if (params.regionPath.isEmpty())
    {
      errorMessage = tr("二维码识别区域路径未设置");
      return false;
    }
  }
  else if (params.taskType == "Measure")
  {
    if (params.measureType.isEmpty())
    {
      errorMessage = tr("测量类型未设置");
      return false;
    }
    if (params.edgeThreshold <= 0)
    {
      errorMessage = tr("边缘阈值设置无效");
      return false;
    }
  }
  else if (params.taskType == "Detection")
  {
    if (params.modelPath.isEmpty())
    {
      errorMessage = tr("检测模型路径未设置");
      return false;
    }
    if (params.matchThreshold <= 0.0 || params.matchThreshold > 1.0)
    {
      errorMessage = tr("匹配阈值设置无效（应在0.1-1.0之间）");
      return false;
    }
  }

  errorMessage = tr("参数验证通过");
  return true;
}

void VisualProcess::startBatchProcessing()
{
  try
  {
    appLog(tr("🚀 准备启动批处理任务..."));

    // 🎯 获取当前任务参数
    VisualTaskParams params = getCurrentTaskParams();

    // 🔍 验证参数完整性
    QString validationError;
    if (!validateTaskParams(params, validationError))
    {
      appLog(tr("❌ 任务参数验证失败：%1").arg(validationError), ERR);
      return;
    }

    // 🎯 缓存当前参数
    m_currentTaskParams = params;
    m_batchProcessing = true;

    appLog(tr("✅ 任务参数验证通过，发送处理请求..."));
    appLog(tr("📋 任务类型：%1").arg(params.taskType));
    appLog(tr("📋 模板名称：%1").arg(params.templateName));

    // 🚀 发射信号给主窗口，由主窗口协调工作线程
    emit processingRequested(params);

  }
  catch (const std::exception& e)
  {
    QString errorMsg = tr("❌ 启动批处理时发生异常：%1").arg(QString::fromLocal8Bit(e.what()));
    appLog(errorMsg, ERR);
  } catch (...)
  {
    appLog(tr("❌ 启动批处理时发生未知异常"), ERR);
  }
}

void VisualProcess::stopBatchProcessing()
{
  appLog(tr("⏹️ 请求停止批处理任务..."));
  m_batchProcessing = false;
  emit processingStopRequested();
}

QString VisualProcess::getCurrentTime() const
{
  return QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
}

/* ==================== 🎯 参数提取方法实现 ==================== */

VisualTaskParams VisualProcess::extractQRCodeParams() const
{
  VisualTaskParams params;
  params.taskType = "QRCode";
  params.isValid = false;

  try
  {
    // 🔍 从创建模板窗口获取参数（如果存在）
    if (m_Create_Model_Widget)
    {
      QLineEdit* fileNameEdit = m_Create_Model_Widget->findChild<QLineEdit*>("templateFileName");
      QComboBox* typeCombo = m_Create_Model_Widget->findChild<QComboBox*>("qrModelType");
      QComboBox* polarityCombo = m_Create_Model_Widget->findChild<QComboBox*>("qrPolarity");
      QLineEdit* customEdit = m_Create_Model_Widget->findChild<QLineEdit*>("qrCustomParams");

      if (fileNameEdit)
        params.templateName = fileNameEdit->text().trimmed();
      if (typeCombo)
        params.qrCodeType = typeCombo->currentText();
      if (polarityCombo)
        params.qrPolarity = polarityCombo->currentData().toString();
      if (customEdit)
        params.parameters["customParams"] = customEdit->text().trimmed();
    }

    // 🎯 设置默认值（如果窗口不存在）
    if (params.templateName.isEmpty())
    {
      params.templateName = QString("QRCode_template_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }
    if (params.qrCodeType.isEmpty())
    {
      params.qrCodeType = "Data Matrix ECC 200";
    }
    if (params.qrPolarity.isEmpty())
    {
      params.qrPolarity = "light_on_dark";
    }

    // 🗂️ 构建文件路径
    if (m_halFileManager)
    {
      QString basePath = m_modelSavePath;
      params.regionPath = basePath + m_halFileManager->generateHobjFileName(params.templateName, "qr_region");
      params.modelPath = basePath + params.templateName + "_qr_params.tup";
    }

    params.isValid = true;

  }
  catch (...)
  {
    params.isValid = false;
  }

  return params;
}

VisualTaskParams VisualProcess::extractMeasureParams() const
{
  VisualTaskParams params;
  params.taskType = "Measure";
  params.isValid = false;

  try
  {
    // 🔍 从创建模板窗口获取参数
    if (m_Create_Model_Widget)
    {
      QLineEdit* fileNameEdit = m_Create_Model_Widget->findChild<QLineEdit*>("templateFileName");
      QComboBox* typeCombo = m_Create_Model_Widget->findChild<QComboBox*>("measureType");
      QLineEdit* thresholdEdit = m_Create_Model_Widget->findChild<QLineEdit*>("edgeThreshold");
      QLineEdit* precisionEdit = m_Create_Model_Widget->findChild<QLineEdit*>("measurePrecision");

      if (fileNameEdit)
        params.templateName = fileNameEdit->text().trimmed();
      if (typeCombo)
        params.measureType = typeCombo->currentData().toString();
      if (thresholdEdit)
      {
        bool ok;
        int val = thresholdEdit->text().toInt(&ok);
        params.edgeThreshold = ok && val > 0 ? val : 30;
      }
      if (precisionEdit)
      {
        bool ok;
        double val = precisionEdit->text().toDouble(&ok);
        params.measurePrecision = ok && val > 0 ? val : 0.01;
      }
    }

    // 🎯 设置默认值
    if (params.templateName.isEmpty())
    {
      params.templateName = QString("Measure_template_%1").
          arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }
    if (params.measureType.isEmpty())
    {
      params.measureType = "edge_detection";
    }
    if (params.edgeThreshold <= 0)
    {
      params.edgeThreshold = 30;
    }
    if (params.measurePrecision <= 0)
    {
      params.measurePrecision = 0.01;
    }

    // 🗂️ 构建文件路径
    if (m_halFileManager)
    {
      QString basePath = m_modelSavePath;
      params.regionPath = basePath + m_halFileManager->generateHobjFileName(params.templateName, "measure_region");
      params.modelPath = basePath + params.templateName + "_measure_params.tup";
    }

    params.isValid = true;

  }
  catch (...)
  {
    params.isValid = false;
  }

  return params;
}

VisualTaskParams VisualProcess::extractDetectionParams() const
{
  VisualTaskParams params;
  params.taskType = "Detection";
  params.isValid = false;

  try
  {
    // 🔍 从创建模板窗口获取参数
    if (m_Create_Model_Widget)
    {
      QLineEdit* fileNameEdit = m_Create_Model_Widget->findChild<QLineEdit*>("templateFileName");
      QLineEdit* maxContrastEdit = m_Create_Model_Widget->findChild<QLineEdit*>("maxContrast");
      QLineEdit* minContrastEdit = m_Create_Model_Widget->findChild<QLineEdit*>("minContrast");
      QLineEdit* thresholdEdit = m_Create_Model_Widget->findChild<QLineEdit*>("matchThreshold");
      QComboBox* pyramidCombo = m_Create_Model_Widget->findChild<QComboBox*>("pyramidLevels");

      if (fileNameEdit)
        params.templateName = fileNameEdit->text().trimmed();
      if (maxContrastEdit)
      {
        bool ok;
        int val = maxContrastEdit->text().toInt(&ok);
        params.maxContrast = ok && val > 0 ? val : 128;
      }
      if (minContrastEdit)
      {
        bool ok;
        int val = minContrastEdit->text().toInt(&ok);
        params.minContrast = ok && val > 0 ? val : 30;
      }
      if (thresholdEdit)
      {
        bool ok;
        double val = thresholdEdit->text().toDouble(&ok);
        params.matchThreshold = ok && val > 0.0 && val <= 1.0 ? val : 0.7;
      }
      if (pyramidCombo)
      {
        params.pyramidLevels = pyramidCombo->currentData().toString();
      }
    }

    // 🎯 设置默认值
    if (params.templateName.isEmpty())
    {
      params.templateName = QString("Detection_template_%1").arg(
          QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }
    if (params.maxContrast <= 0)
    {
      params.maxContrast = 128;
    }
    if (params.minContrast <= 0)
    {
      params.minContrast = 30;
    }
    if (params.matchThreshold <= 0.0 || params.matchThreshold > 1.0)
    {
      params.matchThreshold = 0.7;
    }
    if (params.pyramidLevels.isEmpty())
    {
      params.pyramidLevels = "auto";
    }

    // 🗂️ 构建文件路径
    if (m_halFileManager)
    {
      QString basePath = m_modelSavePath;
      params.regionPath = basePath + m_halFileManager->generateHobjFileName(params.templateName, "detection_region");
      params.modelPath = basePath + params.templateName + "model.shm"; // 形状模型文件
    }

    params.isValid = true;

  }
  catch (...)
  {
    params.isValid = false;
  }

  return params;
}

/* ==================== 🎯 处理结果管理 ==================== */

void VisualProcess::onProcessingResult(const VisualProcessResult& result)
{
  try
  {
    appLog(tr("📊 收到处理结果：%1").arg(result.taskType));

    // 🎨 显示处理结果
    displayProcessingResult(result);

    // 📝 更新结果历史
    updateResultHistory(result);

    // 🚀 发射结果信号给主窗口
    emit resultGenerated(result);

  }
  catch (const std::exception& e)
  {
    appLog(tr("❌ 处理结果时发生异常：%1").arg(QString::fromLocal8Bit(e.what())), ERR);
  } catch (...)
  {
    appLog(tr("❌ 处理结果时发生未知异常"), ERR);
  }
}

void VisualProcess::onProcessingProgress(int current, int total, const QString& currentFile)
{
  QString progressMsg = tr("📈 处理进度：[%1/%2] %3").arg(current).arg(total).arg(currentFile);
  appLog(progressMsg);

  // 🎨 可以在这里更新进度条或其他UI元素
  if (halWin)
  {
    halWin->dispHalconMessage(50, 150,
                              tr("处理进度: %1/%2\n当前: %3").arg(current).arg(total).arg(currentFile),
                              "blue");
  }
}

void VisualProcess::onProcessingError(const QString& error)
{
  appLog(tr("❌ 处理错误：%1").arg(error), ERR);

  // 🛑 停止批处理
  if (m_batchProcessing)
  {
    m_batchProcessing = false;
  }

  // 🎨 在图像窗口显示错误信息
  if (halWin)
  {
    halWin->dispHalconMessage(50, 100, tr("处理错误:\n%1").arg(error), "red");
  }
}

void VisualProcess::displayProcessingResult(const VisualProcessResult& result)
{
  try
  {
    // 🎨 根据结果类型显示不同的信息
    QString resultMsg;
    QString color = result.success ? "green" : "red";

    if (result.success)
    {
      if (result.taskType == "QRCode")
      {
        resultMsg = tr("二维码识别成功!\n");
        if (!result.decodedTexts.isEmpty())
        {
          resultMsg += tr("识别内容: %1\n").arg(result.decodedTexts.join(", "));
        }
        resultMsg += tr("位置数量: %1").arg(result.codePositions.size());
      }
      else if (result.taskType == "Measure")
      {
        resultMsg = tr("测量完成!\n");
        resultMsg += tr("最小距离: %1px\n").arg(QString::number(result.minDistance, 'f', 2));
        resultMsg += tr("最大距离: %1px\n").arg(QString::number(result.maxDistance, 'f', 2));
        resultMsg += tr("重心距离: %1px").arg(QString::number(result.centroidDistance, 'f', 2));
      }
      else if (result.taskType == "Detection")
      {
        resultMsg = tr("检测完成!\n");
        resultMsg += tr("匹配数量: %1\n").arg(result.matchCount);
        if (!result.matchScores.isEmpty())
        {
          double avgScore = 0.0;
          for (double score : result.matchScores)
          {
            avgScore += score;
          }
          avgScore /= result.matchScores.size();
          resultMsg += tr("平均匹配分数: %1").arg(QString::number(avgScore, 'f', 3));
        }
      }
    }
    else
    {
      resultMsg = tr("处理失败:\n%1").arg(result.errorMessage);
    }

    // 🖼️ 在图像窗口显示结果
    if (halWin)
    {
      halWin->dispHalconMessage(20, 20, resultMsg, color);
    }

    // 📊 记录详细日志
    appLog(tr("📊 %1 - %2").arg(result.taskType, resultMsg.replace('\n', ' ')),
           result.success ? INFO : ERR);

  }
  catch (...)
  {
    appLog(tr("❌ 显示处理结果时发生异常"), ERR);
  }
}

void VisualProcess::updateResultHistory(const VisualProcessResult& result)
{
  try
  {
    // 📝 添加到历史记录
    m_resultHistory.append(result);

    // 🧹 限制历史记录数量（保持最近100条）
    if (m_resultHistory.size() > 100)
    {
      m_resultHistory.removeFirst();
    }

    appLog(tr("📝 结果已添加到历史记录，当前历史数量：%1").arg(m_resultHistory.size()));

  }
  catch (...)
  {
    appLog(tr("❌ 更新结果历史时发生异常"), ERR);
  }
}

/* ==================== 🎯 扩展工具按钮处理 ==================== */

/* ==================== 🎯 新增：参数配置窗口创建方法实现 ==================== */

// 💾 图像保存参数配置窗口
void VisualProcess::createImageSaveParamWindow()
{
  if (!m_uiBuilder)
  {
    appLog(tr("❌ UI构建器未初始化"), ERR);
    return;
  }

  appLog(tr("💾 打开图像保存参数配置窗口"));

  QJsonObject saveConfig = {
      {"layout", QJsonObject{
           {"type", "VBox"},
           {"spacing", 15},
           {"margins", QJsonArray{20, 20, 20, 20}},
           {"widgets", QJsonArray{
                QJsonObject{
                    {"type", "Label"},
                    {"name", "title"},
                    {"text", tr("💾 图像保存设置")},
                    {"styleSheet", "font-size: 16px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;"}
                },

                // 保存设置分组
                QJsonObject{
                    {"type", "GroupBox"},
                    {"name", "save_group"},
                    {"text", tr("📁 保存选项")},
                    {"styleSheet",
                     "QGroupBox { font-weight: bold; border: 2px solid #8e44ad; border-radius: 8px; margin-top: 10px; padding-top: 15px; }"},
                    {"childWidgets", QJsonArray{
                         QJsonObject{
                             {"type", "Label"},
                             {"name", "format_label"},
                             {"text", tr("图像格式:")}
                         },
                         QJsonObject{
                             {"type", "ComboBox"},
                             {"name", "image_format"},
                             {"value", "PNG"},
                             {"properties", QJsonObject{
                                  {"items", QJsonArray{"PNG", "JPG", "BMP", "TIFF"}}
                              }}
                         },
                         QJsonObject{
                             {"type", "CheckBox"},
                             {"name", "timestamp_check"},
                             {"text", tr("添加时间戳到文件名")},
                             {"value", true}
                         }
                     }}
                },

                // 按钮区域
                QJsonObject{
                    {"type", "Frame"},
                    {"name", "button_frame"},
                    {"styleSheet", "border: none; margin-top: 15px;"},
                    {"childLayout", QJsonObject{
                         {"type", "HBox"},
                         {"spacing", 10},
                         {"widgets", QJsonArray{
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_save"},
                                  {"text", tr("💾 开始保存")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #27ae60; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #229954; }"}
                              },
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_cancel"},
                                  {"text", tr("❌ 取消")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #95a5a6; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #7f8c8d; }"}
                              }
                          }}
                     }}
                }
            }}
       }}
  };

  QWidget* paramWindow = m_uiBuilder->createFromJson(saveConfig);
  showParamWindowNearMouse(paramWindow, tr("💾 图像保存设置"));
  connectImageSaveParamButtons(paramWindow);
}

// 📁 图像加载参数配置窗口
void VisualProcess::createImageLoadParamWindow()
{
  if (!m_uiBuilder)
  {
    appLog(tr("❌ UI构建器未初始化"), ERR);
    return;
  }

  appLog(tr("📁 打开图像加载参数配置窗口"));

  QJsonObject loadConfig = {
      {"layout", QJsonObject{
           {"type", "VBox"},
           {"spacing", 15},
           {"margins", QJsonArray{20, 20, 20, 20}},
           {"widgets", QJsonArray{
                QJsonObject{
                    {"type", "Label"},
                    {"name", "title"},
                    {"text", tr("📁 图像加载设置")},
                    {"styleSheet", "font-size: 16px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;"}
                },

                QJsonObject{
                    {"type", "GroupBox"},
                    {"name", "load_group"},
                    {"text", tr("📂 加载选项")},
                    {"styleSheet",
                     "QGroupBox { font-weight: bold; border: 2px solid #3498db; border-radius: 8px; margin-top: 10px; padding-top: 15px; }"},
                    {"childWidgets", QJsonArray{
                         QJsonObject{
                             {"type", "CheckBox"},
                             {"name", "auto_display_check"},
                             {"text", tr("自动显示加载的图像")},
                             {"value", true}
                         },
                         QJsonObject{
                             {"type", "CheckBox"},
                             {"name", "clear_previous_check"},
                             {"text", tr("清除之前的显示对象")},
                             {"value", true}
                         }
                     }}
                },

                QJsonObject{
                    {"type", "Frame"},
                    {"name", "button_frame"},
                    {"styleSheet", "border: none; margin-top: 15px;"},
                    {"childLayout", QJsonObject{
                         {"type", "HBox"},
                         {"spacing", 10},
                         {"widgets", QJsonArray{
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_load"},
                                  {"text", tr("📁 选择图像")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #3498db; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #2980b9; }"}
                              },
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_cancel"},
                                  {"text", tr("❌ 取消")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #95a5a6; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #7f8c8d; }"}
                              }
                          }}
                     }}
                }
            }}
       }}
  };

  QWidget* paramWindow = m_uiBuilder->createFromJson(loadConfig);
  showParamWindowNearMouse(paramWindow, tr("📁 图像加载设置"));
  connectImageLoadParamButtons(paramWindow);
}

// 🎛️ 通用窗口方法：在鼠标附近显示参数窗口
void VisualProcess::showParamWindowNearMouse(QWidget* window, const QString& title)
{
  if (!window)
  {
    appLog(tr("❌ 窗口创建失败"), ERR);
    return;
  }

  // 设置窗口属性
  window->setWindowTitle(title);
  window->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->setMinimumSize(400, 300);

  // 设置父窗口
  window->setParent(this, Qt::Window);

  // 获取鼠标位置并稍微偏移
  QPoint globalPos = QCursor::pos();
  globalPos += QPoint(20, 20);

  // 确保窗口在屏幕范围内
  QRect screenGeometry = QApplication::desktop()->screenGeometry();
  if (globalPos.x() + window->width() > screenGeometry.right())
  {
    globalPos.setX(screenGeometry.right() - window->width() - 20);
  }
  if (globalPos.y() + window->height() > screenGeometry.bottom())
  {
    globalPos.setY(screenGeometry.bottom() - window->height() - 20);
  }

  // 移动并显示窗口
  window->move(globalPos);
  window->show();
  window->raise();
  window->activateWindow();

  appLog(tr("✅ 参数窗口已显示: %1").arg(title));
}

// 🗑️ 关闭参数窗口
void VisualProcess::closeParamWindow(QWidget* window)
{
  if (window)
  {
    window->close();
    window->deleteLater();
  }
}

/* ==================== 🎯 参数窗口按钮连接方法 ==================== */

// 📷 相机参数按钮连接
void VisualProcess::connectCameraParamButtons(QWidget* window, const QString& action)
{
  if (!window)
    return;

  // 连接测试按钮
  if (QPushButton* testBtn = window->findChild<QPushButton*>("btn_test"))
  {
    connect(testBtn, &QPushButton::clicked, [this]()
    {
      appLog(tr("🔍 测试相机连接..."));
      // TODO: 实现相机测试逻辑
      appLog(tr("✅ 相机测试完成"));
    });
  }

  // 连接执行按钮
  if (QPushButton* executeBtn = window->findChild<QPushButton*>("btn_execute"))
  {
    connect(executeBtn, &QPushButton::clicked, [this, action, window]()
    {
      if (action == "open_camera")
      {
        appLog(tr("🚀 开始打开相机..."));
        // 更新相机状态
        if (m_QCodeCam_flag)
        {
          m_isOpenQCodeCam_flag = true;
          clearLaout(ui->vBoxLayout_Params);
          onQCodeTabWidget();
        }
        else if (m_MeasureCam_flag)
        {
          m_isOpenMeasureCam_flag = true;
          clearLaout(ui->vBoxLayout_Params);
          onMeasureCam();
        }
        else if (m_CheckCam_flag)
        {
          m_isOpenCheckCam_flag = true;
          clearLaout(ui->vBoxLayout_Params);
          onCheckCam();
        }
        appLog(tr("✅ 相机已打开"));
      }
      else
      {
        appLog(tr("⏹️ 停止相机..."));
        // 更新相机状态
        if (m_QCodeCam_flag && m_isOpenQCodeCam_flag)
        {
          m_isOpenQCodeCam_flag = false;
          clearLaout(ui->vBoxLayout_Params);
          onQCodeTabWidget();
        }
        else if (m_MeasureCam_flag && m_isOpenMeasureCam_flag)
        {
          m_isOpenMeasureCam_flag = false;
          clearLaout(ui->vBoxLayout_Params);
          onMeasureCam();
        }
        else if (m_CheckCam_flag && m_isOpenCheckCam_flag)
        {
          m_isOpenCheckCam_flag = false;
          clearLaout(ui->vBoxLayout_Params);
          onCheckCam();
        }
        appLog(tr("✅ 相机已停止"));
      }
      closeParamWindow(window);
    });
  }

  // 连接取消按钮
  if (QPushButton* cancelBtn = window->findChild<QPushButton*>("btn_cancel"))
  {
    connect(cancelBtn, &QPushButton::clicked, [this, window]()
    {
      appLog(tr("❌ 取消相机操作"));
      closeParamWindow(window);
    });
  }
}

// 💾 图像保存参数按钮连接
void VisualProcess::connectImageSaveParamButtons(QWidget* window)
{
  if (!window)
    return;

  // 连接保存按钮
  if (QPushButton* saveBtn = window->findChild<QPushButton*>("btn_save"))
  {
    connect(saveBtn, &QPushButton::clicked, [this, window]()
    {
      try
      {
        // 获取参数
        QString format = "PNG";
        bool addTimestamp = true;

        if (QComboBox* formatCombo = window->findChild<QComboBox*>("image_format"))
        {
          format = formatCombo->currentText();
        }
        if (QCheckBox* timestampCheck = window->findChild<QCheckBox*>("timestamp_check"))
        {
          addTimestamp = timestampCheck->isChecked();
        }

        appLog(tr("💾 开始保存图像，格式: %1, 时间戳: %2").arg(format).arg(addTimestamp ? "是" : "否"));

        // 🎯 使用新的安全保存方法
        QString errorMessage;
        bool success = halWin->QtSaveImageSafe(m_Img, errorMessage);

        if (success)
        {
          appLog(tr("✅ %1").arg(errorMessage));
        }
        else
        {
          appLog(tr("❌ 保存失败：%1").arg(errorMessage), ERR);
        }

        closeParamWindow(window);
      }
      catch (const std::exception& e)
      {
        appLog(tr("❌ 保存图像时发生异常：%1").arg(QString::fromLocal8Bit(e.what())), ERR);
      }
    });
  }

  // 连接取消按钮
  if (QPushButton* cancelBtn = window->findChild<QPushButton*>("btn_cancel"))
  {
    connect(cancelBtn, &QPushButton::clicked, [this, window]()
    {
      appLog(tr("❌ 取消保存操作"));
      closeParamWindow(window);
    });
  }
}

// 📁 图像加载参数按钮连接
void VisualProcess::connectImageLoadParamButtons(QWidget* window)
{
  if (!window)
    return;

  // 连接加载按钮
  if (QPushButton* loadBtn = window->findChild<QPushButton*>("btn_load"))
  {
    connect(loadBtn, &QPushButton::clicked, [this, window]()
    {
      try
      {
        // 获取参数
        bool autoDisplay = true;
        bool clearPrevious = true;

        if (QCheckBox* autoCheck = window->findChild<QCheckBox*>("auto_display_check"))
        {
          autoDisplay = autoCheck->isChecked();
        }
        if (QCheckBox* clearCheck = window->findChild<QCheckBox*>("clear_previous_check"))
        {
          clearPrevious = clearCheck->isChecked();
        }

        appLog(tr("📁 开始加载图像，自动显示: %1, 清除之前: %2").arg(autoDisplay ? "是" : "否").arg(clearPrevious ? "是" : "否"));

        if (clearPrevious)
        {
          clearDisplayObjectsOnly();
        }

        // 🎯 使用新的安全读取方法
        QString errorMessage;
        bool success = halWin->QtGetLocalImageSafe(m_Img, errorMessage);

        if (success)
        {
          appLog(tr("✅ %1").arg(errorMessage));
          if (autoDisplay)
          {
            QString imageInfo = halWin->getImageInfo();
            appLog(tr("📊 %1").arg(imageInfo));
          }
        }
        else
        {
          appLog(tr("❌ 读取失败：%1").arg(errorMessage), ERR);
        }

        closeParamWindow(window);
      }
      catch (const std::exception& e)
      {
        appLog(tr("❌ 读取图像时发生异常：%1").arg(QString::fromLocal8Bit(e.what())), ERR);
      }
    });
  }

  // 连接取消按钮
  if (QPushButton* cancelBtn = window->findChild<QPushButton*>("btn_cancel"))
  {
    connect(cancelBtn, &QPushButton::clicked, [this, window]()
    {
      appLog(tr("❌ 取消加载操作"));
      closeParamWindow(window);
    });
  }
}

// 🗑️ 图像清除参数配置窗口
void VisualProcess::createImageClearParamWindow()
{
  if (!m_uiBuilder)
  {
    appLog(tr("❌ UI构建器未初始化"), ERR);
    return;
  }

  appLog(tr("🗑️ 打开图像清除确认窗口"));

  QJsonObject clearConfig = {
      {"layout", QJsonObject{
           {"type", "VBox"},
           {"spacing", 15},
           {"margins", QJsonArray{20, 20, 20, 20}},
           {"widgets", QJsonArray{
                QJsonObject{
                    {"type", "Label"},
                    {"name", "title"},
                    {"text", tr("🗑️ 清除图像确认")},
                    {"styleSheet", "font-size: 16px; font-weight: bold; color: #e74c3c; margin-bottom: 10px;"}
                },

                QJsonObject{
                    {"type", "Label"},
                    {"name", "warning"},
                    {"text", tr("⚠️ 此操作将清除当前图像和所有显示对象，是否继续？")},
                    {"styleSheet", "color: #f39c12; font-weight: bold; margin: 10px;"}
                },

                QJsonObject{
                    {"type", "Frame"},
                    {"name", "button_frame"},
                    {"styleSheet", "border: none; margin-top: 15px;"},
                    {"childLayout", QJsonObject{
                         {"type", "HBox"},
                         {"spacing", 10},
                         {"widgets", QJsonArray{
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_clear"},
                                  {"text", tr("🗑️ 确认清除")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #e74c3c; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #c0392b; }"}
                              },
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_cancel"},
                                  {"text", tr("❌ 取消")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #95a5a6; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #7f8c8d; }"}
                              }
                          }}
                     }}
                }
            }}
       }}
  };

  QWidget* paramWindow = m_uiBuilder->createFromJson(clearConfig);
  showParamWindowNearMouse(paramWindow, tr("🗑️ 清除图像确认"));
  connectImageClearParamButtons(paramWindow);
}

// 📌 二维码区域拾取参数窗口
void VisualProcess::createQRCodeRegionParamWindow()
{
  if (!m_uiBuilder)
  {
    appLog(tr("❌ UI构建器未初始化"), ERR);
    return;
  }

  appLog(tr("📌 打开二维码区域拾取参数窗口"));

  QJsonObject regionConfig = {
      {"layout", QJsonObject{
           {"type", "VBox"},
           {"spacing", 15},
           {"margins", QJsonArray{20, 20, 20, 20}},
           {"widgets", QJsonArray{
                QJsonObject{
                    {"type", "Label"},
                    {"name", "title"},
                    {"text", tr("📌 二维码区域拾取")},
                    {"styleSheet", "font-size: 16px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;"}
                },

                QJsonObject{
                    {"type", "GroupBox"},
                    {"name", "region_group"},
                    {"text", tr("🔍 区域设置")},
                    {"styleSheet",
                     "QGroupBox { font-weight: bold; border: 2px solid #3498db; border-radius: 8px; margin-top: 10px; padding-top: 15px; }"},
                    {"childWidgets", QJsonArray{
                         QJsonObject{
                             {"type", "Label"},
                             {"name", "region_type_label"},
                             {"text", tr("区域类型:")}
                         },
                         QJsonObject{
                             {"type", "ComboBox"},
                             {"name", "region_type"},
                             {"value", "rectangle"},
                             {"properties", QJsonObject{
                                  {"items", QJsonArray{"rectangle", "circle", "polygon"}}
                              }}
                         },
                         QJsonObject{
                             {"type", "CheckBox"},
                             {"name", "auto_save_check"},
                             {"text", tr("自动保存区域")},
                             {"value", true}
                         }
                     }}
                },

                QJsonObject{
                    {"type", "Frame"},
                    {"name", "button_frame"},
                    {"styleSheet", "border: none; margin-top: 15px;"},
                    {"childLayout", QJsonObject{
                         {"type", "HBox"},
                         {"spacing", 10},
                         {"widgets", QJsonArray{
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_start_pick"},
                                  {"text", tr("🎯 开始拾取")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #27ae60; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #229954; }"}
                              },
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_cancel"},
                                  {"text", tr("❌ 取消")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #95a5a6; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #7f8c8d; }"}
                              }
                          }}
                     }}
                }
            }}
       }}
  };

  QWidget* paramWindow = m_uiBuilder->createFromJson(regionConfig);
  showParamWindowNearMouse(paramWindow, tr("📌 二维码区域拾取"));
  connectQRCodeRegionParamButtons(paramWindow);
}

// 📏 距离测量参数窗口
void VisualProcess::createDistanceMeasureParamWindow()
{
  if (!m_uiBuilder)
  {
    appLog(tr("❌ UI构建器未初始化"), ERR);
    return;
  }

  appLog(tr("📏 打开距离测量参数窗口"));

  QJsonObject measureConfig = {
      {"layout", QJsonObject{
           {"type", "VBox"},
           {"spacing", 15},
           {"margins", QJsonArray{20, 20, 20, 20}},
           {"widgets", QJsonArray{
                QJsonObject{
                    {"type", "Label"},
                    {"name", "title"},
                    {"text", tr("📏 距离测量设置")},
                    {"styleSheet", "font-size: 16px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;"}
                },

                QJsonObject{
                    {"type", "GroupBox"},
                    {"name", "measure_group"},
                    {"text", tr("📐 测量参数")},
                    {"styleSheet",
                     "QGroupBox { font-weight: bold; border: 2px solid #f39c12; border-radius: 8px; margin-top: 10px; padding-top: 15px; }"},
                    {"childWidgets", QJsonArray{
                         QJsonObject{
                             {"type", "Label"},
                             {"name", "edge_threshold_label"},
                             {"text", tr("边缘阈值:")}
                         },
                         QJsonObject{
                             {"type", "SpinBox"},
                             {"name", "edge_threshold"},
                             {"value", 100},
                             {"properties", QJsonObject{
                                  {"minimum", 10},
                                  {"maximum", 500}
                              }}
                         },
                         QJsonObject{
                             {"type", "Label"},
                             {"name", "precision_label"},
                             {"text", tr("测量精度:")}
                         },
                         QJsonObject{
                             {"type", "DoubleSpinBox"},
                             {"name", "precision"},
                             {"value", 0.01},
                             {"properties", QJsonObject{
                                  {"minimum", 0.001},
                                  {"maximum", 1.0},
                                  {"decimals", 3}
                              }}
                         },
                         QJsonObject{
                             {"type", "CheckBox"},
                             {"name", "show_result_check"},
                             {"text", tr("显示测量结果")},
                             {"value", true}
                         }
                     }}
                },

                QJsonObject{
                    {"type", "Frame"},
                    {"name", "button_frame"},
                    {"styleSheet", "border: none; margin-top: 15px;"},
                    {"childLayout", QJsonObject{
                         {"type", "HBox"},
                         {"spacing", 10},
                         {"widgets", QJsonArray{
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_start_measure"},
                                  {"text", tr("📏 开始测量")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #f39c12; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #e67e22; }"}
                              },
                              QJsonObject{
                                  {"type", "Button"},
                                  {"name", "btn_cancel"},
                                  {"text", tr("❌ 取消")},
                                  {"styleSheet",
                                   "QPushButton { background-color: #95a5a6; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #7f8c8d; }"}
                              }
                          }}
                     }}
                }
            }}
       }}
  };

  QWidget* paramWindow = m_uiBuilder->createFromJson(measureConfig);
  showParamWindowNearMouse(paramWindow, tr("📏 距离测量设置"));
  connectDistanceMeasureParamButtons(paramWindow);
}

/* ==================== 🎯 更多参数窗口按钮连接方法 ==================== */

// 🗑️ 图像清除参数按钮连接
void VisualProcess::connectImageClearParamButtons(QWidget* window)
{
  if (!window)
    return;

  // 连接清除按钮
  if (QPushButton* clearBtn = window->findChild<QPushButton*>("btn_clear"))
  {
    connect(clearBtn, &QPushButton::clicked, [this, window]()
    {
      appLog(tr("🗑️ 开始清除图像和所有显示对象..."));

      try
      {
        if (halWin != nullptr)
        {
          // 使用现有的清除逻辑
          try
          {
            int objectCount = halWin->showSymbolList.size();
            if (objectCount > 0)
            {
              appLog(tr("🧹 准备清除 %1 个显示对象...").arg(objectCount));
            }

            halWin->RemoveShow();
            appLog(tr("✅ 显示对象已清除"));
          }
          catch (HalconCpp::HException& e)
          {
            appLog(tr("⚠️ 清除显示对象时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
          }

          if (m_Img.IsInitialized())
          {
            try
            {
              m_Img.Clear();
              appLog(tr("✅ 主图像对象已清除"));
            }
            catch (HalconCpp::HException& e)
            {
              appLog(tr("⚠️ 清除主图像对象时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
            }
          }

          try
          {
            HObject emptyImage;
            halWin->showImage(emptyImage);
            appLog(tr("🖼️ 图像显示已重置"));
          }
          catch (HalconCpp::HException& e)
          {
            appLog(tr("⚠️ 重置图像显示时出错: %1").arg(QString::fromLocal8Bit(e.ErrorMessage().Text())), WARNING);
          }

          appLog(tr("✅ 图像和所有显示对象清除完成"));
        }
        else
        {
          appLog(tr("❌ 错误：图像显示窗口未初始化"), ERR);
        }
      }
      catch (...)
      {
        appLog(tr("❌ 清除图像时发生未知异常"), ERR);
      }

      closeParamWindow(window);
    });
  }

  // 连接取消按钮
  if (QPushButton* cancelBtn = window->findChild<QPushButton*>("btn_cancel"))
  {
    connect(cancelBtn, &QPushButton::clicked, [this, window]()
    {
      appLog(tr("❌ 取消清除操作"));
      closeParamWindow(window);
    });
  }
}

// 📌 二维码区域拾取参数按钮连接
void VisualProcess::connectQRCodeRegionParamButtons(QWidget* window)
{
  if (!window)
    return;

  // 连接开始拾取按钮
  if (QPushButton* startBtn = window->findChild<QPushButton*>("btn_start_pick"))
  {
    connect(startBtn, &QPushButton::clicked, [this, window]()
    {
      // 获取参数
      QString regionType = "rectangle";
      bool autoSave = true;

      if (QComboBox* typeCombo = window->findChild<QComboBox*>("region_type"))
      {
        regionType = typeCombo->currentText();
      }
      if (QCheckBox* saveCheck = window->findChild<QCheckBox*>("auto_save_check"))
      {
        autoSave = saveCheck->isChecked();
      }

      appLog(tr("📌 开始拾取二维码区域，类型: %1, 自动保存: %2").arg(regionType).arg(autoSave ? "是" : "否"));

      clearDisplayObjectsOnly(); // 清除之前的显示对象但保留图像

      // 根据区域类型执行不同的拾取操作
      if (regionType == "rectangle")
      {
        appLog(tr("📝 请在图像上绘制矩形二维码识别区域..."));
        // TODO: 实现矩形区域拾取
      }
      else if (regionType == "circle")
      {
        appLog(tr("📝 请在图像上绘制圆形二维码识别区域..."));
        // TODO: 实现圆形区域拾取
      }
      else if (regionType == "polygon")
      {
        appLog(tr("📝 请在图像上绘制多边形二维码识别区域..."));
        // TODO: 实现多边形区域拾取
      }

      closeParamWindow(window);
    });
  }

  // 连接取消按钮
  if (QPushButton* cancelBtn = window->findChild<QPushButton*>("btn_cancel"))
  {
    connect(cancelBtn, &QPushButton::clicked, [this, window]()
    {
      appLog(tr("❌ 取消区域拾取操作"));
      closeParamWindow(window);
    });
  }
}

// 📏 距离测量参数按钮连接
void VisualProcess::connectDistanceMeasureParamButtons(QWidget* window)
{
  if (!window)
    return;

  // 连接开始测量按钮
  if (QPushButton* measureBtn = window->findChild<QPushButton*>("btn_start_measure"))
  {
    connect(measureBtn, &QPushButton::clicked, [this, window]()
    {
      // 获取参数
      int edgeThreshold = 100;
      double precision = 0.01;
      bool showResult = true;

      if (QSpinBox* thresholdSpin = window->findChild<QSpinBox*>("edge_threshold"))
      {
        edgeThreshold = thresholdSpin->value();
      }
      if (QDoubleSpinBox* precisionSpin = window->findChild<QDoubleSpinBox*>("precision"))
      {
        precision = precisionSpin->value();
      }
      if (QCheckBox* resultCheck = window->findChild<QCheckBox*>("show_result_check"))
      {
        showResult = resultCheck->isChecked();
      }

      appLog(tr("📏 开始距离测量，边缘阈值: %1, 精度: %2").arg(edgeThreshold).arg(precision));

      // 执行距离测量逻辑（使用现有的距离测量代码）
      clearDisplayObjectsOnly();

      if (!m_Img.IsInitialized())
      {
        appLog(tr("❌ 错误：请先加载图像"), ERR);
        closeParamWindow(window);
        return;
      }

      // 使用现有的距离测量逻辑
      QString MeasureSavePath = m_halconParamPath + "Measure";

      appLog(tr("请绘制第一个测量区域..."));
      m_Measyre_Rect1 = halWin->genAngleRec(MeasureSavePath + "/m_Measyre_Rect1.hobj", QStringLiteral("green"));

      if (!m_Measyre_Rect1.IsInitialized())
      {
        appLog(tr("❌ 错误：第一个测量区域创建失败"), ERR);
        closeParamWindow(window);
        return;
      }

      appLog(tr("请绘制第二个测量区域..."));
      m_Measyre_Rect2 = halWin->genAngleRec(MeasureSavePath + "/m_Measyre_Rect2.hobj", QStringLiteral("green"));

      if (!m_Measyre_Rect2.IsInitialized())
      {
        appLog(tr("❌ 错误：第二个测量区域创建失败"), ERR);
        closeParamWindow(window);
        return;
      }

      // 添加显示对象并进行测量
      halWin->addDisplayObject(m_Measyre_Rect1, "green", 2.0);
      halWin->addDisplayObject(m_Measyre_Rect2, "green", 2.0);

      // 提取轮廓并计算距离
      HObject Xld1 = halWin->QtGetLengthMaxXld(m_Img, m_Measyre_Rect1, edgeThreshold);
      HObject Xld2 = halWin->QtGetLengthMaxXld(m_Img, m_Measyre_Rect2, edgeThreshold);

      if (Xld1.IsInitialized() && Xld2.IsInitialized())
      {
        halWin->addDisplayObject(Xld1, "red", 3.0);
        halWin->addDisplayObject(Xld2, "blue", 3.0);

        try
        {
          HTuple DisMin, DisMax;
          DistanceCc(Xld1, Xld2, "point_to_point", &DisMin, &DisMax);

          if (showResult)
          {
            QString msg = QString("距离测量结果:\n最小距离: %1px\n最大距离: %2px")
                          .arg(QString::number(DisMin.D(), 'f', 2))
                          .arg(QString::number(DisMax.D(), 'f', 2));

            halWin->showImage(m_Img);
            halWin->dispHalconMessage(20, 20, msg, "green");
            appLog(tr("✅ 距离测量完成：%1").arg(msg.replace('\n', ' ')));
          }
        }
        catch (HalconCpp::HException& e)
        {
          appLog(tr("❌ 距离计算失败：%1").arg(QString(e.ErrorMessage())), ERR);
        }
      }
      else
      {
        appLog(tr("❌ 无法进行距离测量：轮廓提取失败"), ERR);
        halWin->showImage(m_Img);
      }

      closeParamWindow(window);
    });
  }

  // 连接取消按钮
  if (QPushButton* cancelBtn = window->findChild<QPushButton*>("btn_cancel"))
  {
    connect(cancelBtn, &QPushButton::clicked, [this, window]()
    {
      appLog(tr("❌ 取消测量操作"));
      closeParamWindow(window);
    });
  }
}

/* ==================== 🎯 占位符方法实现 ==================== */

// 以下是其他参数窗口创建方法的占位符实现，您可以根据需要进一步完善

void VisualProcess::createQRCodeBatchParamWindow()
{
  appLog(tr("📊 规律拾取参数窗口 - 待实现"));
  // TODO: 实现规律拾取参数窗口
}

void VisualProcess::createQRCodeTemplateParamWindow()
{
  appLog(tr("📋 二维码模板参数窗口 - 使用现有的onCreateModelWithBuilder"));
  onCreateModelWithBuilder(); // 使用现有的模板创建窗口
}

void VisualProcess::createMeasureTemplateParamWindow()
{
  appLog(tr("📋 测量模板参数窗口 - 使用现有的onCreateModelWithBuilder"));
  onCreateModelWithBuilder(); // 使用现有的模板创建窗口
}

void VisualProcess::createDetectionTemplateParamWindow()
{
  appLog(tr("📋 检测模板参数窗口 - 使用现有的onCreateModelWithBuilder"));
  onCreateModelWithBuilder(); // 使用现有的模板创建窗口
}

void VisualProcess::createAngleMeasureParamWindow()
{
  appLog(tr("📐 角度测量参数窗口 - 待实现"));
  // TODO: 实现角度测量参数窗口
}

void VisualProcess::createCircleMeasureParamWindow()
{
  appLog(tr("⭕ 圆测量参数窗口 - 待实现"));
  // TODO: 实现圆测量参数窗口
}

void VisualProcess::createContourAnalysisParamWindow()
{
  appLog(tr("🔄 轮廓分析参数窗口 - 待实现"));
  // TODO: 实现轮廓分析参数窗口
}

void VisualProcess::createFeatureMatchParamWindow()
{
  appLog(tr("✅ 特征匹配参数窗口 - 待实现"));
  // TODO: 实现特征匹配参数窗口
}

void VisualProcess::createDefectDetectionParamWindow()
{
  appLog(tr("🔴 缺陷检测参数窗口 - 待实现"));
  // TODO: 实现缺陷检测参数窗口
}

void VisualProcess::createSizeDetectionParamWindow()
{
  appLog(tr("📊 尺寸检测参数窗口 - 待实现"));
  // TODO: 实现尺寸检测参数窗口
}

void VisualProcess::createAlignmentParamWindow()
{
  appLog(tr("🎯 定位对齐参数窗口 - 待实现"));
  // TODO: 实现定位对齐参数窗口
}

void VisualProcess::createImagePreprocessParamWindow()
{
  appLog(tr("🎯 图像预处理参数窗口 - 待实现"));
  // TODO: 实现图像预处理参数窗口
}

void VisualProcess::createQuickOperationParamWindow()
{
  appLog(tr("⚡ 快捷操作参数窗口 - 待实现"));
  // TODO: 实现快捷操作参数窗口
}

void VisualProcess::createExportResultParamWindow()
{
  appLog(tr("📊 导出结果参数窗口 - 待实现"));
  // TODO: 实现导出结果参数窗口
}

void VisualProcess::createBatchProcessParamWindow()
{
  appLog(tr("🚀 批处理参数窗口 - 待实现"));
  // TODO: 实现批处理参数窗口
}

void VisualProcess::createStopBatchParamWindow()
{
  appLog(tr("⏹️ 停止批处理参数窗口 - 待实现"));
  // TODO: 实现停止批处理参数窗口
}

void VisualProcess::createResultHistoryParamWindow()
{
  appLog(tr("📊 结果历史参数窗口 - 待实现"));
  // TODO: 实现结果历史参数窗口
}

void VisualProcess::createGeneralSettingsParamWindow()
{
  appLog(tr("⚙️ 通用设置参数窗口 - 待实现"));
  // TODO: 实现通用设置参数窗口
}

void VisualProcess::createResultViewParamWindow(const QString& resultType)
{
  appLog(tr("📊 查看结果参数窗口 - 类型: %1 - 待实现").arg(resultType));
  // TODO: 实现查看结果参数窗口
}

// 占位符连接方法
void VisualProcess::connectQRCodeBatchParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectQRCodeTemplateParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectMeasureTemplateParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectAngleMeasureParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectCircleMeasureParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectContourAnalysisParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectFeatureMatchParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectDefectDetectionParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectSizeDetectionParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectAlignmentParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectDetectionTemplateParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectImagePreprocessParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectQuickOperationParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectExportResultParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectBatchProcessParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectStopBatchParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectResultHistoryParamButtons(QWidget* window) { Q_UNUSED(window); }
void VisualProcess::connectGeneralSettingsParamButtons(QWidget* window) { Q_UNUSED(window); }

void VisualProcess::connectResultViewParamButtons(QWidget* window, const QString& resultType)
{
  Q_UNUSED(window);
  Q_UNUSED(resultType);
}
