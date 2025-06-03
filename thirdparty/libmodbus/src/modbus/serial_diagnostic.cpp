#include "../../inc/modbus/serial_diagnostic.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>

SerialDiagnostic::SerialDiagnostic(QObject *parent)
    : QObject(parent)
{
}

SerialDiagnostic::~SerialDiagnostic()
{
}

QStringList SerialDiagnostic::getAvailablePorts()
{
    QStringList portList;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &port : ports) {
        portList << port.portName();
    }
    
    return portList;
}

SerialDiagnostic::PortInfo SerialDiagnostic::getPortInfo(const QString &portName)
{
    PortInfo info;
    info.portName = portName;
    info.exists = false;
    info.accessible = false;
    info.inUse = false;
    
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &portInfo : ports) {
        if (portInfo.portName() == portName) {
            info.exists = true;
            info.description = portInfo.description();
            info.manufacturer = portInfo.manufacturer();
            info.serialNumber = portInfo.serialNumber();
            info.vendorId = portInfo.vendorIdentifier();
            info.productId = portInfo.productIdentifier();
            info.systemLocation = portInfo.systemLocation();
            break;
        }
    }
    
    if (info.exists) {
        // 测试串口是否可以打开
        QSerialPort testPort;
        testPort.setPortName(portName);
        
        if (testPort.open(QIODevice::ReadWrite)) {
            info.accessible = true;
            testPort.close();
        } else {
            info.accessible = false;
            info.errorString = testPort.errorString();
            info.errorCode = static_cast<int>(testPort.error());
            
            // 判断是否被占用
            if (testPort.error() == QSerialPort::PermissionError) {
                info.inUse = true;
            }
        }
    }
    
    return info;
}

bool SerialDiagnostic::testPortConnection(const QString &portName, int baudRate, int dataBits, char parity, int stopBits)
{
    QSerialPort testPort;
    testPort.setPortName(portName);
    testPort.setBaudRate(baudRate);
    testPort.setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    
    // 设置校验位
    switch (parity) {
    case 'N': testPort.setParity(QSerialPort::NoParity); break;
    case 'O': testPort.setParity(QSerialPort::OddParity); break;
    case 'E': testPort.setParity(QSerialPort::EvenParity); break;
    case 'M': testPort.setParity(QSerialPort::MarkParity); break;
    case 'S': testPort.setParity(QSerialPort::SpaceParity); break;
    default: testPort.setParity(QSerialPort::NoParity); break;
    }
    
    // 设置停止位
    if (stopBits == 2) {
        testPort.setStopBits(QSerialPort::TwoStop);
    } else {
        testPort.setStopBits(QSerialPort::OneStop);
    }
    
    testPort.setFlowControl(QSerialPort::NoFlowControl);
    
    if (!testPort.open(QIODevice::ReadWrite)) {
        qDebug() << tr("串口测试失败: %1 - %2").arg(portName).arg(testPort.errorString());
        return false;
    }
    
    // 简单的读写测试
    testPort.clear();
    QThread::msleep(100);
    
    testPort.close();
    qDebug() << tr("串口测试成功: %1").arg(portName);
    return true;
}

QString SerialDiagnostic::generateDiagnosticReport()
{
    QString report;
    report += tr("=== 串口诊断报告 ===\n\n");
    
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    
    if (ports.isEmpty()) {
        report += tr("❌ 未检测到任何串口设备\n");
        report += tr("建议:\n");
        report += tr("• 检查设备连接\n");
        report += tr("• 安装USB转串口驱动\n");
        report += tr("• 检查设备管理器\n\n");
        return report;
    }
    
    report += tr("检测到 %1 个串口设备:\n\n").arg(ports.size());
    
    for (const QSerialPortInfo &portInfo : ports) {
        PortInfo info = getPortInfo(portInfo.portName());
        
        report += tr("端口: %1\n").arg(info.portName);
        report += tr("描述: %2\n").arg(info.description);
        report += tr("制造商: %3\n").arg(info.manufacturer);
        report += tr("系统位置: %4\n").arg(info.systemLocation);
        
        if (info.accessible) {
            report += tr("状态: ✅ 可用\n");
        } else if (info.inUse) {
            report += tr("状态: ⚠️  被占用\n");
            report += tr("错误: %1\n").arg(info.errorString);
        } else {
            report += tr("状态: ❌ 不可用\n");
            report += tr("错误: %1\n").arg(info.errorString);
        }
        
        // 测试常见波特率
        report += tr("波特率测试:\n");
        QList<int> commonBaudRates = {9600, 19200, 38400, 57600, 115200};
        
        for (int baudRate : commonBaudRates) {
            if (info.accessible && testPortConnection(info.portName, baudRate, 8, 'N', 1)) {
                report += tr("  %1: ✅\n").arg(baudRate);
            } else {
                report += tr("  %1: ❌\n").arg(baudRate);
            }
        }
        
        report += "\n";
    }
    
    return report;
}

QStringList SerialDiagnostic::getRecommendations(const QString &portName)
{
    QStringList recommendations;
    PortInfo info = getPortInfo(portName);
    
    if (!info.exists) {
        recommendations << tr("串口不存在，请检查设备连接");
        recommendations << tr("确认USB转串口驱动已正确安装");
        recommendations << tr("在设备管理器中检查串口状态");
        return recommendations;
    }
    
    if (info.inUse) {
        recommendations << tr("串口被其他程序占用，请关闭以下可能的程序:");
        recommendations << tr("• 串口调试工具 (如SecureCRT, Putty, 超级终端)");
        recommendations << tr("• 其他Modbus工具");
        recommendations << tr("• 设备配置软件");
        recommendations << tr("• 重启计算机可能有助于释放串口");
        return recommendations;
    }
    
    if (!info.accessible) {
        recommendations << tr("串口无法访问，可能的解决方案:");
        recommendations << tr("• 以管理员身份运行程序");
        recommendations << tr("• 检查串口权限设置");
        recommendations << tr("• 重新插拔USB设备");
        recommendations << tr("• 更新或重新安装串口驱动");
        return recommendations;
    }
    
    // 如果串口可用，提供Modbus连接建议
    recommendations << tr("串口可用，Modbus连接建议:");
    recommendations << tr("• 尝试标准参数: 9600,8,N,1");
    recommendations << tr("• 检查从机ID设置 (通常为1)");
    recommendations << tr("• 确认RS485接线正确 (A+/B-)");
    recommendations << tr("• 检查设备供电状态");
    
    return recommendations;
} 