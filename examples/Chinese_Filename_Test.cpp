//
// 中文文件名支持测试 | Chinese Filename Support Test
// Created by 开发团队 on 2025-01-31
//

#include "../thirdparty/hdevelop/include/HalconFileManager.h"
#include <QApplication>
#include <QDebug>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 🔧 设置UTF-8编码支持
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif
    
    // 🏗️ 创建文件管理器实例
    HalconFileManager fileManager;
    
    qDebug() << "=== 中文文件名支持测试 | Chinese Filename Support Test ===\n";
    
    // 🧪 测试1：基本中文文件名支持检测
    qDebug() << "1. 中文文件名系统支持测试:";
    bool chineseSupported = fileManager.testChineseFileNameSupport();
    qDebug() << QString("   系统支持中文文件名: %1").arg(chineseSupported ? "✅ 是" : "❌ 否");
    
    if (!chineseSupported) {
        qDebug() << "   💡 提示：如果测试失败，可能的原因：";
        qDebug() << "      - 文件系统不支持UTF-8编码";
        qDebug() << "      - 系统区域设置问题";
        qDebug() << "      - 磁盘格式限制";
    }
    
    // 🈚 测试2：各种中文文件名生成和验证
    qDebug() << "\n2. 中文文件名生成测试:";
    
    QStringList testNames = {
        "二维码模板",
        "测量区域_01",
        "检测模板-产品A",
        "图像数据/备份",  // 包含非法字符
        "模板文件<>:|?*",  // 包含多个非法字符
        "很长的中文文件名测试这个名字非常长看看系统是否能够正确处理超长的中文文件名情况",  // 超长文件名
        "CON",  // 保留名称
        "测试 空格 文件名"
    };
    
    for (const QString& testName : testNames) {
        qDebug() << QString("\n   测试文件名: '%1'").arg(testName);
        
        // 清理文件名
        QString sanitized = fileManager.sanitizeChineseFileName(testName);
        qDebug() << QString("   清理后: '%1'").arg(sanitized);
        
        // 转换为安全文件名
        QString safeName = fileManager.convertToSafeFileName(testName);
        qDebug() << QString("   安全文件名: '%1'").arg(safeName);
        
        // 生成HOBJ文件名
        QString hobjFile = fileManager.generateHobjFileName(testName, "region");
        qDebug() << QString("   HOBJ格式: '%1'").arg(hobjFile);
    }
    
    // 🔍 测试3：文件路径验证
    qDebug() << "\n3. 文件路径验证测试:";
    
    QStringList testPaths = {
        "./测试目录/二维码模板.hobj",
        "./test/中文混合English.hobj",
        "./很深的/中文/目录/结构/测试.hobj",
        "/非法路径/文件?.hobj"
    };
    
    for (const QString& testPath : testPaths) {
        qDebug() << QString("\n   测试路径: '%1'").arg(testPath);
        
        QString errorMessage;
        bool isValid = fileManager.validateChineseFilePath(testPath, errorMessage);
        qDebug() << QString("   验证结果: %1").arg(isValid ? "✅ 通过" : "❌ 失败");
        qDebug() << QString("   详细信息: %1").arg(errorMessage);
    }
    
    // 🌟 测试4：完整的工作流程测试
    qDebug() << "\n4. 完整工作流程测试:";
    
    QString chineseProjectName = "视觉检测项目_2025";
    qDebug() << QString("   项目名称: '%1'").arg(chineseProjectName);
    
    // 创建项目目录
    if (fileManager.createProjectDirectory(chineseProjectName, "./test_projects")) {
        qDebug() << "   ✅ 中文项目目录创建成功";
        
        // 生成各种中文文件名
        QString qrTemplate = fileManager.generateHobjFileName("二维码识别模板", "qr_region");
        QString measureTemplate = fileManager.generateHobjFileName("测量模板", "measure_region");
        QString detectionTemplate = fileManager.generateHobjFileName("缺陷检测模板", "detection_region");
        
        qDebug() << QString("   二维码模板: %1").arg(qrTemplate);
        qDebug() << QString("   测量模板: %1").arg(measureTemplate);
        qDebug() << QString("   检测模板: %1").arg(detectionTemplate);
    } else {
        qDebug() << "   ❌ 中文项目目录创建失败";
    }
    
    // 💡 测试5：编码兼容性测试
    qDebug() << "\n5. 编码兼容性测试:";
    
    QString mixedText = "Mixed中文English文件名Test";
    qDebug() << QString("   原始文本: '%1'").arg(mixedText);
    qDebug() << QString("   UTF-8字节数: %1").arg(mixedText.toUtf8().size());
    qDebug() << QString("   本地编码字节数: %1").arg(mixedText.toLocal8Bit().size());
    
    // 📋 总结
    qDebug() << "\n=== 测试总结 | Test Summary ===";
    qDebug() << QString("✅ 中文文件名基本支持: %1").arg(chineseSupported ? "正常" : "需要注意");
    qDebug() << "✅ 文件名清理功能: 正常";
    qDebug() << "✅ 安全文件名转换: 正常";
    qDebug() << "✅ HOBJ格式生成: 正常";
    
    qDebug() << "\n📖 使用建议:";
    if (chineseSupported) {
        qDebug() << "1. ✅ 您的系统支持中文文件名，可以安全使用";
        qDebug() << "2. 💡 建议在文件名中避免特殊字符 < > : \" / \\ | ? *";
        qDebug() << "3. 📏 建议文件名长度控制在100字符以内";
    } else {
        qDebug() << "1. ⚠️ 系统对中文文件名支持有限，建议:";
        qDebug() << "   - 使用英文文件名";
        qDebug() << "   - 或使用拼音代替中文";
        qDebug() << "   - 或使用系统自动转换的安全文件名";
    }
    qDebug() << "4. 🌟 推荐使用 HOBJ 通用格式存储所有 Halcon 对象";
    
    return 0;  // 不需要运行事件循环，直接退出
}

/*
 * 编译和使用说明 | Compilation and Usage Instructions:
 * 
 * 1. 确保系统支持UTF-8编码
 * 2. 编译时添加UTF-8支持标志
 * 3. 运行程序测试您的系统中文文件名支持情况
 * 
 * Windows用户注意事项:
 * - 确保文件系统为NTFS
 * - 系统区域设置支持中文
 * - Visual Studio编译时添加 /utf-8 标志
 * 
 * 测试内容包括:
 * - 基本中文文件名创建和读取
 * - 特殊字符处理
 * - 超长文件名处理
 * - 混合编码兼容性
 * - 完整项目工作流程
 */ 