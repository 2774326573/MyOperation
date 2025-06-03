# 配置管理器 CMakeLists.txt / Config Manager CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(ConfigManager
    VERSION 1.0.0
    DESCRIPTION "Configuration Management Library"
    LANGUAGES CXX
)

# 查找Qt组件 / Find Qt components
find_package(Qt5 REQUIRED COMPONENTS Core)

# 设置Qt自动处理 / Set Qt automatic processing
set(CMAKE_AUTOMOC ON)

# 包含目录 / Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${Qt5Core_INCLUDE_DIRS}
)

# 查找源文件 / Find source files
file(GLOB_RECURSE CONFIG_SOURCES
    "src/*.cpp"
    "src/*.c"
)

file(GLOB_RECURSE CONFIG_HEADERS
    "inc/*.h"
    "inc/*.hpp"
)

# 创建静态库 / Create static library
add_library(ConfigManager STATIC
    ${CONFIG_SOURCES}
    ${CONFIG_HEADERS}
)

# 链接Qt库 / Link Qt libraries
target_link_libraries(ConfigManager
    Qt5::Core
)

# 设置目标属性 / Set target properties
set_target_properties(ConfigManager PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${CONFIG_HEADERS}"
)

# 导出包含目录 / Export include directories
target_include_directories(ConfigManager PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:include/config>
)

# 编译定义 / Compile definitions
target_compile_definitions(ConfigManager PRIVATE
    QT_NO_KEYWORDS
    QT_DEPRECATED_WARNINGS
)

# 安装规则 / Install rules
install(TARGETS ConfigManager
    EXPORT ConfigManagerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/config
) 