#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>


/**
 * @brief 用户权限枚举
 */
enum class UserPermission
{
  None = 0x00,
  Read = 0x01, // 读取权限
  Write = 0x02, // 写入权限
  Execute = 0x04, // 执行权限
  Delete = 0x08, // 删除权限
  UserManagement = 0x10, // 用户管理权限
  SystemConfig = 0x20, // 系统配置权限
  DataExport = 0x40, // 数据导出权限
  DataImport = 0x80, // 数据导入权限
  Admin = 0xFF // 管理员权限（所有权限）
};

Q_DECLARE_FLAGS(UserPermissions, UserPermission)
Q_DECLARE_OPERATORS_FOR_FLAGS(UserPermissions)

/**
 * @brief 用户角色枚举
 */
enum class UserRole
{
  Guest, // 访客
  Operator, // 操作员
  Engineer, // 工程师
  Manager, // 管理员
  Administrator // 超级管理员
};

/**
 * @brief 用户信息结构
 */
struct UserInfo
{
  QString username; // 用户名
  QString passwordHash; // 密码哈希
  QString displayName; // 显示名称
  QString email; // 邮箱
  UserRole role; // 用户角色
  UserPermissions permissions; // 用户权限
  QDateTime createTime; // 创建时间
  QDateTime lastLoginTime; // 最后登录时间
  bool isActive; // 是否激活
  QString description; // 描述信息

  UserInfo() : role(UserRole::Guest), permissions(UserPermission::None), isActive(true) {}

  // 转换为JSON对象
  QJsonObject toJson() const;

  // 从JSON对象创建
  static UserInfo fromJson(const QJsonObject& json);
};

/**
 * @brief 用户管理器类
 */
class UserManager : public QObject
{
  Q_OBJECT

  public:
  /**
   * @brief 获取用户管理器实例
   */
  static UserManager* getInstance();

  /**
   * @brief 释放用户管理器实例
   */
  static void releaseInstance();

  /**
   * @brief 初始化用户管理器
   * @return 是否初始化成功
   */
  bool init();

  /**
   * @brief 用户登录验证
   * @param username 用户名
   * @param password 密码
   * @return 是否登录成功
   */
  bool login(const QString& username, const QString& password);

  /**
   * @brief 用户登出
   */
  void logout();

  /**
   * @brief 获取当前登录用户
   * @return 当前用户信息，如果未登录返回空指针
   */
  const UserInfo* getCurrentUser() const;

  /**
   * @brief 检查当前用户是否有指定权限
   * @param permission 权限
   * @return 是否有权限
   */
  bool hasPermission(UserPermission permission) const;

  /**
   * @brief 检查当前用户是否有指定权限组合
   * @param permissions 权限组合
   * @return 是否有权限
   */
  bool hasPermissions(UserPermissions permissions) const;

  /**
   * @brief 添加用户
   * @param userInfo 用户信息
   * @param password 密码
   * @return 是否添加成功
   */
  bool addUser(const UserInfo& userInfo, const QString& password);

  /**
   * @brief 删除用户
   * @param username 用户名
   * @return 是否删除成功
   */
  bool removeUser(const QString& username);

  /**
   * @brief 更新用户信息
   * @param username 用户名
   * @param userInfo 新的用户信息
   * @return 是否更新成功
   */
  bool updateUser(const QString& username, const UserInfo& userInfo);

  /**
   * @brief 修改用户密码
   * @param username 用户名
   * @param newPassword 新密码
   * @return 是否修改成功
   */
  bool changePassword(const QString& username, const QString& newPassword);

  /**
   * @brief 获取用户信息
   * @param username 用户名
   * @return 用户信息，如果不存在返回空指针
   */
  const UserInfo* getUserInfo(const QString& username) const;

  /**
   * @brief 获取所有用户列表
   * @return 用户名列表
   */
  QStringList getAllUsers() const;

  /**
   * @brief 检查用户是否存在
   * @param username 用户名
   * @return 是否存在
   */
  bool userExists(const QString& username) const;

  /**
   * @brief 激活/禁用用户
   * @param username 用户名
   * @param active 是否激活
   * @return 是否操作成功
   */
  bool setUserActive(const QString& username, bool active);

  /**
   * @brief 获取角色对应的默认权限
   * @param role 用户角色
   * @return 权限组合
   */
  static UserPermissions getDefaultPermissions(UserRole role);

  /**
   * @brief 权限转换为字符串
   * @param permissions 权限组合
   * @return 权限描述字符串
   */
  static QString permissionsToString(UserPermissions permissions);

  /**
   * @brief 角色转换为字符串
   * @param role 用户角色
   * @return 角色描述字符串
   */
  static QString roleToString(UserRole role);

  /**
   * @brief 字符串转换为角色
   * @param roleStr 角色字符串
   * @return 用户角色
   */
  static UserRole stringToRole(const QString& roleStr);

  signals:
  /**
   * @brief 用户登录信号
   * @param username 用户名
   */
  void userLoggedIn(const QString& username);

  /**
   * @brief 用户登出信号
   * @param username 用户名
   */
  void userLoggedOut(const QString& username);

  /**
   * @brief 用户添加信号
   * @param username 用户名
   */
  void userAdded(const QString& username);

  /**
   * @brief 用户删除信号
   * @param username 用户名
   */
  void userRemoved(const QString& username);

  /**
   * @brief 用户更新信号
   * @param username 用户名
   */
  void userUpdated(const QString& username);

  private:
  explicit UserManager(QObject* parent = nullptr);
  ~UserManager();
  UserManager(const UserManager&) = delete;
  UserManager& operator=(const UserManager&) = delete;

  static UserManager* instance;
  static QMutex mutex;

  QMap<QString, UserInfo> users; // 用户信息映射
  QString currentUsername; // 当前登录用户名
  bool initialized; // 是否已初始化

  /**
   * @brief 生成密码哈希
   * @param password 原始密码
   * @param salt 盐值
   * @return 密码哈希
   */
  QString generatePasswordHash(const QString& password, const QString& salt = QString()) const;

  /**
   * @brief 验证密码
   * @param password 原始密码
   * @param hash 存储的哈希值
   * @return 是否匹配
   */
  bool verifyPassword(const QString& password, const QString& hash) const;

  /**
   * @brief 生成随机盐值
   * @return 盐值字符串
   */
  QString generateSalt() const;

  /**
   * @brief 保存用户数据
   * @return 是否保存成功
   */
  bool saveUsers();

  /**
   * @brief 加载用户数据
   * @return 是否加载成功
   */
  bool loadUsers();

  /**
   * @brief 创建默认管理员账户
   */
  void createDefaultAdmin();
};

#endif // USER_MANAGER_H
