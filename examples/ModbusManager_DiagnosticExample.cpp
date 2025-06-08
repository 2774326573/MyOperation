//
// ModbusManager 串口诊断功能使用示例
// ModbusManager Serial Diagnostic Functions Usage Example
//
// 此示例展示如何使用 ModbusManager 类中集成的串口诊断功能
// This example shows how to use the integrated serial diagnostic functions in ModbusManager class
//
// 创建时间: 2025-06-06
// Created on: 2025-06-06

#include "../thirdparty/libmodbus/inc/modbus/modbusmanager.h"
#include "../thirdparty/libmodbus/inc/modbus/modbus_rw_manager.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

void demonstrateSerialDiagnostics()
{
    qDebug() << "=== ModbusManager 串口诊断功能演示 ===\n";
    
    // 1. 获取可用串口列表
    qDebug() << "1. 获取可用串口列表:";
    QStringList availablePorts = ModbusManager::getAvailablePorts();
    if (availablePorts.isEmpty()) {
        qDebug() << "   未检测到任何串口设备";
    } else {
        qDebug() << "   可用串口:" << availablePorts.join(", ");
    }
    qDebug() << "";
    
    // 2. 生成完整的串口诊断报告
    qDebug() << "2. 生成串口诊断报告:";
    QString diagnosticReport = ModbusManager::generateDiagnosticReport();
    qDebug() << diagnosticReport;
    
    // 3. 如果有可用串口，对第一个串口进行详细诊断
    if (!availablePorts.isEmpty()) {
        QString firstPort = availablePorts.first();
        qDebug() << "3. 对串口" << firstPort << "进行详细诊断:";
        
        // 获取串口详细信息
        ModbusManager::PortInfo portInfo = ModbusManager::getPortInfo(firstPort);
        qDebug() << "   端口名称:" << portInfo.portName;
        qDebug() << "   描述:" << portInfo.description;
        qDebug() << "   制造商:" << portInfo.manufacturer;
        qDebug() << "   系统位置:" << portInfo.systemLocation;
        qDebug() << "   是否存在:" << (portInfo.exists ? "是" : "否");
        qDebug() << "   是否可访问:" << (portInfo.accessible ? "是" : "否");
        qDebug() << "   是否被占用:" << (portInfo.inUse ? "是" : "否");
        
        // 生成针对特定串口的诊断报告
        QString specificReport = ModbusManager::diagnoseSerialPort(firstPort);
        qDebug() << "\n详细诊断报告:";
        qDebug() << specificReport;
        
        // 获取建议
        QStringList recommendations = ModbusManager::getRecommendations(firstPort);
        qDebug() << "\n建议:";
        for (const QString &rec : recommendations) {
            qDebug() << "   " << rec;
        }
        
        // 测试串口连接
        qDebug() << "\n4. 测试串口连接 (9600,8,N,1):";
        bool testResult = ModbusManager::testPortConnection(firstPort, 9600, 8, 'N', 1);
        qDebug() << "   测试结果:" << (testResult ? "成功" : "失败");
    }
    
    // 5. 获取快速解决方案
    qDebug() << "\n5. 快速解决方案:";
    QStringList quickFixes = ModbusManager::getQuickFixes();
    for (const QString &fix : quickFixes) {
        qDebug() << fix;
    }
    
    // 6. 生成系统信息
    qDebug() << "\n6. 系统信息:";
    QString systemInfo = ModbusManager::generateSystemInfo();
    qDebug() << systemInfo;
}

void demonstrateModbusRwManagerDiagnostics()
{
    qDebug() << "\n=== ModbusRwManager 串口诊断功能演示 ===\n";
    
    // 通过 ModbusRwManager 使用诊断功能
    QStringList ports = ModbusRwManager::getAvailablePorts();
    qDebug() << "通过 ModbusRwManager 获取的可用串口:" << ports.join(", ");
    
    // 生成诊断报告
    QString report = ModbusRwManager::getSerialDiagnosticReport();
    qDebug() << "\n通过 ModbusRwManager 生成的诊断报告:";
    qDebug() << report;
    
    // 获取快速解决方案
    QStringList fixes = ModbusRwManager::getQuickFixes();
    qDebug() << "\n通过 ModbusRwManager 获取的快速解决方案:";
    for (const QString &fix : fixes) {
        qDebug() << fix;
    }
}

void demonstrateModbusConnectionWithDiagnostics()
{
    qDebug() << "\n=== 带诊断功能的 Modbus 连接演示 ===\n";
    
    ModbusManager modbusManager;
    
    // 获取可用串口
    QStringList ports = ModbusManager::getAvailablePorts();
    if (ports.isEmpty()) {
        qDebug() << "没有可用串口，无法演示连接";
        return;
    }
    
    QString portToTest = ports.first();
    qDebug() << "尝试连接到串口:" << portToTest;
    
    // 在连接前进行诊断
    qDebug() << "\n连接前诊断:";
    ModbusManager::PortInfo portInfo = ModbusManager::getPortInfo(portToTest);
    if (!portInfo.exists) {
        qDebug() << "串口不存在，无法连接";
        return;
    }
    
    if (!portInfo.accessible) {
        qDebug() << "串口不可访问:" << portInfo.errorString;
        QStringList recommendations = ModbusManager::getRecommendations(portToTest);
        qDebug() << "建议:";
        for (const QString &rec : recommendations) {
            qDebug() << "  " << rec;
        }
        return;
    }
    
    // 尝试连接
    qDebug() << "\n尝试 Modbus RTU 连接...";
    bool connected = modbusManager.connectRTU(portToTest, 9600, 8, 'N', 1);
    
    if (connected) {
        qDebug() << "连接成功！";
        
        // 设置从机ID并尝试读取
        modbusManager.setSlaveID(1);
        QVector<quint16> values;
        bool readSuccess = modbusManager.readHoldingRegisters(0, 1, values);
        
        if (readSuccess) {
            qDebug() << "成功读取寄存器，值:" << values;
        } else {
            qDebug() << "读取寄存器失败:" << modbusManager.getLastError();
        }
        
        // 断开连接
        modbusManager.disconnect();
        qDebug() << "连接已断开";
    } else {
        qDebug() << "连接失败:" << modbusManager.getLastError();
        
        // 生成详细诊断报告
        QString detailedReport = ModbusManager::diagnoseSerialPort(portToTest);
        qDebug() << "\n详细诊断报告:";
        qDebug() << detailedReport;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    try {
        // 演示串口诊断功能
        demonstrateSerialDiagnostics();
        
        // 演示通过 ModbusRwManager 使用诊断功能
        demonstrateModbusRwManagerDiagnostics();
        
        // 演示带诊断功能的 Modbus 连接
        demonstrateModbusConnectionWithDiagnostics();
        
        qDebug() << "\n=== 演示完成 ===";
        
    } catch (const std::exception &e) {
        qDebug() << "演示过程中发生异常:" << e.what();
        return 1;
    }
    
    return 0;
}

// 编译说明 (Compilation Instructions):
// 
// 使用 qmake:
// qmake && make
// 
// 使用 CMake:
// mkdir build && cd build
// cmake .. && make
// 
// 依赖项 (Dependencies):
// - Qt5 Core, SerialPort
// - libmodbus
//
// 注意事项 (Notes):
// 1. 确保有适当的串口设备连接或USB转串口适配器
// 2. 在Windows上可能需要管理员权限来访问串口
// 3. 某些诊断功能可能需要真实的串口设备
