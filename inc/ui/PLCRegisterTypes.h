#ifndef PLC_REGISTER_TYPES_H
#define PLC_REGISTER_TYPES_H

#include <QString>
#include <QMap>

/**
 * @brief PLC寄存器类型枚举
 */
enum class PLCRegisterType
{
    X_INPUT,        // X点 - 输入点
    Y_OUTPUT,       // Y点 - 输出点  
    D_DATA,         // D寄存器 - 数据寄存器
    M_INTERNAL,     // M寄存器 - 内部继电器
    HOLDING_REG,    // Modbus保持寄存器
    INPUT_REG,      // Modbus输入寄存器
    COILS,          // Modbus线圈
    DISCRETE_INPUT  // Modbus离散输入
};

/**
 * @brief PLC寄存器管理工具类
 */
class PLCRegisterHelper
{
public:
    /**
     * @brief 从字符串获取寄存器类型
     * @param typeStr 类型字符串
     * @return 寄存器类型
     */
    static PLCRegisterType getTypeFromString(const QString& typeStr);
    
    /**
     * @brief 获取寄存器类型的显示名称
     * @param type 寄存器类型
     * @return 显示名称
     */
    static QString getTypeName(PLCRegisterType type);
    
    /**
     * @brief 解析PLC地址
     * @param address 地址字符串 (如: X00, Y17, D100, M500)
     * @param type 寄存器类型
     * @param modbusAddr 输出的Modbus地址
     * @return 解析是否成功
     */
    static bool parseAddress(const QString& address, PLCRegisterType type, int& modbusAddr);
    
    /**
     * @brief 将Modbus地址转换为PLC地址显示
     * @param modbusAddr Modbus地址
     * @param type 寄存器类型
     * @param index 索引偏移
     * @return PLC地址字符串
     */
    static QString formatAddress(int modbusAddr, PLCRegisterType type, int index = 0);
    
    /**
     * @brief 检查地址是否在有效范围内
     * @param address 地址值
     * @param type 寄存器类型
     * @return 是否有效
     */
    static bool isValidAddress(int address, PLCRegisterType type);
    
    /**
     * @brief 获取寄存器类型的最大地址
     * @param type 寄存器类型
     * @return 最大地址值
     */
    static int getMaxAddress(PLCRegisterType type);
    
    /**
     * @brief 获取所有支持的寄存器类型列表
     * @return 类型名称列表
     */
    static QStringList getSupportedTypes();

private:
    static QMap<QString, PLCRegisterType> s_typeMap;
    static QMap<PLCRegisterType, QString> s_nameMap;
    
    static void initializeMaps();
};

#endif // PLC_REGISTER_TYPES_H
