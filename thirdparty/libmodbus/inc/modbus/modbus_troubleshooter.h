#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief Modbus连接故障排除工具 (Modbus Connection Troubleshooter)
 * 
 * 用于诊断和解决Modbus RTU串口连接问题
 * Used to diagnose and solve Modbus RTU serial port connection issues
 */
class ModbusTroubleshooter : public QObject
{
    Q_OBJECT

public:
    explicit ModbusTroubleshooter(QObject *parent = nullptr);
    ~ModbusTroubleshooter();

    /**
     * @brief 诊断指定串口的问题 (Diagnose issues with specified serial port)
     * @param portName 串口名称 (Port name)
     * @return 诊断报告 (Diagnostic report)
     */
    static QString diagnoseSerialPort(const QString &portName);

    /**
     * @brief 测试串口连接 (Test serial port connection)
     * @param portName 串口名称 (Port name)
     * @param baudRate 波特率 (Baud rate)
     * @param dataBits 数据位 (Data bits)
     * @param parity 校验位 (Parity)
     * @param stopBits 停止位 (Stop bits)
     * @return 测试结果 (Test result)
     */
    static bool testSerialConnection(const QString &portName, int baudRate, int dataBits, char parity, int stopBits);

    /**
     * @brief 获取快速解决方案 (Get quick fixes)
     * @return 解决方案列表 (List of solutions)
     */
    static QStringList getQuickFixes();

    /**
     * @brief 生成系统信息报告 (Generate system information report)
     * @return 系统信息 (System information)
     */
    static QString generateSystemInfo();
}; 