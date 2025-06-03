#include "../inc/user_manager.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include "../inc/config_manager.h"


// 初始化静态成员
UserManager* UserManager::instance = nullptr;
QMutex UserManager::mutex;

// UserInfo 方法实现
QJsonObject UserInfo::toJson() const
{
  QJsonObject obj;
  obj["username"] = username;
  obj["passwordHash"] = passwordHash;
  obj["displayName"] = displayName;
  obj["email"] = email;
  obj["role"] = static_cast<int>(role);
  obj["permissions"] = static_cast<int>(permissions);
  obj["createTime"] = createTime.toString(Qt::ISODate);
  obj["lastLoginTime"] = lastLoginTime.toString(Qt::ISODate);
  obj["isActive"] = isActive;
  obj["description"] = description;
  return obj;
}

UserInfo UserInfo::fromJson(const QJsonObject& json)
{
  UserInfo info;
  info.username = json["username"].toString();
  info.passwordHash = json["passwordHash"].toString();
  info.displayName = json["displayName"].toString();
  info.email = json["email"].toString();
  info.role = static_cast<UserRole>(json["role"].toInt());
  info.permissions = static_cast<UserPermissions>(json["permissions"].toInt());
  info.createTime = QDateTime::fromString(json["createTime"].toString(), Qt::ISODate);
  info.lastLoginTime = QDateTime::fromString(json["lastLoginTime"].toString(), Qt::ISODate);
  info.isActive = json["isActive"].toBool();
  info.description = json["description"].toString();
  return info;
}

// UserManager 方法实现
UserManager* UserManager::getInstance()
{
  if (instance == nullptr)
  {
    mutex.lock();
    if (instance == nullptr)
    {
      instance = new UserManager();
    }
    mutex.unlock();
  }
  return instance;
}

void UserManager::releaseInstance()
{
  mutex.lock();
  if (instance != nullptr)
  {
    delete instance;
    instance = nullptr;
  }
  mutex.unlock();
}

UserManager::UserManager(QObject* parent) : QObject(parent) { initialized = false; }

UserManager::~UserManager()
{
  if (initialized)
  {
    saveUsers();
  }
}

bool UserManager::init()
{
  if (initialized)
  {
    return true;
  }

  // 确保配置管理器已初始化
  ConfigManager* configMgr = ConfigManager::getInstance();
  if (!configMgr)
  {
    qWarning() << "ConfigManager not available";
    return false;
  }

  // 加载用户数据
  if (!loadUsers())
  {
    qWarning() << "Failed to load users";
    // 如果加载失败，创建默认管理员账户
    createDefaultAdmin();
  }

  initialized = true;
  return true;
}

bool UserManager::login(const QString& username, const QString& password)
{
  if (!initialized)
  {
    qWarning() << "UserManager not initialized";
    return false;
  }

  auto it = users.find(username);
  if (it == users.end())
  {
    qWarning() << "User not found:" << username;
    return false;
  }

  UserInfo& user = it.value();
  if (!user.isActive)
  {
    qWarning() << "User is not active:" << username;
    return false;
  }

  if (!verifyPassword(password, user.passwordHash))
  {
    qWarning() << "Invalid password for user:" << username;
    return false;
  }

  // 更新最后登录时间
  user.lastLoginTime = QDateTime::currentDateTime();
  currentUsername = username;

  // 保存用户数据
  saveUsers();

  emit userLoggedIn(username);
  return true;
}

void UserManager::logout()
{
  if (!currentUsername.isEmpty())
  {
    QString username = currentUsername;
    currentUsername.clear();
    emit userLoggedOut(username);
  }
}

const UserInfo* UserManager::getCurrentUser() const
{
  if (currentUsername.isEmpty())
  {
    return nullptr;
  }

  auto it = users.find(currentUsername);
  if (it != users.end())
  {
    return &it.value();
  }

  return nullptr;
}

bool UserManager::hasPermission(UserPermission permission) const
{
  const UserInfo* user = getCurrentUser();
  if (!user)
  {
    return false;
  }

  return user->permissions.testFlag(permission);
}

bool UserManager::hasPermissions(UserPermissions permissions) const
{
  const UserInfo* user = getCurrentUser();
  if (!user)
  {
    return false;
  }

  return (user->permissions & permissions) == permissions;
}

bool UserManager::addUser(const UserInfo& userInfo, const QString& password)
{
  if (!initialized)
  {
    qWarning() << "UserManager not initialized";
    return false;
  }

  // 检查当前用户是否有用户管理权限
  if (!hasPermission(UserPermission::UserManagement))
  {
    qWarning() << "Current user does not have user management permission";
    return false;
  }

  if (users.contains(userInfo.username))
  {
    qWarning() << "User already exists:" << userInfo.username;
    return false;
  }

  UserInfo newUser = userInfo;
  newUser.passwordHash = generatePasswordHash(password);
  newUser.createTime = QDateTime::currentDateTime();

  users[userInfo.username] = newUser;

  if (saveUsers())
  {
    emit userAdded(userInfo.username);
    return true;
  }

  return false;
}

bool UserManager::removeUser(const QString& username)
{
  if (!initialized)
  {
    qWarning() << "UserManager not initialized";
    return false;
  }

  // 检查当前用户是否有用户管理权限
  if (!hasPermission(UserPermission::UserManagement))
  {
    qWarning() << "Current user does not have user management permission";
    return false;
  }

  // 不能删除当前登录用户
  if (username == currentUsername)
  {
    qWarning() << "Cannot delete current logged in user";
    return false;
  }

  if (!users.contains(username))
  {
    qWarning() << "User not found:" << username;
    return false;
  }

  users.remove(username);

  if (saveUsers())
  {
    emit userRemoved(username);
    return true;
  }

  return false;
}

bool UserManager::updateUser(const QString& username, const UserInfo& userInfo)
{
  if (!initialized)
  {
    qWarning() << "UserManager not initialized";
    return false;
  }

  // 检查当前用户是否有用户管理权限
  if (!hasPermission(UserPermission::UserManagement))
  {
    qWarning() << "Current user does not have user management permission";
    return false;
  }

  if (!users.contains(username))
  {
    qWarning() << "User not found:" << username;
    return false;
  }

  UserInfo updatedUser = userInfo;
  // 保留原有的密码哈希和创建时间
  updatedUser.passwordHash = users[username].passwordHash;
  updatedUser.createTime = users[username].createTime;

  users[username] = updatedUser;

  if (saveUsers())
  {
    emit userUpdated(username);
    return true;
  }

  return false;
}

bool UserManager::changePassword(const QString& username, const QString& newPassword)
{
  if (!initialized)
  {
    qWarning() << "UserManager not initialized";
    return false;
  }

  // 只有管理员或用户本人可以修改密码
  if (!hasPermission(UserPermission::UserManagement) && username != currentUsername)
  {
    qWarning() << "Permission denied to change password for user:" << username;
    return false;
  }

  if (!users.contains(username))
  {
    qWarning() << "User not found:" << username;
    return false;
  }

  users[username].passwordHash = generatePasswordHash(newPassword);

  return saveUsers();
}

const UserInfo* UserManager::getUserInfo(const QString& username) const
{
  auto it = users.find(username);
  if (it != users.end())
  {
    return &it.value();
  }
  return nullptr;
}

QStringList UserManager::getAllUsers() const { return users.keys(); }

bool UserManager::userExists(const QString& username) const { return users.contains(username); }

bool UserManager::setUserActive(const QString& username, bool active)
{
  if (!initialized)
  {
    qWarning() << "UserManager not initialized";
    return false;
  }

  // 检查当前用户是否有用户管理权限
  if (!hasPermission(UserPermission::UserManagement))
  {
    qWarning() << "Current user does not have user management permission";
    return false;
  }

  if (!users.contains(username))
  {
    qWarning() << "User not found:" << username;
    return false;
  }

  users[username].isActive = active;

  return saveUsers();
}

UserPermissions UserManager::getDefaultPermissions(UserRole role)
{
  switch (role)
  {
  case UserRole::Guest:
    return UserPermission::Read;
  case UserRole::Operator:
    return UserPermission::Read | UserPermission::Write;
  case UserRole::Engineer:
    return UserPermission::Read | UserPermission::Write | UserPermission::Execute | UserPermission::DataExport;
  case UserRole::Manager:
    return UserPermission::Read | UserPermission::Write | UserPermission::Execute | UserPermission::Delete |
        UserPermission::DataExport | UserPermission::DataImport;
  case UserRole::Administrator:
    return UserPermission::Admin;
  default:
    return UserPermission::None;
  }
}

QString UserManager::permissionsToString(UserPermissions permissions)
{
  QStringList permList;

  if (permissions.testFlag(UserPermission::Admin))
  {
    return QObject::tr("管理员权限");
  }

  if (permissions.testFlag(UserPermission::Read))
  {
    permList << QObject::tr("读取");
  }
  if (permissions.testFlag(UserPermission::Write))
  {
    permList << QObject::tr("写入");
  }
  if (permissions.testFlag(UserPermission::Execute))
  {
    permList << QObject::tr("执行");
  }
  if (permissions.testFlag(UserPermission::Delete))
  {
    permList << QObject::tr("删除");
  }
  if (permissions.testFlag(UserPermission::UserManagement))
  {
    permList << QObject::tr("用户管理");
  }
  if (permissions.testFlag(UserPermission::SystemConfig))
  {
    permList << QObject::tr("系统配置");
  }
  if (permissions.testFlag(UserPermission::DataExport))
  {
    permList << QObject::tr("数据导出");
  }
  if (permissions.testFlag(UserPermission::DataImport))
  {
    permList << QObject::tr("数据导入");
  }

  return permList.join(", ");
}

QString UserManager::roleToString(UserRole role)
{
  switch (role)
  {
  case UserRole::Guest:
    return QObject::tr("访客");
  case UserRole::Operator:
    return QObject::tr("操作员");
  case UserRole::Engineer:
    return QObject::tr("工程师");
  case UserRole::Manager:
    return QObject::tr("管理员");
  case UserRole::Administrator:
    return QObject::tr("超级管理员");
  default:
    return QObject::tr("未知");
  }
}

UserRole UserManager::stringToRole(const QString& roleStr)
{
  if (roleStr == QObject::tr("访客"))
    return UserRole::Guest;
  if (roleStr == QObject::tr("操作员"))
    return UserRole::Operator;
  if (roleStr == QObject::tr("工程师"))
    return UserRole::Engineer;
  if (roleStr == QObject::tr("管理员"))
    return UserRole::Manager;
  if (roleStr == QObject::tr("超级管理员"))
    return UserRole::Administrator;
  return UserRole::Guest;
}

QString UserManager::generatePasswordHash(const QString& password, const QString& salt) const
{
  QString saltToUse = salt.isEmpty() ? generateSalt() : salt;
  QString combined = password + saltToUse;
  QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
  return saltToUse + ":" + hash.toHex();
}

bool UserManager::verifyPassword(const QString& password, const QString& hash) const
{
  QStringList parts = hash.split(":");
  if (parts.size() != 2)
  {
    return false;
  }

  QString salt = parts[0];
  QString expectedHash = generatePasswordHash(password, salt);
  return expectedHash == hash;
}

QString UserManager::generateSalt() const
{
  QByteArray salt;
  for (int i = 0; i < 16; ++i)
  {
    salt.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
  }
  return salt.toHex();
}

bool UserManager::saveUsers()
{
  ConfigManager* configMgr = ConfigManager::getInstance();
  if (!configMgr)
  {
    return false;
  }

  QJsonArray usersArray;
  for (auto it = users.begin(); it != users.end(); ++it)
  {
    usersArray.append(it.value().toJson());
  }

  QJsonDocument doc(usersArray);
  QString usersData = doc.toJson(QJsonDocument::Compact);

  return configMgr->setValue("Users", "data", usersData);
}

bool UserManager::loadUsers()
{
  ConfigManager* configMgr = ConfigManager::getInstance();
  if (!configMgr)
  {
    return false;
  }

  QString usersData = configMgr->getValue("Users", "data").toString();
  if (usersData.isEmpty())
  {
    return false;
  }

  QJsonDocument doc = QJsonDocument::fromJson(usersData.toUtf8());
  if (!doc.isArray())
  {
    return false;
  }

  QJsonArray usersArray = doc.array();
  users.clear();

  for (const QJsonValue& value : usersArray)
  {
    if (value.isObject())
    {
      UserInfo user = UserInfo::fromJson(value.toObject());
      users[user.username] = user;
    }
  }

  return true;
}

void UserManager::createDefaultAdmin()
{
  UserInfo admin;
  admin.username = "admin";
  admin.displayName = QObject::tr("系统管理员");
  admin.email = "admin@system.local";
  admin.role = UserRole::Administrator;
  admin.permissions = UserPermission::Admin;
  admin.createTime = QDateTime::currentDateTime();
  admin.isActive = true;
  admin.description = QObject::tr("默认系统管理员账户");

  admin.passwordHash = generatePasswordHash("admin123");

  users["admin"] = admin;
  saveUsers();

  qDebug() << "Created default admin user: admin/admin123";
}
