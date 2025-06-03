//
// Halcon文件管理器使用示例 | HalconFileManager Usage Example
// Created by 开发团队 on 2025-01-31
//

#include "HalconFileManager.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 🏗️ 创建文件管理器实例
    HalconFileManager fileManager;
    
    qDebug() << "=== Halcon文件管理器示例 | HalconFileManager Example ===\n";
    
    // 📂 示例1：创建标准目录结构
    QString basePath = "./HalconData";
    qDebug() << "1. 创建目录结构 | Creating directory structure...";
    if (fileManager.createDirectoryStructure(basePath)) {
        qDebug() << "✅ 目录结构创建成功 | Directory structure created successfully";
    } else {
        qDebug() << "❌ 目录结构创建失败 | Directory structure creation failed";
    }
    
    // 🎯 示例2：生成不同类型的文件名
    qDebug() << "\n2. 文件名生成示例 | Filename generation examples:";
    
    QString qrTemplate = fileManager.generateFileName("QRCode_Production", "region", "v2.1");
    QString shapeModel = fileManager.generateFileName("ProductTemplate", "shape_model", "v1.0");
    QString measureData = fileManager.generateFileNameSimple("MeasureResult_001", "tuple");
    QString calibData = fileManager.generateFileName("Camera_Calibration", "calib_data");
    
    qDebug() << "🔍 二维码区域文件 | QR Code region file:" << qrTemplate;
    qDebug() << "🎯 形状模板文件 | Shape model file:" << shapeModel;
    qDebug() << "📏 测量数据文件 | Measurement data file:" << measureData;
    qDebug() << "📷 标定数据文件 | Calibration data file:" << calibData;
    
    // 🗂️ 示例3：显示所有支持的扩展名
    qDebug() << "\n3. 支持的文件扩展名 | Supported file extensions:";
    QMap<QString, QString> extensions = fileManager.getAllExtensions();
    for (auto it = extensions.begin(); it != extensions.end(); ++it) {
        qDebug() << QString("   %1 → %2").arg(it.key(), -15).arg(it.value());
    }
    
    // 📊 示例4：扫描现有文件（如果存在）
    qDebug() << "\n4. 扫描Halcon文件 | Scanning Halcon files:";
    QStringList halconFiles = fileManager.scanHalconFiles(basePath);
    if (halconFiles.isEmpty()) {
        qDebug() << "   📭 未找到Halcon文件 | No Halcon files found";
    } else {
        qDebug() << QString("   📁 找到 %1 个Halcon文件:").arg(halconFiles.size());
        for (const QString& file : halconFiles) {
            qDebug() << "   📄" << file;
        }
    }
    
    // 📈 示例5：获取文件统计信息
    qDebug() << "\n5. 文件统计信息 | File statistics:";
    FileStats stats = fileManager.getFileStatistics(basePath);
    qDebug() << QString("   📊 总文件数 | Total files: %1").arg(stats.totalFiles);
    qDebug() << QString("   💾 总大小 | Total size: %1").arg(fileManager.formatFileSize(stats.totalSize));
    
    if (!stats.fileTypeCount.isEmpty()) {
        qDebug() << "   📈 文件类型分布 | File type distribution:";
        for (auto it = stats.fileTypeCount.begin(); it != stats.fileTypeCount.end(); ++it) {
            qDebug() << QString("      %1: %2 个文件").arg(it.key(), -15).arg(it.value());
        }
    }
    
    if (!stats.largestFileName.isEmpty()) {
        qDebug() << QString("   🔍 最大文件 | Largest file: %1 (%2)")
                    .arg(stats.largestFileName)
                    .arg(fileManager.formatFileSize(stats.largestFileSize));
    }
    
    // 🧹 示例6：清理功能演示
    qDebug() << "\n6. 清理功能演示 | Cleanup function demo:";
    int cleanedFiles = fileManager.cleanupTempFiles(basePath, 30); // 清理30天前的临时文件
    if (cleanedFiles > 0) {
        qDebug() << QString("✅ 清理了 %1 个临时文件").arg(cleanedFiles);
    } else {
        qDebug() << "📭 没有需要清理的临时文件 | No temporary files to clean";
    }
    
    // 🎯 示例7：文件验证演示
    qDebug() << "\n7. 文件验证示例 | File validation examples:";
    
    // 创建一些示例文件路径进行演示
    QStringList testFiles = {
        basePath + "/Models/Shape/test_model.shm",
        basePath + "/Regions/test_region.hreg", 
        basePath + "/Parameters/test_params.tup",
        basePath + "/Images/test_image.himage"
    };
    
    for (const QString& testFile : testFiles) {
        bool isValid = fileManager.validateFile(testFile);
        QString status = isValid ? "✅ 有效" : "❌ 无效/不存在";
        qDebug() << QString("   %1 → %2").arg(QFileInfo(testFile).fileName(), -20).arg(status);
    }
    
    // 💡 示例8：实际使用建议
    qDebug() << "\n8. 实际使用建议 | Practical usage recommendations:";
    qDebug() << "   💡 建议在项目初始化时调用 createDirectoryStructure()";
    qDebug() << "   💡 Recommend calling createDirectoryStructure() during project initialization";
    qDebug() << "   💡 使用 generateFileName() 来确保文件名的一致性";  
    qDebug() << "   💡 Use generateFileName() to ensure filename consistency";
    qDebug() << "   💡 定期调用 getFileStatistics() 来监控存储使用情况";
    qDebug() << "   💡 Regularly call getFileStatistics() to monitor storage usage";
    qDebug() << "   💡 使用 validateFile() 在加载文件前进行验证";
    qDebug() << "   💡 Use validateFile() to verify files before loading";
    
    qDebug() << "\n=== 示例完成 | Example completed ===";
    
    return 0; // 不启动事件循环，直接退出
}

/*
 * 编译和运行说明 | Compilation and Running Instructions:
 * 
 * 1. 确保已包含Qt和Halcon库 | Ensure Qt and Halcon libraries are included
 * 2. 编译命令示例 | Compilation example:
 *    qmake && make
 * 
 * 3. 运行程序后会看到：| After running, you will see:
 *    - 创建的目录结构 | Created directory structure
 *    - 生成的文件名示例 | Generated filename examples  
 *    - 支持的扩展名列表 | List of supported extensions
 *    - 文件统计信息 | File statistics
 *    - 验证结果 | Validation results
 * 
 * 4. 在实际项目中的集成 | Integration in actual projects:
 *    - 在主应用程序中创建 HalconFileManager 实例
 *    - Create HalconFileManager instance in main application
 *    - 使用其方法来管理所有Halcon相关文件
 *    - Use its methods to manage all Halcon-related files
 *    - 可以连接信号来监听文件操作状态
 *    - Connect signals to monitor file operation status
 */ 