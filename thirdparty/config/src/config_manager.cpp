#include "../inc/config_manager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QDateTime>
#include <QDebug>

// 初始化静态成员
ConfigManager* ConfigManager::instance = nullptr;
QMutex ConfigManager::mutex;

ConfigManager* ConfigManager::getInstance()
{
    if (instance == nullptr) {
        mutex.lock();
        if (instance == nullptr) {
            instance = new ConfigManager();
        }
        mutex.unlock();
    }
    return instance;
}

void ConfigManager::releaseInstance()
{
    mutex.lock();
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
    mutex.unlock();
}

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{
    settings = nullptr;
    initialized = false;
}

ConfigManager::~ConfigManager()
{
    if (settings) {
        delete settings;
        settings = nullptr;
    }
}

bool ConfigManager::init(const QString& configFilePath)
{
    // 如果已经初始化，先清理现有资源
    if (settings) {
        delete settings;
        settings = nullptr;
    }

    // 确定配置文件路径
    if (!configFilePath.isEmpty()) {
        configPath = configFilePath;
    } else {
        configPath = QApplication::applicationDirPath() + "/config/config.ini";
    }

    // 确保包含文件的目录存在
    QFileInfo fileInfo(configPath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 创建QSettings对象
    settings = new QSettings(configPath, QSettings::IniFormat);
    if (!settings->isWritable()) {
        qWarning() << "Config file is not writable:" << configPath;
        delete settings;
        settings = nullptr;
        return false;
    }

    initialized = true;
    return true;
}

bool ConfigManager::setValue(const QString& group, const QString& key, const QVariant& value)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return false;
    }

    // 在设置值前发射变更前信号
    QVariant oldValue = getValue(group, key);
    emit valueChanging(group, key, oldValue, value);

    // 设置值
    settings->beginGroup(group);
    settings->setValue(key, value);
    settings->endGroup();
    settings->sync();

    // 发射变更信号
    emitValueChanged(group, key, value);
    return true;
}

QVariant ConfigManager::getValue(const QString& group, const QString& key, const QVariant& defaultValue)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return defaultValue;
    }

    settings->beginGroup(group);
    QVariant value = settings->value(key, defaultValue);
    settings->endGroup();
    return value;
}

bool ConfigManager::hasValue(const QString& group, const QString& key)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return false;
    }

    settings->beginGroup(group);
    bool has = settings->contains(key);
    settings->endGroup();
    return has;
}

bool ConfigManager::removeValue(const QString& group, const QString& key)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return false;
    }

    settings->beginGroup(group);
    bool exists = settings->contains(key);
    if (exists) {
        settings->remove(key);
    }
    settings->endGroup();
    settings->sync();

    if (exists) {
        emit valueRemoved(group, key);
    }
    return exists;
}

bool ConfigManager::removeGroup(const QString& group)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return false;
    }

    settings->beginGroup(group);
    bool isEmpty = settings->childKeys().isEmpty();
    settings->endGroup();

    if (!isEmpty) {
        settings->remove(group);
        settings->sync();
        emit groupRemoved(group);
        return true;
    }
    return false;
}

QStringList ConfigManager::getGroups()
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return QStringList();
    }

    return settings->childGroups();
}

QStringList ConfigManager::getKeys(const QString& group)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return QStringList();
    }

    settings->beginGroup(group);
    QStringList keys = settings->childKeys();
    settings->endGroup();
    return keys;
}

bool ConfigManager::saveConfig(const QString& filePath)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return false;
    }

    if (!filePath.isEmpty() && filePath != configPath) {
        QSettings tempSettings(filePath, QSettings::IniFormat);
        
        // 获取所有组
        QStringList groups = settings->childGroups();
        for (const QString& group : groups) {
            settings->beginGroup(group);
            QStringList keys = settings->childKeys();
            
            // 拷贝每个组的所有键值
            for (const QString& key : keys) {
                tempSettings.beginGroup(group);
                tempSettings.setValue(key, settings->value(key));
                tempSettings.endGroup();
            }
            
            settings->endGroup();
        }
        
        tempSettings.sync();
        return tempSettings.status() == QSettings::NoError;
    } else {
        settings->sync();
        return settings->status() == QSettings::NoError;
    }
}

bool ConfigManager::loadConfig(const QString& filePath, bool merge)
{
    QString pathToLoad = filePath.isEmpty() ? configPath : filePath;
    
    if (!QFile::exists(pathToLoad)) {
        qWarning() << "Config file does not exist:" << pathToLoad;
        return false;
    }
    
    if (!merge && initialized && settings) {
        // 如果不合并，先清除现有配置
        clearConfig();
    }
    
    // 加载配置
    QSettings loadSettings(pathToLoad, QSettings::IniFormat);
    QStringList groups = loadSettings.childGroups();
    
    for (const QString& group : groups) {
        loadSettings.beginGroup(group);
        QStringList keys = loadSettings.childKeys();
        
        for (const QString& key : keys) {
            setValue(group, key, loadSettings.value(key));
        }
        
        loadSettings.endGroup();
    }
    
    return true;
}

bool ConfigManager::clearConfig()
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return false;
    }
    
    QStringList groups = settings->childGroups();
    for (const QString& group : groups) {
        settings->remove(group);
        emit groupRemoved(group);
    }
    
    settings->sync();
    return settings->status() == QSettings::NoError;
}

bool ConfigManager::exportToJson(const QString& filePath)
{
    if (!initialized || !settings) {
        qWarning() << "ConfigManager not initialized.";
        return false;
    }
    
    QJsonObject rootObject;
    QStringList groups = settings->childGroups();
    
    for (const QString& group : groups) {
        QJsonObject groupObject;
        settings->beginGroup(group);
        QStringList keys = settings->childKeys();
        
        for (const QString& key : keys) {
            QVariant value = settings->value(key);
            
            // 转换QVariant到JSON支持的类型
            switch (value.type()) {
                case QVariant::Bool:
                    groupObject[key] = value.toBool();
                    break;
                case QVariant::Int:
                case QVariant::UInt:
                case QVariant::LongLong:
                case QVariant::ULongLong:
                    groupObject[key] = value.toInt();
                    break;
                case QVariant::Double:
                    groupObject[key] = value.toDouble();
                    break;
                case QVariant::List:
                case QVariant::StringList: {
                    QJsonArray array;
                    QStringList list = value.toStringList();
                    for (const QString& item : list) {
                        array.append(item);
                    }
                    groupObject[key] = array;
                    break;
                }
                default:
                    groupObject[key] = value.toString();
            }
        }
        
        settings->endGroup();
        rootObject[group] = groupObject;
    }
    
    QJsonDocument doc(rootObject);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    return true;
}

bool ConfigManager::importFromJson(const QString& filePath, bool merge)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << filePath;
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format in file:" << filePath;
        return false;
    }
    
    if (!merge) {
        clearConfig();
    }
    
    QJsonObject rootObject = doc.object();
    for (auto it = rootObject.constBegin(); it != rootObject.constEnd(); ++it) {
        QString group = it.key();
        
        if (it.value().isObject()) {
            QJsonObject groupObject = it.value().toObject();
            
            for (auto keyIt = groupObject.constBegin(); keyIt != groupObject.constEnd(); ++keyIt) {
                QString key = keyIt.key();
                QJsonValue jsonValue = keyIt.value();
                QVariant value;
                
                // 将JSON值转换为QVariant
                if (jsonValue.isString()) {
                    value = jsonValue.toString();
                } else if (jsonValue.isBool()) {
                    value = jsonValue.toBool();
                } else if (jsonValue.isDouble()) {
                    value = jsonValue.toDouble();
                } else if (jsonValue.isArray()) {
                    QJsonArray array = jsonValue.toArray();
                    QStringList list;
                    for (const QJsonValue& item : array) {
                        list.append(item.toString());
                    }
                    value = list;
                }
                
                setValue(group, key, value);
            }
        }
    }
    
    return true;
}

QString ConfigManager::getSettingsKey(const QString& group, const QString& key)
{
    return group + "/" + key;
}

void ConfigManager::emitValueChanged(const QString& group, const QString& key, const QVariant& value)
{
    emit valueChanged(group, key, value);
}
