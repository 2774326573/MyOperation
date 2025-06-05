# 📋 SettingManager 用户手册

**版本**: 1.0.0  
**创建时间**: 2025年6月5日  
**作者**: jinxi  

---

## 📑 目录

1. [概述](#概述)
2. [功能特性](#功能特性)
3. [快速开始](#快速开始)
4. [API 参考](#api-参考)
5. [使用示例](#使用示例)
6. [高级功能](#高级功能)
7. [最佳实践](#最佳实践)
8. [故障排除](#故障排除)
9. [常见问题](#常见问题)

---

## 🎯 概述

`SettingManager` 是基于 Qt 框架开发的强大配置管理类，为应用程序提供了完整的配置管理解决方案。它封装了底层的 `ConfigManager`，提供了更加简洁和易用的接口。

### 🌟 核心特性

- **简单易用**: 提供直观的键值对配置接口
- **分组管理**: 支持 `group/key` 格式的分层配置
- **备份恢复**: 内置配置备份和恢复功能
- **配置验证**: 自动验证配置文件的完整性
- **版本管理**: 支持配置版本控制和迁移
- **类型安全**: 支持多种 Qt 数据类型

---

## 🚀 功能特性

### 基础功能
- ✅ 配置文件读写 (INI 格式)
- ✅ 键值对存储和检索
- ✅ 默认值支持
- ✅ 数据类型转换

### 高级功能
- ✅ 自动配置备份 (带时间戳)
- ✅ 配置恢复和回滚
- ✅ 配置文件验证 (支持 INI 和 JSON 格式)
- ✅ 版本间配置迁移
- ✅ 迁移规则映射
- ✅ 配置版本号管理

---

## ⚡ 快速开始

### 1. 包含头文件

```cpp
#include "inc/config/SettingManager.h"
```

### 2. 创建和初始化

```cpp
// 创建 SettingManager 实例
SettingManager* settingManager = new SettingManager(this);

// 初始化配置文件
QString configPath = QApplication::applicationDirPath() + "/config/app.ini";
if (!settingManager->init(configPath)) {
    qWarning() << "配置管理器初始化失败";
    return;
}
```

### 3. 基本操作

```cpp
// 设置配置值
settingManager->setValue("app/name", "MyApplication");
settingManager->setValue("ui/theme", "dark");
settingManager->setValue("network/port", 8080);

// 获取配置值
QString appName = settingManager->getValue("app/name", "Unknown").toString();
QString theme = settingManager->getValue("ui/theme", "light").toString();
int port = settingManager->getValue("network/port", 80).toInt();
```

---

## 📚 API 参考

### 构造和初始化

#### `SettingManager(QObject* parent = nullptr)`
创建 SettingManager 实例。

**参数**:
- `parent`: 父对象指针

#### `bool init(const QString& configFilePath)`
初始化配置管理器。

**参数**:
- `configFilePath`: 配置文件路径

**返回值**: 初始化是否成功

---

### 基础配置操作

#### `QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const`
获取配置值。

**参数**:
- `key`: 配置键名，格式为 "group/key"
- `defaultValue`: 默认值

**返回值**: 配置值

**示例**:
```cpp
// 获取字符串值
QString theme = settingManager->getValue("ui/theme", "light").toString();

// 获取整数值
int width = settingManager->getValue("ui/window_width", 1024).toInt();

// 获取布尔值
bool debugMode = settingManager->getValue("app/debug_mode", false).toBool();
```

#### `void setValue(const QString& key, const QVariant& value)`
设置配置值。

**参数**:
- `key`: 配置键名，格式为 "group/key"
- `value`: 配置值

**示例**:
```cpp
settingManager->setValue("app/name", "MyApp");
settingManager->setValue("ui/window_width", 1920);
settingManager->setValue("app/auto_save", true);
```

---

### 备份和恢复

#### `bool backupConfig(const QString& backupFilePath)`
备份当前配置到指定文件。

**参数**:
- `backupFilePath`: 备份文件路径

**返回值**: 备份是否成功

**特性**:
- 自动添加时间戳到文件名
- 自动创建备份目录
- 支持完整配置备份

**示例**:
```cpp
QString backupPath = QApplication::applicationDirPath() + "/backups/config.ini";
if (settingManager->backupConfig(backupPath)) {
    qDebug() << "配置备份成功";
}
```

#### `bool restoreConfig(const QString& backupFilePath)`
从备份文件恢复配置。

**参数**:
- `backupFilePath`: 备份文件路径

**返回值**: 恢复是否成功

**特性**:
- 恢复前自动验证备份文件
- 恢复前自动备份当前配置
- 支持完整配置恢复

**示例**:
```cpp
QString backupPath = "/path/to/backup/config_backup_20250605_143022.ini";
if (settingManager->restoreConfig(backupPath)) {
    qDebug() << "配置恢复成功";
}
```

---

### 配置验证

#### `bool validateConfig(const QString& configFilePath = QString())`
验证配置文件的完整性和有效性。

**参数**:
- `configFilePath`: 要验证的配置文件路径（为空则验证当前配置）

**返回值**: 验证是否通过

**支持的格式**:
- INI 文件格式
- JSON 文件格式

**示例**:
```cpp
// 验证当前配置
if (settingManager->validateConfig()) {
    qDebug() << "当前配置验证通过";
}

// 验证指定配置文件
if (settingManager->validateConfig("/path/to/config.ini")) {
    qDebug() << "配置文件验证通过";
}
```

---

### 配置迁移

#### `bool migrateConfig(const QString& oldConfigPath, const QString& newConfigPath, const QMap<QString, QString>& migrationRules = QMap<QString, QString>())`
迁移旧版本配置到新版本。

**参数**:
- `oldConfigPath`: 旧配置文件路径
- `newConfigPath`: 新配置文件路径
- `migrationRules`: 迁移规则映射（旧key->新key）

**返回值**: 迁移是否成功

**特性**:
- 支持键名重新映射
- 自动添加版本信息
- 自动备份旧配置
- 验证旧配置有效性

**示例**:
```cpp
// 定义迁移规则
QMap<QString, QString> rules;
rules["old_group/old_key"] = "new_group/new_key";
rules["app/app_name"] = "application/name";
rules["ui/skin"] = "ui/theme";

// 执行迁移
QString oldPath = "/path/to/old_config.ini";
QString newPath = "/path/to/new_config.ini";
if (settingManager->migrateConfig(oldPath, newPath, rules)) {
    qDebug() << "配置迁移成功";
}
```

---

### 版本管理

#### `QString getConfigVersion() const`
获取配置版本号。

**返回值**: 配置版本号字符串

#### `void setConfigVersion(const QString& version)`
设置配置版本号。

**参数**:
- `version`: 版本号字符串

**示例**:
```cpp
// 设置配置版本
settingManager->setConfigVersion("2.1.0");

// 获取配置版本
QString version = settingManager->getConfigVersion();
qDebug() << "当前配置版本:" << version;
```

---

## 💡 使用示例

### 示例 1: 应用程序设置管理

```cpp
class AppSettings
{
public:
    AppSettings() {
        settingManager = new SettingManager();
        QString configPath = QApplication::applicationDirPath() + "/config/app.ini";
        settingManager->init(configPath);
    }
    
    // 应用程序设置
    QString getAppName() const {
        return settingManager->getValue("app/name", "MyApp").toString();
    }
    
    void setAppName(const QString& name) {
        settingManager->setValue("app/name", name);
    }
    
    // 窗口设置
    QSize getWindowSize() const {
        int width = settingManager->getValue("window/width", 1024).toInt();
        int height = settingManager->getValue("window/height", 768).toInt();
        return QSize(width, height);
    }
    
    void setWindowSize(const QSize& size) {
        settingManager->setValue("window/width", size.width());
        settingManager->setValue("window/height", size.height());
    }
    
    // 网络设置
    QString getServerHost() const {
        return settingManager->getValue("network/host", "localhost").toString();
    }
    
    void setServerHost(const QString& host) {
        settingManager->setValue("network/host", host);
    }

private:
    SettingManager* settingManager;
};
```

### 示例 2: 配置备份管理器

```cpp
class ConfigBackupManager
{
public:
    ConfigBackupManager(SettingManager* manager) : settingManager(manager) {}
    
    bool createBackup(const QString& backupName = QString()) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString fileName = backupName.isEmpty() ? 
                          QString("config_backup_%1.ini").arg(timestamp) : 
                          QString("%1_%2.ini").arg(backupName, timestamp);
        
        QString backupDir = QApplication::applicationDirPath() + "/backups";
        QDir().mkpath(backupDir);
        QString backupPath = QDir(backupDir).filePath(fileName);
        
        return settingManager->backupConfig(backupPath);
    }
    
    bool restoreFromBackup(const QString& backupPath) {
        if (!QFile::exists(backupPath)) {
            qWarning() << "备份文件不存在:" << backupPath;
            return false;
        }
        
        return settingManager->restoreConfig(backupPath);
    }
    
    QStringList getAvailableBackups() {
        QString backupDir = QApplication::applicationDirPath() + "/backups";
        QDir dir(backupDir);
        return dir.entryList(QStringList() << "*.ini", QDir::Files, QDir::Time);
    }

private:
    SettingManager* settingManager;
};
```

### 示例 3: 配置迁移管理器

```cpp
class ConfigMigrationManager
{
public:
    ConfigMigrationManager(SettingManager* manager) : settingManager(manager) {}
    
    bool migrateFromV1ToV2(const QString& oldConfigPath) {
        // 定义 v1 到 v2 的迁移规则
        QMap<QString, QString> migrationRules;
        migrationRules["application/app_name"] = "app/name";
        migrationRules["application/version"] = "app/version";
        migrationRules["user_interface/theme"] = "ui/theme";
        migrationRules["user_interface/language"] = "ui/language";
        migrationRules["networking/server_host"] = "network/host";
        migrationRules["networking/server_port"] = "network/port";
        
        QString newConfigPath = oldConfigPath + ".v2";
        
        if (settingManager->migrateConfig(oldConfigPath, newConfigPath, migrationRules)) {
            // 更新配置版本
            settingManager->setConfigVersion("2.0.0");
            qDebug() << "成功迁移到版本 2.0.0";
            return true;
        }
        
        return false;
    }
    
    bool checkConfigVersion() {
        QString currentVersion = settingManager->getConfigVersion();
        
        if (currentVersion == "1.0.0") {
            qDebug() << "检测到旧版本配置，建议升级";
            return false;
        } else if (currentVersion.isEmpty()) {
            qDebug() << "未知配置版本，设置为当前版本";
            settingManager->setConfigVersion("2.0.0");
        }
        
        return true;
    }

private:
    SettingManager* settingManager;
};
```

---

## 🔧 高级功能

### 配置文件格式

SettingManager 支持标准的 INI 文件格式：

```ini
[app]
name=MyApplication
version=2.1.0
debug_mode=true

[ui]
theme=dark
language=zh_CN
window_width=1920
window_height=1080

[network]
host=192.168.1.100
port=8080
timeout=5000

[system]
config_version=2.1.0
```

### 键名约定

使用 `group/key` 格式的键名：

```cpp
// 推荐的键名格式
"app/name"              // 应用程序名称
"app/version"           // 应用程序版本
"ui/theme"              // 用户界面主题
"ui/language"           // 界面语言
"window/width"          // 窗口宽度
"window/height"         // 窗口高度
"network/host"          // 网络主机
"network/port"          // 网络端口
"system/config_version" // 配置版本
```

### 数据类型支持

SettingManager 支持所有 QVariant 支持的数据类型：

```cpp
// 字符串
settingManager->setValue("app/name", QString("MyApp"));

// 整数
settingManager->setValue("network/port", 8080);

// 浮点数
settingManager->setValue("graphics/scale", 1.5);

// 布尔值
settingManager->setValue("app/debug_mode", true);

// 字符串列表
QStringList themes = {"dark", "light", "auto"};
settingManager->setValue("ui/available_themes", themes);

// 字节数组
QByteArray data = "binary data";
settingManager->setValue("cache/data", data);
```

---

## 📋 最佳实践

### 1. 配置文件组织

```cpp
// 推荐的配置分组结构
app/          // 应用程序相关设置
├── name
├── version
├── debug_mode
└── auto_save

ui/           // 用户界面设置
├── theme
├── language
├── font_size
└── window_*

network/      // 网络相关设置
├── host
├── port
├── timeout
└── retry_count

system/       // 系统设置
├── config_version
├── last_update
└── migration_*
```

### 2. 错误处理

```cpp
class SafeSettingManager
{
public:
    SafeSettingManager() {
        settingManager = new SettingManager(this);
    }
    
    bool initialize(const QString& configPath) {
        if (!settingManager->init(configPath)) {
            qCritical() << "无法初始化配置文件:" << configPath;
            return false;
        }
        
        // 验证配置文件
        if (!settingManager->validateConfig()) {
            qWarning() << "配置文件验证失败，尝试恢复";
            return attemptRecovery(configPath);
        }
        
        return true;
    }
    
private:
    bool attemptRecovery(const QString& configPath) {
        // 尝试从备份恢复
        QString backupPath = configPath + ".backup";
        if (QFile::exists(backupPath)) {
            return settingManager->restoreConfig(backupPath);
        }
        
        // 创建默认配置
        createDefaultConfig();
        return true;
    }
    
    void createDefaultConfig() {
        settingManager->setValue("app/name", "MyApplication");
        settingManager->setValue("app/version", "1.0.0");
        settingManager->setValue("ui/theme", "light");
        settingManager->setValue("ui/language", "zh_CN");
        settingManager->setConfigVersion("1.0.0");
    }
};
```

### 3. 定期备份

```cpp
class AutoBackupManager : public QObject
{
    Q_OBJECT
    
public:
    AutoBackupManager(SettingManager* manager) : settingManager(manager) {
        // 设置定时备份（每天一次）
        backupTimer = new QTimer(this);
        connect(backupTimer, &QTimer::timeout, this, &AutoBackupManager::performBackup);
        backupTimer->start(24 * 60 * 60 * 1000); // 24小时
        
        // 应用启动时立即备份
        performBackup();
    }
    
private slots:
    void performBackup() {
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd");
        QString backupPath = QApplication::applicationDirPath() + 
                            QString("/backups/daily_backup_%1.ini").arg(timestamp);
        
        if (settingManager->backupConfig(backupPath)) {
            qDebug() << "自动备份完成:" << backupPath;
            cleanOldBackups();
        }
    }
    
    void cleanOldBackups() {
        // 只保留最近7天的备份
        QString backupDir = QApplication::applicationDirPath() + "/backups";
        QDir dir(backupDir);
        QFileInfoList backups = dir.entryInfoList(
            QStringList() << "daily_backup_*.ini", 
            QDir::Files, 
            QDir::Time | QDir::Reversed
        );
        
        // 删除超过7个的备份文件
        for (int i = 7; i < backups.size(); ++i) {
            QFile::remove(backups[i].absoluteFilePath());
        }
    }

private:
    SettingManager* settingManager;
    QTimer* backupTimer;
};
```

### 4. 配置验证

```cpp
class ConfigValidator
{
public:
    static bool validateAppConfig(SettingManager* manager) {
        QStringList requiredKeys = {
            "app/name",
            "app/version",
            "ui/theme",
            "network/host"
        };
        
        for (const QString& key : requiredKeys) {
            QVariant value = manager->getValue(key);
            if (!value.isValid() || value.toString().isEmpty()) {
                qWarning() << "缺少必需的配置项:" << key;
                return false;
            }
        }
        
        // 验证数值范围
        int port = manager->getValue("network/port", 0).toInt();
        if (port < 1 || port > 65535) {
            qWarning() << "无效的端口号:" << port;
            return false;
        }
        
        return true;
    }
    
    static void fixInvalidConfig(SettingManager* manager) {
        // 修复无效配置
        if (manager->getValue("app/name").toString().isEmpty()) {
            manager->setValue("app/name", "DefaultApp");
        }
        
        int port = manager->getValue("network/port", 0).toInt();
        if (port < 1 || port > 65535) {
            manager->setValue("network/port", 8080);
        }
        
        QString theme = manager->getValue("ui/theme").toString();
        if (theme != "light" && theme != "dark") {
            manager->setValue("ui/theme", "light");
        }
    }
};
```

---

## 🔍 故障排除

### 常见错误及解决方案

#### 1. 配置管理器初始化失败

**错误信息**: "配置管理器初始化失败"

**可能原因**:
- 配置文件路径不存在
- 没有写入权限
- 配置文件被其他进程占用

**解决方案**:
```cpp
bool initializeWithRetry(const QString& configPath) {
    // 确保目录存在
    QFileInfo fileInfo(configPath);
    QDir().mkpath(fileInfo.absolutePath());
    
    // 检查权限
    if (!fileInfo.dir().exists()) {
        qCritical() << "无法创建配置目录:" << fileInfo.absolutePath();
        return false;
    }
    
    // 多次尝试初始化
    for (int i = 0; i < 3; ++i) {
        if (settingManager->init(configPath)) {
            return true;
        }
        QThread::msleep(100); // 等待100毫秒后重试
    }
    
    return false;
}
```

#### 2. 配置文件验证失败

**错误信息**: "配置文件验证失败"

**可能原因**:
- 配置文件格式错误
- 文件损坏
- 编码问题

**解决方案**:
```cpp
bool repairConfigFile(const QString& configPath) {
    // 尝试从备份恢复
    QString backupPath = configPath + ".backup";
    if (QFile::exists(backupPath)) {
        if (QFile::copy(backupPath, configPath)) {
            qDebug() << "从备份恢复配置文件";
            return true;
        }
    }
    
    // 创建新的配置文件
    QFile::remove(configPath);
    SettingManager newManager;
    if (newManager.init(configPath)) {
        // 设置默认值
        newManager.setValue("app/name", "MyApp");
        newManager.setValue("app/version", "1.0.0");
        newManager.setConfigVersion("1.0.0");
        qDebug() << "创建新的配置文件";
        return true;
    }
    
    return false;
}
```

#### 3. 配置迁移失败

**错误信息**: "配置迁移失败"

**可能原因**:
- 旧配置文件不存在
- 迁移规则错误
- 目标路径无权限

**解决方案**:
```cpp
bool safeMigration(const QString& oldPath, const QString& newPath) {
    // 检查源文件
    if (!QFile::exists(oldPath)) {
        qWarning() << "源配置文件不存在:" << oldPath;
        return false;
    }
    
    // 验证源文件
    if (!settingManager->validateConfig(oldPath)) {
        qWarning() << "源配置文件无效，尝试修复";
        // 可以尝试修复或跳过无效配置
    }
    
    // 确保目标目录存在
    QFileInfo newFileInfo(newPath);
    QDir().mkpath(newFileInfo.absolutePath());
    
    // 备份源文件
    QString backupPath = oldPath + ".migration_backup";
    QFile::copy(oldPath, backupPath);
    
    // 执行迁移
    QMap<QString, QString> rules;
    // ... 设置迁移规则
    
    return settingManager->migrateConfig(oldPath, newPath, rules);
}
```

---

## ❓ 常见问题

### Q1: 如何处理多线程环境下的配置访问？

**A**: SettingManager 基于 QSettings，在多线程环境下需要注意线程安全：

```cpp
class ThreadSafeSettingManager
{
private:
    static QMutex mutex;
    SettingManager* settingManager;
    
public:
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) {
        QMutexLocker locker(&mutex);
        return settingManager->getValue(key, defaultValue);
    }
    
    void setValue(const QString& key, const QVariant& value) {
        QMutexLocker locker(&mutex);
        settingManager->setValue(key, value);
    }
};

QMutex ThreadSafeSettingManager::mutex;
```

### Q2: 如何实现配置的实时监控？

**A**: 可以通过底层 ConfigManager 的信号实现：

```cpp
class ConfigMonitor : public QObject
{
    Q_OBJECT
    
public:
    ConfigMonitor(SettingManager* manager) {
        // 连接 ConfigManager 的变更信号
        connect(manager->configManager, &ConfigManager::valueChanged,
                this, &ConfigMonitor::onConfigChanged);
    }
    
private slots:
    void onConfigChanged(const QString& group, const QString& key, const QVariant& value) {
        QString fullKey = group.isEmpty() ? key : QString("%1/%2").arg(group, key);
        qDebug() << "配置项变更:" << fullKey << "=" << value.toString();
        
        // 处理特定配置的变更
        if (fullKey == "ui/theme") {
            emit themeChanged(value.toString());
        }
    }
    
signals:
    void themeChanged(const QString& theme);
};
```

### Q3: 如何优化大量配置的读写性能？

**A**: 使用批量操作和缓存策略：

```cpp
class OptimizedSettingManager
{
private:
    SettingManager* settingManager;
    QMap<QString, QVariant> cache;
    QTimer* saveTimer;
    bool isDirty;
    
public:
    OptimizedSettingManager() {
        settingManager = new SettingManager(this);
        saveTimer = new QTimer(this);
        saveTimer->setSingleShot(true);
        saveTimer->setInterval(1000); // 1秒延迟保存
        connect(saveTimer, &QTimer::timeout, this, &OptimizedSettingManager::flushCache);
        isDirty = false;
    }
    
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) {
        if (cache.contains(key)) {
            return cache[key];
        }
        
        QVariant value = settingManager->getValue(key, defaultValue);
        cache[key] = value;
        return value;
    }
    
    void setValue(const QString& key, const QVariant& value) {
        cache[key] = value;
        isDirty = true;
        
        if (!saveTimer->isActive()) {
            saveTimer->start();
        }
    }
    
private slots:
    void flushCache() {
        if (isDirty) {
            for (auto it = cache.begin(); it != cache.end(); ++it) {
                settingManager->setValue(it.key(), it.value());
            }
            isDirty = false;
        }
    }
};
```

### Q4: 如何实现配置的加密存储？

**A**: 可以在 setValue 和 getValue 时进行加密/解密：

```cpp
class EncryptedSettingManager
{
private:
    SettingManager* settingManager;
    QStringList encryptedKeys;
    
public:
    EncryptedSettingManager() {
        settingManager = new SettingManager(this);
        encryptedKeys << "user/password" << "network/api_key";
    }
    
    void setValue(const QString& key, const QVariant& value) {
        if (encryptedKeys.contains(key)) {
            QString encrypted = encrypt(value.toString());
            settingManager->setValue(key, encrypted);
        } else {
            settingManager->setValue(key, value);
        }
    }
    
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) {
        QVariant value = settingManager->getValue(key, defaultValue);
        
        if (encryptedKeys.contains(key) && value.isValid()) {
            QString decrypted = decrypt(value.toString());
            return decrypted;
        }
        
        return value;
    }
    
private:
    QString encrypt(const QString& text) {
        // 实现加密逻辑（例如使用 QCryptographicHash）
        return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256).toHex();
    }
    
    QString decrypt(const QString& hash) {
        // 注意：这里只是示例，实际应该使用可逆的加密算法
        // 如 AES 等
        return hash; // 简化示例
    }
};
```

---

## 📞 技术支持

如果您在使用 SettingManager 过程中遇到问题，可以通过以下方式获得帮助：

1. **查看示例代码**: `examples/SettingManager_Example.cpp`
2. **检查 API 文档**: 参考本手册的 API 参考部分
3. **查看源代码**: `src/config/SettingManager.cpp` 和 `inc/config/SettingManager.h`

---

## 📋 更新日志

### v1.0.0 (2025-06-05)
- ✅ 首个稳定版本发布
- ✅ 实现基础配置管理功能
- ✅ 添加备份和恢复功能
- ✅ 实现配置验证机制
- ✅ 支持配置迁移功能
- ✅ 添加版本管理功能

---

**© 2025 SettingManager. 版权所有.**
