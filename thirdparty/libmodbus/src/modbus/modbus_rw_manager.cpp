//
// Created by jinxi on 25-5-26.
//

#include "../../inc/modbus/modbus_rw_manager.h"
#include <QStringList>
#include <QRegExp>

ModbusRwManager::ModbusRwManager(QObject *parent)
    : QObject(parent), m_modbusManager(nullptr)
{
}

ModbusRwManager::~ModbusRwManager()
{
}

void ModbusRwManager::setModbusManager(ModbusManager* modbusManager)
{
    m_modbusManager = modbusManager;
}

ModbusRwManager::OperationResult ModbusRwManager::performRead(const OperationParams& params)
{
    OperationResult result;
    
    if (!m_modbusManager) {
        result.message = tr("Modbus管理器未初始化");
        emit errorOccurred(result.message);
        return result;
    }
    
    if (!m_modbusManager->isConnected()) {
        result.message = tr("设备未连接");
        emit errorOccurred(result.message);
        return result;
    }
    
    // 验证参数
    QString validationError = validateParams(params);
    if (!validationError.isEmpty()) {
        result.message = validationError;
        emit errorOccurred(result.message);
        return result;
    }
    
    // 设置从站ID
    m_modbusManager->setSlaveID(params.slaveId);
    
    bool success = false;
    
    switch (params.functionCode) {
    case ReadCoils: {
        QVector<bool> values;
        success = m_modbusManager->readCoils(params.startAddress, params.quantity, values);
        if (success) {
            result.data = QVariant::fromValue(values);
            result.formattedResult = formatReadResult(values);
        }
        break;
    }
    case ReadDiscreteInputs: {
        QVector<bool> values;
        success = m_modbusManager->readDiscreteInputs(params.startAddress, params.quantity, values);
        if (success) {
            result.data = QVariant::fromValue(values);
            result.formattedResult = formatReadResult(values);
        }
        break;
    }
    case ReadHoldingRegisters: {
        QVector<quint16> values;
        success = m_modbusManager->readHoldingRegisters(params.startAddress, params.quantity, values);
        if (success) {
            result.data = QVariant::fromValue(values);
            result.formattedResult = formatReadResult(values);
        }
        break;
    }
    case ReadInputRegisters: {
        QVector<quint16> values;
        success = m_modbusManager->readInputRegisters(params.startAddress, params.quantity, values);
        if (success) {
            result.data = QVariant::fromValue(values);
            result.formattedResult = formatReadResult(values);
        }
        break;
    }
    default:
        result.message = tr("不支持的读取功能码: %1").arg(params.functionCode);
        emit errorOccurred(result.message);
        return result;
    }
    
    result.success = success;
    if (success) {
        result.message = tr("读取成功 - 功能码:%1, 地址:%2, 数量:%3, 结果:%4")
                        .arg(params.functionCode).arg(params.startAddress)
                        .arg(params.quantity).arg(result.formattedResult);
    } else {
        result.message = tr("读取失败 - 功能码:%1, 地址:%2, 数量:%3")
                        .arg(params.functionCode).arg(params.startAddress).arg(params.quantity);
        emit errorOccurred(result.message);
    }
    
    emit operationCompleted(result);
    return result;
}

ModbusRwManager::OperationResult ModbusRwManager::performWrite(const OperationParams& params)
{
    OperationResult result;
    
    if (!m_modbusManager) {
        result.message = tr("Modbus管理器未初始化");
        emit errorOccurred(result.message);
        return result;
    }
    
    if (!m_modbusManager->isConnected()) {
        result.message = tr("设备未连接");
        emit errorOccurred(result.message);
        return result;
    }
    
    // 验证参数
    QString validationError = validateParams(params);
    if (!validationError.isEmpty()) {
        result.message = validationError;
        emit errorOccurred(result.message);
        return result;
    }
    
    if (params.writeValue.isEmpty()) {
        result.message = tr("写入值为空");
        emit errorOccurred(result.message);
        return result;
    }
    
    // 设置从站ID
    m_modbusManager->setSlaveID(params.slaveId);
    
    bool success = false;
    
    switch (params.functionCode) {
    case WriteSingleCoil: {
        bool value = (params.writeValue.toLower() == "true" || params.writeValue == "1");
        success = m_modbusManager->writeSingleCoil(params.startAddress, value);
        break;
    }
    case WriteSingleRegister: {
        bool ok;
        quint16 value = params.writeValue.toUShort(&ok);
        if (!ok) {
            result.message = tr("无效的寄存器值: %1").arg(params.writeValue);
            emit errorOccurred(result.message);
            return result;
        }
        success = m_modbusManager->writeSingleRegister(params.startAddress, value);
        break;
    }
    case WriteMultipleCoils: {
        QVector<bool> values = parseWriteBoolValues(params.writeValue, params.quantity);
        if (values.isEmpty()) {
            result.message = tr("无效的线圈值格式");
            emit errorOccurred(result.message);
            return result;
        }
        success = m_modbusManager->writeMultipleCoils(params.startAddress, values);
        break;
    }
    case WriteMultipleRegisters: {
        QVector<quint16> values = parseWriteValues(params.writeValue, params.quantity);
        if (values.isEmpty()) {
            result.message = tr("无效的寄存器值格式");
            emit errorOccurred(result.message);
            return result;
        }
        success = m_modbusManager->writeMultipleRegisters(params.startAddress, values);
        break;
    }
    default:
        result.message = tr("不支持的写入功能码: %1").arg(params.functionCode);
        emit errorOccurred(result.message);
        return result;
    }
    
    result.success = success;
    if (success) {
        result.message = tr("写入成功 - 功能码:%1, 地址:%2, 值:%3")
                        .arg(params.functionCode).arg(params.startAddress).arg(params.writeValue);
    } else {
        result.message = tr("写入失败 - 功能码:%1, 地址:%2, 值:%3")
                        .arg(params.functionCode).arg(params.startAddress).arg(params.writeValue);
        emit errorOccurred(result.message);
    }
    
    emit operationCompleted(result);
    return result;
}

bool ModbusRwManager::isReadOperation(FunctionCode functionCode)
{
    return (functionCode == ReadCoils || 
            functionCode == ReadDiscreteInputs || 
            functionCode == ReadHoldingRegisters || 
            functionCode == ReadInputRegisters);
}

bool ModbusRwManager::isWriteOperation(FunctionCode functionCode)
{
    return (functionCode == WriteSingleCoil || 
            functionCode == WriteSingleRegister || 
            functionCode == WriteMultipleCoils || 
            functionCode == WriteMultipleRegisters);
}

bool ModbusRwManager::isSingleOperation(FunctionCode functionCode)
{
    return (functionCode == WriteSingleCoil || functionCode == WriteSingleRegister);
}

bool ModbusRwManager::isCoilOperation(FunctionCode functionCode)
{
    return (functionCode == ReadCoils || 
            functionCode == ReadDiscreteInputs || 
            functionCode == WriteSingleCoil || 
            functionCode == WriteMultipleCoils);
}

QString ModbusRwManager::validateParams(const OperationParams& params)
{
    // 验证从站ID
    if (params.slaveId < 1 || params.slaveId > 247) {
        return tr("从站ID必须在1-247范围内");
    }
    
    // 验证地址
    if (params.startAddress < 0 || params.startAddress > 65535) {
        return tr("起始地址必须在0-65535范围内");
    }
    
    // 验证数量
    if (params.quantity < 1 || params.quantity > 125) {
        return tr("数量必须在1-125范围内");
    }
    
    // 对于写入操作，验证功能码
    if (isWriteOperation(params.functionCode)) {
        if (!isWriteOperation(params.functionCode)) {
            return tr("功能码 %1 不支持写入操作").arg(params.functionCode);
        }
    } else if (isReadOperation(params.functionCode)) {
        // 读取操作验证通过
    } else {
        return tr("无效的功能码: %1").arg(params.functionCode);
    }
    
    return QString(); // 验证通过
}

QString ModbusRwManager::getFunctionCodeName(FunctionCode functionCode)
{
    switch (functionCode) {
    case ReadCoils:
        return tr("01-读线圈");
    case ReadDiscreteInputs:
        return tr("02-读离散输入");
    case ReadHoldingRegisters:
        return tr("03-读保持寄存器");
    case ReadInputRegisters:
        return tr("04-读输入寄存器");
    case WriteSingleCoil:
        return tr("05-写单个线圈");
    case WriteSingleRegister:
        return tr("06-写单个寄存器");
    case WriteMultipleCoils:
        return tr("15-写多个线圈");
    case WriteMultipleRegisters:
        return tr("16-写多个寄存器");
    default:
        return tr("未知功能码");
    }
}

ModbusRwManager::FunctionCode ModbusRwManager::getFunctionCodeFromIndex(int index)
{
    static const QVector<FunctionCode> functionCodes = {
        ReadCoils,              // 0
        ReadDiscreteInputs,     // 1
        ReadHoldingRegisters,   // 2
        ReadInputRegisters,     // 3
        WriteSingleCoil,        // 4
        WriteSingleRegister,    // 5
        WriteMultipleCoils,     // 6
        WriteMultipleRegisters  // 7
    };
    
    if (index >= 0 && index < functionCodes.size()) {
        return functionCodes[index];
    }
    return ReadHoldingRegisters; // 默认值
}

int ModbusRwManager::getIndexFromFunctionCode(FunctionCode functionCode)
{
    switch (functionCode) {
    case ReadCoils: return 0;
    case ReadDiscreteInputs: return 1;
    case ReadHoldingRegisters: return 2;
    case ReadInputRegisters: return 3;
    case WriteSingleCoil: return 4;
    case WriteSingleRegister: return 5;
    case WriteMultipleCoils: return 6;
    case WriteMultipleRegisters: return 7;
    default: return 2; // 默认为读保持寄存器
    }
}

QString ModbusRwManager::formatReadResult(const QVector<quint16>& values)
{
    QStringList result;
    for (quint16 value : values) {
        result << QString::number(value);
    }
    return result.join(", ");
}

QString ModbusRwManager::formatReadResult(const QVector<bool>& values)
{
    QStringList result;
    for (bool value : values) {
        result << (value ? "1" : "0");
    }
    return result.join(", ");
}

QVector<quint16> ModbusRwManager::parseWriteValues(const QString& input, int count)
{
    QVector<quint16> values;
    QStringList parts = input.split(',');
    
    for (const QString& part : parts) {
        bool ok;
        quint16 value = part.trimmed().toUShort(&ok);
        if (ok) {
            values.append(value);
        } else {
            return QVector<quint16>(); // 返回空向量表示解析失败
        }
    }
    
    // 如果只输入了一个值但需要多个，则重复该值
    if (values.size() == 1 && count > 1) {
        quint16 singleValue = values.first();
        values.clear();
        for (int i = 0; i < count; ++i) {
            values.append(singleValue);
        }
    }
    
    return values;
}

QVector<bool> ModbusRwManager::parseWriteBoolValues(const QString& input, int count)
{
    QVector<bool> values;
    QStringList parts = input.split(',');
    
    for (const QString& part : parts) {
        QString trimmed = part.trimmed().toLower();
        if (trimmed == "true" || trimmed == "1") {
            values.append(true);
        } else if (trimmed == "false" || trimmed == "0") {
            values.append(false);
        } else {
            return QVector<bool>(); // 返回空向量表示解析失败
        }
    }
    
    // 如果只输入了一个值但需要多个，则重复该值
    if (values.size() == 1 && count > 1) {
        bool singleValue = values.first();
        values.clear();
        for (int i = 0; i < count; ++i) {
            values.append(singleValue);
        }
    }
    
    return values;
} 