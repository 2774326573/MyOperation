#include "../include/DynamicUIBuilder.h"
#include <QIcon>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QEasingCurve>
#include <QMovie>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QHash>
#include <QVariant>
#include <QMetaObject>
#include <QMetaMethod>
#include <QDateTime>
#include <functional>

DynamicUIBuilder::DynamicUIBuilder(QObject *parent)
    : QObject(parent)
{
    // 初始化默认主题 / Initialize default themes
    m_themes["modern"] = R"(
        QPushButton {
            background-color: #3498db;
            border: none;
            color: white;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #21618c;
        }
        QToolButton {
            background-color: #ecf0f1;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            padding: 6px;
        }
        QToolButton:hover {
            background-color: #d5dbdb;
        }
        QLineEdit {
            border: 2px solid #bdc3c7;
            border-radius: 4px;
            padding: 6px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #3498db;
        }
    )";
    
    m_themes["dark"] = R"(
        QPushButton {
            background-color: #2c3e50;
            border: none;
            color: white;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #34495e;
        }
        QToolButton {
            background-color: #34495e;
            border: 1px solid #2c3e50;
            border-radius: 4px;
            padding: 6px;
            color: white;
        }
        QLineEdit {
            background-color: #2c3e50;
            border: 2px solid #34495e;
            border-radius: 4px;
            padding: 6px;
            color: white;
            font-size: 14px;
        }
    )";
}

DynamicUIBuilder::~DynamicUIBuilder()
{
    // 清理动画资源 / Clean up animation resources
    for (auto movie : m_animations) {
        if (movie) {
            movie->stop();
            delete movie;
        }
    }
    m_animations.clear();
}

QWidget* DynamicUIBuilder::createFromJson(const QJsonObject& jsonConfig, QWidget* parent)
{
    if (jsonConfig.contains("layout")) {
        LayoutConfig config = parseJsonLayoutConfig(jsonConfig["layout"].toObject());
        return createFromConfig(config, parent);
    } else if (jsonConfig.contains("widget")) {
        WidgetConfig config = parseJsonWidgetConfig(jsonConfig["widget"].toObject());
        return createWidget(config, parent);
    }
    
    qWarning() << tr("无效的JSON配置 / Invalid JSON configuration");
    return nullptr;
}

QWidget* DynamicUIBuilder::createFromConfig(const LayoutConfig& layoutConfig, QWidget* parent)
{
    QWidget* container = new QWidget(parent);
    QLayout* layout = createLayout(layoutConfig, container);
    
    if (layout) {
        container->setLayout(layout);
        
        // 添加子控件 / Add child widgets
        for (const auto& widgetConfig : layoutConfig.widgets) {
            QWidget* widget = createWidget(widgetConfig, container);
            if (widget) {
                layout->addWidget(widget);
            }
        }
        
        // 添加子布局 / Add sub layouts
        for (const auto& subLayoutConfig : layoutConfig.subLayouts) {
            QWidget* subWidget = createFromConfig(subLayoutConfig, container);
            if (subWidget) {
                layout->addWidget(subWidget);
            }
        }
    }
    
    return container;
}

QWidget* DynamicUIBuilder::createWidget(const WidgetConfig& config, QWidget* parent)
{
    QWidget* widget = nullptr;
    
    switch (config.type) {
        case WidgetType::Button:
            widget = createButtonWidget(config, parent);
            break;
        case WidgetType::ToolButton:
            widget = createToolButtonWidget(config, parent);
            break;
        case WidgetType::Label:
            widget = createLabelWidget(config, parent);
            break;
        case WidgetType::LineEdit:
            widget = createLineEditWidget(config, parent);
            break;
        case WidgetType::TextEdit:
            widget = createTextEditWidget(config, parent);
            break;
        case WidgetType::SpinBox:
            widget = createSpinBoxWidget(config, parent);
            break;
        case WidgetType::DoubleSpinBox:
            widget = createDoubleSpinBoxWidget(config, parent);
            break;
        case WidgetType::CheckBox:
            widget = createCheckBoxWidget(config, parent);
            break;
        case WidgetType::RadioButton:
            widget = createRadioButtonWidget(config, parent);
            break;
        case WidgetType::ComboBox:
            widget = createComboBoxWidget(config, parent);
            break;
        case WidgetType::Slider:
            widget = createSliderWidget(config, parent);
            break;
        case WidgetType::ProgressBar:
            widget = createProgressBarWidget(config, parent);
            break;
        case WidgetType::GroupBox:
            widget = createGroupBoxWidget(config, parent);
            break;
        case WidgetType::TabWidget:
            widget = createTabWidgetWidget(config, parent);
            break;
        case WidgetType::Splitter:
            widget = createSplitterWidget(config, parent);
            break;
        case WidgetType::ScrollArea:
            widget = createScrollAreaWidget(config, parent);
            break;
        case WidgetType::TreeWidget:
            widget = createTreeWidgetWidget(config, parent);
            break;
        case WidgetType::TableWidget:
            widget = createTableWidgetWidget(config, parent);
            break;
        case WidgetType::ListWidget:
            widget = createListWidgetWidget(config, parent);
            break;
        case WidgetType::Frame:
            widget = createFrameWidget(config, parent);
            break;
        case WidgetType::Custom:
            // 处理自定义控件 / Handle custom widgets
            if (m_customCreators.contains(config.name)) {
                widget = m_customCreators[config.name](config, parent);
            }
            break;
    }
    
    if (widget) {
        applyWidgetProperties(widget, config);
        setupWidgetAnimation(widget, config);
        connectWidgetSignals(widget, config);
        
        // 存储控件引用 / Store widget reference
        if (!config.name.isEmpty()) {
            m_widgets[config.name] = widget;
        }
        
        emit widgetCreated(widget, config.name);
    }
    
    return widget;
}

QLayout* DynamicUIBuilder::createLayout(const LayoutConfig& config, QWidget* parent)
{
    QLayout* layout = nullptr;
    
    switch (config.type) {
        case LayoutType::VBox:
            layout = new QVBoxLayout();
            break;
        case LayoutType::HBox:
            layout = new QHBoxLayout();
            break;
        case LayoutType::Grid:
            layout = new QGridLayout();
            break;
        case LayoutType::Form:
            layout = new QFormLayout();
            break;
    }
    
    if (layout) {
        layout->setSpacing(config.spacing);
        layout->setContentsMargins(config.margins);
    }
    
    return layout;
}

QWidget* DynamicUIBuilder::createButtonWidget(const WidgetConfig& config, QWidget* parent)
{
    QPushButton* button = new QPushButton(config.text, parent);
    
    if (!config.iconPath.isEmpty()) {
        button->setIcon(QIcon(config.iconPath));
    }
    
    return button;
}

QWidget* DynamicUIBuilder::createToolButtonWidget(const WidgetConfig& config, QWidget* parent)
{
    QToolButton* toolButton = new QToolButton(parent);
    toolButton->setText(config.text);
    
    if (!config.iconPath.isEmpty()) {
        toolButton->setIcon(QIcon(config.iconPath));
        toolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    
    return toolButton;
}

QWidget* DynamicUIBuilder::createLabelWidget(const WidgetConfig& config, QWidget* parent)
{
    QLabel* label = new QLabel(config.text, parent);
    
    if (!config.iconPath.isEmpty()) {
        QPixmap pixmap(config.iconPath);
        if (!pixmap.isNull()) {
            label->setPixmap(pixmap);
        }
    }
    
    return label;
}

QWidget* DynamicUIBuilder::createLineEditWidget(const WidgetConfig& config, QWidget* parent)
{
    QLineEdit* lineEdit = new QLineEdit(parent);
    lineEdit->setText(config.value.toString());
    lineEdit->setPlaceholderText(config.text);
    
    return lineEdit;
}

QWidget* DynamicUIBuilder::createTextEditWidget(const WidgetConfig& config, QWidget* parent)
{
    QTextEdit* textEdit = new QTextEdit(parent);
    textEdit->setPlainText(config.value.toString());
    
    return textEdit;
}

QWidget* DynamicUIBuilder::createSpinBoxWidget(const WidgetConfig& config, QWidget* parent)
{
    QSpinBox* spinBox = new QSpinBox(parent);
    spinBox->setValue(config.value.toInt());
    
    if (config.properties.contains("minimum")) {
        spinBox->setMinimum(config.properties["minimum"].toInt());
    }
    if (config.properties.contains("maximum")) {
        spinBox->setMaximum(config.properties["maximum"].toInt());
    }
    
    return spinBox;
}

QWidget* DynamicUIBuilder::createDoubleSpinBoxWidget(const WidgetConfig& config, QWidget* parent)
{
    QDoubleSpinBox* doubleSpinBox = new QDoubleSpinBox(parent);
    doubleSpinBox->setValue(config.value.toDouble());
    
    if (config.properties.contains("minimum")) {
        doubleSpinBox->setMinimum(config.properties["minimum"].toDouble());
    }
    if (config.properties.contains("maximum")) {
        doubleSpinBox->setMaximum(config.properties["maximum"].toDouble());
    }
    if (config.properties.contains("decimals")) {
        doubleSpinBox->setDecimals(config.properties["decimals"].toInt());
    }
    
    return doubleSpinBox;
}

QWidget* DynamicUIBuilder::createCheckBoxWidget(const WidgetConfig& config, QWidget* parent)
{
    QCheckBox* checkBox = new QCheckBox(config.text, parent);
    checkBox->setChecked(config.value.toBool());
    
    return checkBox;
}

QWidget* DynamicUIBuilder::createRadioButtonWidget(const WidgetConfig& config, QWidget* parent)
{
    QRadioButton* radioButton = new QRadioButton(config.text, parent);
    radioButton->setChecked(config.value.toBool());
    
    return radioButton;
}

QWidget* DynamicUIBuilder::createComboBoxWidget(const WidgetConfig& config, QWidget* parent)
{
    QComboBox* comboBox = new QComboBox(parent);
    
    if (config.properties.contains("items")) {
        QStringList items = config.properties["items"].toStringList();
        comboBox->addItems(items);
    }
    
    comboBox->setCurrentText(config.value.toString());
    
    return comboBox;
}

QWidget* DynamicUIBuilder::createSliderWidget(const WidgetConfig& config, QWidget* parent)
{
    QSlider* slider = new QSlider(Qt::Horizontal, parent);
    slider->setValue(config.value.toInt());
    
    if (config.properties.contains("minimum")) {
        slider->setMinimum(config.properties["minimum"].toInt());
    }
    if (config.properties.contains("maximum")) {
        slider->setMaximum(config.properties["maximum"].toInt());
    }
    
    return slider;
}

QWidget* DynamicUIBuilder::createProgressBarWidget(const WidgetConfig& config, QWidget* parent)
{
    QProgressBar* progressBar = new QProgressBar(parent);
    progressBar->setValue(config.value.toInt());
    
    if (config.properties.contains("minimum")) {
        progressBar->setMinimum(config.properties["minimum"].toInt());
    }
    if (config.properties.contains("maximum")) {
        progressBar->setMaximum(config.properties["maximum"].toInt());
    }
    
    return progressBar;
}

QWidget* DynamicUIBuilder::createGroupBoxWidget(const WidgetConfig& config, QWidget* parent)
{
    QGroupBox* groupBox = new QGroupBox(config.text, parent);
    
    // 新增：处理GroupBox的内部布局 / New: Handle GroupBox internal layout
    setupContainerLayout(groupBox, config);
    
    return groupBox;
}

QWidget* DynamicUIBuilder::createTabWidgetWidget(const WidgetConfig& config, QWidget* parent)
{
    QTabWidget* tabWidget = new QTabWidget(parent);
    
    // 新增：处理TabWidget的内部布局 / New: Handle TabWidget internal layout
    setupContainerLayout(tabWidget, config);
    
    return tabWidget;
}

QWidget* DynamicUIBuilder::createSplitterWidget(const WidgetConfig& config, QWidget* parent)
{
    Qt::Orientation orientation = Qt::Horizontal;
    if (config.properties.contains("orientation")) {
        QString orientationStr = config.properties["orientation"].toString();
        if (orientationStr == "vertical") {
            orientation = Qt::Vertical;
        }
    }
    
    QSplitter* splitter = new QSplitter(orientation, parent);
    
    // 新增：处理Splitter的内部布局 / New: Handle Splitter internal layout
    setupContainerLayout(splitter, config);
    
    return splitter;
}

QWidget* DynamicUIBuilder::createScrollAreaWidget(const WidgetConfig& config, QWidget* parent)
{
    QScrollArea* scrollArea = new QScrollArea(parent);
    
    // 新增：处理ScrollArea的内部布局 / New: Handle ScrollArea internal layout
    setupContainerLayout(scrollArea, config);
    
    return scrollArea;
}

QWidget* DynamicUIBuilder::createTreeWidgetWidget(const WidgetConfig& config, QWidget* parent)
{
    QTreeWidget* treeWidget = new QTreeWidget(parent);
    
    if (config.properties.contains("headers")) {
        QStringList headers = config.properties["headers"].toStringList();
        treeWidget->setHeaderLabels(headers);
    }
    
    return treeWidget;
}

QWidget* DynamicUIBuilder::createTableWidgetWidget(const WidgetConfig& config, QWidget* parent)
{
    QTableWidget* tableWidget = new QTableWidget(parent);
    
    if (config.properties.contains("rows")) {
        tableWidget->setRowCount(config.properties["rows"].toInt());
    }
    if (config.properties.contains("columns")) {
        tableWidget->setColumnCount(config.properties["columns"].toInt());
    }
    if (config.properties.contains("headers")) {
        QStringList headers = config.properties["headers"].toStringList();
        tableWidget->setHorizontalHeaderLabels(headers);
    }
    
    return tableWidget;
}

QWidget* DynamicUIBuilder::createListWidgetWidget(const WidgetConfig& config, QWidget* parent)
{
    QListWidget* listWidget = new QListWidget(parent);
    
    if (config.properties.contains("items")) {
        QStringList items = config.properties["items"].toStringList();
        listWidget->addItems(items);
    }
    
    return listWidget;
}

QWidget* DynamicUIBuilder::createFrameWidget(const WidgetConfig& config, QWidget* parent)
{
    QFrame* frame = new QFrame(parent);
    
    if (config.properties.contains("frameShape")) {
        QString shape = config.properties["frameShape"].toString();
        if (shape == "Box") frame->setFrameShape(QFrame::Box);
        else if (shape == "Panel") frame->setFrameShape(QFrame::Panel);
        else if (shape == "StyledPanel") frame->setFrameShape(QFrame::StyledPanel);
    }
    
    // 新增：处理Frame的内部布局 / New: Handle Frame internal layout
    setupContainerLayout(frame, config);
    
    return frame;
}

void DynamicUIBuilder::applyWidgetProperties(QWidget* widget, const WidgetConfig& config)
{
    if (!widget) return;
    
    // 设置基本属性 / Set basic properties
    if (!config.name.isEmpty()) {
        widget->setObjectName(config.name);
    }
    
    if (!config.tooltip.isEmpty()) {
        widget->setToolTip(config.tooltip);
    }
    
    if (config.size.isValid()) {
        widget->resize(config.size);
    }
    
    if (config.minSize.isValid()) {
        widget->setMinimumSize(config.minSize);
    }
    
    if (config.maxSize.isValid()) {
        widget->setMaximumSize(config.maxSize);
    }
    
    // 应用样式表 / Apply style sheet
    QString styleSheet = config.styleSheet;
    if (styleSheet.isEmpty()) {
        styleSheet = getDefaultStyleSheet(config.type);
    }
    if (!styleSheet.isEmpty()) {
        widget->setStyleSheet(styleSheet);
    }
    
    // 应用其他属性 / Apply other properties
    for (auto it = config.properties.begin(); it != config.properties.end(); ++it) {
        widget->setProperty(it.key().toUtf8().constData(), it.value());
    }
}

void DynamicUIBuilder::setupWidgetAnimation(QWidget* widget, const WidgetConfig& config)
{
    if (!widget || config.animation == AnimationType::None) return;
    
    // 设置动画GIF / Setup animation GIF
    if (!config.animationGifPath.isEmpty()) {
        QMovie* movie = new QMovie(config.animationGifPath);
        if (movie->isValid()) {
            QLabel* label = qobject_cast<QLabel*>(widget);
            if (label) {
                label->setMovie(movie);
                movie->start();
                m_animations[config.name] = movie;
            } else {
                delete movie;
            }
        } else {
            delete movie;
        }
    }
    
    // 播放入场动画 / Play entrance animation
    if (config.animation != AnimationType::None) {
        playAnimation(widget, config.animation, config.animationDuration);
    }
}

void DynamicUIBuilder::connectWidgetSignals(QWidget* widget, const WidgetConfig& config)
{
    if (!widget) return;
    
    // 连接点击信号 / Connect click signals
    QPushButton* button = qobject_cast<QPushButton*>(widget);
    QToolButton* toolButton = qobject_cast<QToolButton*>(widget);
    
    if (button) {
        connect(button, &QPushButton::clicked, this, &DynamicUIBuilder::onWidgetClicked);
        if (config.clickCallback) {
            connect(button, &QPushButton::clicked, config.clickCallback);
        }
    } else if (toolButton) {
        connect(toolButton, &QToolButton::clicked, this, &DynamicUIBuilder::onWidgetClicked);
        if (config.clickCallback) {
            connect(toolButton, &QToolButton::clicked, config.clickCallback);
        }
    }
    
    // 连接值改变信号 / Connect value changed signals
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
    QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget);
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
    QComboBox* comboBox = qobject_cast<QComboBox*>(widget);
    QSlider* slider = qobject_cast<QSlider*>(widget);
    
    if (lineEdit) {
        connect(lineEdit, &QLineEdit::textChanged, [this, config](const QString& text) {
            onWidgetValueChanged(QVariant(text));
            if (config.valueChangedCallback) {
                config.valueChangedCallback(QVariant(text));
            }
        });
    } else if (spinBox) {
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this, config](int value) {
            onWidgetValueChanged(QVariant(value));
            if (config.valueChangedCallback) {
                config.valueChangedCallback(QVariant(value));
            }
        });
    } else if (doubleSpinBox) {
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, config](double value) {
            onWidgetValueChanged(QVariant(value));
            if (config.valueChangedCallback) {
                config.valueChangedCallback(QVariant(value));
            }
        });
    } else if (checkBox) {
        connect(checkBox, &QCheckBox::toggled, [this, config](bool checked) {
            onWidgetValueChanged(QVariant(checked));
            if (config.valueChangedCallback) {
                config.valueChangedCallback(QVariant(checked));
            }
        });
    } else if (comboBox) {
        connect(comboBox, &QComboBox::currentTextChanged, [this, config](const QString& text) {
            onWidgetValueChanged(QVariant(text));
            if (config.valueChangedCallback) {
                config.valueChangedCallback(QVariant(text));
            }
        });
    } else if (slider) {
        connect(slider, &QSlider::valueChanged, [this, config](int value) {
            onWidgetValueChanged(QVariant(value));
            if (config.valueChangedCallback) {
                config.valueChangedCallback(QVariant(value));
            }
        });
    }
}

void DynamicUIBuilder::applyTheme(QWidget* widget, const QString& themeName)
{
    if (!widget || !m_themes.contains(themeName)) return;
    
    widget->setStyleSheet(m_themes[themeName]);
}

void DynamicUIBuilder::playAnimation(QWidget* widget, AnimationType animation, int duration)
{
    if (!widget || animation == AnimationType::None) return;
    
    QPropertyAnimation* anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(duration);
    
    QRect startGeometry = widget->geometry();
    QRect endGeometry = startGeometry;
    
    switch (animation) {
        case AnimationType::FadeIn: {
            QPropertyAnimation* fadeAnim = new QPropertyAnimation(widget, "windowOpacity");
            fadeAnim->setDuration(duration);
            fadeAnim->setStartValue(0.0);
            fadeAnim->setEndValue(1.0);
            fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            return;
        }
        case AnimationType::FadeOut: {
            QPropertyAnimation* fadeAnim = new QPropertyAnimation(widget, "windowOpacity");
            fadeAnim->setDuration(duration);
            fadeAnim->setStartValue(1.0);
            fadeAnim->setEndValue(0.0);
            fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            return;
        }
        case AnimationType::SlideIn:
            startGeometry.moveLeft(-startGeometry.width());
            break;
        case AnimationType::SlideOut:
            endGeometry.moveLeft(widget->parentWidget() ? widget->parentWidget()->width() : 1000);
            break;
        case AnimationType::Bounce:
            anim->setEasingCurve(QEasingCurve::OutBounce);
            startGeometry.setHeight(0);
            break;
        case AnimationType::Pulse: {
            QPropertyAnimation* scaleAnim = new QPropertyAnimation(widget, "geometry");
            scaleAnim->setDuration(duration);
            QRect pulseGeometry = startGeometry;
            pulseGeometry.adjust(-10, -10, 10, 10);
            scaleAnim->setStartValue(startGeometry);
            scaleAnim->setKeyValueAt(0.5, pulseGeometry);
            scaleAnim->setEndValue(startGeometry);
            scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
            return;
        }
        case AnimationType::Shake: {
            QPropertyAnimation* shakeAnim = new QPropertyAnimation(widget, "pos");
            shakeAnim->setDuration(duration);
            QPoint originalPos = widget->pos();
            shakeAnim->setStartValue(originalPos);
            shakeAnim->setKeyValueAt(0.1, originalPos + QPoint(10, 0));
            shakeAnim->setKeyValueAt(0.3, originalPos - QPoint(10, 0));
            shakeAnim->setKeyValueAt(0.5, originalPos + QPoint(10, 0));
            shakeAnim->setKeyValueAt(0.7, originalPos - QPoint(10, 0));
            shakeAnim->setKeyValueAt(0.9, originalPos + QPoint(10, 0));
            shakeAnim->setEndValue(originalPos);
            shakeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            return;
        }
        default:
            delete anim;
            return;
    }
    
    anim->setStartValue(startGeometry);
    anim->setEndValue(endGeometry);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void DynamicUIBuilder::registerCustomWidget(const QString& typeName, std::function<QWidget*(const WidgetConfig&, QWidget*)> creator)
{
    m_customCreators[typeName] = creator;
}

QWidget* DynamicUIBuilder::getWidget(const QString& name) const
{
    return m_widgets.value(name, nullptr);
}

void DynamicUIBuilder::setGlobalStyle(const QString& styleSheet)
{
    m_globalStyle = styleSheet;
    QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
    if (app) {
        app->setStyleSheet(styleSheet);
    }
}

bool DynamicUIBuilder::loadThemeFile(const QString& themeFilePath)
{
    QFile file(themeFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << tr("无法打开主题文件 / Cannot open theme file:") << themeFilePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << tr("无效的主题文件格式 / Invalid theme file format");
        return false;
    }
    
    QJsonObject themeObj = doc.object();
    for (auto it = themeObj.begin(); it != themeObj.end(); ++it) {
        m_themes[it.key()] = it.value().toString();
    }
    
    return true;
}

void DynamicUIBuilder::onWidgetClicked()
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (widget) {
        QString name = widget->objectName();
        emit widgetClicked(name);
    }
}

void DynamicUIBuilder::onWidgetValueChanged(const QVariant& value)
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (widget) {
        QString name = widget->objectName();
        emit widgetValueChanged(name, value);
    }
}

DynamicUIBuilder::WidgetConfig DynamicUIBuilder::parseJsonWidgetConfig(const QJsonObject& json)
{
    WidgetConfig config;
    
    // 解析控件类型 / Parse widget type
    QString typeStr = json["type"].toString();
    if (typeStr == "Button") config.type = WidgetType::Button;
    else if (typeStr == "ToolButton") config.type = WidgetType::ToolButton;
    else if (typeStr == "Label") config.type = WidgetType::Label;
    else if (typeStr == "LineEdit") config.type = WidgetType::LineEdit;
    else if (typeStr == "TextEdit") config.type = WidgetType::TextEdit;
    else if (typeStr == "SpinBox") config.type = WidgetType::SpinBox;
    else if (typeStr == "DoubleSpinBox") config.type = WidgetType::DoubleSpinBox;
    else if (typeStr == "CheckBox") config.type = WidgetType::CheckBox;
    else if (typeStr == "RadioButton") config.type = WidgetType::RadioButton;
    else if (typeStr == "ComboBox") config.type = WidgetType::ComboBox;
    else if (typeStr == "Slider") config.type = WidgetType::Slider;
    else if (typeStr == "ProgressBar") config.type = WidgetType::ProgressBar;
    else if (typeStr == "GroupBox") config.type = WidgetType::GroupBox;
    else if (typeStr == "TabWidget") config.type = WidgetType::TabWidget;
    else if (typeStr == "Splitter") config.type = WidgetType::Splitter;
    else if (typeStr == "ScrollArea") config.type = WidgetType::ScrollArea;
    else if (typeStr == "TreeWidget") config.type = WidgetType::TreeWidget;
    else if (typeStr == "TableWidget") config.type = WidgetType::TableWidget;
    else if (typeStr == "ListWidget") config.type = WidgetType::ListWidget;
    else if (typeStr == "Frame") config.type = WidgetType::Frame;
    else config.type = WidgetType::Custom;
    
    // 解析其他属性 / Parse other properties
    config.name = json["name"].toString();
    config.text = json["text"].toString();
    config.tooltip = json["tooltip"].toString();
    config.iconPath = json["iconPath"].toString();
    config.styleSheet = json["styleSheet"].toString();
    config.value = json["value"].toVariant();
    config.animationGifPath = json["animationGifPath"].toString();
    config.animationDuration = json["animationDuration"].toInt(300);
    
    // 解析尺寸 / Parse sizes
    if (json.contains("size")) {
        QJsonArray sizeArray = json["size"].toArray();
        if (sizeArray.size() >= 2) {
            config.size = QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
        }
    }
    
    // 解析动画类型 / Parse animation type
    QString animStr = json["animation"].toString();
    if (animStr == "FadeIn") config.animation = AnimationType::FadeIn;
    else if (animStr == "FadeOut") config.animation = AnimationType::FadeOut;
    else if (animStr == "SlideIn") config.animation = AnimationType::SlideIn;
    else if (animStr == "SlideOut") config.animation = AnimationType::SlideOut;
    else if (animStr == "Bounce") config.animation = AnimationType::Bounce;
    else if (animStr == "Pulse") config.animation = AnimationType::Pulse;
    else if (animStr == "Shake") config.animation = AnimationType::Shake;
    else config.animation = AnimationType::None;
    
    // 解析属性映射 / Parse properties map
    if (json.contains("properties")) {
        QJsonObject propsObj = json["properties"].toObject();
        for (auto it = propsObj.begin(); it != propsObj.end(); ++it) {
            config.properties[it.key()] = it.value().toVariant();
        }
    }
    
    // 新增：解析子控件配置 / New: Parse child widgets configuration
    if (json.contains("childWidgets")) {
        QJsonArray childArray = json["childWidgets"].toArray();
        for (const auto& childValue : childArray) {
            config.childWidgets.append(parseJsonWidgetConfig(childValue.toObject()));
        }
    }
    
    // 新增：解析子布局配置 / New: Parse child layout configuration
    if (json.contains("childLayout")) {
        config.childLayout = new LayoutConfig();
        *config.childLayout = parseJsonLayoutConfig(json["childLayout"].toObject());
    }
    
    return config;
}

DynamicUIBuilder::LayoutConfig DynamicUIBuilder::parseJsonLayoutConfig(const QJsonObject& json)
{
    LayoutConfig config;
    
    // 解析布局类型 / Parse layout type
    QString typeStr = json["type"].toString();
    if (typeStr == "VBox") config.type = LayoutType::VBox;
    else if (typeStr == "HBox") config.type = LayoutType::HBox;
    else if (typeStr == "Grid") config.type = LayoutType::Grid;
    else if (typeStr == "Form") config.type = LayoutType::Form;
    else config.type = LayoutType::VBox;
    
    config.spacing = json["spacing"].toInt(6);
    config.columns = json["columns"].toInt(3);
    
    // 解析边距 / Parse margins
    if (json.contains("margins")) {
        QJsonArray marginsArray = json["margins"].toArray();
        if (marginsArray.size() >= 4) {
            config.margins = QMargins(
                marginsArray[0].toInt(),
                marginsArray[1].toInt(),
                marginsArray[2].toInt(),
                marginsArray[3].toInt()
            );
        }
    }
    
    // 解析子控件 / Parse child widgets
    if (json.contains("widgets")) {
        QJsonArray widgetsArray = json["widgets"].toArray();
        for (const auto& widgetValue : widgetsArray) {
            config.widgets.append(parseJsonWidgetConfig(widgetValue.toObject()));
        }
    }
    
    // 解析子布局 / Parse sub layouts
    if (json.contains("subLayouts")) {
        QJsonArray layoutsArray = json["subLayouts"].toArray();
        for (const auto& layoutValue : layoutsArray) {
            config.subLayouts.append(parseJsonLayoutConfig(layoutValue.toObject()));
        }
    }
    
    return config;
}

QString DynamicUIBuilder::getDefaultStyleSheet(WidgetType type, const QString& themeName)
{
    if (m_themes.contains(themeName)) {
        return m_themes[themeName];
    }
    return m_themes["modern"];
}

// 便捷构建器实现 / Convenience Builder Implementation
namespace DynamicUI {

WidgetConfigBuilder::WidgetConfigBuilder(DynamicUIBuilder::WidgetType type)
{
    m_config.type = type;
    m_config.animation = DynamicUIBuilder::AnimationType::None;
    m_config.animationDuration = 300;
}

WidgetConfigBuilder& WidgetConfigBuilder::name(const QString& name)
{
    m_config.name = name;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::text(const QString& text)
{
    m_config.text = text;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::tooltip(const QString& tooltip)
{
    m_config.tooltip = tooltip;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::icon(const QString& iconPath)
{
    m_config.iconPath = iconPath;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::style(const QString& styleSheet)
{
    m_config.styleSheet = styleSheet;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::size(const QSize& size)
{
    m_config.size = size;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::minSize(const QSize& minSize)
{
    m_config.minSize = minSize;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::maxSize(const QSize& maxSize)
{
    m_config.maxSize = maxSize;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::value(const QVariant& value)
{
    m_config.value = value;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::property(const QString& key, const QVariant& value)
{
    m_config.properties[key] = value;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::animationGif(const QString& gifPath)
{
    m_config.animationGifPath = gifPath;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::animation(DynamicUIBuilder::AnimationType type, int duration)
{
    m_config.animation = type;
    m_config.animationDuration = duration;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::onClick(std::function<void()> callback)
{
    m_config.clickCallback = callback;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::onValueChanged(std::function<void(const QVariant&)> callback)
{
    m_config.valueChangedCallback = callback;
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::addChildWidget(const DynamicUIBuilder::WidgetConfig& childConfig)
{
    m_config.childWidgets.append(childConfig);
    return *this;
}

WidgetConfigBuilder& WidgetConfigBuilder::setChildLayout(const DynamicUIBuilder::LayoutConfig& layoutConfig)
{
    if (m_config.childLayout) {
        delete m_config.childLayout;
    }
    m_config.childLayout = new DynamicUIBuilder::LayoutConfig(layoutConfig);
    return *this;
}

DynamicUIBuilder::WidgetConfig WidgetConfigBuilder::build()
{
    return m_config;
}

LayoutConfigBuilder::LayoutConfigBuilder(DynamicUIBuilder::LayoutType type)
{
    m_config.type = type;
    m_config.spacing = 6;
    m_config.margins = QMargins(9, 9, 9, 9);
    m_config.columns = 3;
}

LayoutConfigBuilder& LayoutConfigBuilder::spacing(int spacing)
{
    m_config.spacing = spacing;
    return *this;
}

LayoutConfigBuilder& LayoutConfigBuilder::margins(const QMargins& margins)
{
    m_config.margins = margins;
    return *this;
}

LayoutConfigBuilder& LayoutConfigBuilder::columns(int columns)
{
    m_config.columns = columns;
    return *this;
}

LayoutConfigBuilder& LayoutConfigBuilder::addWidget(const DynamicUIBuilder::WidgetConfig& config)
{
    m_config.widgets.append(config);
    return *this;
}

LayoutConfigBuilder& LayoutConfigBuilder::addLayout(const DynamicUIBuilder::LayoutConfig& config)
{
    m_config.subLayouts.append(config);
    return *this;
}

DynamicUIBuilder::LayoutConfig LayoutConfigBuilder::build() const
{
    return m_config;
}

// 便捷函数实现 / Convenience Functions Implementation
WidgetConfigBuilder button(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::Button).name(name); }
WidgetConfigBuilder toolButton(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::ToolButton).name(name); }
WidgetConfigBuilder label(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::Label).name(name); }
WidgetConfigBuilder lineEdit(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::LineEdit).name(name); }
WidgetConfigBuilder textEdit(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::TextEdit).name(name); }
WidgetConfigBuilder spinBox(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::SpinBox).name(name); }
WidgetConfigBuilder doubleSpinBox(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::DoubleSpinBox).name(name); }
WidgetConfigBuilder checkBox(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::CheckBox).name(name); }
WidgetConfigBuilder radioButton(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::RadioButton).name(name); }
WidgetConfigBuilder comboBox(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::ComboBox).name(name); }
WidgetConfigBuilder slider(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::Slider).name(name); }
WidgetConfigBuilder progressBar(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::ProgressBar).name(name); }
WidgetConfigBuilder groupBox(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::GroupBox).name(name); }
WidgetConfigBuilder tabWidget(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::TabWidget).name(name); }
WidgetConfigBuilder splitter(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::Splitter).name(name); }
WidgetConfigBuilder scrollArea(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::ScrollArea).name(name); }
WidgetConfigBuilder treeWidget(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::TreeWidget).name(name); }
WidgetConfigBuilder tableWidget(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::TableWidget).name(name); }
WidgetConfigBuilder listWidget(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::ListWidget).name(name); }
WidgetConfigBuilder frame(const QString& name) { return WidgetConfigBuilder(DynamicUIBuilder::WidgetType::Frame).name(name); }

LayoutConfigBuilder vBoxLayout() { return LayoutConfigBuilder(DynamicUIBuilder::LayoutType::VBox); }
LayoutConfigBuilder hBoxLayout() { return LayoutConfigBuilder(DynamicUIBuilder::LayoutType::HBox); }
LayoutConfigBuilder gridLayout(int columns) { return LayoutConfigBuilder(DynamicUIBuilder::LayoutType::Grid).columns(columns); }
LayoutConfigBuilder formLayout() { return LayoutConfigBuilder(DynamicUIBuilder::LayoutType::Form); }

} // namespace DynamicUI

/*============================ 新增：容器控件内部布局处理方法实现 ============================*/

void DynamicUIBuilder::setupContainerLayout(QWidget* containerWidget, const WidgetConfig& config)
{
    if (!containerWidget || !isContainerWidget(config.type)) {
        logDebug(QString("setupContainerLayout: Invalid container widget or type for %1").arg(config.name));
        return;
    }
    
    logDebug(QString("Setting up container layout for %1 (%2)").arg(config.name).arg(static_cast<int>(config.type)));
    
    // 如果有子布局配置，创建并应用布局
    if (config.childLayout) {
        QLayout* layout = createLayoutForContainer(*config.childLayout, containerWidget);
        if (layout) {
            // 对于特殊容器控件的处理
            if (config.type == WidgetType::ScrollArea) {
                QScrollArea* scrollArea = qobject_cast<QScrollArea*>(containerWidget);
                if (scrollArea) {
                    QWidget* contentWidget = new QWidget();
                    contentWidget->setLayout(layout);
                    scrollArea->setWidget(contentWidget);
                    scrollArea->setWidgetResizable(true);
                    logDebug(QString("Applied layout to ScrollArea content widget: %1").arg(config.name));
                }
            } else if (config.type == WidgetType::TabWidget) {
                // TabWidget需要特殊处理，每个子控件作为一个标签页
                QTabWidget* tabWidget = qobject_cast<QTabWidget*>(containerWidget);
                if (tabWidget) {
                    for (const auto& childConfig : config.childWidgets) {
                        QWidget* childWidget = createWidget(childConfig, tabWidget);
                        if (childWidget) {
                            tabWidget->addTab(childWidget, childConfig.text);
                            logDebug(QString("Added tab %1 to TabWidget %2").arg(childConfig.text).arg(config.name));
                        }
                    }
                }
            } else if (config.type == WidgetType::Splitter) {
                // Splitter不使用布局，直接添加子控件
                QSplitter* splitter = qobject_cast<QSplitter*>(containerWidget);
                if (splitter) {
                    for (const auto& childConfig : config.childWidgets) {
                        QWidget* childWidget = createWidget(childConfig, splitter);
                        if (childWidget) {
                            splitter->addWidget(childWidget);
                            logDebug(QString("Added widget %1 to Splitter %2").arg(childConfig.name).arg(config.name));
                        }
                    }
                }
            } else {
                // 普通容器控件（GroupBox、Frame等）
                containerWidget->setLayout(layout);
                logDebug(QString("Applied layout to container widget: %1").arg(config.name));
            }
        }
    }
    
    // 如果没有子布局但有子控件，创建默认的垂直布局
    else if (!config.childWidgets.isEmpty()) {
        if (config.type == WidgetType::TabWidget || config.type == WidgetType::Splitter) {
            // 这些控件已在上面处理
            return;
        }
        
        QVBoxLayout* defaultLayout = new QVBoxLayout();
        defaultLayout->setSpacing(6);
        defaultLayout->setContentsMargins(9, 9, 9, 9);
        
        addChildWidgetsToContainer(containerWidget, config.childWidgets);
        
        // 为子控件添加到默认布局
        for (const auto& childConfig : config.childWidgets) {
            if (QWidget* childWidget = getWidget(childConfig.name)) {
                defaultLayout->addWidget(childWidget);
                logDebug(QString("Added child widget %1 to default layout of %2").arg(childConfig.name).arg(config.name));
            }
        }
        
        if (config.type == WidgetType::ScrollArea) {
            QScrollArea* scrollArea = qobject_cast<QScrollArea*>(containerWidget);
            if (scrollArea) {
                QWidget* contentWidget = new QWidget();
                contentWidget->setLayout(defaultLayout);
                scrollArea->setWidget(contentWidget);
                scrollArea->setWidgetResizable(true);
            }
        } else {
            containerWidget->setLayout(defaultLayout);
        }
        
        logDebug(QString("Applied default VBox layout to container: %1").arg(config.name));
    }
}

QLayout* DynamicUIBuilder::createLayoutForContainer(const LayoutConfig& layoutConfig, QWidget* container)
{
    QLayout* layout = createLayout(layoutConfig, container);
    if (!layout) {
        logDebug("Failed to create layout for container");
        return nullptr;
    }
    
    // 添加子控件到布局
    for (const auto& widgetConfig : layoutConfig.widgets) {
        QWidget* widget = createWidget(widgetConfig, container);
        if (widget) {
            layout->addWidget(widget);
            logDebug(QString("Added widget %1 to container layout").arg(widgetConfig.name));
        }
    }
    
    // 递归处理子布局
    for (const auto& subLayoutConfig : layoutConfig.subLayouts) {
        QWidget* subContainer = createFromConfig(subLayoutConfig, container);
        if (subContainer) {
            layout->addWidget(subContainer);
            logDebug("Added sub-layout container to layout");
        }
    }
    
    return layout;
}

void DynamicUIBuilder::addChildWidgetsToContainer(QWidget* container, const QVector<WidgetConfig>& childConfigs)
{
    for (const auto& childConfig : childConfigs) {
        QWidget* childWidget = createWidget(childConfig, container);
        if (childWidget) {
            logDebug(QString("Created child widget %1 for container").arg(childConfig.name));
        } else {
            logDebug(QString("Failed to create child widget %1").arg(childConfig.name));
        }
    }
}

bool DynamicUIBuilder::isContainerWidget(WidgetType type) const
{
    return type == WidgetType::GroupBox ||
           type == WidgetType::TabWidget ||
           type == WidgetType::Splitter ||
           type == WidgetType::ScrollArea ||
           type == WidgetType::Frame;
}

/*============================ 新增：调试和日志方法实现 ============================*/

void DynamicUIBuilder::setDebugMode(bool enabled)
{
    m_debugMode = enabled;
    if (enabled) {
        logDebug("Debug mode enabled for DynamicUIBuilder");
    }
}

void DynamicUIBuilder::logDebug(const QString& message) const
{
    if (m_debugMode) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        QString debugMessage = QString("[%1] DynamicUI: %2").arg(timestamp).arg(message);
        m_debugLog.append(debugMessage);
        qDebug() << debugMessage;
        
        // 限制日志大小，保持最近的1000条记录
        if (m_debugLog.size() > 1000) {
            m_debugLog.removeFirst();
        }
    }
} 