#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief 串口诊断工具类 (Serial Port Diagnostic Tool)
 * 
 * 用于检测和诊断串口连接问题，提供详细的错误信息和解决建议
 * Used to detect and diagnose serial port connection issues, providing detailed error information and solutions
 */
class SerialDiagnostic : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 串口信息结构体 (Port Information Structure)
     */
    struct PortInfo {
        QString portName;           ///< 端口名称 (Port name)
        QString description;        ///< 端口描述 (Port description)
        QString manufacturer;       ///< 制造商 (Manufacturer)
        QString serialNumber;       ///< 序列号 (Serial number)
        QString systemLocation;     ///< 系统位置 (System location)
        quint16 vendorId;          ///< 厂商ID (Vendor ID)
        quint16 productId;         ///< 产品ID (Product ID)
        bool exists;               ///< 端口是否存在 (Port exists)
        bool accessible;           ///< 端口是否可访问 (Port accessible)
        bool inUse;                ///< 端口是否被占用 (Port in use)
        QString errorString;       ///< 错误信息 (Error message)
        int errorCode;             ///< 错误代码 (Error code)
    };

public:
    explicit SerialDiagnostic(QObject *parent = nullptr);
    ~SerialDiagnostic();

    /**
     * @brief 获取可用串口列表 (Get available ports list)
     * @return 串口名称列表 (List of port names)
     */
    static QStringList getAvailablePorts();

    /**
     * @brief 获取指定串口的详细信息 (Get detailed information of specified port)
     * @param portName 串口名称 (Port name)
     * @return 串口信息 (Port information)
     */
    static PortInfo getPortInfo(const QString &portName);

    /**
     * @brief 测试串口连接 (Test port connection)
     * @param portName 串口名称 (Port name)
     * @param baudRate 波特率 (Baud rate)
     * @param dataBits 数据位 (Data bits)
     * @param parity 校验位 (Parity)
     * @param stopBits 停止位 (Stop bits)
     * @return 测试是否成功 (Test success)
     */
    static bool testPortConnection(const QString &portName, int baudRate, int dataBits, char parity, int stopBits);

    /**
     * @brief 生成诊断报告 (Generate diagnostic report)
     * @return 诊断报告文本 (Diagnostic report text)
     */
    static QString generateDiagnosticReport();

    /**
     * @brief 获取针对特定串口的建议 (Get recommendations for specific port)
     * @param portName 串口名称 (Port name)
     * @return 建议列表 (List of recommendations)
     */
    static QStringList getRecommendations(const QString &portName);
}; 