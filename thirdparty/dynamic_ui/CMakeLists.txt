# 动态UI构建器库 / Dynamic UI Builder Library
cmake_minimum_required(VERSION 3.16)

project(DynamicUI VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准 / Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt组件 / Find Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 设置Qt自动处理 / Set Qt automatic processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 头文件目录 / Header directories
set(DYNAMIC_UI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件 / Source files
set(DYNAMIC_UI_SOURCES
    src/DynamicUIBuilder.cpp
)

# 头文件 / Header files
set(DYNAMIC_UI_HEADERS
    include/DynamicUIBuilder.h
)

# 创建库 / Create library
add_library(DynamicUI STATIC ${DYNAMIC_UI_SOURCES} ${DYNAMIC_UI_HEADERS})

# 设置目标属性 / Set target properties
target_include_directories(DynamicUI
    PUBLIC
        $<BUILD_INTERFACE:${DYNAMIC_UI_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接Qt库 / Link Qt libraries
target_link_libraries(DynamicUI
    PUBLIC
        Qt6::Core
        Qt6::Widgets
)

# 设置编译定义 / Set compile definitions
target_compile_definitions(DynamicUI
    PRIVATE
        QT_NO_KEYWORDS
)

# 导出目标 / Export targets
set_target_properties(DynamicUI PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    EXPORT_NAME DynamicUI
)

# 安装规则 / Install rules
install(TARGETS DynamicUI
    EXPORT DynamicUITargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${DYNAMIC_UI_HEADERS}
    DESTINATION include/DynamicUI
)

# 导出配置 / Export configuration
install(EXPORT DynamicUITargets
    FILE DynamicUITargets.cmake
    NAMESPACE DynamicUI::
    DESTINATION lib/cmake/DynamicUI
)

# 创建配置文件 / Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    DynamicUIConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/DynamicUIConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DynamicUIConfig.cmake
    INSTALL_DESTINATION lib/cmake/DynamicUI
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DynamicUIConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DynamicUIConfigVersion.cmake
    DESTINATION lib/cmake/DynamicUI
) 