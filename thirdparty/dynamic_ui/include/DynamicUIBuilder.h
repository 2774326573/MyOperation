#ifndef DYNAMIC_UI_BUILDER_H
#define DYNAMIC_UI_BUILDER_H

#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSlider>
#include <QProgressBar>
#include <QGroupBox>
#include <QTabWidget>
#include <QSplitter>
#include <QScrollArea>
#include <QTreeWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QFrame>
#include <QMovie>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QHash>
#include <QVariant>
#include <QMetaObject>
#include <QMetaMethod>
#include <functional>

/**
 * @brief 动态UI构建器类 / Dynamic UI Builder Class
 * 
 * 这个类提供了一个强大的动态UI创建系统，可以根据JSON配置或代码配置
 * 动态创建各种Qt控件，支持样式定制、事件绑定、动画效果等
 * 
 * This class provides a powerful dynamic UI creation system that can 
 * dynamically create various Qt widgets based on JSON configuration or code configuration,
 * supporting style customization, event binding, animation effects, etc.
 */
class DynamicUIBuilder : public QObject
{
    Q_OBJECT

public:
    // 前向声明 / Forward declarations
    struct LayoutConfig;

    /**
     * @brief 控件类型枚举 / Widget Type Enumeration
     */
    enum class WidgetType {
        Button,           // 普通按钮 / Normal Button
        ToolButton,       // 工具按钮 / Tool Button  
        Label,            // 标签 / Label
        LineEdit,         // 单行编辑器 / Line Edit
        TextEdit,         // 多行编辑器 / Text Edit
        SpinBox,          // 整数选择器 / Spin Box
        DoubleSpinBox,    // 浮点数选择器 / Double Spin Box
        CheckBox,         // 复选框 / Check Box
        RadioButton,      // 单选按钮 / Radio Button
        ComboBox,         // 下拉框 / Combo Box
        Slider,           // 滑块 / Slider
        ProgressBar,      // 进度条 / Progress Bar
        GroupBox,         // 分组框 / Group Box
        TabWidget,        // 标签页控件 / Tab Widget
        Splitter,         // 分割器 / Splitter
        ScrollArea,       // 滚动区域 / Scroll Area
        TreeWidget,       // 树形控件 / Tree Widget
        TableWidget,      // 表格控件 / Table Widget
        ListWidget,       // 列表控件 / List Widget
        Frame,            // 框架 / Frame
        Custom            // 自定义控件 / Custom Widget
    };

    /**
     * @brief 布局类型枚举 / Layout Type Enumeration
     */
    enum class LayoutType {
        VBox,     // 垂直布局 / Vertical Box Layout
        HBox,     // 水平布局 / Horizontal Box Layout
        Grid,     // 网格布局 / Grid Layout
        Form      // 表单布局 / Form Layout
    };

    /**
     * @brief 动画类型枚举 / Animation Type Enumeration
     */
    enum class AnimationType {
        None,           // 无动画 / No Animation
        FadeIn,         // 淡入 / Fade In
        FadeOut,        // 淡出 / Fade Out
        SlideIn,        // 滑入 / Slide In
        SlideOut,       // 滑出 / Slide Out
        Bounce,         // 弹跳 / Bounce
        Pulse,          // 脉冲 / Pulse
        Shake,          // 震动 / Shake
        Custom          // 自定义动画 / Custom Animation
    };

    /**
     * @brief 控件配置结构 / Widget Configuration Structure
     */
    struct WidgetConfig {
        WidgetType type;                    // 控件类型 / Widget Type
        QString name;                       // 控件名称 / Widget Name
        QString text;                       // 显示文本 / Display Text
        QString tooltip;                    // 工具提示 / Tooltip
        QString iconPath;                   // 图标路径 / Icon Path
        QString styleSheet;                 // 样式表 / Style Sheet
        QSize size;                         // 尺寸 / Size
        QSize minSize;                      // 最小尺寸 / Minimum Size
        QSize maxSize;                      // 最大尺寸 / Maximum Size
        QVariant value;                     // 控件值 / Widget Value
        QVariantMap properties;             // 其他属性 / Other Properties
        QString animationGifPath;           // 动画GIF路径 / Animation GIF Path
        AnimationType animation;            // 动画类型 / Animation Type
        int animationDuration;              // 动画持续时间(ms) / Animation Duration (ms)
        std::function<void()> clickCallback; // 点击回调 / Click Callback
        std::function<void(const QVariant&)> valueChangedCallback; // 值改变回调 / Value Changed Callback
        
        // 新增：容器控件的子布局支持 / New: Sub-layout support for container widgets
        LayoutConfig* childLayout = nullptr;                   // 子布局配置 / Child Layout Configuration
        QVector<WidgetConfig> childWidgets;                    // 子控件配置 / Child Widget Configurations
    };

    /**
     * @brief 布局配置结构 / Layout Configuration Structure
     */
    struct LayoutConfig {
        LayoutType type;                    // 布局类型 / Layout Type
        int spacing;                        // 间距 / Spacing
        QMargins margins;                   // 边距 / Margins
        int columns;                        // 网格布局列数 / Grid Layout Columns
        QVector<WidgetConfig> widgets;      // 子控件配置 / Child Widget Configurations
        QVector<LayoutConfig> subLayouts;   // 子布局配置 / Sub Layout Configurations
    };

public:
    explicit DynamicUIBuilder(QObject *parent = nullptr);
    ~DynamicUIBuilder();

    /**
     * @brief 从JSON配置创建UI / Create UI from JSON Configuration
     * @param jsonConfig JSON配置对象 / JSON Configuration Object
     * @param parent 父控件 / Parent Widget
     * @return 创建的控件 / Created Widget
     */
    QWidget* createFromJson(const QJsonObject& jsonConfig, QWidget* parent = nullptr);

    /**
     * @brief 从代码配置创建UI / Create UI from Code Configuration
     * @param layoutConfig 布局配置 / Layout Configuration
     * @param parent 父控件 / Parent Widget
     * @return 创建的控件 / Created Widget
     */
    QWidget* createFromConfig(const LayoutConfig& layoutConfig, QWidget* parent = nullptr);

    /**
     * @brief 创建单个控件 / Create Single Widget
     * @param config 控件配置 / Widget Configuration
     * @param parent 父控件 / Parent Widget
     * @return 创建的控件 / Created Widget
     */
    QWidget* createWidget(const WidgetConfig& config, QWidget* parent = nullptr);

    /**
     * @brief 创建布局 / Create Layout
     * @param config 布局配置 / Layout Configuration
     * @param parent 父控件 / Parent Widget
     * @return 创建的布局 / Created Layout
     */
    QLayout* createLayout(const LayoutConfig& config, QWidget* parent = nullptr);

    /**
     * @brief 应用样式主题 / Apply Style Theme
     * @param widget 目标控件 / Target Widget
     * @param themeName 主题名称 / Theme Name
     */
    void applyTheme(QWidget* widget, const QString& themeName);

    /**
     * @brief 播放控件动画 / Play Widget Animation
     * @param widget 目标控件 / Target Widget
     * @param animation 动画类型 / Animation Type
     * @param duration 持续时间 / Duration
     */
    void playAnimation(QWidget* widget, AnimationType animation, int duration = 300);

    /**
     * @brief 注册自定义控件创建器 / Register Custom Widget Creator
     * @param typeName 类型名称 / Type Name
     * @param creator 创建器函数 / Creator Function
     */
    void registerCustomWidget(const QString& typeName, std::function<QWidget*(const WidgetConfig&, QWidget*)> creator);

    /**
     * @brief 获取控件通过名称 / Get Widget by Name
     * @param name 控件名称 / Widget Name
     * @return 控件指针 / Widget Pointer
     */
    QWidget* getWidget(const QString& name) const;

    /**
     * @brief 设置全局样式 / Set Global Style
     * @param styleSheet 样式表 / Style Sheet
     */
    void setGlobalStyle(const QString& styleSheet);

    /**
     * @brief 加载样式主题文件 / Load Style Theme File
     * @param themeFilePath 主题文件路径 / Theme File Path
     */
    bool loadThemeFile(const QString& themeFilePath);

    /**
     * @brief 设置调试模式 / Set Debug Mode
     * @param enabled 是否启用调试模式 / Enable debug mode
     */
    void setDebugMode(bool enabled);

signals:
    /**
     * @brief 控件创建完成信号 / Widget Created Signal
     * @param widget 创建的控件 / Created Widget
     * @param name 控件名称 / Widget Name
     */
    void widgetCreated(QWidget* widget, const QString& name);

    /**
     * @brief 控件点击信号 / Widget Clicked Signal
     * @param name 控件名称 / Widget Name
     */
    void widgetClicked(const QString& name);

    /**
     * @brief 控件值改变信号 / Widget Value Changed Signal
     * @param name 控件名称 / Widget Name
     * @param value 新值 / New Value
     */
    void widgetValueChanged(const QString& name, const QVariant& value);

private slots:
    void onWidgetClicked();
    void onWidgetValueChanged(const QVariant& value);

private:
    // 内部辅助方法 / Internal Helper Methods
    QWidget* createButtonWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createToolButtonWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createLabelWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createLineEditWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createTextEditWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createSpinBoxWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createDoubleSpinBoxWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createCheckBoxWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createRadioButtonWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createComboBoxWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createSliderWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createProgressBarWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createGroupBoxWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createTabWidgetWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createSplitterWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createScrollAreaWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createTreeWidgetWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createTableWidgetWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createListWidgetWidget(const WidgetConfig& config, QWidget* parent);
    QWidget* createFrameWidget(const WidgetConfig& config, QWidget* parent);

    void applyWidgetProperties(QWidget* widget, const WidgetConfig& config);
    void setupWidgetAnimation(QWidget* widget, const WidgetConfig& config);
    void connectWidgetSignals(QWidget* widget, const WidgetConfig& config);
    WidgetConfig parseJsonWidgetConfig(const QJsonObject& json);
    LayoutConfig parseJsonLayoutConfig(const QJsonObject& json);
    QString getDefaultStyleSheet(WidgetType type, const QString& themeName = "modern");
    
    // 新增：容器控件内部布局处理方法 / New: Container widget internal layout handling methods
    void setupContainerLayout(QWidget* containerWidget, const WidgetConfig& config);
    QLayout* createLayoutForContainer(const LayoutConfig& layoutConfig, QWidget* container);
    void addChildWidgetsToContainer(QWidget* container, const QVector<WidgetConfig>& childConfigs);
    bool isContainerWidget(WidgetType type) const;
    
    // 新增：调试和日志方法 / New: Debug and logging methods
    void logDebug(const QString& message) const;

private:
    QHash<QString, QWidget*> m_widgets;           // 控件映射 / Widget Map
    QHash<QString, QMovie*> m_animations;         // 动画映射 / Animation Map
    QHash<QString, std::function<QWidget*(const WidgetConfig&, QWidget*)>> m_customCreators; // 自定义创建器 / Custom Creators
    QHash<QString, QString> m_themes;             // 主题样式 / Theme Styles
    QString m_globalStyle;                        // 全局样式 / Global Style
    
    // 新增：调试和优化相关成员 / New: Debug and optimization related members
    bool m_debugMode = false;                     // 调试模式 / Debug Mode
    mutable QStringList m_debugLog;               // 调试日志 / Debug Log
};

// 便捷的配置构建器 / Convenient Configuration Builder
namespace DynamicUI {
    /**
     * @brief 控件配置构建器 / Widget Configuration Builder
     */
    class WidgetConfigBuilder {
    public:
        WidgetConfigBuilder(DynamicUIBuilder::WidgetType type);
        
        WidgetConfigBuilder& name(const QString& name);
        WidgetConfigBuilder& text(const QString& text);
        WidgetConfigBuilder& tooltip(const QString& tooltip);
        WidgetConfigBuilder& icon(const QString& iconPath);
        WidgetConfigBuilder& style(const QString& styleSheet);
        WidgetConfigBuilder& size(const QSize& size);
        WidgetConfigBuilder& minSize(const QSize& minSize);
        WidgetConfigBuilder& maxSize(const QSize& maxSize);
        WidgetConfigBuilder& value(const QVariant& value);
        WidgetConfigBuilder& property(const QString& key, const QVariant& value);
        WidgetConfigBuilder& animationGif(const QString& gifPath);
        WidgetConfigBuilder& animation(DynamicUIBuilder::AnimationType type, int duration = 300);
        WidgetConfigBuilder& onClick(std::function<void()> callback);
        WidgetConfigBuilder& onValueChanged(std::function<void(const QVariant&)> callback);
        
        // 新增：子控件和子布局配置方法 / New: Child widget and layout configuration methods
        WidgetConfigBuilder& addChildWidget(const DynamicUIBuilder::WidgetConfig& childConfig);
        WidgetConfigBuilder& setChildLayout(const DynamicUIBuilder::LayoutConfig& layoutConfig);
        
        DynamicUIBuilder::WidgetConfig build();
        
    private:
        DynamicUIBuilder::WidgetConfig m_config;
    };

    /**
     * @brief 布局配置构建器 / Layout Configuration Builder
     */
    class LayoutConfigBuilder {
    public:
        LayoutConfigBuilder(DynamicUIBuilder::LayoutType type);
        
        LayoutConfigBuilder& spacing(int spacing);
        LayoutConfigBuilder& margins(const QMargins& margins);
        LayoutConfigBuilder& columns(int columns);
        LayoutConfigBuilder& addWidget(const DynamicUIBuilder::WidgetConfig& config);
        LayoutConfigBuilder& addLayout(const DynamicUIBuilder::LayoutConfig& config);
        
        DynamicUIBuilder::LayoutConfig build() const;
        
    private:
        DynamicUIBuilder::LayoutConfig m_config;
    };

    // 便捷函数 / Convenience Functions
    WidgetConfigBuilder button(const QString& name = "");
    WidgetConfigBuilder toolButton(const QString& name = "");
    WidgetConfigBuilder label(const QString& name = "");
    WidgetConfigBuilder lineEdit(const QString& name = "");
    WidgetConfigBuilder textEdit(const QString& name = "");
    WidgetConfigBuilder spinBox(const QString& name = "");
    WidgetConfigBuilder doubleSpinBox(const QString& name = "");
    WidgetConfigBuilder checkBox(const QString& name = "");
    WidgetConfigBuilder radioButton(const QString& name = "");
    WidgetConfigBuilder comboBox(const QString& name = "");
    WidgetConfigBuilder slider(const QString& name = "");
    WidgetConfigBuilder progressBar(const QString& name = "");
    WidgetConfigBuilder groupBox(const QString& name = "");
    WidgetConfigBuilder tabWidget(const QString& name = "");
    WidgetConfigBuilder splitter(const QString& name = "");
    WidgetConfigBuilder scrollArea(const QString& name = "");
    WidgetConfigBuilder treeWidget(const QString& name = "");
    WidgetConfigBuilder tableWidget(const QString& name = "");
    WidgetConfigBuilder listWidget(const QString& name = "");
    WidgetConfigBuilder frame(const QString& name = "");

    LayoutConfigBuilder vBoxLayout();
    LayoutConfigBuilder hBoxLayout();
    LayoutConfigBuilder gridLayout(int columns = 3);
    LayoutConfigBuilder formLayout();
}

#endif // DYNAMIC_UI_BUILDER_H 