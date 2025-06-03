# 不同编译模式下的日志行为

在软件开发过程中，针对不同的编译模式(DEBUG/RELEASE)，日志的行为应该有所区别。本文档介绍如何在分类日志系统中有效地利用这一特性。

## DEBUG模式下的日志行为

在DEBUG模式下，通常需要更详细的日志信息，包括调试信息，并且同时输出到终端和文件：

```cpp
// 在DEBUG模式下的宏定义
#ifdef _DEBUG
#define LOG_DEBUG(category, message) SIMPLE_DEBUG_LOG_DEBUG(category, message)
#define LOG_INFO(category, message) SIMPLE_DEBUG_LOG_INFO(category, message)
#define LOG_WARNING(category, message) SIMPLE_DEBUG_LOG_WARNING(category, message)
#define LOG_ERROR(category, message) SIMPLE_DEBUG_LOG_ERROR(category, message)
#define LOG_ASSERT(category, message) SIMPLE_ASSERT(true, message)
#endif
```

这样配置的优势：
- 调试信息同时显示在控制台和写入文件，便于实时监控
- 开发过程中可以快速查看日志输出
- 断言功能在DEBUG模式下完全启用

## RELEASE模式下的日志行为

在RELEASE模式下，日志应当更加精简，减少资源占用，提高性能：

```cpp
// 在RELEASE模式下的宏定义
#ifndef _DEBUG
#define LOG_INFO(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::INFO, false, true)
#define LOG_WARNING(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::WARNING, false, true)
#define LOG_ERROR(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::ERR, false, true)
#define LOG_DEBUG(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::DEBUG, false, true)
#define LOG_TRACE(category, message) SIMPLE_LOGGER.logWithConfig(category, message, LogLevel::TRACE, false, true)
#endif
```

这样配置的优势：
- 日志仅写入文件，不显示在终端，减少I/O和UI线程的阻塞
- 保留完整日志记录功能，便于问题追踪
- 优化性能，减少资源占用

## 使用实例

通过这样的配置，在不同编译模式下使用相同的日志接口，能够自动适应不同的环境需求：

```cpp
// 在代码中使用日志
void processUserData(const UserData& data) {
    LOG_DEBUG("UserData", "开始处理用户数据");
    
    try {
        // 处理逻辑...
        LOG_INFO("UserData", "用户数据处理完成");
    } catch (const std::exception& e) {
        LOG_ERROR("UserData", QString("处理用户数据时出错: %1").arg(e.what()));
    }
}
```

无论在DEBUG还是RELEASE模式下，上述代码都能正常运行，只是日志的行为会根据编译模式自动调整。

## 性能优化建议

- **DEBUG模式**：可以适当增加日志输出，帮助诊断问题
- **RELEASE模式**：减少DEBUG和TRACE级别的日志记录，只保留关键节点的INFO和ERROR日志

这种区分编译模式的日志策略，能够在保证开发效率的同时，不会影响产品的运行性能。
