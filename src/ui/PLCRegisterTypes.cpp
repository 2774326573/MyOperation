#include "../inc/ui/PLCRegisterTypes.h"

QMap<QString, PLCRegisterType> PLCRegisterHelper::s_typeMap;
QMap<PLCRegisterType, QString> PLCRegisterHelper::s_nameMap;

void PLCRegisterHelper::initializeMaps()
{
    if (!s_typeMap.isEmpty()) return;
    
    // 初始化类型映射
    s_typeMap["X点"] = PLCRegisterType::X_INPUT;
    s_typeMap["X Points"] = PLCRegisterType::X_INPUT;
    s_typeMap["输入点"] = PLCRegisterType::X_INPUT;
    s_typeMap["Input Points"] = PLCRegisterType::X_INPUT;
    
    s_typeMap["Y点"] = PLCRegisterType::Y_OUTPUT;
    s_typeMap["Y Points"] = PLCRegisterType::Y_OUTPUT;
    s_typeMap["输出点"] = PLCRegisterType::Y_OUTPUT;
    s_typeMap["Output Points"] = PLCRegisterType::Y_OUTPUT;
    
    s_typeMap["D寄存器"] = PLCRegisterType::D_DATA;
    s_typeMap["D Registers"] = PLCRegisterType::D_DATA;
    s_typeMap["数据寄存器"] = PLCRegisterType::D_DATA;
    s_typeMap["Data Registers"] = PLCRegisterType::D_DATA;
    
    s_typeMap["M寄存器"] = PLCRegisterType::M_INTERNAL;
    s_typeMap["M Registers"] = PLCRegisterType::M_INTERNAL;
    s_typeMap["内部继电器"] = PLCRegisterType::M_INTERNAL;
    s_typeMap["Internal Relays"] = PLCRegisterType::M_INTERNAL;
    
    s_typeMap["Holding Registers"] = PLCRegisterType::HOLDING_REG;
    s_typeMap["保持寄存器"] = PLCRegisterType::HOLDING_REG;
    
    s_typeMap["Input Registers"] = PLCRegisterType::INPUT_REG;
    s_typeMap["输入寄存器"] = PLCRegisterType::INPUT_REG;
    
    s_typeMap["Coils"] = PLCRegisterType::COILS;
    s_typeMap["线圈"] = PLCRegisterType::COILS;
    
    s_typeMap["Discrete Inputs"] = PLCRegisterType::DISCRETE_INPUT;
    s_typeMap["离散输入"] = PLCRegisterType::DISCRETE_INPUT;
    
    // 初始化名称映射
    s_nameMap[PLCRegisterType::X_INPUT] = "X点 (输入点)";
    s_nameMap[PLCRegisterType::Y_OUTPUT] = "Y点 (输出点)";
    s_nameMap[PLCRegisterType::D_DATA] = "D寄存器 (数据寄存器)";
    s_nameMap[PLCRegisterType::M_INTERNAL] = "M寄存器 (内部继电器)";
    s_nameMap[PLCRegisterType::HOLDING_REG] = "Holding Registers (保持寄存器)";
    s_nameMap[PLCRegisterType::INPUT_REG] = "Input Registers (输入寄存器)";
    s_nameMap[PLCRegisterType::COILS] = "Coils (线圈)";
    s_nameMap[PLCRegisterType::DISCRETE_INPUT] = "Discrete Inputs (离散输入)";
}

PLCRegisterType PLCRegisterHelper::getTypeFromString(const QString& typeStr)
{
    initializeMaps();
    return s_typeMap.value(typeStr, PLCRegisterType::HOLDING_REG);
}

QString PLCRegisterHelper::getTypeName(PLCRegisterType type)
{
    initializeMaps();
    return s_nameMap.value(type, "Unknown");
}

bool PLCRegisterHelper::parseAddress(const QString& address, PLCRegisterType type, int& modbusAddr)
{
    QString addr = address.trimmed().toUpper();
    
    switch (type)
    {
    case PLCRegisterType::X_INPUT:
        if (addr.startsWith("X"))
        {
            QString octalStr = addr.mid(1);
            bool ok;
            modbusAddr = octalStr.toInt(&ok, 8); // 八进制转换
            return ok && modbusAddr >= 0 && modbusAddr <= 255; // X00-X377(八进制)
        }
        break;
        
    case PLCRegisterType::Y_OUTPUT:
        if (addr.startsWith("Y"))
        {
            QString octalStr = addr.mid(1);
            bool ok;
            modbusAddr = octalStr.toInt(&ok, 8); // 八进制转换
            return ok && modbusAddr >= 0 && modbusAddr <= 255; // Y00-Y377(八进制)
        }
        break;
        
    case PLCRegisterType::D_DATA:
        if (addr.startsWith("D"))
        {
            QString numStr = addr.mid(1);
            bool ok;
            modbusAddr = numStr.toInt(&ok);
            return ok && modbusAddr >= 0 && modbusAddr <= 7999; // D0-D7999
        }
        break;
        
    case PLCRegisterType::M_INTERNAL:
        if (addr.startsWith("M"))
        {
            QString numStr = addr.mid(1);
            bool ok;
            int mAddr = numStr.toInt(&ok);
            if (ok && mAddr >= 0 && mAddr <= 7999) // M0-M7999
            {
                modbusAddr = mAddr + 1000; // 地址偏移避免与Y点冲突
                return true;
            }
        }
        break;
        
    default:
        // 对于标准Modbus类型，直接解析数字
        bool ok;
        modbusAddr = addr.toInt(&ok);
        return ok && modbusAddr >= 0;
    }
    
    return false;
}

QString PLCRegisterHelper::formatAddress(int modbusAddr, PLCRegisterType type, int index)
{
    switch (type)
    {
    case PLCRegisterType::X_INPUT:
        return QString("X%1").arg(modbusAddr + index, 2, 8, QChar('0')); // 八进制显示
        
    case PLCRegisterType::Y_OUTPUT:
        return QString("Y%1").arg(modbusAddr + index, 2, 8, QChar('0')); // 八进制显示
        
    case PLCRegisterType::D_DATA:
        return QString("D%1").arg(modbusAddr + index);
        
    case PLCRegisterType::M_INTERNAL:
        return QString("M%1").arg((modbusAddr - 1000) + index); // 减去偏移
        
    default:
        return QString("%1").arg(modbusAddr + index);
    }
}

bool PLCRegisterHelper::isValidAddress(int address, PLCRegisterType type)
{
    switch (type)
    {
    case PLCRegisterType::X_INPUT:
    case PLCRegisterType::Y_OUTPUT:
        return address >= 0 && address <= 255; // 八进制00-377
        
    case PLCRegisterType::D_DATA:
    case PLCRegisterType::M_INTERNAL:
        return address >= 0 && address <= 7999;
        
    default:
        return address >= 0 && address <= 65535; // 标准Modbus范围
    }
}

int PLCRegisterHelper::getMaxAddress(PLCRegisterType type)
{
    switch (type)
    {
    case PLCRegisterType::X_INPUT:
    case PLCRegisterType::Y_OUTPUT:
        return 255; // 八进制377 = 十进制255
        
    case PLCRegisterType::D_DATA:
    case PLCRegisterType::M_INTERNAL:
        return 7999;
        
    default:
        return 65535;
    }
}

QStringList PLCRegisterHelper::getSupportedTypes()
{
    initializeMaps();
    return QStringList{
        "X点 (输入点)",
        "Y点 (输出点)", 
        "D寄存器 (数据寄存器)",
        "M寄存器 (内部继电器)",
        "Holding Registers (保持寄存器)",
        "Input Registers (输入寄存器)",
        "Coils (线圈)",
        "Discrete Inputs (离散输入)"
    };
}
