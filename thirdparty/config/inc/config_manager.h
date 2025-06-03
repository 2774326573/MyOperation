#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QSettings>
#include <QMutex>
#include <QStringList>

/**
 * @brief 配置管理器类，用于管理应用程序的各种配置
 *
 * 采用单例模式，支持多种配置类型，支持配置的保存和加载，
 * 提供灵活的API接口，支持配置变更通知
 */
class ConfigManager : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief 获取配置管理器实例
   * @return 配置管理器指针
   */
  static ConfigManager* getInstance();

  /**
   * @brief 释放配置管理器实例
   */
  static void releaseInstance();

  /**
   * @brief 初始化配置管理器
   * @param configFilePath 配置文件路径，默认为应用程序目录下的config.ini
   * @return 是否初始化成功
   */
  bool init(const QString& configFilePath = QString());

  /**
   * @brief 设置配置值
   * @param group 配置组名
   * @param key 配置键名
   * @param value 配置值
   * @return 是否设置成功
   */
  bool setValue(const QString& group, const QString& key, const QVariant& value);

  /**
   * @brief 获取配置值
   * @param group 配置组名
   * @param key 配置键名
   * @param defaultValue 默认值，如果配置不存在则返回此值
   * @return 配置值
   */
  QVariant getValue(const QString& group, const QString& key, const QVariant& defaultValue = QVariant());

  /**
   * @brief 检查配置是否存在
   * @param group 配置组名
   * @param key 配置键名
   * @return 配置是否存在
   */
  bool hasValue(const QString& group, const QString& key);

  /**
   * @brief 移除配置
   * @param group 配置组名
   * @param key 配置键名
   * @return 是否移除成功
   */
  bool removeValue(const QString& group, const QString& key);

  /**
   * @brief 移除配置组
   * @param group 配置组名
   * @return 是否移除成功
   */
  bool removeGroup(const QString& group);

  /**
   * @brief 获取所有配置组
   * @return 配置组列表
   */
  QStringList getGroups();

  /**
   * @brief 获取组内所有配置键
   * @param group 配置组名
   * @return 配置键列表
   */
  QStringList getKeys(const QString& group);

  /**
   * @brief 保存配置
   * @param filePath 配置文件路径，留空则使用初始化时的路径
   * @return 是否保存成功
   */
  bool saveConfig(const QString& filePath = QString());

  /**
   * @brief 加载配置
   * @param filePath 配置文件路径，留空则使用初始化时的路径
   * @param merge 是否合并配置，true为合并，false为覆盖
   * @return 是否加载成功
   */
  bool loadConfig(const QString& filePath = QString(), bool merge = false);

  /**
   * @brief 清除所有配置
   * @return 是否清除成功
   */
  bool clearConfig();

  /**
   * @brief 导出配置到JSON
   * @param filePath JSON文件路径
   * @return 是否导出成功
   */
  bool exportToJson(const QString& filePath);

  /**
   * @brief 从JSON导入配置
   * @param filePath JSON文件路径
   * @param merge 是否合并配置，true为合并，false为覆盖
   * @return 是否导入成功
   */
  bool importFromJson(const QString& filePath, bool merge = false);

signals:
  /**
   * @brief 配置值变更信号
   * @param group 配置组名
   * @param key 配置键名
   * @param value 新配置值
   */
  void valueChanged(const QString& group, const QString& key, const QVariant& value);

  /**
   * @brief 配置组移除信号
   * @param group 配置组名
   */
  void groupRemoved(const QString& group);

  /**
   * @brief 配置移除信号
   * @param group 配置组名
   * @param key 配置键名
   */
  void valueRemoved(const QString& group, const QString& key);

  /**
   * @brief 配置变更前信号
   * @param group 配置组名
   * @param key 配置键名
   * @param oldValue 旧配置值
   * @param newValue 新配置值
   */
  void valueChanging(const QString& group, const QString& key, const QVariant& oldValue, const QVariant& newValue);
private:
  // 单例相关
  explicit ConfigManager(QObject* parent = nullptr);
  ~ConfigManager();
  ConfigManager(const ConfigManager&) = delete;
  ConfigManager& operator=(const ConfigManager&) = delete;
  static ConfigManager* instance;
  static QMutex mutex;

  // 配置存储和文件路径
  QSettings* settings;
  QString configPath;
  bool initialized;

  // 辅助方法
  QString getSettingsKey(const QString& group, const QString& key);
  void emitValueChanged(const QString& group, const QString& key, const QVariant& value);
};

#endif // CONFIG_MANAGER_H
