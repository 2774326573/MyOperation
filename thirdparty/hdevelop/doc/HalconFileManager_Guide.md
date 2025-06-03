# Halcon文件管理器指南 | HalconFileManager Guide

## 📚 概述 | Overview

**HalconFileManager** 是一个专门为Halcon视觉处理库设计的文件管理工具类，帮助开发者正确管理和组织各种Halcon专用文件格式。

**HalconFileManager** is a specialized file management utility class designed for the Halcon vision processing library, helping developers properly manage and organize various Halcon-specific file formats.

---

## 🎯 主要功能 | Key Features

### 1. 文件扩展名管理 | File Extension Management
- 🗂️ **标准化文件命名** - 统一的Halcon文件扩展名规范
- 🔄 **自动文件名生成** - 带时间戳的唯一文件名
- ✅ **文件类型验证** - 验证文件是否为有效的Halcon格式

### 2. 目录结构管理 | Directory Structure Management
- 📁 **标准目录创建** - 创建推荐的Halcon项目目录结构
- 🏗️ **项目目录初始化** - 一键创建完整的视觉处理项目框架

### 3. 文件统计分析 | File Statistics & Analysis
- 📊 **目录文件统计** - 分析目录中的Halcon文件分布
- 🔍 **文件类型搜索** - 按文件类型快速查找文件
- 📈 **使用情况分析** - 文件大小、修改时间等统计信息

---

## 🗂️ 支持的文件类型 | Supported File Types

| 扩展名 | 文件类型 | 用途 | 使用场景 |
|--------|----------|------|----------|
| `.hreg` | 区域文件 | 存储Halcon区域对象 | ROI定义、掩膜区域 |
| `.himage` | 图像文件 | 存储Halcon格式图像 | 高精度图像、多通道图像 |
| `.hxld` | XLD轮廓文件 | 存储轮廓数据 | 边缘检测结果、几何测量 |
| `.hobj` | 通用对象文件 | 存储任意Halcon对象 | 通用对象存储 |
| `.shm` | 形状模型 | Shape Model文件 | 模板匹配、目标定位 |
| `.ncm` | NCC模型 | Normalized Cross Correlation模型 | 灰度匹配 |
| `.tup` | Tuple参数 | 参数和配置数据 | 算法参数、标定数据 |
| `.hdict` | 字典文件 | Halcon字典数据 | 键值对数据存储 |
| `.calb` | 标定数据 | 相机标定信息 | 相机内外参数 |
| `.hmea` | 测量模板 | 测量算法模板 | 精密测量应用 |

---

## 💻 使用示例 | Usage Examples

### 基本使用 | Basic Usage

```cpp
#include "thirdparty/hdevelop/include/HalconFileManager.h"

// 🏗️ 创建文件管理器实例
HalconFileManager fileManager;

// 📁 获取文件扩展名
QString regionExt = fileManager.getFileExtension("region");  // 返回 ".hreg"
QString modelExt = fileManager.getFileExtension("shape_model");  // 返回 ".shm"

// 🎯 生成文件名
QString fileName = fileManager.generateFileName("qr_template", "region");
// 结果: "qr_template_20250131_143022.hreg"

// 📂 创建目录结构
bool success = fileManager.createDirectoryStructure("./MyProject");
if (success) {
    qDebug() << "✅ 目录结构创建成功";
}
```

### 在VisualProcess中的集成使用 | Integration in VisualProcess

```cpp
// 在VisualProcess类中使用
void VisualProcess::handleQRCodeTemplateCreation(const QString& fileName)
{
    // 🔲 生成二维码识别区域文件路径
    QString regionPath = m_modelSavePath + fileName + "_qr_region" + 
                        m_halFileManager->getFileExtension("region");
    
    // 💾 保存参数文件
    QString paramsPath = m_modelSavePath + fileName + "_qr_params" + 
                        m_halFileManager->getFileExtension("tuple");
    
    // ... 其他处理逻辑
}
```

### 高级功能 | Advanced Features

```cpp
// 📊 分析目录中的文件
FileStats stats = fileManager.analyzeDirectory("./data");
qDebug() << "总文件数:" << stats.totalFiles;
qDebug() << "总大小:" << stats.totalSize;
qDebug() << "最大文件:" << stats.largestFileName;

// 🔍 查找特定类型的文件
QStringList regionFiles = fileManager.findFilesByType("./data", "region");
QStringList allHalconFiles = fileManager.getAllHalconFiles("./data");

// 🧹 清理旧文件（30天前的文件）
bool cleaned = fileManager.cleanupOldFiles("./temp", 30);

// 💾 备份重要文件
bool backed = fileManager.backupFile("important_model.shm", "./backup");
```

---

## 📁 推荐目录结构 | Recommended Directory Structure

使用 `createDirectoryStructure()` 创建的标准目录结构：

```
MyProject/
├── models/          # 模型文件 (*.shm, *.ncm, *.som)
├── images/          # 图像文件 (*.himage, *.jpg, *.png)
├── regions/         # 区域文件 (*.hreg, *.hroi)
├── params/          # 参数文件 (*.tup, *.hdict)
├── results/         # 结果文件 (测量数据、检测结果)
├── calibration/     # 标定数据 (*.calb, *.hmat)
├── temp/           # 临时文件
└── backup/         # 备份文件
```

---

## 🚀 集成到项目 | Project Integration

### 1. 添加到项目中 | Add to Project

```cpp
// 在你的类头文件中
#include "thirdparty/hdevelop/include/HalconFileManager.h"

class YourVisionClass {
private:
    HalconFileManager* m_fileManager;
};

// 在构造函数中初始化
YourVisionClass::YourVisionClass() {
    m_fileManager = new HalconFileManager(this);
}
```

### 2. 最佳实践 | Best Practices

- ✅ **统一使用HalconFileManager** - 所有Halcon文件操作都通过此类进行
- ✅ **遵循命名规范** - 使用推荐的文件扩展名和命名约定
- ✅ **定期备份重要文件** - 使用备份功能保护重要模型和数据
- ✅ **清理临时文件** - 定期清理过期的临时文件释放空间
- ✅ **验证文件有效性** - 在加载文件前验证其格式正确性

---

## 🔧 API参考 | API Reference

### 核心方法 | Core Methods

| 方法 | 描述 | 返回值 |
|------|------|--------|
| `getFileExtension(QString)` | 获取文件类型对应的扩展名 | QString |
| `generateFileName(QString, QString)` | 生成带时间戳的文件名 | QString |
| `createDirectoryStructure(QString)` | 创建标准目录结构 | bool |
| `analyzeDirectory(QString)` | 分析目录统计信息 | FileStats |
| `isValidHalconFile(QString)` | 验证是否为有效Halcon文件 | bool |

### 工具方法 | Utility Methods

| 方法 | 描述 | 返回值 |
|------|------|--------|
| `getSupportedFileTypes()` | 获取所有支持的文件类型 | QStringList |
| `getFileTypeDescriptions()` | 获取文件类型说明 | QMap<QString, QString> |
| `cleanupOldFiles(QString, int)` | 清理指定天数前的文件 | bool |
| `backupFile(QString, QString)` | 备份文件到指定目录 | bool |

---

## 💡 常见问题 | FAQ

**Q: 如何添加新的文件类型？**
A: 在HalconFileManager构造函数中的`initializeExtensionMap()`方法中添加新的映射关系。

**Q: 生成的文件名太长怎么办？**
A: 可以通过修改`generateFileName`方法的时间戳格式来缩短文件名。

**Q: 如何批量转换旧项目的文件名？**
A: 使用`getAllHalconFiles()`扫描现有文件，然后逐个重命名为标准格式。

---

## 🔄 版本历史 | Version History

- **v1.0** (2025-01-31) - 初始版本，支持基本的文件管理功能
- 支持19种Halcon专用文件格式
- 标准目录结构创建
- 文件统计和分析功能
- 文件验证和备份功能

---

## 👥 联系我们 | Contact

如有问题或建议，请联系开发团队。

For questions or suggestions, please contact the development team.

**开发团队 | Development Team**  
📧 Email: dev@company.com  
🌐 Website: www.company.com 