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

### 🌟 推荐格式：HOBJ通用格式 | Recommended: HOBJ Universal Format

**强烈推荐使用 `.hobj` 格式** - 这是HALCON的通用二进制格式，具有以下优势：

✅ **万能兼容** - 可以存储图像、区域、XLD等所有HALCON对象类型  
✅ **高效存储** - 二进制格式，文件体积小，读写速度快  
✅ **完整保真** - 完全保留HALCON对象的所有属性和数据  
✅ **版本兼容** - 不同版本的HALCON都支持此格式  
✅ **简化管理** - 统一的文件扩展名，易于管理和识别  

| 对象类型 | HOBJ用途 | 优势说明 |
|----------|----------|----------|
| 🖼️ **图像对象** | 存储图像数据 | 支持多通道、高精度图像 |
| 📐 **区域对象** | 存储ROI、掩膜 | 精确的像素级区域定义 |
| 📏 **XLD轮廓** | 存储边缘、线条 | 亚像素精度的轮廓数据 |
| 🎯 **模板对象** | 存储匹配模板 | 包含完整的特征信息 |
| 📊 **测量数据** | 存储测量结果 | 保留原始测量对象 |

### 其他专用格式 | Other Specialized Formats

| 扩展名 | 文件类型 | 使用场景 | 备注 |
|--------|----------|----------|------|
| `.shm` | 形状模型 | Shape Model文件 | 模板匹配专用 |
| `.ncm` | NCC模型 | 灰度匹配 | 归一化互相关 |
| `.tup` | Tuple参数 | 算法参数、标定数据 | 参数存储专用 |
| `.hdict` | 字典文件 | 键值对数据 | 结构化数据存储 |
| `.calb` | 标定数据 | 相机内外参数 | 标定专用格式 |

---

## 💻 使用示例 | Usage Examples

### 🌟 HOBJ格式使用（推荐）| HOBJ Format Usage (Recommended)

```cpp
#include "thirdparty/hdevelop/include/HalconFileManager.h"

// 🏗️ 创建文件管理器实例
HalconFileManager fileManager;

// 🌟 推荐：使用HOBJ通用格式
QString regionFile = fileManager.generateHobjFileName("qr_template", "region");
// 结果: "qr_template_region_20250131_143022.hobj"

QString imageFile = fileManager.generateHobjFileName("test_image", "image");
// 结果: "test_image_image_20250131_143022.hobj"

QString xldFile = fileManager.generateHobjFileName("contour_data", "xld");
// 结果: "contour_data_xld_20250131_143022.hobj"

// 🎯 生成唯一文件名（避免覆盖）
QString uniqueFile = fileManager.generateUniqueHobjFileName("./data", "template", "detection");
// 如果文件存在，会自动添加数字后缀

// ✅ 验证是否为HOBJ文件
bool isHobj = fileManager.isHobjFile("template_detection_1.hobj");  // true

// 📋 获取HOBJ文件说明
QString description = fileManager.getHobjFileDescription("region");
// 返回: "HOBJ区域对象文件 | HOBJ Region Object File - 存储ROI、掩膜等区域数据"
```

### 基本使用 | Basic Usage

```cpp
// 📁 获取文件扩展名（现在主要返回.hobj）
QString regionExt = fileManager.getFileExtension("region");  // 返回 ".hobj"
QString imageExt = fileManager.getFileExtension("image");    // 返回 ".hobj"
QString xldExt = fileManager.getFileExtension("xld");        // 返回 ".hobj"

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

### 🌟 HOBJ专用方法 | HOBJ Specific Methods

| 方法 | 描述 | 返回值 |
|------|------|--------|
| `generateHobjFileName(QString, QString)` | 生成HOBJ格式文件名 | QString |
| `generateUniqueHobjFileName(QString, QString, QString)` | 生成唯一HOBJ文件名 | QString |
| `isHobjFile(QString)` | 验证是否为HOBJ文件 | bool |
| `getHobjFileDescription(QString)` | 获取HOBJ文件类型说明 | QString |

### 工具方法 | Utility Methods

| 方法 | 描述 | 返回值 |
|------|------|--------|
| `getSupportedFileTypes()` | 获取所有支持的文件类型 | QStringList |
| `getFileTypeDescriptions()` | 获取文件类型说明 | QMap<QString, QString> |
| `cleanupOldFiles(QString, int)` | 清理指定天数前的文件 | bool |
| `backupFile(QString, QString)` | 备份文件到指定目录 | bool |

---

## 💡 常见问题 | FAQ

**Q: 为什么推荐使用.hobj格式？**
A: `.hobj`是HALCON的通用二进制格式，可以存储所有类型的HALCON对象，具有更好的兼容性和性能。

**Q: 使用.hobj格式会影响现有代码吗？**
A: 不会。现有的HALCON API完全支持.hobj格式，只需要修改文件名生成部分即可。

**Q: 如何从旧格式(.hreg, .himage)迁移到.hobj？**
A: 可以使用HALCON的ReadObject/WriteObject函数进行格式转换，或者直接重新保存对象。

**Q: 如何添加新的文件类型？**
A: 在HalconFileManager构造函数中的`initializeExtensionMap()`方法中添加新的映射关系。

**Q: 生成的文件名太长怎么办？**
A: 可以通过修改`generateHobjFileName`方法的时间戳格式来缩短文件名，或使用`generateUniqueHobjFileName`的简化版本。

---

## 🔄 版本历史 | Version History

- **v1.1** (2025-01-31) - 重点支持HOBJ通用格式
  - 🌟 优先推荐使用.hobj格式
  - 🔧 新增HOBJ专用方法
  - 📖 更新文档和示例
  - ✨ 改进用户体验

- **v1.0** (2025-01-31) - 初始版本，支持基本的文件管理功能
  - 支持19种Halcon专用文件格式
  - 标准目录结构创建
  - 文件统计和分析功能
  - 文件验证和备份功能

---

## 🎯 最佳实践建议 | Best Practice Recommendations

### ✅ 推荐做法

1. **优先使用HOBJ格式** - 统一使用`.hobj`扩展名存储所有HALCON对象
2. **明确对象类型** - 在文件名中包含对象类型标识（如region、image、xld）
3. **使用时间戳** - 利用自动生成的时间戳避免文件覆盖
4. **标准目录结构** - 使用推荐的项目目录组织文件

### ❌ 避免做法

1. **混用文件格式** - 避免在同一项目中使用多种扩展名
2. **手动命名文件** - 避免手动拼接文件名，使用提供的方法
3. **忽略文件验证** - 在加载文件前应验证文件有效性

---

## 🈚 中文文件名支持 | Chinese Filename Support

### 🎯 功能特点

HalconFileManager **完全支持中文文件名**，提供以下功能：

✅ **自动检测** - 自动检测系统对中文文件名的支持情况  
✅ **智能转换** - 自动清理和转换不安全的文件名  
✅ **编码兼容** - 确保UTF-8编码正确处理  
✅ **路径验证** - 验证中文路径的有效性和可写性  
✅ **安全回退** - 不支持时自动转换为安全格式  

### 📝 使用示例

```cpp
HalconFileManager fileManager;

// 🈚 直接使用中文文件名
QString chineseFileName = fileManager.generateHobjFileName("二维码模板", "region");
// 结果: "二维码模板_region_20250131_143022.hobj"

// 🔍 验证中文文件名支持
QString errorMessage;
bool isValid = fileManager.validateChineseFilePath("./模板/测试文件.hobj", errorMessage);

// 🛡️ 转换为安全文件名（如果需要）
QString safeName = fileManager.convertToSafeFileName("包含特殊字符<>的文件名");
// 结果: "zh_包含特殊字符__的文件名_20250131_143022"

// 🧹 清理文件名
QString cleaned = fileManager.sanitizeChineseFileName("文件名/包含:非法*字符");
// 结果: "文件名_包含_非法_字符"
```

### ⚠️ 注意事项

1. **系统要求**：
   - Windows: NTFS文件系统，区域设置支持中文
   - Linux: UTF-8 locale设置
   - macOS: 默认支持

2. **编译要求**：
   - 添加 `/utf-8` 编译标志（Visual Studio）
   - 确保Qt UTF-8编码支持

3. **文件名限制**：
   - 避免特殊字符：`< > : " / \ | ? *`
   - 推荐长度：100字符以内
   - 避免Windows保留名称：CON, PRN, AUX等

### 🧪 测试工具

项目提供了专门的中文文件名测试工具：

```bash
# 编译测试工具
qmake examples/Chinese_Filename_Test.pro
make

# 运行测试
./Chinese_Filename_Test
```

测试内容包括：
- 系统中文文件名支持检测
- 各种中文文件名处理测试
- 编码兼容性验证
- 完整工作流程测试

---

## 👥 联系我们 | Contact

如有问题或建议，请联系开发团队。

For questions or suggestions, please contact the development team.

**开发团队 | Development Team**  
📧 Email: dev@company.com  
🌐 Website: www.company.com