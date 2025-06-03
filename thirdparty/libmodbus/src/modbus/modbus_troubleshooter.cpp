#include "../../inc/modbus/modbus_troubleshooter.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QDir>

ModbusTroubleshooter::ModbusTroubleshooter(QObject *parent)
    : QObject(parent)
{
}

ModbusTroubleshooter::~ModbusTroubleshooter()
{
}

QString ModbusTroubleshooter::diagnoseSerialPort(const QString &portName)
{
    QString report;
    report += tr("=== Modbus RTU 串口诊断报告 ===\n\n");
    
    // 1. 检查串口是否存在
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    bool portExists = false;
    QSerialPortInfo targetPort;
    
    for (const QSerialPortInfo &port : ports) {
        if (port.portName() == portName) {
            portExists = true;
            targetPort = port;
            break;
        }
    }
    
    if (!portExists) {
        report += tr("❌ 问题: 串口 %1 不存在\n\n").arg(portName);
        report += tr("可用串口列表:\n");
        if (ports.isEmpty()) {
            report += tr("  (无可用串口)\n");
        } else {
            for (const QSerialPortInfo &port : ports) {
                report += tr("  • %1 - %2\n").arg(port.portName()).arg(port.description());
            }
        }
        report += tr("\n解决方案:\n");
        report += tr("1. 检查设备是否正确连接\n");
        report += tr("2. 检查USB转串口驱动是否安装\n");
        report += tr("3. 在设备管理器中查看串口状态\n");
        report += tr("4. 尝试重新插拔USB设备\n\n");
        return report;
    }
    
    report += tr("✅ 串口 %1 存在\n").arg(portName);
    report += tr("   描述: %1\n").arg(targetPort.description());
    report += tr("   制造商: %1\n").arg(targetPort.manufacturer());
    report += tr("   系统位置: %1\n\n").arg(targetPort.systemLocation());
    
    // 2. 测试串口访问权限
    QSerialPort testPort;
    testPort.setPortName(portName);
    
    if (!testPort.open(QIODevice::ReadWrite)) {
        report += tr("❌ 问题: 无法打开串口\n");
        report += tr("   错误: %1\n\n").arg(testPort.errorString());
        
        if (testPort.error() == QSerialPort::PermissionError) {
            report += tr("🔒 权限问题解决方案:\n");
            report += tr("1. 以管理员身份运行程序\n");
            report += tr("2. 检查串口是否被其他程序占用:\n");
            report += tr("   • 串口调试工具 (SecureCRT, Putty, 超级终端)\n");
            report += tr("   • 其他Modbus工具\n");
            report += tr("   • 设备配置软件\n");
            report += tr("3. 重启计算机释放串口资源\n\n");
        } else if (testPort.error() == QSerialPort::DeviceNotFoundError) {
            report += tr("🔌 设备问题解决方案:\n");
            report += tr("1. 重新插拔USB设备\n");
            report += tr("2. 更换USB端口\n");
            report += tr("3. 检查USB线缆是否损坏\n");
            report += tr("4. 更新或重新安装驱动程序\n\n");
        } else {
            report += tr("⚠️  其他问题解决方案:\n");
            report += tr("1. 检查设备管理器中的串口状态\n");
            report += tr("2. 禁用并重新启用串口设备\n");
            report += tr("3. 更新系统驱动程序\n\n");
        }
        return report;
    }
    
    testPort.close();
    report += tr("✅ 串口可以正常打开\n\n");
    
    // 3. 测试常见Modbus参数
    report += tr("📋 Modbus RTU 参数测试:\n");
    
    QList<QPair<int, QString>> baudRates = {
        {1200, "1200 (很慢)"},
        {2400, "2400 (慢)"},
        {4800, "4800 (较慢)"},
        {9600, "9600 (标准)"},
        {19200, "19200 (快)"},
        {38400, "38400 (较快)"},
        {57600, "57600 (很快)"},
        {115200, "115200 (最快)"}
    };
    
    for (const auto &baudRate : baudRates) {
        bool success = testSerialConnection(portName, baudRate.first, 8, 'N', 1);
        report += tr("  %1: %2\n").arg(baudRate.second).arg(success ? "✅" : "❌");
    }
    
    report += tr("\n🔧 Modbus 连接建议:\n");
    report += tr("1. 最常用参数: 9600,8,N,1\n");
    report += tr("2. 检查设备手册确认正确参数\n");
    report += tr("3. 确认从机ID设置 (通常为1)\n");
    report += tr("4. 检查RS485接线:\n");
    report += tr("   • A+ 连接 A+\n");
    report += tr("   • B- 连接 B-\n");
    report += tr("   • 确保接地良好\n");
    report += tr("5. 检查设备供电状态\n");
    report += tr("6. 如果是RS232，确认RXD/TXD/GND连接正确\n\n");
    
    return report;
}

bool ModbusTroubleshooter::testSerialConnection(const QString &portName, int baudRate, int dataBits, char parity, int stopBits)
{
    QSerialPort testPort;
    testPort.setPortName(portName);
    testPort.setBaudRate(baudRate);
    testPort.setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    
    switch (parity) {
    case 'N': testPort.setParity(QSerialPort::NoParity); break;
    case 'O': testPort.setParity(QSerialPort::OddParity); break;
    case 'E': testPort.setParity(QSerialPort::EvenParity); break;
    default: testPort.setParity(QSerialPort::NoParity); break;
    }
    
    testPort.setStopBits(stopBits == 2 ? QSerialPort::TwoStop : QSerialPort::OneStop);
    testPort.setFlowControl(QSerialPort::NoFlowControl);
    
    if (!testPort.open(QIODevice::ReadWrite)) {
        return false;
    }
    
    // 简单测试
    testPort.clear();
    QThread::msleep(50);
    testPort.close();
    
    return true;
}

QStringList ModbusTroubleshooter::getQuickFixes()
{
    QStringList fixes;
    
    fixes << tr("🚀 快速解决方案:");
    fixes << tr("");
    fixes << tr("1. 检查串口占用:");
    fixes << tr("   • 关闭所有串口调试工具");
    fixes << tr("   • 关闭其他Modbus软件");
    fixes << tr("   • 重启计算机");
    fixes << tr("");
    fixes << tr("2. 权限问题:");
    fixes << tr("   • 右键程序 → 以管理员身份运行");
    fixes << tr("");
    fixes << tr("3. 驱动问题:");
    fixes << tr("   • 设备管理器 → 端口 → 更新驱动");
    fixes << tr("   • 重新插拔USB设备");
    fixes << tr("");
    fixes << tr("4. 参数设置:");
    fixes << tr("   • 尝试 9600,8,N,1 (最常用)");
    fixes << tr("   • 从机ID设为1");
    fixes << tr("   • 检查设备手册");
    fixes << tr("");
    fixes << tr("5. 硬件检查:");
    fixes << tr("   • RS485: 确认A+/B-接线");
    fixes << tr("   • RS232: 确认RXD/TXD/GND");
    fixes << tr("   • 检查设备供电");
    
    return fixes;
}

QString ModbusTroubleshooter::generateSystemInfo()
{
    QString info;
    info += tr("=== 系统信息 ===\n");
    
    // 获取可用串口
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    info += tr("检测到 %1 个串口设备:\n").arg(ports.size());
    
    for (const QSerialPortInfo &port : ports) {
        info += tr("\n端口: %1\n").arg(port.portName());
        info += tr("  描述: %1\n").arg(port.description());
        info += tr("  制造商: %1\n").arg(port.manufacturer());
        info += tr("  序列号: %1\n").arg(port.serialNumber());
        info += tr("  系统位置: %1\n").arg(port.systemLocation());
        info += tr("  VID: 0x%1\n").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
        info += tr("  PID: 0x%1\n").arg(port.productIdentifier(), 4, 16, QChar('0'));
        
        // 测试可访问性
        QSerialPort testPort;
        testPort.setPortName(port.portName());
        if (testPort.open(QIODevice::ReadWrite)) {
            info += tr("  状态: ✅ 可用\n");
            testPort.close();
        } else {
            info += tr("  状态: ❌ 不可用 (%1)\n").arg(testPort.errorString());
        }
    }
    
    return info;
} 