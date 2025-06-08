//
// Created by jinxi on 25-5-26.
//

#ifndef MODBUS_RW_MANAGER_H
#define MODBUS_RW_MANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>
#include "modbusmanager.h"

/**
 * @brief Modbus读写操作管理器
 * 负责处理Modbus读写的业务逻辑，与UI分离
 */
class ModbusRwManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Modbus功能码枚举
     */
    enum FunctionCode {
        ReadCoils = 1,              // 01 - 读线圈
        ReadDiscreteInputs = 2,     // 02 - 读离散输入
        ReadHoldingRegisters = 3,   // 03 - 读保持寄存器
        ReadInputRegisters = 4,     // 04 - 读输入寄存器
        WriteSingleCoil = 5,        // 05 - 写单个线圈
        WriteSingleRegister = 6,    // 06 - 写单个寄存器
        WriteMultipleCoils = 15,    // 15 - 写多个线圈
        WriteMultipleRegisters = 16 // 16 - 写多个寄存器
    };

    /**
     * @brief 读写操作参数结构体
     */
    struct OperationParams {
        FunctionCode functionCode;  // 功能码
        int slaveId;               // 从站ID
        int startAddress;          // 起始地址
        int quantity;              // 数量
        QString writeValue;        // 写入值（仅写操作使用）
        
        OperationParams() : functionCode(ReadHoldingRegisters), slaveId(1), 
                           startAddress(0), quantity(1) {}
    };

    /**
     * @brief 操作结果结构体
     */
    struct OperationResult {
        bool success;              // 操作是否成功
        QString message;           // 结果消息
        QVariant data;             // 读取的数据（仅读操作）
        QString formattedResult;   // 格式化的结果字符串
        
        OperationResult() : success(false) {}
    };

public:
    explicit ModbusRwManager(QObject *parent = nullptr);
    ~ModbusRwManager();

    /**
     * @brief 设置Modbus管理器
     * @param modbusManager Modbus管理器实例
     */
    void setModbusManager(ModbusManager* modbusManager);

    /**
     * @brief 执行读取操作
     * @param params 操作参数
     * @return 操作结果
     */
    OperationResult performRead(const OperationParams& params);

    /**
     * @brief 执行写入操作
     * @param params 操作参数
     * @return 操作结果
     */
    OperationResult performWrite(const OperationParams& params);

    /**
     * @brief 检查功能码是否为读取操作
     * @param functionCode 功能码
     * @return 是否为读取操作
     */
    static bool isReadOperation(FunctionCode functionCode);

    /**
     * @brief 检查功能码是否为写入操作
     * @param functionCode 功能码
     * @return 是否为写入操作
     */
    static bool isWriteOperation(FunctionCode functionCode);

    /**
     * @brief 检查功能码是否为单个操作
     * @param functionCode 功能码
     * @return 是否为单个操作
     */
    static bool isSingleOperation(FunctionCode functionCode);

    /**
     * @brief 检查功能码是否为线圈操作
     * @param functionCode 功能码
     * @return 是否为线圈操作
     */
    static bool isCoilOperation(FunctionCode functionCode);

    /**
     * @brief 验证操作参数
     * @param params 操作参数
     * @return 验证结果消息，空字符串表示验证通过
     */
    QString validateParams(const OperationParams& params);

    /**
     * @brief 获取功能码的显示名称
     * @param functionCode 功能码
     * @return 显示名称
     */
    static QString getFunctionCodeName(FunctionCode functionCode);

    /**
     * @brief 从索引获取功能码
     * @param index UI下拉框索引
     * @return 功能码
     */
    static FunctionCode getFunctionCodeFromIndex(int index);

    /**
     * @brief 从功能码获取索引
     * @param functionCode 功能码
     * @return UI下拉框索引
     */
    static int getIndexFromFunctionCode(FunctionCode functionCode);

    // ========== 串口诊断功能 (Serial Diagnostic Functions) ==========
    /**
     * @brief 获取可用串口列表
     * @return 串口名称列表
     */
    static QStringList getAvailablePorts();

    /**
     * @brief 获取串口诊断报告
     * @param portName 串口名称
     * @return 诊断报告
     */
    static QString getSerialDiagnosticReport(const QString &portName = QString());

    /**
     * @brief 获取串口建议
     * @param portName 串口名称
     * @return 建议列表
     */
    static QStringList getSerialRecommendations(const QString &portName);

    /**
     * @brief 获取快速解决方案
     * @return 解决方案列表
     */
    static QStringList getQuickFixes();

signals:
    /**
     * @brief 操作完成信号
     * @param result 操作结果
     */
    void operationCompleted(const OperationResult& result);

    /**
     * @brief 错误发生信号
     * @param message 错误消息
     */
    void errorOccurred(const QString& message);

private:
    /**
     * @brief 格式化读取结果
     * @param values 寄存器值
     * @return 格式化字符串
     */
    QString formatReadResult(const QVector<quint16>& values);

    /**
     * @brief 格式化读取结果
     * @param values 线圈值
     * @return 格式化字符串
     */
    QString formatReadResult(const QVector<bool>& values);

    /**
     * @brief 解析写入值
     * @param input 输入字符串
     * @param count 数量
     * @return 寄存器值向量
     */
    QVector<quint16> parseWriteValues(const QString& input, int count);

    /**
     * @brief 解析布尔写入值
     * @param input 输入字符串
     * @param count 数量
     * @return 布尔值向量
     */
    QVector<bool> parseWriteBoolValues(const QString& input, int count);

private:
    ModbusManager* m_modbusManager;  // Modbus管理器
};

#endif // MODBUS_RW_MANAGER_H