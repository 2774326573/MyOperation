cmake_minimum_required(VERSION 3.31)
project(MyOperation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置AUTOMOC搜索路径，包含thirdparty目录
set(CMAKE_AUTOMOC_PATH_PREFIX ON)

set_property(DIRECTORY PROPERTY AUTOMOC_SOURCE_PATH $
        {CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/include
)

# Windows平台下处理中文编码
if(WIN32)
    # 添加Unicode编译定义
    add_definitions(-DUNICODE -D_UNICODE)

    # 设置默认代码页为UTF-8
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    
    # 添加Visual Studio标准库的包含路径
    if(MSVC)
        # 添加编译选项以处理MSVC特定的问题
        add_compile_options(/wd4828) # 禁用关于无效字符的警告
    endif()
endif()

# 设置Qt国际化支持
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_USE_QSTRINGBUILDER")


# 设置MSVC编译器支持UTF-8
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 检测Qt路径
if(NOT DEFINED CMAKE_PREFIX_PATH)
    # 常见的Qt安装路径
    set(POSSIBLE_QT_PATHS
        "D:/install/Qt/Qt5.12/5.12.12/msvc2017_64"
        "C:/Qt/Qt5.12.12/5.12.12/msvc2017_64"
        "C:/Qt/5.12.12/msvc2017_64"
        "D:/Qt/Qt5.12/5.12.12/msvc2017_64"
        "D:/Qt/5.12.12/msvc2017_64"
    )
    
    foreach(QT_PATH ${POSSIBLE_QT_PATHS})
        if(EXISTS "${QT_PATH}")
            set(CMAKE_PREFIX_PATH "${QT_PATH}")
            message(STATUS "找到Qt安装路径: ${QT_PATH}")
            break()
        endif()
    endforeach()
    
    if(NOT DEFINED CMAKE_PREFIX_PATH)
        message(FATAL_ERROR "未找到Qt安装路径，请手动指定: -DCMAKE_PREFIX_PATH=<path_to_qt>")
    endif()
endif()

# 添加AUTOUIC支持
set(CMAKE_AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/ui
)

# 添加头文件目录
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/include
)

# 收集源文件和头文件
file(GLOB_RECURSE SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/log_manager/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dynamic_ui/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/config/src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/log_manager/inc/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dynamic_ui/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/config/inc/*.h
)

# 添加QXlsx库头文件
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
         ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/include
)

# Resource files
file(GLOB_RECURSE QRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/resources/*.qrc
)

file(GLOB_RECURSE UI_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/ui/*.ui
)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Network
        SerialPort
        SerialBus
        Charts
        Network
        Svg
        Sql
        PrintSupport
        WebSockets
        Concurrent
        LinguistTools
        REQUIRED)

# 可选的测试模块
find_package(Qt5Test QUIET)

add_executable(${PROJECT_NAME} 
    main.cpp
    ${QRC_FILES}
    ${UI_FILES}
    ${SOURCE_FILES}
    ${HEADER_FILES}
    # 显式添加HalconLable以确保MOC处理
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/include/HalconLable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/src/HalconLable.cpp
)

# 添加HALCON库路径
link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/lib
)

# 现在包含通信模块，因为目标已经创建
#include(${CMAKE_SOURCE_DIR}/cmake/communication_module.cmake)

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        Qt5::SerialPort
        Qt5::SerialBus
        Qt5::Charts
        Qt5::Network
        Qt5::Svg
        Qt5::Sql
        Qt5::PrintSupport
)

# 链接第三方库
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/lib/halconcpp.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/lib/halcon.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hdevelop/lib/hdevenginecpp.lib
    )
endif()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets Network SerialPort)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
