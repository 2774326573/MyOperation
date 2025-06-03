/**
 * @file example_usage.cpp
 * @brief 动态UI构建器使用示例 / Dynamic UI Builder Usage Example
 * 
 * 这个文件展示了如何使用DynamicUIBuilder来动态创建各种UI控件
 * This file demonstrates how to use DynamicUIBuilder to dynamically create various UI widgets
 */

#include "../include/DynamicUIBuilder.h"
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

class ExampleWindow : public QMainWindow
{
    Q_OBJECT

public:
    ExampleWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setWindowTitle(tr("动态UI构建器示例 / Dynamic UI Builder Example"));
        resize(800, 600);
    }

private slots:
    void onButtonClicked(const QString& name)
    {
        QMessageBox::information(this, tr("按钮点击 / Button Clicked"), 
                               tr("按钮 '%1' 被点击了！/ Button '%1' was clicked!").arg(name));
    }

    void onValueChanged(const QString& name, const QVariant& value)
    {
        qDebug() << tr("控件值改变 / Widget value changed:") << name << "=" << value;
    }

private:
    void setupUI()
    {
        // 创建动态UI构建器 / Create dynamic UI builder
        DynamicUIBuilder* builder = new DynamicUIBuilder(this);
        
        // 连接信号 / Connect signals
        connect(builder, &DynamicUIBuilder::widgetClicked, this, &ExampleWindow::onButtonClicked);
        connect(builder, &DynamicUIBuilder::widgetValueChanged, this, &ExampleWindow::onValueChanged);

        // 示例1: 使用便捷构建器创建工具栏 / Example 1: Create toolbar using convenience builder
        createToolbarExample(builder);
        
        // 示例2: 使用JSON配置创建表单 / Example 2: Create form using JSON configuration
        createFormExample(builder);
        
        // 示例3: 创建复杂布局 / Example 3: Create complex layout
        createComplexLayoutExample(builder);
    }

    void createToolbarExample(DynamicUIBuilder* builder)
    {
        // 使用便捷构建器创建工具按钮 / Create tool buttons using convenience builder
        using namespace DynamicUI;
        
        auto toolbarLayout = hBoxLayout()
            .spacing(5)
            .margins(QMargins(10, 5, 10, 5))
            .addWidget(
                toolButton("btn_open")
                .text(tr("打开 / Open"))
                .tooltip(tr("打开文件 / Open File"))
                .icon(":/icons/open.png")
                .onClick([this]() {
                    QMessageBox::information(this, tr("信息 / Info"), tr("打开文件功能 / Open file function"));
                })
                .animation(DynamicUIBuilder::AnimationType::FadeIn, 500)
                .build()
            )
            .addWidget(
                toolButton("btn_save")
                .text(tr("保存 / Save"))
                .tooltip(tr("保存文件 / Save File"))
                .icon(":/icons/save.png")
                .onClick([this]() {
                    QMessageBox::information(this, tr("信息 / Info"), tr("保存文件功能 / Save file function"));
                })
                .animation(DynamicUIBuilder::AnimationType::SlideIn, 300)
                .build()
            )
            .addWidget(
                toolButton("btn_settings")
                .text(tr("设置 / Settings"))
                .tooltip(tr("应用设置 / Application Settings"))
                .icon(":/icons/settings.png")
                .onClick([this]() {
                    QMessageBox::information(this, tr("信息 / Info"), tr("设置功能 / Settings function"));
                })
                .animation(DynamicUIBuilder::AnimationType::Bounce, 400)
                .build()
            )
            .build();

        QWidget* toolbar = builder->createFromConfig(toolbarLayout, this);
        
        // 将工具栏添加到主窗口 / Add toolbar to main window
        QWidget* centralWidget = new QWidget(this);
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->addWidget(toolbar);
        
        setCentralWidget(centralWidget);
    }

    void createFormExample(DynamicUIBuilder* builder)
    {
        // 使用JSON配置创建表单 / Create form using JSON configuration
        QJsonObject formJson;
        formJson["layout"] = QJsonObject{
            {"type", "Form"},
            {"spacing", 10},
            {"margins", QJsonArray{15, 15, 15, 15}},
            {"widgets", QJsonArray{
                QJsonObject{
                    {"type", "Label"},
                    {"name", "lbl_name"},
                    {"text", tr("姓名 / Name:")},
                    {"styleSheet", "font-weight: bold; color: #2c3e50;"}
                },
                QJsonObject{
                    {"type", "LineEdit"},
                    {"name", "edit_name"},
                    {"text", tr("请输入姓名 / Please enter name")},
                    {"animation", "FadeIn"},
                    {"animationDuration", 600}
                },
                QJsonObject{
                    {"type", "Label"},
                    {"name", "lbl_age"},
                    {"text", tr("年龄 / Age:")},
                    {"styleSheet", "font-weight: bold; color: #2c3e50;"}
                },
                QJsonObject{
                    {"type", "SpinBox"},
                    {"name", "spin_age"},
                    {"value", 25},
                    {"properties", QJsonObject{
                        {"minimum", 0},
                        {"maximum", 120}
                    }},
                    {"animation", "SlideIn"},
                    {"animationDuration", 400}
                },
                QJsonObject{
                    {"type", "Label"},
                    {"name", "lbl_email"},
                    {"text", tr("邮箱 / Email:")},
                    {"styleSheet", "font-weight: bold; color: #2c3e50;"}
                },
                QJsonObject{
                    {"type", "LineEdit"},
                    {"name", "edit_email"},
                    {"text", tr("请输入邮箱 / Please enter email")},
                    {"animation", "Pulse"},
                    {"animationDuration", 500}
                },
                QJsonObject{
                    {"type", "CheckBox"},
                    {"name", "chk_subscribe"},
                    {"text", tr("订阅新闻 / Subscribe to newsletter")},
                    {"value", true},
                    {"animation", "Bounce"},
                    {"animationDuration", 300}
                }
            }}
        };

        QWidget* formWidget = builder->createFromJson(formJson, centralWidget());
        if (formWidget && centralWidget() && centralWidget()->layout()) {
            centralWidget()->layout()->addWidget(formWidget);
        }
    }

    void createComplexLayoutExample(DynamicUIBuilder* builder)
    {
        // 创建复杂的嵌套布局 / Create complex nested layout
        using namespace DynamicUI;
        
        auto complexLayout = vBoxLayout()
            .spacing(10)
            .margins(QMargins(15, 10, 15, 10))
            .addWidget(
                groupBox("group_controls")
                .text(tr("控制面板 / Control Panel"))
                .style("QGroupBox { font-weight: bold; border: 2px solid #3498db; border-radius: 5px; margin: 5px; padding-top: 10px; }")
                .build()
            )
            .addLayout(
                hBoxLayout()
                .spacing(15)
                .addWidget(
                    button("btn_start")
                    .text(tr("开始 / Start"))
                    .style("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px 20px; border-radius: 5px; }")
                    .onClick([this]() {
                        QMessageBox::information(this, tr("操作 / Action"), tr("开始操作 / Start operation"));
                    })
                    .animation(DynamicUIBuilder::AnimationType::FadeIn, 400)
                    .build()
                )
                .addWidget(
                    button("btn_stop")
                    .text(tr("停止 / Stop"))
                    .style("QPushButton { background-color: #e74c3c; color: white; font-weight: bold; padding: 10px 20px; border-radius: 5px; }")
                    .onClick([this]() {
                        QMessageBox::information(this, tr("操作 / Action"), tr("停止操作 / Stop operation"));
                    })
                    .animation(DynamicUIBuilder::AnimationType::SlideIn, 300)
                    .build()
                )
                .addWidget(
                    progressBar("progress_main")
                    .value(50)
                    .property("minimum", 0)
                    .property("maximum", 100)
                    .style("QProgressBar { border: 2px solid #bdc3c7; border-radius: 5px; text-align: center; } QProgressBar::chunk { background-color: #3498db; }")
                    .build()
                )
                .build()
            )
            .addWidget(
                tabWidget("tab_main")
                .style("QTabWidget::pane { border: 1px solid #bdc3c7; } QTabBar::tab { background-color: #ecf0f1; padding: 8px 16px; margin-right: 2px; } QTabBar::tab:selected { background-color: #3498db; color: white; }")
                .build()
            )
            .build();

        QWidget* complexWidget = builder->createFromConfig(complexLayout, centralWidget());
        if (complexWidget && centralWidget() && centralWidget()->layout()) {
            centralWidget()->layout()->addWidget(complexWidget);
        }

        // 应用主题 / Apply theme
        builder->applyTheme(complexWidget, "modern");
    }
};

// int main(int argc, char *argv[])
// {
//     QApplication app(argc, argv);

//     // 设置应用程序信息 / Set application information
//     app.setApplicationName("动态UI构建器示例 / Dynamic UI Builder Example");
//     app.setApplicationVersion("1.0.0");
//     app.setOrganizationName("MyCompany");

//     ExampleWindow window;
//     window.show();

//     return app.exec();
// } 