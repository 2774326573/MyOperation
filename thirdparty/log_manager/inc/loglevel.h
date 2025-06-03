#ifndef LOGLEVEL_H
#define LOGLEVEL_H

#include <QString>

/**
 * @brief 日志级别枚举 | Log level enumeration
 */
enum class LogLevel {
    TRACE,   ///< 最详细的跟踪信息
    DEBUG,   ///< 调试信息
    INFO,    ///< 普通信息
    WARNING, ///< 警告信息
    ERR,     ///< 错误信息
    FATAL    ///< 致命错误信息
};

/**
 * @brief 将LogLevel转换为可读字符串 | Convert LogLevel to readable string
 * @param level 日志级别 | Log level
 * @return 对应的字符串表示 | Corresponding string representation
 */
inline QString logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE:   return QStringLiteral("TRACE");
        case LogLevel::DEBUG:   return QStringLiteral("DEBUG");
        case LogLevel::INFO:    return QStringLiteral("INFO");
        case LogLevel::WARNING: return QStringLiteral("WARNING");
        case LogLevel::ERR:     return QStringLiteral("ERR");
        case LogLevel::FATAL:   return QStringLiteral("FATAL");
        default:               return QStringLiteral("UNKNOWN");
    }
}

/**
 * @brief 将字符串转换为LogLevel | Convert string to LogLevel
 * @param str 日志级别字符串 | Log level string
 * @return 对应的LogLevel | Corresponding LogLevel
 */
inline LogLevel stringToLogLevel(const QString& str) {
    QString upperStr = str.toUpper();
    if (upperStr == QLatin1String("TRACE"))   return LogLevel::TRACE;
    if (upperStr == QLatin1String("DEBUG"))   return LogLevel::DEBUG;
    if (upperStr == QLatin1String("INFO"))    return LogLevel::INFO;
    if (upperStr == QLatin1String("WARNING")) return LogLevel::WARNING;
    if (upperStr == QLatin1String("ERR"))     return LogLevel::ERR;
    if (upperStr == QLatin1String("FATAL"))   return LogLevel::FATAL;
    return LogLevel::INFO; // Default to INFO for unknown values
}

#endif // LOGLEVEL_H
