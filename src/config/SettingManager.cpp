//
// Created by jinxi on 25-6-5.
//

#include "../../inc/config/SettingManager.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QSettings>

SettingManager::SettingManager(QObject* parent) : QObject(parent)
{
  configManager = ConfigManager::getInstance();
}

SettingManager::~SettingManager()
{
  // 析构函数实现
}

bool SettingManager::init(const QString& configFilePath)
{
  if (configFilePath.isEmpty())
  {
    qDebug() << "请输入配置文件路径";
    return false;
  }
  if (!configManager->init(configFilePath))
  {
    qDebug() << "配置管理器初始化失败";
    return false;
  }else
  {
    qDebug() << "配置管理器初始化成功";
    return true;
  }
}

QVariant SettingManager::getValue(const QString &key, const QVariant &defaultValue) const
{
  // 从key中解析group和实际的key
  QStringList parts = key.split('/');
  if (parts.size() >= 2)
  {
    QString group = parts[0];
    QString actualKey = parts.mid(1).join('/');
    return configManager->getValue(group, actualKey, defaultValue);
  }
  else
  {
    // 如果没有group，使用空group
    return configManager->getValue("", key, defaultValue);
  }
}

void SettingManager::setValue(const QString& key, const QVariant& value)
{
  // 从key中解析group和实际的key
  QStringList parts = key.split('/');
  if (parts.size() >= 2)
  {
    QString group = parts[0];
    QString actualKey = parts.mid(1).join('/');
    configManager->setValue(group, actualKey, value);
  }
  else
  {
    // 如果没有group，使用空group
    configManager->setValue("", key, value);
  }
}

bool SettingManager::backupConfig(const QString& backupFilePath)
{
  if (backupFilePath.isEmpty())
  {
    qDebug() << "备份文件路径不能为空";
    return false;
  }

  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return false;
  }

  // 确保备份目录存在
  QFileInfo fileInfo(backupFilePath);
  QDir dir = fileInfo.absoluteDir();
  if (!dir.exists())
  {
    if (!dir.mkpath("."))
    {
      qDebug() << "无法创建备份目录:" << dir.absolutePath();
      return false;
    }
  }

  // 添加时间戳到备份文件名
  QString timestampedPath = backupFilePath;
  if (!backupFilePath.contains("_backup_"))
  {
    QFileInfo info(backupFilePath);
    QString baseName = info.completeBaseName();
    QString suffix = info.suffix();
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    timestampedPath = QString("%1/%2_backup_%3.%4")
                     .arg(info.absolutePath())
                     .arg(baseName)
                     .arg(timestamp)
                     .arg(suffix);
  }
  if (configManager->saveConfig(timestampedPath))
  {
    qDebug() << "配置已备份到:" << timestampedPath;
    return true;
  }
  else
  {
    qDebug() << "配置备份失败";
    return false;
  }
}

bool SettingManager::restoreConfig(const QString& backupFilePath)
{
  if (backupFilePath.isEmpty())
  {
    qDebug() << "备份文件路径不能为空";
    return false;
  }

  if (!QFile::exists(backupFilePath))
  {
    qDebug() << "备份文件不存在:" << backupFilePath;
    return false;
  }

  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return false;
  }

  // 验证备份文件
  if (!validateConfig(backupFilePath))
  {
    qDebug() << "备份文件验证失败，无法恢复";
    return false;
  }

  // 先备份当前配置
  QString currentBackupPath = backupFilePath + ".current_backup";
  backupConfig(currentBackupPath);
  if (configManager->loadConfig(backupFilePath))
  {
    qDebug() << "配置已从备份恢复:" << backupFilePath;
    return true;
  }
  else
  {
    qDebug() << "配置恢复失败";
    return false;
  }
}

bool SettingManager::validateConfig(const QString& configFilePath)
{  QString pathToValidate = configFilePath;
  if (pathToValidate.isEmpty())
  {
    // 如果没有提供路径，跳过路径验证（ConfigManager内部路径是私有的）
    qDebug() << "未提供配置文件路径，跳过路径验证";
    return true;
  }

  if (!QFile::exists(pathToValidate))
  {
    qDebug() << "配置文件不存在:" << pathToValidate;
    return false;
  }

  QFile file(pathToValidate);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    qDebug() << "无法打开配置文件进行验证:" << pathToValidate;
    return false;
  }

  // 基本文件完整性检查
  if (file.size() == 0)
  {
    qDebug() << "配置文件为空:" << pathToValidate;
    return false;
  }

  // 尝试解析配置文件格式
  QTextStream in(&file);
  QString content = in.readAll();
  file.close();

  // 检查是否包含基本的配置结构
  if (content.contains("[") && content.contains("]"))
  {
    // INI格式基本验证
    qDebug() << "配置文件验证通过 (INI格式):" << pathToValidate;
    return true;
  }
  else if (content.contains("{") && content.contains("}"))
  {
    // JSON格式验证
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError)
    {
      qDebug() << "配置文件验证通过 (JSON格式):" << pathToValidate;
      return true;
    }
    else
    {
      qDebug() << "JSON配置文件格式错误:" << error.errorString();
      return false;
    }
  }

  qDebug() << "配置文件格式不被识别:" << pathToValidate;
  return false;
}

bool SettingManager::migrateConfig(const QString& oldConfigPath, 
                                   const QString& newConfigPath,
                                   const QMap<QString, QString>& migrationRules)
{
  if (oldConfigPath.isEmpty() || newConfigPath.isEmpty())
  {
    qDebug() << "迁移路径不能为空";
    return false;
  }

  if (!QFile::exists(oldConfigPath))
  {
    qDebug() << "旧配置文件不存在:" << oldConfigPath;
    return false;
  }

  // 验证旧配置文件
  if (!validateConfig(oldConfigPath))
  {
    qDebug() << "旧配置文件验证失败";
    return false;
  }

  // 确保新配置目录存在
  QFileInfo newFileInfo(newConfigPath);
  QDir newDir = newFileInfo.absoluteDir();
  if (!newDir.exists())
  {
    if (!newDir.mkpath("."))
    {
      qDebug() << "无法创建新配置目录:" << newDir.absolutePath();
      return false;
    }
  }

  // 使用QSettings直接处理配置文件迁移
  try
  {
    QSettings oldSettings(oldConfigPath, QSettings::IniFormat);
    QSettings newSettings(newConfigPath, QSettings::IniFormat);
    
    if (oldSettings.status() != QSettings::NoError)
    {
      qDebug() << "无法读取旧配置文件:" << oldConfigPath;
      return false;
    }

    // 获取所有旧配置的组和键
    QStringList groups = oldSettings.childGroups();
    
    for (const QString& group : groups)
    {
      oldSettings.beginGroup(group);
      QStringList keys = oldSettings.childKeys();
      
      for (const QString& key : keys)
      {
        QString fullOldKey = group.isEmpty() ? key : QString("%1/%2").arg(group, key);
        QString fullNewKey = fullOldKey;
        
        // 应用迁移规则
        if (migrationRules.contains(fullOldKey))
        {
          fullNewKey = migrationRules[fullOldKey];
        }
        
        // 获取旧值
        QVariant value = oldSettings.value(key);
        
        // 解析新的组和键并设置到新配置
        QStringList newParts = fullNewKey.split('/');
        if (newParts.size() >= 2)
        {
          QString newGroup = newParts[0];
          QString newKey = newParts.mid(1).join('/');
          newSettings.beginGroup(newGroup);
          newSettings.setValue(newKey, value);
          newSettings.endGroup();
        }
        else
        {
          newSettings.setValue(fullNewKey, value);
        }
      }
      oldSettings.endGroup();
    }

    // 设置配置版本信息
    QString currentVersion = getConfigVersion();
    if (currentVersion.isEmpty())
    {
      currentVersion = "1.0.0";
    }
    newSettings.beginGroup("system");
    newSettings.setValue("config_version", currentVersion);
    newSettings.setValue("migration_date", QDateTime::currentDateTime().toString(Qt::ISODate));
    newSettings.setValue("migrated_from", oldConfigPath);
    newSettings.endGroup();
    
    // 同步保存新配置
    newSettings.sync();
    
    if (newSettings.status() == QSettings::NoError)
    {
      qDebug() << "配置迁移成功，从" << oldConfigPath << "到" << newConfigPath;
      
      // 备份旧配置
      QString backupPath = oldConfigPath + ".migration_backup";
      if (QFile::copy(oldConfigPath, backupPath))
      {
        qDebug() << "旧配置已备份到:" << backupPath;
      }
      return true;
    }
    else
    {
      qDebug() << "新配置文件保存失败";
      return false;
    }
  }
  catch (...)
  {
    qDebug() << "配置迁移过程中发生异常";
    return false;
  }
}

QString SettingManager::getConfigVersion() const
{
  return getValue("system/config_version", "1.0.0").toString();
}

void SettingManager::setConfigVersion(const QString& version)
{
  setValue("system/config_version", version);
  setValue("system/version_update_date", QDateTime::currentDateTime().toString(Qt::ISODate));
}

QStringList SettingManager::getAllGroups() const
{
  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return QStringList();
  }
  
  return configManager->getGroups();
}

QStringList SettingManager::getGroupKeys(const QString& group) const
{
  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return QStringList();
  }
  
  return configManager->getKeys(group);
}

bool SettingManager::hasValue(const QString& key) const
{
  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return false;
  }
  
  // 从key中解析group和实际的key
  QStringList parts = key.split('/');
  if (parts.size() >= 2)
  {
    QString group = parts[0];
    QString actualKey = parts.mid(1).join('/');
    return configManager->hasValue(group, actualKey);
  }
  else
  {
    // 如果没有group，使用空group
    return configManager->hasValue("", key);
  }
}

bool SettingManager::removeValue(const QString& key)
{
  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return false;
  }
  
  // 从key中解析group和实际的key
  QStringList parts = key.split('/');
  if (parts.size() >= 2)
  {
    QString group = parts[0];
    QString actualKey = parts.mid(1).join('/');
    return configManager->removeValue(group, actualKey);
  }
  else
  {
    // 如果没有group，使用空group
    return configManager->removeValue("", key);
  }
}

bool SettingManager::clearAllConfig()
{
  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return false;
  }
  
  return configManager->clearConfig();
}

bool SettingManager::exportToJson(const QString& jsonFilePath)
{
  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return false;
  }
  
  if (jsonFilePath.isEmpty())
  {
    qDebug() << "JSON文件路径不能为空";
    return false;
  }
  
  // 确保导出目录存在
  QFileInfo fileInfo(jsonFilePath);
  QDir dir = fileInfo.absoluteDir();
  if (!dir.exists())
  {
    if (!dir.mkpath("."))
    {
      qDebug() << "无法创建导出目录:" << dir.absolutePath();
      return false;
    }
  }
  
  if (configManager->exportToJson(jsonFilePath))
  {
    qDebug() << "配置已导出到JSON文件:" << jsonFilePath;
    return true;
  }
  else
  {
    qDebug() << "JSON导出失败";
    return false;
  }
}

bool SettingManager::importFromJson(const QString& jsonFilePath, bool merge)
{
  if (!configManager)
  {
    qDebug() << "配置管理器未初始化";
    return false;
  }
  
  if (jsonFilePath.isEmpty())
  {
    qDebug() << "JSON文件路径不能为空";
    return false;
  }
  
  if (!QFile::exists(jsonFilePath))
  {
    qDebug() << "JSON文件不存在:" << jsonFilePath;
    return false;
  }
  
  if (configManager->importFromJson(jsonFilePath, merge))
  {
    qDebug() << "配置已从JSON文件导入:" << jsonFilePath;
    return true;
  }
  else
  {
    qDebug() << "JSON导入失败";
    return false;
  }
}
