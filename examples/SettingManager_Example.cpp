// SettingManager 使用示例
// Created by jinxi on 25-6-5.

#include "../inc/config/SettingManager.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

/**
 * @brief SettingManager 使用示例类
 */
class SettingManagerExample : public QObject
{
    Q_OBJECT

public:
    SettingManagerExample(QObject* parent = nullptr) : QObject(parent)
    {
        settingManager = new SettingManager(this);
        setupExampleConfig();
    }

private:
    void setupExampleConfig()
    {
        // 初始化配置文件路径
        QString configDir = QApplication::applicationDirPath() + "/config/examples";
        QDir().mkpath(configDir);
        configFilePath = configDir + "/app_config.ini";
        
        if (!settingManager->init(configFilePath))
        {
            qWarning() << "配置管理器初始化失败";
            return;
        }
        
        qDebug() << "配置管理器初始化成功，配置文件路径:" << configFilePath;
    }

public slots:
    /**
     * @brief 演示基本配置操作
     */
    void demonstrateBasicOperations()
    {
        qDebug() << "\n=== 基本配置操作演示 ===";
        
        // 设置各种类型的配置值
        settingManager->setValue("app/name", "MyOperation");
        settingManager->setValue("app/version", "1.0.0");
        settingManager->setValue("app/debug_mode", true);
        
        settingManager->setValue("ui/theme", "dark");
        settingManager->setValue("ui/language", "zh_CN");
        settingManager->setValue("ui/window_width", 1024);
        settingManager->setValue("ui/window_height", 768);
        
        settingManager->setValue("network/host", "192.168.1.100");
        settingManager->setValue("network/port", 502);
        settingManager->setValue("network/timeout", 5000);
        
        // 读取配置值
        QString appName = settingManager->getValue("app/name", "Unknown").toString();
        QString version = settingManager->getValue("app/version", "0.0.0").toString();
        bool debugMode = settingManager->getValue("app/debug_mode", false).toBool();
        
        qDebug() << "应用名称:" << appName;
        qDebug() << "版本号:" << version;
        qDebug() << "调试模式:" << debugMode;
        
        // 设置和获取配置版本
        settingManager->setConfigVersion("1.2.0");
        QString configVersion = settingManager->getConfigVersion();
        qDebug() << "配置版本:" << configVersion;
    }
    
    /**
     * @brief 演示配置备份功能
     */
    void demonstrateBackupRestore()
    {
        qDebug() << "\n=== 配置备份和恢复演示 ===";
        
        // 创建备份目录
        QString backupDir = QApplication::applicationDirPath() + "/config/backups";
        QDir().mkpath(backupDir);
        
        // 备份当前配置
        QString backupPath = backupDir + "/config_backup.ini";
        if (settingManager->backupConfig(backupPath))
        {
            qDebug() << "配置备份成功";
            
            // 修改一些配置值
            settingManager->setValue("app/name", "ModifiedApp");
            settingManager->setValue("ui/theme", "light");
            qDebug() << "修改后的应用名称:" << settingManager->getValue("app/name").toString();
            qDebug() << "修改后的主题:" << settingManager->getValue("ui/theme").toString();
            
            // 从备份恢复
            if (settingManager->restoreConfig(backupPath))
            {
                qDebug() << "配置恢复成功";
                qDebug() << "恢复后的应用名称:" << settingManager->getValue("app/name").toString();
                qDebug() << "恢复后的主题:" << settingManager->getValue("ui/theme").toString();
            }
        }
    }
    
    /**
     * @brief 演示配置验证功能
     */
    void demonstrateValidation()
    {
        qDebug() << "\n=== 配置验证演示 ===";
        
        // 验证当前配置
        if (settingManager->validateConfig())
        {
            qDebug() << "当前配置验证通过";
        }
        
        // 创建一个无效的配置文件进行测试
        QString invalidConfigPath = QApplication::applicationDirPath() + "/config/examples/invalid_config.ini";
        QFile invalidFile(invalidConfigPath);
        if (invalidFile.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&invalidFile);
            stream << "这是一个无效的配置文件内容";
            invalidFile.close();
            
            // 验证无效配置文件
            if (!settingManager->validateConfig(invalidConfigPath))
            {
                qDebug() << "无效配置文件验证失败（预期结果）";
            }
            
            // 清理测试文件
            QFile::remove(invalidConfigPath);
        }
    }
    
    /**
     * @brief 演示配置迁移功能
     */
    void demonstrateMigration()
    {
        qDebug() << "\n=== 配置迁移演示 ===";
        
        // 创建旧版本配置文件
        QString oldConfigDir = QApplication::applicationDirPath() + "/config/migration";
        QDir().mkpath(oldConfigDir);
        QString oldConfigPath = oldConfigDir + "/old_config.ini";
        
        // 创建旧配置管理器并设置一些旧格式的配置
        SettingManager* oldManager = new SettingManager(this);
        if (oldManager->init(oldConfigPath))
        {
            // 旧版本的配置格式
            oldManager->setValue("application/app_name", "OldApp");
            oldManager->setValue("application/app_version", "0.9.0");
            oldManager->setValue("user_interface/skin", "blue");
            oldManager->setValue("networking/server_address", "localhost");
            oldManager->setValue("networking/server_port", 8080);
            
            qDebug() << "创建了旧版本配置文件";
            
            // 定义迁移规则
            QMap<QString, QString> migrationRules;
            migrationRules["application/app_name"] = "app/name";
            migrationRules["application/app_version"] = "app/version";
            migrationRules["user_interface/skin"] = "ui/theme";
            migrationRules["networking/server_address"] = "network/host";
            migrationRules["networking/server_port"] = "network/port";
            
            // 执行迁移
            QString newConfigPath = oldConfigDir + "/new_config.ini";
            if (settingManager->migrateConfig(oldConfigPath, newConfigPath, migrationRules))
            {
                qDebug() << "配置迁移成功";
                
                // 验证迁移结果
                SettingManager* newManager = new SettingManager(this);
                if (newManager->init(newConfigPath))
                {
                    qDebug() << "迁移后的应用名称:" << newManager->getValue("app/name").toString();
                    qDebug() << "迁移后的版本:" << newManager->getValue("app/version").toString();
                    qDebug() << "迁移后的主题:" << newManager->getValue("ui/theme").toString();
                    qDebug() << "迁移后的主机:" << newManager->getValue("network/host").toString();
                    qDebug() << "迁移后的端口:" << newManager->getValue("network/port").toInt();
                    qDebug() << "配置版本:" << newManager->getConfigVersion();
                }
                delete newManager;
            }
        }
        delete oldManager;
    }
    
    /**
     * @brief 演示完整的配置管理流程
     */
    void demonstrateCompleteWorkflow()
    {
        qDebug() << "\n=== 完整配置管理流程演示 ===";
        
        qDebug() << "1. 初始化配置";
        demonstrateBasicOperations();
        
        qDebug() << "\n2. 备份和恢复测试";
        demonstrateBackupRestore();
        
        qDebug() << "\n3. 配置验证测试";
        demonstrateValidation();
        
        qDebug() << "\n4. 配置迁移测试";
        demonstrateMigration();
        
        qDebug() << "\n=== 配置管理演示完成 ===";
    }

private:
    SettingManager* settingManager;
    QString configFilePath;
};

// int main(int argc, char *argv[])
// {
//     QApplication app(argc, argv);
    
//     qDebug() << "SettingManager 功能演示";
//     qDebug() << "========================";
    
//     SettingManagerExample example;
//     example.demonstrateCompleteWorkflow();
    
//     return 0;
// }

// #include "SettingManager_Example.moc"
