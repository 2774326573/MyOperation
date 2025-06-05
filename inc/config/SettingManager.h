//
// Created by jinxi on 25-6-5.
//

#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include "../thirdparty/config/inc/config_manager.h" // 引入Modbus管理器头文件

class SettingManager : public QObject
{
  Q_OBJECT
public:
  explicit SettingManager(QObject *parent = nullptr);
  ~SettingManager() override;

  /**
   * @param configFilePath
   * @return
   */
  bool init(const QString &configFilePath);

  /**
   * @brief 获取配置值
   * @param key 配置键名，格式为 "group/key"
   * @param defaultValue 默认值，如果配置不存在则返回此值
   * @return 配置值
   */
  QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
  /**
   * @brief 设置配置值
   * @param key 配置键名，格式为 "group/key"
   * @param value 配置值
   */
  void setValue(const QString& key, const QVariant& value);

  /**
   * @brief 备份当前配置到指定文件
   * @param backupFilePath 备份文件路径
   * @return 备份是否成功
   */
  bool backupConfig(const QString& backupFilePath);

  /**
   * @brief 从备份文件恢复配置
   * @param backupFilePath 备份文件路径
   * @return 恢复是否成功
   */
  bool restoreConfig(const QString& backupFilePath);

  /**
   * @brief 验证配置文件的完整性和有效性
   * @param configFilePath 要验证的配置文件路径（为空则验证当前配置）
   * @return 验证是否通过
   */
  bool validateConfig(const QString& configFilePath = QString());

  /**
   * @brief 迁移旧版本配置到新版本
   * @param oldConfigPath 旧配置文件路径
   * @param newConfigPath 新配置文件路径
   * @param migrationRules 迁移规则映射（旧key->新key）
   * @return 迁移是否成功
   */
  bool migrateConfig(const QString& oldConfigPath, 
                     const QString& newConfigPath,
                     const QMap<QString, QString>& migrationRules = QMap<QString, QString>());

  /**
   * @brief 获取配置版本号
   * @return 配置版本号
   */
  QString getConfigVersion() const;

  /**
   * @brief 设置配置版本号
   * @param version 版本号
   */
  void setConfigVersion(const QString& version);

  /**
   * @brief 获取所有配置组
   * @return 配置组列表
   */
  QStringList getAllGroups() const;

  /**
   * @brief 获取指定组中的所有键
   * @param group 配置组名
   * @return 配置键列表
   */
  QStringList getGroupKeys(const QString& group) const;

  /**
   * @brief 检查配置是否存在
   * @param key 配置键名，格式为 "group/key"
   * @return 配置是否存在
   */
  bool hasValue(const QString& key) const;

  /**
   * @brief 移除配置项
   * @param key 配置键名，格式为 "group/key"
   * @return 是否移除成功
   */
  bool removeValue(const QString& key);

  /**
   * @brief 清除所有配置
   * @return 是否清除成功
   */
  bool clearAllConfig();

  /**
   * @brief 导出配置到JSON文件
   * @param jsonFilePath JSON文件路径
   * @return 是否导出成功
   */
  bool exportToJson(const QString& jsonFilePath);

  /**
   * @brief 从JSON文件导入配置
   * @param jsonFilePath JSON文件路径
   * @param merge 是否合并配置，true为合并，false为覆盖
   * @return 是否导入成功
   */
  bool importFromJson(const QString& jsonFilePath, bool merge = false);

  ConfigManager *configManager;

};

#endif //SETTINGMANAGER_H
