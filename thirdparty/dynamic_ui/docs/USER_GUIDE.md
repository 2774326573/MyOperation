# 🎨 动态UI构建器用户指南 / Dynamic UI Builder User Guide

<div align="center">

![Dynamic UI Builder](https://img.shields.io/badge/DynamicUI-v1.0.0-blue.svg)
![Qt Version](https://img.shields.io/badge/Qt-6.0+-green.svg)
![C++ Standard](https://img.shields.io/badge/C++-17-orange.svg)
![License](https://img.shields.io/badge/license-MIT-yellow.svg)

**一个强大而优雅的Qt动态UI创建库**  
*A powerful and elegant Qt dynamic UI creation library*

[🚀 快速开始](#-快速开始--quick-start) | [📚 API文档](#-api参考--api-reference) | [🎯 示例](#-使用示例--examples) | [🎨 主题](#-主题系统--theme-system)

</div>

---

## 🌟 特性概览 / Features Overview

<table>
<tr>
<td align="center">🎛️</td>
<td><strong>20+ 控件类型</strong><br/>支持所有常用Qt控件，从按钮到复杂容器</td>
</tr>
<tr>
<td align="center">⚡</td>
<td><strong>多种配置方式</strong><br/>支持代码配置、JSON配置和便捷构建器</td>
</tr>
<tr>
<td align="center">🎭</td>
<td><strong>丰富动画效果</strong><br/>内置7种动画类型，让UI更生动</td>
</tr>
<tr>
<td align="center">🎨</td>
<td><strong>主题系统</strong><br/>现代/暗色主题，支持自定义样式</td>
</tr>
<tr>
<td align="center">🔗</td>
<td><strong>灵活事件绑定</strong><br/>Lambda函数、信号槽，多种绑定方式</td>
</tr>
<tr>
<td align="center">🧩</td>
<td><strong>可扩展架构</strong><br/>支持自定义控件注册和扩展</td>
</tr>
</table>

---

## 🚀 快速开始 / Quick Start

### 📦 安装和配置

#### 1. CMake 集成

```cmake
# 添加到你的CMakeLists.txt
add_subdirectory(thirdparty/dynamic_ui)

target_link_libraries(your_project PRIVATE DynamicUI)
target_include_directories(your_project PRIVATE 
    thirdparty/dynamic_ui/include
)
```

#### 2. 基本包含

```cpp
#include "DynamicUIBuilder.h"
using namespace DynamicUI;  // 使用便捷构建器
```

### 🎯 30秒创建第一个控件

```cpp
// 1. 创建构建器
DynamicUIBuilder builder;

// 2. 创建按钮
auto myButton = builder.createWidget(
    button("hello_btn")
    .text("你好世界 / Hello World")
    .onClick([]() { 
        qDebug() << "Hello from Dynamic UI!"; 
    })
    .animation(DynamicUIBuilder::AnimationType::FadeIn)
    .build(),
    parentWidget
);

// 3. 显示
myButton->show();
```

---

## 🎛️ 支持的控件类型 / Supported Widgets

<div align="center">

| 类别 | 控件 | 描述 |
|------|------|------|
| 🔘 **按钮** | `QPushButton`<br/>`QToolButton` | 标准按钮和工具按钮 |
| ✏️ **输入** | `QLineEdit`<br/>`QTextEdit`<br/>`QSpinBox`<br/>`QDoubleSpinBox` | 文本输入和数值输入 |
| ☑️ **选择** | `QCheckBox`<br/>`QRadioButton`<br/>`QComboBox`<br/>`QSlider` | 选择和滑块控件 |
| 📊 **显示** | `QLabel`<br/>`QProgressBar` | 文本标签和进度条 |
| 📦 **容器** | `QGroupBox`<br/>`QTabWidget`<br/>`QSplitter`<br/>`QScrollArea`<br/>`QFrame` | 各种容器控件 |
| 📋 **列表** | `QTreeWidget`<br/>`QTableWidget`<br/>`QListWidget` | 树形、表格和列表 |

</div>

---

## 🎨 使用示例 / Examples

### 📋 示例1: 创建登录表单

```cpp
// 创建登录表单布局
auto loginLayout = vBoxLayout()
    .spacing(15)
    .margins(QMargins(30, 20, 30, 20))
    
    // 标题
    .addWidget(
        label("title")
        .text("🔐 用户登录 / User Login")
        .style("font-size: 18px; font-weight: bold; color: #2c3e50; text-align: center;")
        .build()
    )
    
    // 用户名输入
    .addWidget(
        lineEdit("username")
        .text("请输入用户名 / Enter username")
        .icon(":/icons/user.png")
        .animation(DynamicUIBuilder::AnimationType::SlideIn, 400)
        .build()
    )
    
    // 密码输入
    .addWidget(
        lineEdit("password")
        .text("请输入密码 / Enter password")
        .property("echoMode", QLineEdit::Password)
        .icon(":/icons/lock.png")
        .animation(DynamicUIBuilder::AnimationType::SlideIn, 600)
        .build()
    )
    
    // 登录按钮
    .addWidget(
        button("login_btn")
        .text("🚀 登录 / Login")
        .style(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #4CAF50, stop:1 #45a049);
                color: white;
                border-radius: 8px;
                padding: 10px 20px;
                font-weight: bold;
                font-size: 14px;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #45a049, stop:1 #3d8b40);
            }
        )")
        .onClick([this]() {
            // 登录逻辑
            QString username = builder->getWidget("username")->property("text").toString();
            QString password = builder->getWidget("password")->property("text").toString();
            handleLogin(username, password);
        })
        .animation(DynamicUIBuilder::AnimationType::Bounce, 800)
        .build()
    )
    .build();

QWidget* loginForm = builder.createFromConfig(loginLayout, this);
```

### 🛠️ 示例2: 创建工具栏

```cpp
auto toolbar = hBoxLayout()
    .spacing(8)
    .margins(QMargins(10, 5, 10, 5))
    
    .addWidget(
        toolButton("new_file")
        .text("新建 / New")
        .icon(":/icons/new.png")
        .tooltip("创建新文件 / Create new file (Ctrl+N)")
        .onClick([this]() { newFile(); })
        .build()
    )
    
    .addWidget(frame("separator").build())  // 分隔符
    
    .addWidget(
        toolButton("open_file")
        .text("打开 / Open")
        .icon(":/icons/open.png")
        .tooltip("打开文件 / Open file (Ctrl+O)")
        .onClick([this]() { openFile(); })
        .build()
    )
    
    .addWidget(
        toolButton("save_file")
        .text("保存 / Save")
        .icon(":/icons/save.png")
        .tooltip("保存文件 / Save file (Ctrl+S)")
        .onClick([this]() { saveFile(); })
        .build()
    )
    .build();
```

### 📊 示例3: JSON配置的仪表板

```cpp
QJsonObject dashboardConfig = {
    {"layout", QJsonObject{
        {"type", "Grid"},
        {"columns", 2},
        {"spacing", 15},
        {"widgets", QJsonArray{
            // CPU使用率
            QJsonObject{
                {"type", "GroupBox"},
                {"name", "cpu_group"},
                {"text", "💻 CPU使用率 / CPU Usage"},
                {"styleSheet", "QGroupBox { font-weight: bold; border: 2px solid #3498db; border-radius: 8px; margin: 10px; padding-top: 15px; }"}
            },
            // 内存使用率
            QJsonObject{
                {"type", "GroupBox"},
                {"name", "memory_group"},
                {"text", "🧠 内存使用率 / Memory Usage"},
                {"styleSheet", "QGroupBox { font-weight: bold; border: 2px solid #e74c3c; border-radius: 8px; margin: 10px; padding-top: 15px; }"}
            },
            // 网络状态
            QJsonObject{
                {"type", "GroupBox"},
                {"name", "network_group"},
                {"text", "🌐 网络状态 / Network Status"},
                {"styleSheet", "QGroupBox { font-weight: bold; border: 2px solid #f39c12; border-radius: 8px; margin: 10px; padding-top: 15px; }"}
            },
            // 系统信息
            QJsonObject{
                {"type", "GroupBox"},
                {"name", "system_group"},
                {"text", "ℹ️ 系统信息 / System Info"},
                {"styleSheet", "QGroupBox { font-weight: bold; border: 2px solid #9b59b6; border-radius: 8px; margin: 10px; padding-top: 15px; }"}
            }
        }}
    }}
};

QWidget* dashboard = builder.createFromJson(dashboardConfig, this);
```

---

## 🎭 动画效果系统 / Animation System

### 🎬 可用动画类型

<table>
<tr>
<th>动画类型</th>
<th>效果描述</th>
<th>适用场景</th>
<th>持续时间建议</th>
</tr>
<tr>
<td><code>FadeIn</code> 🌅</td>
<td>淡入效果</td>
<td>内容加载、弹窗显示</td>
<td>300-500ms</td>
</tr>
<tr>
<td><code>FadeOut</code> 🌄</td>
<td>淡出效果</td>
<td>内容隐藏、弹窗关闭</td>
<td>200-400ms</td>
</tr>
<tr>
<td><code>SlideIn</code> ➡️</td>
<td>滑入效果</td>
<td>侧边栏、抽屉菜单</td>
<td>250-400ms</td>
</tr>
<tr>
<td><code>SlideOut</code> ⬅️</td>
<td>滑出效果</td>
<td>面板隐藏</td>
<td>200-350ms</td>
</tr>
<tr>
<td><code>Bounce</code> 🎾</td>
<td>弹跳效果</td>
<td>按钮点击、成功提示</td>
<td>400-600ms</td>
</tr>
<tr>
<td><code>Pulse</code> 💓</td>
<td>脉冲效果</td>
<td>重要通知、警告</td>
<td>500-800ms</td>
</tr>
<tr>
<td><code>Shake</code> 📳</td>
<td>震动效果</td>
<td>错误提示、无效操作</td>
<td>300-500ms</td>
</tr>
</table>

### 🎨 动画使用示例

```cpp
// 成功提示动画
button("success")
    .text("✅ 保存成功")
    .animation(DynamicUIBuilder::AnimationType::Bounce, 500)
    .build();

// 错误提示动画
lineEdit("error_input")
    .text("输入有误")
    .animation(DynamicUIBuilder::AnimationType::Shake, 400)
    .build();

// 加载动画
progressBar("loading")
    .animation(DynamicUIBuilder::AnimationType::Pulse, 1000)
    .build();
```

---

## 🎨 主题系统 / Theme System

### 🌈 内置主题

#### 🌞 Modern 主题（现代风格）
```cpp
builder.applyTheme(widget, "modern");
```
- 🎨 蓝色主色调 (#3498db)
- ⭕ 圆角边框 (4px)
- 🌟 悬停效果
- 💫 平滑过渡

#### 🌙 Dark 主题（暗色风格）
```cpp
builder.applyTheme(widget, "dark");
```
- 🖤 深色背景 (#2c3e50, #34495e)
- ⚪ 白色文字
- 🔗 高对比度
- 👁️ 护眼设计

### 🛠️ 自定义主题

```json
{
  "custom_theme": {
    "QPushButton": {
      "background-color": "#e74c3c",
      "color": "white",
      "border-radius": "6px",
      "padding": "10px 15px",
      "font-weight": "bold"
    },
    "QPushButton:hover": {
      "background-color": "#c0392b"
    },
    "QLineEdit": {
      "border": "2px solid #95a5a6",
      "border-radius": "4px",
      "padding": "8px",
      "font-size": "14px"
    }
  }
}
```

```cpp
// 加载自定义主题
builder.loadThemeFile("themes/custom.json");
builder.applyTheme(widget, "custom_theme");
```

---

## 🔗 事件处理系统 / Event Handling

### 🎯 Lambda 函数绑定

```cpp
// 按钮点击事件
button("action_btn")
    .onClick([this]() {
        QMessageBox::information(this, "提示", "按钮被点击了！");
    })
    .build();

// 值变化事件
spinBox("count")
    .onValueChanged([this](const QVariant& value) {
        qDebug() << "新值:" << value.toInt();
        updateDisplay(value.toInt());
    })
    .build();
```

### 📡 信号槽连接

```cpp
// 连接构建器信号
connect(&builder, &DynamicUIBuilder::widgetClicked,
    [this](const QString& name) {
        handleWidgetClick(name);
    });

connect(&builder, &DynamicUIBuilder::widgetValueChanged,
    [this](const QString& name, const QVariant& value) {
        handleValueChange(name, value);
    });
```

---

## 🧩 自定义控件扩展 / Custom Widget Extension

### 📝 注册自定义控件

```cpp
// 注册自定义图表控件
builder.registerCustomWidget("ChartWidget", 
    [](const DynamicUIBuilder::WidgetConfig& config, QWidget* parent) -> QWidget* {
        auto chart = new QCustomPlot(parent);
        
        // 配置图表
        chart->setObjectName(config.name);
        if (config.properties.contains("chartType")) {
            QString type = config.properties["chartType"].toString();
            setupChart(chart, type);
        }
        
        return chart;
    });

// 使用自定义控件
auto chartConfig = DynamicUIBuilder::WidgetConfig{
    .type = DynamicUIBuilder::WidgetType::Custom,
    .name = "ChartWidget",
    .text = "销售数据图表",
    .properties = {{"chartType", "line"}}
};

QWidget* chart = builder.createWidget(chartConfig, parent);
```

---

## 📚 API参考 / API Reference

### 🏗️ DynamicUIBuilder 类

#### 主要方法

```cpp
class DynamicUIBuilder : public QObject {
public:
    // 控件创建
    QWidget* createWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    QWidget* createFromConfig(const LayoutConfig& config, QWidget* parent = nullptr);
    QWidget* createFromJson(const QJsonObject& json, QWidget* parent = nullptr);
    
    // 主题管理
    void applyTheme(QWidget* widget, const QString& themeName);
    bool loadThemeFile(const QString& filePath);
    void setGlobalStyle(const QString& styleSheet);
    
    // 动画控制
    void playAnimation(QWidget* widget, AnimationType type, int duration = 300);
    
    // 控件管理
    QWidget* getWidget(const QString& name) const;
    void registerCustomWidget(const QString& typeName, 
                             std::function<QWidget*(const WidgetConfig&, QWidget*)> creator);
    
signals:
    void widgetCreated(QWidget* widget, const QString& name);
    void widgetClicked(const QString& name);
    void widgetValueChanged(const QString& name, const QVariant& value);
};
```

### 🔧 便捷构建器

```cpp
namespace DynamicUI {
    // 控件构建器
    WidgetConfigBuilder button(const QString& name = "");
    WidgetConfigBuilder toolButton(const QString& name = "");
    WidgetConfigBuilder label(const QString& name = "");
    WidgetConfigBuilder lineEdit(const QString& name = "");
    // ... 更多控件类型
    
    // 布局构建器
    LayoutConfigBuilder vBoxLayout();
    LayoutConfigBuilder hBoxLayout();
    LayoutConfigBuilder gridLayout(int columns = 3);
    LayoutConfigBuilder formLayout();
}
```

---

## 🛠️ 最佳实践 / Best Practices

### ✅ 推荐做法

1. **🏷️ 使用有意义的控件名称**
   ```cpp
   button("save_document_btn")  // ✅ 清晰明确
   button("btn1")               // ❌ 不够描述性
   ```

2. **⏱️ 合理设置动画时长**
   ```cpp
   .animation(AnimationType::FadeIn, 300)    // ✅ 适中
   .animation(AnimationType::FadeIn, 2000)   // ❌ 过长
   ```

3. **🎨 保持主题一致性**
   ```cpp
   // ✅ 统一应用主题
   builder.applyTheme(mainWidget, "modern");
   ```

4. **🔧 合理使用自定义属性**
   ```cpp
   spinBox("age")
       .property("minimum", 0)
       .property("maximum", 120)
       .property("suffix", " 岁")
   ```

### ⚠️ 注意事项

- 🚫 避免在循环中频繁创建控件
- 🔄 及时清理不需要的动画资源
- 📱 考虑不同屏幕分辨率的适配
- 🌍 支持国际化文本

---

## 🔧 故障排除 / Troubleshooting

### ❓ 常见问题

<details>
<summary><strong>Q: 控件创建后不显示？</strong></summary>

**A:** 检查以下几点：
1. 确保设置了正确的父控件
2. 调用了 `show()` 方法
3. 添加到了布局中

```cpp
QWidget* widget = builder.createWidget(config, parent);
widget->show();  // 或者添加到布局
layout->addWidget(widget);
```
</details>

<details>
<summary><strong>Q: 动画效果不生效？</strong></summary>

**A:** 可能的原因：
1. 控件尚未显示
2. 动画持续时间过短
3. 控件大小为0

```cpp
// 确保控件可见后再播放动画
widget->show();
QTimer::singleShot(50, [=]() {
    builder.playAnimation(widget, AnimationType::FadeIn, 500);
});
```
</details>

<details>
<summary><strong>Q: 主题样式不生效？</strong></summary>

**A:** 检查：
1. 主题文件路径是否正确
2. JSON格式是否有效
3. 选择器是否匹配

```cpp
// 先加载主题文件
if (builder.loadThemeFile("themes/custom.json")) {
    builder.applyTheme(widget, "custom");
} else {
    qWarning() << "主题文件加载失败";
}
```
</details>

---

## 📈 性能优化建议 / Performance Tips

### 🚀 优化技巧

1. **📦 批量创建控件**
   ```cpp
   // ✅ 批量配置
   auto layout = vBoxLayout();
   for (const auto& config : widgetConfigs) {
       layout.addWidget(createWidget(config));
   }
   ```

2. **🎭 按需加载动画**
   ```cpp
   // ✅ 只在需要时播放动画
   if (animationEnabled) {
       widget.animation(AnimationType::FadeIn);
   }
   ```

3. **💾 缓存常用配置**
   ```cpp
   // ✅ 缓存样式配置
   static const QString buttonStyle = "QPushButton { ... }";
   ```

---

## 📞 技术支持 / Support

### 🆘 获取帮助

- 📖 **文档**: 查看完整API文档
- 🐛 **问题**: 提交到 GitHub Issues
- 💬 **讨论**: 参与社区讨论
- 📧 **联系**: 发送邮件获取支持

### 🤝 贡献代码

欢迎提交 Pull Request！请确保：

1. ✅ 遵循代码规范
2. 📝 添加适当的文档
3. 🧪 包含单元测试
4. 🔍 通过代码审查

---

<div align="center">

**感谢使用 Dynamic UI Builder！**  
*Thank you for using Dynamic UI Builder!*

🌟 如果这个库对您有帮助，请给我们一个Star！  
*If this library helps you, please give us a star!*

</div> 