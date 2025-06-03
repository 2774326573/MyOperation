# ⚙️ 配置管理器文档 / Config Manager Documentation

<div align="center">

![Config](https://img.shields.io/badge/Config-Manager-yellow?style=for-the-badge&logo=data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMjQiIGhlaWdodD0iMjQiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTEyIDJMMTMuMDkgOC4yNkwyMCA5TDEzLjA5IDE1Ljc0TDEyIDIyTDEwLjkxIDE1Ljc0TDQgOUwxMC45MSA4LjI2TDEyIDJaIiBmaWxsPSJ3aGl0ZSIvPgo8L3N2Zz4K)
![Qt](https://img.shields.io/badge/Qt-6.0+-brightgreen?style=for-the-badge&logo=qt)
![JSON](https://img.shields.io/badge/JSON-Support-blue?style=for-the-badge)

**🚀 强大的配置管理系统 | Powerful Configuration Management System**

*支持多格式、分组管理和变更通知的完整配置解决方案*  
*Complete configuration solution supporting multiple formats, group management and change notifications*

</div>

---

## 📖 概述 / Overview

**ConfigManager** 是一个功能强大的配置管理器，支持多种配置类型、分组管理、配置变更通知和多种导入导出格式。采用单例模式设计，提供线程安全的配置操作。

**ConfigManager** is a powerful configuration manager supporting multiple configuration types, group management, change notifications and various import/export formats. Uses singleton pattern design with thread-safe configuration operations.

### 🌟 核心特性 / Key Features

<table>
<tr>
<td width="50%">

#### 📂 分组管理 / Group Management
- 分层配置组织
- 灵活的键值管理
- 批量操作支持

#### 🔄 格式支持 / Format Support
- INI 文件格式
- JSON 导入导出
- 自定义格式扩展

</td>
<td width="50%">

#### 🔔 变更通知 / Change Notifications
- 实时变更信号
- 变更前后事件
- 批量变更监听

#### 🛡️ 安全性 / Security
- 线程安全操作
- 数据验证机制
- 异常处理保护

</td>
</tr>
</table>

---

## ⚡ 快速开始 / Quick Start

### 📦 基本使用 / Basic Usage

```cpp
#include "config_manager.h"

int main()
{
    auto* configMgr = ConfigManager::getInstance();
    
    // 初始化配置管理器 (Initialize config manager)
    if (!configMgr->init("./config/app.ini")) {
        qWarning() << tr("配置管理器初始化失败 Config manager initialization failed");
        return -1;
    }
    
    // 设置配置值 (Set configuration values)
    configMgr->setValue("database", "host", "localhost");
    configMgr->setValue("database", "port", 3306);
    configMgr->setValue("ui", "theme", "dark");
    
    // 获取配置值 (Get configuration values)
    QString host = configMgr->getValue("database", "host", "127.0.0.1").toString();
    int port = configMgr->getValue("database", "port", 3306).toInt();
    
    qDebug() << tr("数据库配置 Database config:") << host << ":" << port;
    
    // 保存配置 (Save configuration)
    configMgr->saveConfig();
    
    return 0;
}
```

### 🔧 高级配置 / Advanced Configuration

```cpp
class ApplicationConfig : public QObject
{
    Q_OBJECT

public:
    ApplicationConfig()
    {
        m_configMgr = ConfigManager::getInstance();
        setupConfigManager();
        connectSignals();
    }

private:
    void setupConfigManager()
    {
        // 初始化配置管理器 (Initialize config manager)
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir().mkpath(configPath);
        
        QString configFile = QDir(configPath).filePath("testmodbus.ini");
        if (!m_configMgr->init(configFile)) {
            qWarning() << tr("无法初始化配置文件 Cannot initialize config file:") << configFile;
        }
        
        // 设置默认配置 (Set default configuration)
        setDefaultValues();
    }
    
    void connectSignals()
    {
        // 连接配置变更信号 (Connect configuration change signals)
        connect(m_configMgr, &ConfigManager::valueChanged,
                this, &ApplicationConfig::onConfigValueChanged);
        
        connect(m_configMgr, &ConfigManager::valueChanging,
                this, &ApplicationConfig::onConfigValueChanging);
    }
    
    void setDefaultValues()
    {
        // 应用程序设置 (Application settings)
        if (!m_configMgr->hasValue("app", "language")) {
            m_configMgr->setValue("app", "language", "zh_CN");
        }
        if (!m_configMgr->hasValue("app", "theme")) {
            m_configMgr->setValue("app", "theme", "light");
        }
        
        // 网络设置 (Network settings)
        if (!m_configMgr->hasValue("network", "timeout")) {
            m_configMgr->setValue("network", "timeout", 5000);
        }
        if (!m_configMgr->hasValue("network", "retry_count")) {
            m_configMgr->setValue("network", "retry_count", 3);
        }
    }

private slots:
    void onConfigValueChanged(const QString& group, const QString& key, const QVariant& value)
    {
        qDebug() << tr("配置已更改 Configuration changed:") 
                << QString("%1.%2 = %3").arg(group, key, value.toString());
        
        // 根据配置变更执行相应操作 (Execute corresponding operations based on config changes)
        if (group == "app" && key == "theme") {
            emit themeChanged(value.toString());
        } else if (group == "app" && key == "language") {
            emit languageChanged(value.toString());
        }
    }
    
    void onConfigValueChanging(const QString& group, const QString& key, 
                              const QVariant& oldValue, const QVariant& newValue)
    {
        qDebug() << tr("配置即将更改 Configuration changing:") 
                << QString("%1.%2: %3 -> %4").arg(group, key, oldValue.toString(), newValue.toString());
    }

signals:
    void themeChanged(const QString& theme);
    void languageChanged(const QString& language);

private:
    ConfigManager* m_configMgr;
};
```

---

## 🛠️ API 参考 / API Reference

### 📋 核心方法 / Core Methods

<table>
<tr>
<th>🔧 方法 / Method</th>
<th>📝 描述 / Description</th>
<th>💡 示例 / Example</th>
</tr>
<tr>
<td><code>getInstance()</code></td>
<td>获取单例实例<br/>Get singleton instance</td>
<td><code>auto* mgr = ConfigManager::getInstance();</code></td>
</tr>
<tr>
<td><code>init()</code></td>
<td>初始化配置管理器<br/>Initialize config manager</td>
<td><code>mgr->init("config.ini");</code></td>
</tr>
<tr>
<td><code>setValue()</code></td>
<td>设置配置值<br/>Set configuration value</td>
<td><code>mgr->setValue("group", "key", value);</code></td>
</tr>
<tr>
<td><code>getValue()</code></td>
<td>获取配置值<br/>Get configuration value</td>
<td><code>auto val = mgr->getValue("group", "key", default);</code></td>
</tr>
<tr>
<td><code>hasValue()</code></td>
<td>检查配置是否存在<br/>Check if config exists</td>
<td><code>bool exists = mgr->hasValue("group", "key");</code></td>
</tr>
</table>

### 🎛️ 高级操作 / Advanced Operations

<table>
<tr>
<th>🔧 方法 / Method</th>
<th>📝 描述 / Description</th>
<th>💡 示例 / Example</th>
</tr>
<tr>
<td><code>removeValue()</code></td>
<td>移除配置项<br/>Remove configuration item</td>
<td><code>mgr->removeValue("group", "key");</code></td>
</tr>
<tr>
<td><code>removeGroup()</code></td>
<td>移除配置组<br/>Remove configuration group</td>
<td><code>mgr->removeGroup("group");</code></td>
</tr>
<tr>
<td><code>getGroups()</code></td>
<td>获取所有配置组<br/>Get all configuration groups</td>
<td><code>auto groups = mgr->getGroups();</code></td>
</tr>
<tr>
<td><code>getKeys()</code></td>
<td>获取组内所有键<br/>Get all keys in group</td>
<td><code>auto keys = mgr->getKeys("group");</code></td>
</tr>
<tr>
<td><code>exportToJson()</code></td>
<td>导出到JSON<br/>Export to JSON</td>
<td><code>mgr->exportToJson("config.json");</code></td>
</tr>
<tr>
<td><code>importFromJson()</code></td>
<td>从JSON导入<br/>Import from JSON</td>
<td><code>mgr->importFromJson("config.json");</code></td>
</tr>
</table>

---

## 💡 使用示例 / Usage Examples

### 1️⃣ **应用程序设置管理 / Application Settings Management**

```cpp
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    SettingsManager()
    {
        m_config = ConfigManager::getInstance();
        initializeSettings();
    }

    // 应用程序设置 (Application Settings)
    QString getLanguage() const
    {
        return m_config->getValue("app", "language", "zh_CN").toString();
    }
    
    void setLanguage(const QString& language)
    {
        m_config->setValue("app", "language", language);
        emit languageChanged(language);
    }
    
    QString getTheme() const
    {
        return m_config->getValue("app", "theme", "light").toString();
    }
    
    void setTheme(const QString& theme)
    {
        m_config->setValue("app", "theme", theme);
        emit themeChanged(theme);
    }
    
    // 窗口设置 (Window Settings)
    QSize getWindowSize() const
    {
        int width = m_config->getValue("window", "width", 1024).toInt();
        int height = m_config->getValue("window", "height", 768).toInt();
        return QSize(width, height);
    }
    
    void setWindowSize(const QSize& size)
    {
        m_config->setValue("window", "width", size.width());
        m_config->setValue("window", "height", size.height());
    }
    
    QPoint getWindowPosition() const
    {
        int x = m_config->getValue("window", "x", 100).toInt();
        int y = m_config->getValue("window", "y", 100).toInt();
        return QPoint(x, y);
    }
    
    void setWindowPosition(const QPoint& position)
    {
        m_config->setValue("window", "x", position.x());
        m_config->setValue("window", "y", position.y());
    }
    
    bool isMaximized() const
    {
        return m_config->getValue("window", "maximized", false).toBool();
    }
    
    void setMaximized(bool maximized)
    {
        m_config->setValue("window", "maximized", maximized);
    }

    // 网络设置 (Network Settings)
    QString getServerHost() const
    {
        return m_config->getValue("network", "host", "localhost").toString();
    }
    
    void setServerHost(const QString& host)
    {
        m_config->setValue("network", "host", host);
    }
    
    int getServerPort() const
    {
        return m_config->getValue("network", "port", 502).toInt();
    }
    
    void setServerPort(int port)
    {
        m_config->setValue("network", "port", port);
    }
    
    int getConnectionTimeout() const
    {
        return m_config->getValue("network", "timeout", 5000).toInt();
    }
    
    void setConnectionTimeout(int timeout)
    {
        m_config->setValue("network", "timeout", timeout);
    }

public slots:
    void saveSettings()
    {
        if (!m_config->saveConfig()) {
            qWarning() << tr("保存配置失败 Failed to save configuration");
        } else {
            qDebug() << tr("配置已保存 Configuration saved");
        }
    }
    
    void resetToDefaults()
    {
        // 重置为默认值 (Reset to default values)
        m_config->setValue("app", "language", "zh_CN");
        m_config->setValue("app", "theme", "light");
        m_config->setValue("window", "width", 1024);
        m_config->setValue("window", "height", 768);
        m_config->setValue("window", "x", 100);
        m_config->setValue("window", "y", 100);
        m_config->setValue("window", "maximized", false);
        m_config->setValue("network", "host", "localhost");
        m_config->setValue("network", "port", 502);
        m_config->setValue("network", "timeout", 5000);
        
        qDebug() << tr("配置已重置为默认值 Configuration reset to defaults");
    }

signals:
    void languageChanged(const QString& language);
    void themeChanged(const QString& theme);

private:
    void initializeSettings()
    {
        // 连接配置变更信号 (Connect configuration change signals)
        connect(m_config, &ConfigManager::valueChanged,
                this, &SettingsManager::onConfigChanged);
    }

private slots:
    void onConfigChanged(const QString& group, const QString& key, const QVariant& value)
    {
        qDebug() << tr("设置已更改 Setting changed:") 
                << QString("%1.%2 = %3").arg(group, key, value.toString());
    }

private:
    ConfigManager* m_config;
};
```

### 2️⃣ **配置备份和恢复 / Configuration Backup and Restore**

```cpp
class ConfigBackupManager : public QObject
{
    Q_OBJECT

public:
    ConfigBackupManager()
    {
        m_config = ConfigManager::getInstance();
    }

    bool createBackup(const QString& backupName = QString())
    {
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString fileName = backupName.isEmpty() ? 
                          QString("config_backup_%1.json").arg(timestamp) : 
                          QString("%1_%2.json").arg(backupName, timestamp);
        
        QString backupDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups";
        QDir().mkpath(backupDir);
        
        QString backupPath = QDir(backupDir).filePath(fileName);
        
        if (m_config->exportToJson(backupPath)) {
            qDebug() << tr("配置备份已创建 Configuration backup created:") << backupPath;
            m_backupHistory.append({backupPath, QDateTime::currentDateTime(), backupName});
            return true;
        } else {
            qWarning() << tr("配置备份创建失败 Failed to create configuration backup");
            return false;
        }
    }
    
    bool restoreFromBackup(const QString& backupPath, bool merge = false)
    {
        if (!QFile::exists(backupPath)) {
            qWarning() << tr("备份文件不存在 Backup file does not exist:") << backupPath;
            return false;
        }
        
        // 创建当前配置的自动备份 (Create automatic backup of current config)
        createBackup("auto_before_restore");
        
        if (m_config->importFromJson(backupPath, merge)) {
            qDebug() << tr("配置已从备份恢复 Configuration restored from backup:") << backupPath;
            return true;
        } else {
            qWarning() << tr("配置恢复失败 Failed to restore configuration from backup");
            return false;
        }
    }
    
    QStringList getBackupList() const
    {
        QStringList backups;
        for (const auto& backup : m_backupHistory) {
            QString info = QString("%1 - %2").arg(
                backup.dateTime.toString("yyyy-MM-dd hh:mm:ss"),
                backup.name.isEmpty() ? tr("自动备份 Auto backup") : backup.name
            );
            backups.append(info);
        }
        return backups;
    }
    
    void cleanOldBackups(int keepCount = 10)
    {
        QString backupDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups";
        QDir dir(backupDir);
        
        QFileInfoList files = dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time);
        
        // 删除超过保留数量的旧备份 (Delete old backups exceeding keep count)
        for (int i = keepCount; i < files.size(); ++i) {
            if (QFile::remove(files[i].absoluteFilePath())) {
                qDebug() << tr("已删除旧备份 Deleted old backup:") << files[i].fileName();
            }
        }
    }

private:
    struct BackupInfo {
        QString filePath;
        QDateTime dateTime;
        QString name;
    };

    ConfigManager* m_config;
    QList<BackupInfo> m_backupHistory;
};
```

### 3️⃣ **配置验证和迁移 / Configuration Validation and Migration**

```cpp
class ConfigValidator : public QObject
{
    Q_OBJECT

public:
    ConfigValidator()
    {
        m_config = ConfigManager::getInstance();
        setupValidationRules();
    }

    bool validateConfiguration()
    {
        bool isValid = true;
        QStringList errors;
        
        // 验证必需的配置项 (Validate required configuration items)
        for (const auto& rule : m_validationRules) {
            if (!validateRule(rule, errors)) {
                isValid = false;
            }
        }
        
        if (!isValid) {
            qWarning() << tr("配置验证失败 Configuration validation failed:");
            for (const QString& error : errors) {
                qWarning() << "  -" << error;
            }
        }
        
        return isValid;
    }
    
    bool migrateConfiguration(int fromVersion, int toVersion)
    {
        qDebug() << tr("开始配置迁移 Starting configuration migration from version") 
                << fromVersion << tr("到 to") << toVersion;
        
        for (int version = fromVersion; version < toVersion; ++version) {
            if (!migrateFromVersion(version)) {
                qWarning() << tr("配置迁移失败 Configuration migration failed at version") << version;
                return false;
            }
        }
        
        // 更新配置版本 (Update configuration version)
        m_config->setValue("app", "config_version", toVersion);
        
        qDebug() << tr("配置迁移完成 Configuration migration completed");
        return true;
    }
    
    void fixInvalidConfigurations()
    {
        // 修复无效的配置值 (Fix invalid configuration values)
        
        // 检查端口范围 (Check port range)
        int port = m_config->getValue("network", "port", 502).toInt();
        if (port < 1 || port > 65535) {
            qWarning() << tr("无效的端口号，重置为默认值 Invalid port number, resetting to default");
            m_config->setValue("network", "port", 502);
        }
        
        // 检查超时值 (Check timeout value)
        int timeout = m_config->getValue("network", "timeout", 5000).toInt();
        if (timeout < 1000 || timeout > 60000) {
            qWarning() << tr("无效的超时值，重置为默认值 Invalid timeout value, resetting to default");
            m_config->setValue("network", "timeout", 5000);
        }
        
        // 检查主题设置 (Check theme setting)
        QString theme = m_config->getValue("app", "theme", "light").toString();
        QStringList validThemes = {"light", "dark", "auto"};
        if (!validThemes.contains(theme)) {
            qWarning() << tr("无效的主题设置，重置为默认值 Invalid theme setting, resetting to default");
            m_config->setValue("app", "theme", "light");
        }
        
        // 检查语言设置 (Check language setting)
        QString language = m_config->getValue("app", "language", "zh_CN").toString();
        QStringList validLanguages = {"zh_CN", "en_US", "ja_JP", "ko_KR"};
        if (!validLanguages.contains(language)) {
            qWarning() << tr("无效的语言设置，重置为默认值 Invalid language setting, resetting to default");
            m_config->setValue("app", "language", "zh_CN");
        }
    }

private:
    struct ValidationRule {
        QString group;
        QString key;
        QVariant::Type type;
        QVariant minValue;
        QVariant maxValue;
        QStringList allowedValues;
        bool required;
    };
    
    void setupValidationRules()
    {
        // 网络配置验证规则 (Network configuration validation rules)
        m_validationRules.append({
            "network", "port", QVariant::Int, 1, 65535, {}, true
        });
        
        m_validationRules.append({
            "network", "timeout", QVariant::Int, 1000, 60000, {}, true
        });
        
        // 应用程序配置验证规则 (Application configuration validation rules)
        m_validationRules.append({
            "app", "theme", QVariant::String, {}, {}, {"light", "dark", "auto"}, true
        });
        
        m_validationRules.append({
            "app", "language", QVariant::String, {}, {}, {"zh_CN", "en_US", "ja_JP", "ko_KR"}, true
        });
    }
    
    bool validateRule(const ValidationRule& rule, QStringList& errors)
    {
        if (!m_config->hasValue(rule.group, rule.key)) {
            if (rule.required) {
                errors.append(tr("缺少必需的配置项 Missing required configuration: %1.%2")
                             .arg(rule.group, rule.key));
                return false;
            }
            return true;
        }
        
        QVariant value = m_config->getValue(rule.group, rule.key);
        
        // 类型检查 (Type check)
        if (value.type() != rule.type) {
            errors.append(tr("配置项类型错误 Invalid configuration type: %1.%2")
                         .arg(rule.group, rule.key));
            return false;
        }
        
        // 范围检查 (Range check)
        if (rule.minValue.isValid() && value < rule.minValue) {
            errors.append(tr("配置值过小 Configuration value too small: %1.%2")
                         .arg(rule.group, rule.key));
            return false;
        }
        
        if (rule.maxValue.isValid() && value > rule.maxValue) {
            errors.append(tr("配置值过大 Configuration value too large: %1.%2")
                         .arg(rule.group, rule.key));
            return false;
        }
        
        // 允许值检查 (Allowed values check)
        if (!rule.allowedValues.isEmpty() && !rule.allowedValues.contains(value.toString())) {
            errors.append(tr("配置值不在允许范围内 Configuration value not in allowed range: %1.%2")
                         .arg(rule.group, rule.key));
            return false;
        }
        
        return true;
    }
    
    bool migrateFromVersion(int version)
    {
        switch (version) {
        case 1:
            // 从版本1迁移到版本2 (Migrate from version 1 to 2)
            return migrateV1ToV2();
        case 2:
            // 从版本2迁移到版本3 (Migrate from version 2 to 3)
            return migrateV2ToV3();
        default:
            qWarning() << tr("未知的配置版本 Unknown configuration version:") << version;
            return false;
        }
    }
    
    bool migrateV1ToV2()
    {
        // 示例：重命名配置项 (Example: Rename configuration items)
        if (m_config->hasValue("network", "server_host")) {
            QVariant value = m_config->getValue("network", "server_host");
            m_config->setValue("network", "host", value);
            m_config->removeValue("network", "server_host");
        }
        
        return true;
    }
    
    bool migrateV2ToV3()
    {
        // 示例：添加新的默认配置 (Example: Add new default configurations)
        if (!m_config->hasValue("app", "auto_save")) {
            m_config->setValue("app", "auto_save", true);
        }
        
        return true;
    }

private:
    ConfigManager* m_config;
    QList<ValidationRule> m_validationRules;
};
```

---

## ⚠️ 注意事项 / Precautions

### 🚨 大项目注意事项 / Large Project Considerations

#### 1️⃣ **性能优化 / Performance Optimization**

```cpp
// ❌ 错误：频繁的配置读写 (Wrong: Frequent config read/write)
void badConfigPractice()
{
    auto* config = ConfigManager::getInstance();
    for (int i = 0; i < 1000; ++i) {
        config->setValue("temp", QString("key_%1").arg(i), i);
        config->saveConfig(); // 每次都保存，性能问题！
    }
}

// ✅ 正确：批量操作和延迟保存 (Correct: Batch operations and delayed saving)
class ConfigBatcher
{
private:
    QTimer* m_saveTimer;
    bool m_hasChanges;

public:
    ConfigBatcher()
    {
        m_saveTimer = new QTimer(this);
        m_saveTimer->setSingleShot(true);
        m_saveTimer->setInterval(1000); // 1秒延迟保存
        connect(m_saveTimer, &QTimer::timeout, this, &ConfigBatcher::saveConfig);
        m_hasChanges = false;
    }

    void setValue(const QString& group, const QString& key, const QVariant& value)
    {
        auto* config = ConfigManager::getInstance();
        config->setValue(group, key, value);
        
        m_hasChanges = true;
        if (!m_saveTimer->isActive()) {
            m_saveTimer->start();
        }
    }

private slots:
    void saveConfig()
    {
        if (m_hasChanges) {
            auto* config = ConfigManager::getInstance();
            config->saveConfig();
            m_hasChanges = false;
        }
    }
};
```

#### 2️⃣ **内存管理 / Memory Management**

```cpp
// 推荐：正确的单例生命周期管理 (Recommended: Proper singleton lifecycle management)
class ConfigManagerWrapper
{
public:
    static ConfigManager* getConfig()
    {
        static ConfigManager* instance = nullptr;
        if (!instance) {
            instance = ConfigManager::getInstance();
        }
        return instance;
    }
    
    static void cleanup()
    {
        ConfigManager::releaseInstance();
    }
};

// 在应用程序退出时清理 (Cleanup on application exit)
class Application : public QApplication
{
public:
    Application(int argc, char* argv[]) : QApplication(argc, argv) {}
    
    ~Application()
    {
        ConfigManagerWrapper::cleanup();
    }
};
```

#### 3️⃣ **线程安全 / Thread Safety**

```cpp
// ⚠️ 注意：配置操作的线程安全 (Caution: Thread safety of config operations)
class ThreadSafeConfigAccess
{
private:
    static QMutex s_mutex;

public:
    static QVariant getValue(const QString& group, const QString& key, const QVariant& defaultValue = QVariant())
    {
        QMutexLocker locker(&s_mutex);
        auto* config = ConfigManager::getInstance();
        return config->getValue(group, key, defaultValue);
    }
    
    static void setValue(const QString& group, const QString& key, const QVariant& value)
    {
        QMutexLocker locker(&s_mutex);
        auto* config = ConfigManager::getInstance();
        config->setValue(group, key, value);
    }
};

QMutex ThreadSafeConfigAccess::s_mutex;
```

---

## 🔍 故障排除 / Troubleshooting

### 📋 常见问题 / Common Issues

<table>
<tr>
<th>⚠️ 问题 / Issue</th>
<th>🔍 原因 / Cause</th>
<th>✅ 解决方案 / Solution</th>
</tr>
<tr>
<td>配置文件加载失败</td>
<td>文件路径错误或权限不足</td>
<td>检查文件路径和目录权限</td>
</tr>
<tr>
<td>配置值类型错误</td>
<td>类型转换失败</td>
<td>使用正确的类型转换方法</td>
</tr>
<tr>
<td>配置保存失败</td>
<td>磁盘空间不足或权限问题</td>
<td>检查磁盘空间和写入权限</td>
</tr>
<tr>
<td>JSON导入导出失败</td>
<td>JSON格式错误或文件损坏</td>
<td>验证JSON格式，使用备份文件</td>
</tr>
</table>

---

<div align="center">

## 🎉 总结 / Summary

**ConfigManager 提供了完整而强大的配置管理解决方案，支持多种格式、分组管理和变更通知。通过合理使用配置系统，可以构建灵活、可维护的应用程序配置架构。**

**ConfigManager provides a complete and powerful configuration management solution supporting multiple formats, group management and change notifications. Through proper use of the configuration system, you can build flexible and maintainable application configuration architectures.**

---

<img src="https://img.shields.io/badge/Status-Document%20Complete-success?style=for-the-badge" alt="Complete">
<img src="https://img.shields.io/badge/Quality-Professional-blue?style=for-the-badge" alt="Quality">
<img src="https://img.shields.io/badge/Language-中英双语-orange?style=for-the-badge" alt="Bilingual">

**📝 文档版本 / Document Version:** v1.0  
**📅 最后更新 / Last Updated:** 2024  
**👨‍💻 维护者 / Maintainer:** TestModbus Team

</div> 