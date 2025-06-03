# 🚀 高级示例指南 / Advanced Examples Guide

<div align="center">

![Advanced Examples](https://img.shields.io/badge/Examples-Advanced-orange.svg)
![Complexity](https://img.shields.io/badge/Complexity-High-red.svg)

**深入学习复杂UI构建技术**  
*Deep dive into complex UI construction techniques*

[🏢 企业级应用](#-企业级应用) | [🎮 游戏界面](#-游戏界面) | [📊 数据可视化](#-数据可视化) | [🛠️ 开发工具](#️-开发工具)

</div>

---

## 🏢 企业级应用示例 / Enterprise Application Examples

### 📋 示例1: 复杂的CRM客户管理界面

这个示例展示如何构建一个功能完整的客户管理界面，包含搜索、筛选、数据展示和操作功能。

```cpp
class CRMCustomerManager {
private:
    DynamicUIBuilder m_builder;
    QWidget* m_mainWidget;
    
public:
    QWidget* createCRMInterface(QWidget* parent = nullptr) {
        // 🎨 主布局 - 使用垂直布局分区
        auto mainLayout = vBoxLayout()
            .spacing(0)  // 无间距，让各区域紧密连接
            .margins(QMargins(0, 0, 0, 0))
            
            // 📊 顶部工具栏区域
            .addWidget(createToolbarSection())
            
            // 🔍 搜索和筛选区域  
            .addWidget(createSearchSection())
            
            // 📋 主内容区域 - 使用水平分割器
            .addWidget(createMainContentSplitter())
            
            // 📈 底部状态栏
            .addWidget(createStatusSection())
            
            .build();
            
        m_mainWidget = m_builder.createFromConfig(mainLayout, parent);
        
        // 🎭 应用现代主题
        m_builder.applyTheme(m_mainWidget, "modern");
        
        // ⚡ 初始化数据加载
        initializeData();
        
        return m_mainWidget;
    }
    
private:
    // 🛠️ 创建顶部工具栏
    QWidget* createToolbarSection() {
        auto toolbar = hBoxLayout()
            .spacing(8)
            .margins(QMargins(15, 10, 15, 10))
            .styleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #34495e, stop:1 #2c3e50);")
            
            // 🏷️ 应用标题
            .addWidget(
                label("app_title")
                .text("🏢 客户关系管理系统 / CRM System")
                .style(R"(
                    color: white; 
                    font-size: 18px; 
                    font-weight: bold;
                    padding: 5px 0px;
                )")
                .build()
            )
            
            .addStretch()  // 弹性空间
            
            // 📁 快速操作按钮组
            .addWidget(createQuickActionButtons())
            
            // 👤 用户信息区域
            .addWidget(createUserInfoSection())
            
            .build();
            
        return m_builder.createFromConfig(toolbar);
    }
    
    // ⚡ 快速操作按钮组
    QWidget* createQuickActionButtons() {
        auto buttonGroup = hBoxLayout()
            .spacing(5)
            
            .addWidget(
                toolButton("btn_new_customer")
                .text("➕ 新建客户")
                .tooltip("创建新的客户记录 (Ctrl+N)")
                .icon(":/icons/add_customer.png")
                .style(getToolbarButtonStyle())
                .onClick([this]() { createNewCustomer(); })
                .animation(DynamicUIBuilder::AnimationType::FadeIn, 300)
                .build()
            )
            
            .addWidget(
                toolButton("btn_import_data")  
                .text("📥 导入")
                .tooltip("从Excel/CSV导入客户数据")
                .icon(":/icons/import.png")
                .style(getToolbarButtonStyle())
                .onClick([this]() { importCustomerData(); })
                .build()
            )
            
            .addWidget(
                toolButton("btn_export_data")
                .text("📤 导出") 
                .tooltip("导出客户数据到Excel")
                .icon(":/icons/export.png")
                .style(getToolbarButtonStyle())
                .onClick([this]() { exportCustomerData(); })
                .build()
            )
            
            .addWidget(
                toolButton("btn_bulk_operations")
                .text("🔧 批量操作")
                .tooltip("批量编辑、删除客户")
                .icon(":/icons/bulk.png") 
                .style(getToolbarButtonStyle())
                .onClick([this]() { showBulkOperationsMenu(); })
                .build()
            )
            
            .build();
            
        return m_builder.createFromConfig(buttonGroup);
    }
    
    // 🔍 创建搜索和筛选区域
    QWidget* createSearchSection() {
        auto searchLayout = hBoxLayout()
            .spacing(15)
            .margins(QMargins(20, 15, 20, 15))
            .styleSheet("background: #ecf0f1; border-bottom: 2px solid #bdc3c7;")
            
            // 🔎 主搜索框
            .addWidget(
                lineEdit("search_main")
                .text("🔍 搜索客户姓名、公司、电话...")
                .minSize(QSize(300, 35))
                .style(R"(
                    QLineEdit {
                        border: 2px solid #3498db;
                        border-radius: 18px;
                        padding: 8px 15px;
                        font-size: 14px;
                        background: white;
                    }
                    QLineEdit:focus {
                        border-color: #2980b9;
                        background: #f8f9fa;
                    }
                )")
                .onTextChanged([this](const QString& text) { performSearch(text); })
                .build()
            )
            
            // 📊 高级筛选区域
            .addWidget(createAdvancedFilters())
            
            .addStretch()
            
            // 🔄 刷新按钮
            .addWidget(
                button("btn_refresh")
                .text("🔄 刷新")
                .tooltip("重新加载客户数据")
                .onClick([this]() { refreshCustomerList(); })
                .animation(DynamicUIBuilder::AnimationType::Pulse, 500)
                .build()
            )
            
            .build();
            
        return m_builder.createFromConfig(searchLayout);
    }
} 