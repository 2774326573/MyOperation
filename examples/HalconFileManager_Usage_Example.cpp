//
// Halcon文件管理器使用示例 | HalconFileManager Usage Example
// Created by 开发团队 on 2025-01-31
//

#include "../thirdparty/hdevelop/include/HalconFileManager.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 🏗️ 创建文件管理器实例
    HalconFileManager fileManager;
    
    qDebug() << "=== Halcon文件管理器示例 | HalconFileManager Example ===\n";
    
    // 📂 示例1：创建标准目录结构
    QString basePath = "./HalconTestData";
    qDebug() << "1. 创建目录结构 | Creating directory structure...";
    if (fileManager.createDirectoryStructure(basePath)) {
        qDebug() << "✅ 目录结构创建成功 | Directory structure created successfully";
    } else {
        qDebug() << "❌ 目录结构创建失败 | Directory structure creation failed";
    }
    
    // 🌟 示例2：HOBJ通用格式的使用 - 重点推荐！
    qDebug() << "\n🌟 2. HOBJ通用格式使用示例 | HOBJ Universal Format Usage (RECOMMENDED):";
    
    // 生成不同类型的HOBJ文件名
    QString qrHobjFile = fileManager.generateHobjFileName("qr_template", "region");
    QString measureHobjFile = fileManager.generateHobjFileName("measure_template", "measurement");
    QString detectionHobjFile = fileManager.generateHobjFileName("detection_template", "template");
    QString imageHobjFile = fileManager.generateHobjFileName("test_image", "image");
    QString xldHobjFile = fileManager.generateHobjFileName("contour_data", "xld");
    
    qDebug() << "🔍 二维码区域HOBJ:" << qrHobjFile;
    qDebug() << "📏 测量对象HOBJ:" << measureHobjFile;
    qDebug() << "🎯 检测模板HOBJ:" << detectionHobjFile;
    qDebug() << "🖼️ 图像对象HOBJ:" << imageHobjFile;
    qDebug() << "📐 轮廓数据HOBJ:" << xldHobjFile;
    
    // 显示HOBJ格式的描述
    qDebug() << "\n📋 HOBJ格式说明:";
    qDebug() << "  🌟" << fileManager.getHobjFileDescription("region");
    qDebug() << "  🌟" << fileManager.getHobjFileDescription("image");
    qDebug() << "  🌟" << fileManager.getHobjFileDescription("xld");
    
    // 🎯 示例3：生成传统格式的文件名（向后兼容）
    qDebug() << "\n3. 传统格式文件名生成 | Traditional format filename generation:";
    
    QString tupleFileName = fileManager.generateFileName("params", "tuple");
    QString modelFileName = fileManager.generateFileName("shape_model", "shape_model");
    
    qDebug() << "📋 参数文件 (传统):" << tupleFileName;
    qDebug() << "🎯 形状模型 (传统):" << modelFileName;
    
    // 📊 示例4：获取支持的文件类型
    qDebug() << "\n4. 支持的文件类型 | Supported file types:";
    QStringList supportedTypes = fileManager.getSupportedFileTypes();
    qDebug() << "📋 支持的类型数量:" << supportedTypes.size();
    
    for (const QString& type : supportedTypes) {
        QString extension = fileManager.getFileExtension(type);
        qDebug() << QString("  • %1 → %2").arg(type, extension);
    }
    
    // 🔍 示例5：文件类型描述
    qDebug() << "\n5. 文件类型描述 | File type descriptions:";
    QMap<QString, QString> descriptions = fileManager.getFileTypeDescriptions();
    auto it = descriptions.constBegin();
    int count = 0;
    while (it != descriptions.constEnd() && count < 5) {  // 只显示前5个
        qDebug() << QString("  📄 %1: %2").arg(it.key(), it.value());
        ++it;
        ++count;
    }
    qDebug() << "  ... (" << (descriptions.size() - 5) << " more types)";
    
    // 🔎 示例6：验证文件扩展名
    qDebug() << "\n6. 文件验证示例 | File validation examples:";
    QStringList testFiles = {
        "test.hreg",
        "model.shm", 
        "image.himage",
        "unknown.txt",
        "params.tup"
    };
    
    for (const QString& testFile : testFiles) {
        bool isValid = fileManager.isValidHalconFile(testFile);
        QString fileType = fileManager.getFileTypeFromExtension(testFile);
        qDebug() << QString("  📁 %1 → %2 (类型: %3)")
                    .arg(testFile)
                    .arg(isValid ? "✅ 有效" : "❌ 无效")
                    .arg(fileType.isEmpty() ? "未知" : fileType);
    }
    
    // 🎯 示例7：创建项目目录
    qDebug() << "\n7. 创建项目目录 | Creating project directory:";
    if (fileManager.createProjectDirectory("MyVisionProject", basePath)) {
        qDebug() << "✅ 项目目录创建成功: MyVisionProject";
    }
    
    qDebug() << "\n=== 示例运行完成 | Example completed ===";
    
    return 0;  // 不需要运行事件循环，直接退出
}

/*
 * 编译和运行说明 | Compilation and Usage Instructions:
 * 
 * 1. 确保已配置Qt和Halcon环境
 * 2. 编译命令示例:
 *    qmake && make
 * 
 * 3. 运行程序将在当前目录创建 HalconTestData 文件夹
 *    并展示各种文件管理功能
 * 
 * 4. 输出将显示:
 *    - 目录结构创建结果
 *    - 各种Halcon文件名生成示例
 *    - 支持的文件类型列表
 *    - 文件验证结果
 */ 