# 🎯 复杂UI设计模式 / Complex UI Design Patterns

<div align="center">

![Design Patterns](https://img.shields.io/badge/Patterns-Design-success.svg)
![Architecture](https://img.shields.io/badge/Architecture-Advanced-blue.svg)

**掌握复杂UI构建的设计模式**  
*Master design patterns for complex UI construction*

</div>

---

## 🏗️ 模式索引 / Pattern Index

| 模式 | 适用场景 | 复杂度 | 示例 |
|------|----------|--------|------|
| [🔄 观察者模式](#-观察者模式) | 数据绑定、状态同步 | ⭐⭐⭐ | 实时数据面板 |
| [🏭 抽象工厂](#-抽象工厂模式) | 主题切换、多平台UI | ⭐⭐⭐⭐ | 跨平台界面 |
| [🎮 命令模式](#-命令模式) | 撤销重做、批量操作 | ⭐⭐⭐ | 编辑器操作 |
| [📦 组合模式](#-组合模式) | 树形结构、嵌套组件 | ⭐⭐⭐⭐⭐ | 文件管理器 |
| [🔗 策略模式](#-策略模式) | 算法切换、行为变化 | ⭐⭐ | 排序筛选 |

---

## 🔄 观察者模式 - 响应式数据绑定 / Observer Pattern

### 💡 设计思路

当你需要多个UI组件响应同一数据变化时，观察者模式是最佳选择。就像你看新闻，多个朋友都想知道最新消息，你只需要告诉一个人，然后这个消息就会传播给所有关注的人。

### 🛠️ 实现代码

```cpp
// 📊 数据主题 - 被观察者
class CustomerDataSubject {
private:
    QList<std::function<void(const CustomerData&)>> m_observers;
    CustomerData m_currentData;
    
public:
    // 🔔 添加观察者（订阅者）
    void addObserver(std::function<void(const CustomerData&)> observer) {
        m_observers.append(observer);
    }
    
    // 📢 通知所有观察者
    void updateCustomerData(const CustomerData& newData) {
        m_currentData = newData;
        
        // 通知所有订阅的UI组件
        for (auto& observer : m_observers) {
            observer(m_currentData);
        }
    }
    
    const CustomerData& getCurrentData() const { return m_currentData; }
};

// 🎯 响应式UI组件管理器
class ReactiveUIManager {
private:
    CustomerDataSubject* m_dataSubject;
    DynamicUIBuilder m_builder;
    QHash<QString, QWidget*> m_boundWidgets;
    
public:
    ReactiveUIManager(CustomerDataSubject* subject) : m_dataSubject(subject) {}
    
    // 📈 创建响应式客户信息卡片
    QWidget* createReactiveCustomerCard(QWidget* parent = nullptr) {
        auto cardLayout = vBoxLayout()
            .spacing(10)
            .margins(QMargins(15, 15, 15, 15))
            .styleSheet(R"(
                QWidget {
                    background: white;
                    border: 1px solid #e0e0e0;
                    border-radius: 8px;
                }
            )")
            
            // 👤 客户头像和基本信息
            .addLayout(
                hBoxLayout()
                .spacing(15)
                
                .addWidget(
                    label("customer_avatar")
                    .text("👤")
                    .style(R"(
                        font-size: 48px;
                        border: 2px solid #3498db;
                        border-radius: 35px;
                        padding: 10px;
                        background: #ecf0f1;
                    )")
                    .build()
                )
                
                .addLayout(
                    vBoxLayout()
                    .addWidget(
                        label("customer_name")
                        .text("客户姓名 / Customer Name")
                        .style("font-size: 18px; font-weight: bold; color: #2c3e50;")
                        .build()
                    )
                    .addWidget(
                        label("customer_company") 
                        .text("公司名称 / Company")
                        .style("font-size: 14px; color: #7f8c8d;")
                        .build()
                    )
                    .addWidget(
                        label("customer_phone")
                        .text("📞 联系电话 / Phone")
                        .style("font-size: 12px; color: #95a5a6;")
                        .build()
                    )
                    .build()
                )
                
                .addStretch()
                .build()
            )
            
            // 📊 客户统计信息
            .addWidget(createCustomerStatsSection())
            
            .build();
            
        QWidget* card = m_builder.createFromConfig(cardLayout, parent);
        
        // 🔗 绑定数据观察者
        bindCardToData(card);
        
        return card;
    }
    
private:
    // 🔗 绑定卡片到数据变化
    void bindCardToData(QWidget* card) {
        // 📝 注册观察者回调
        m_dataSubject->addObserver([this, card](const CustomerData& data) {
            updateCardDisplay(card, data);
        });
    }
    
    // 🔄 更新卡片显示
    void updateCardDisplay(QWidget* card, const CustomerData& data) {
        // 🎭 使用动画更新内容
        auto nameLabel = card->findChild<QLabel*>("customer_name");
        if (nameLabel) {
            animateTextChange(nameLabel, data.name);
        }
        
        auto companyLabel = card->findChild<QLabel*>("customer_company");
        if (companyLabel) {
            animateTextChange(companyLabel, data.company);
        }
        
        auto phoneLabel = card->findChild<QLabel*>("customer_phone");
        if (phoneLabel) {
            animateTextChange(phoneLabel, "📞 " + data.phone);
        }
    }
    
    // ✨ 文本变化动画
    void animateTextChange(QLabel* label, const QString& newText) {
        // 淡出 -> 更改文本 -> 淡入
        QPropertyAnimation* fadeOut = new QPropertyAnimation(label, "windowOpacity");
        fadeOut->setDuration(150);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.3);
        
        QPropertyAnimation* fadeIn = new QPropertyAnimation(label, "windowOpacity");
        fadeIn->setDuration(150);
        fadeIn->setStartValue(0.3);
        fadeIn->setEndValue(1.0);
        
        // 连接动画完成信号
        QObject::connect(fadeOut, &QPropertyAnimation::finished, [label, newText, fadeIn]() {
            label->setText(newText);
            fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
        });
        
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    }
};
```

### 🎮 使用示例

```cpp
// 💻 在主应用中使用响应式UI
class MainApplication {
private:
    CustomerDataSubject m_customerData;
    ReactiveUIManager m_uiManager;
    
public:
    MainApplication() : m_uiManager(&m_customerData) {}
    
    void setupReactiveInterface() {
        // 📋 创建多个响应同一数据的组件
        auto customerCard = m_uiManager.createReactiveCustomerCard();
        auto customerChart = createReactiveChart();
        auto customerHistory = createReactiveHistory();
        
        // 🔄 当用户选择不同客户时，所有组件自动更新
        connect(customerListWidget, &QListWidget::currentItemChanged, 
                [this](QListWidgetItem* current) {
            if (current) {
                CustomerData data = getCustomerData(current->text());
                m_customerData.updateCustomerData(data);  // 一次更新，所有组件响应
            }
        });
    }
};
```

---

## 🏭 抽象工厂模式 - 主题化UI工厂 / Abstract Factory Pattern

### 💡 设计思路

想象你开一家服装店，需要为不同季节准备不同风格的服装系列。抽象工厂模式就像是你的设计总监，可以根据季节（主题）来生产整套协调一致的服装（UI组件）。

### 🛠️ 实现代码

```cpp
// 🎨 UI组件抽象工厂
class AbstractUIFactory {
public:
    virtual ~AbstractUIFactory() = default;
    
    // 📦 创建各种UI组件
    virtual QWidget* createButton(const QString& name, const QString& text) = 0;
    virtual QWidget* createInputField(const QString& name, const QString& placeholder) = 0;
    virtual QWidget* createCard(const QString& name) = 0;
    virtual QWidget* createDialog(const QString& title) = 0;
    
    // 🎭 获取主题样式
    virtual QString getThemeName() const = 0;
    virtual QColor getPrimaryColor() const = 0;
    virtual QColor getSecondaryColor() const = 0;
};

// 🌞 现代主题工厂
class ModernUIFactory : public AbstractUIFactory {
private:
    DynamicUIBuilder m_builder;
    
public:
    QWidget* createButton(const QString& name, const QString& text) override {
        return m_builder.createWidget(
            button(name)
            .text(text)
            .style(R"(
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #3498db, stop:1 #2980b9);
                    color: white;
                    border: none;
                    border-radius: 6px;
                    padding: 12px 24px;
                    font-weight: 500;
                    font-size: 14px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #2980b9, stop:1 #1f618d);
                    transform: translateY(-1px);
                }
                QPushButton:pressed {
                    background: #1f618d;
                    transform: translateY(0px);
                }
            )")
            .animation(DynamicUIBuilder::AnimationType::FadeIn, 300)
            .build()
        );
    }
    
    QWidget* createInputField(const QString& name, const QString& placeholder) override {
        return m_builder.createWidget(
            lineEdit(name)
            .text(placeholder)
            .style(R"(
                QLineEdit {
                    border: 2px solid #e0e0e0;
                    border-radius: 8px;
                    padding: 12px 16px;
                    font-size: 14px;
                    background: white;
                    selection-background-color: #3498db;
                }
                QLineEdit:focus {
                    border-color: #3498db;
                    background: #f8f9fa;
                    box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.1);
                }
            )")
            .build()
        );
    }
    
    QString getThemeName() const override { return "Modern"; }
    QColor getPrimaryColor() const override { return QColor("#3498db"); }
    QColor getSecondaryColor() const override { return QColor("#2c3e50"); }
};

// 🌙 暗色主题工厂  
class DarkUIFactory : public AbstractUIFactory {
private:
    DynamicUIBuilder m_builder;
    
public:
    QWidget* createButton(const QString& name, const QString& text) override {
        return m_builder.createWidget(
            button(name)
            .text(text)
            .style(R"(
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #5d4e75, stop:1 #4a3c5c);
                    color: #ecf0f1;
                    border: 1px solid #6c5ce7;
                    border-radius: 8px;
                    padding: 12px 24px;
                    font-weight: 500;
                    font-size: 14px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #6c5ce7, stop:1 #5d4e75);
                    border-color: #a29bfe;
                    box-shadow: 0 4px 12px rgba(108, 92, 231, 0.3);
                }
            )")
            .animation(DynamicUIBuilder::AnimationType::Pulse, 400)
            .build()
        );
    }
    
    QWidget* createInputField(const QString& name, const QString& placeholder) override {
        return m_builder.createWidget(
            lineEdit(name)
            .text(placeholder)
            .style(R"(
                QLineEdit {
                    background: #2c3e50;
                    border: 2px solid #34495e;
                    border-radius: 8px;
                    padding: 12px 16px;
                    color: #ecf0f1;
                    font-size: 14px;
                    selection-background-color: #6c5ce7;
                }
                QLineEdit:focus {
                    border-color: #6c5ce7;
                    background: #34495e;
                    box-shadow: 0 0 0 3px rgba(108, 92, 231, 0.2);
                }
            )")
            .build()
        );
    }
    
    QString getThemeName() const override { return "Dark"; }
    QColor getPrimaryColor() const override { return QColor("#6c5ce7"); }
    QColor getSecondaryColor() const override { return QColor("#2c3e50"); }
};
``` 