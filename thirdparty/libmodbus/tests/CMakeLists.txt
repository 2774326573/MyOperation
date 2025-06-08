# CMakeLists.txt for libmodbus tests and examples
cmake_minimum_required(VERSION 3.16)

# Find required Qt components for testing
find_package(Qt5 REQUIRED COMPONENTS Core Test SerialPort)

# Test configuration
enable_testing()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/modbus
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/modbus
)

# Test executable
set(TEST_SOURCES
    test_modbus_performance.cpp
)

set(TEST_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/modbus/modbus_performance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/modbus/optimized_modbus_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/modbus/modbus_benchmark.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/modbus/modbusmanager.h
)

set(TEST_IMPLEMENTATION
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/modbus/modbus_performance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/modbus/optimized_modbus_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/modbus/modbus_benchmark.cpp
)

# Create test executable
add_executable(test_modbus_performance
    ${TEST_SOURCES}
    ${TEST_HEADERS}
    ${TEST_IMPLEMENTATION}
)

# Link Qt libraries
target_link_libraries(test_modbus_performance
    Qt5::Core
    Qt5::Test
    Qt5::SerialPort
)

# Add test to CTest
add_test(NAME ModbusPerformanceTests COMMAND test_modbus_performance)

# Example executables
set(EXAMPLE_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../examples)

# Simple optimization example
add_executable(simple_optimization_example
    ${EXAMPLE_SOURCES_DIR}/simple_optimization_example.cpp
    ${TEST_HEADERS}
    ${TEST_IMPLEMENTATION}
)

target_link_libraries(simple_optimization_example
    Qt5::Core
    Qt5::SerialPort
)

# Comprehensive optimization example
add_executable(optimization_example
    ${EXAMPLE_SOURCES_DIR}/optimization_example.cpp
    ${TEST_HEADERS}
    ${TEST_IMPLEMENTATION}
)

target_link_libraries(optimization_example
    Qt5::Core
    Qt5::SerialPort
)

# Benchmark example
add_executable(benchmark_example
    ${EXAMPLE_SOURCES_DIR}/benchmark_example.cpp
    ${TEST_HEADERS}
    ${TEST_IMPLEMENTATION}
)

target_link_libraries(benchmark_example
    Qt5::Core
    Qt5::SerialPort
)

# Set output directories
set_target_properties(test_modbus_performance PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

set_target_properties(simple_optimization_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

set_target_properties(optimization_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

set_target_properties(benchmark_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# Compiler options
if(MSVC)
    target_compile_options(test_modbus_performance PRIVATE /W3)
    target_compile_options(simple_optimization_example PRIVATE /W3)
    target_compile_options(optimization_example PRIVATE /W3)
    target_compile_options(benchmark_example PRIVATE /W3)
else()
    target_compile_options(test_modbus_performance PRIVATE -Wall -Wextra)
    target_compile_options(simple_optimization_example PRIVATE -Wall -Wextra)
    target_compile_options(optimization_example PRIVATE -Wall -Wextra)
    target_compile_options(benchmark_example PRIVATE -Wall -Wextra)
endif()

# Set C++ standard
set_property(TARGET test_modbus_performance PROPERTY CXX_STANDARD 17)
set_property(TARGET simple_optimization_example PROPERTY CXX_STANDARD 17)
set_property(TARGET optimization_example PROPERTY CXX_STANDARD 17)
set_property(TARGET benchmark_example PROPERTY CXX_STANDARD 17)

# Install targets (optional)
install(TARGETS test_modbus_performance DESTINATION bin/tests)
install(TARGETS simple_optimization_example DESTINATION bin/examples)
install(TARGETS optimization_example DESTINATION bin/examples)
install(TARGETS benchmark_example DESTINATION bin/examples)

# Custom targets for running tests and examples
add_custom_target(run_tests
    COMMAND ${CMAKE_BINARY_DIR}/tests/test_modbus_performance
    DEPENDS test_modbus_performance
    COMMENT "Running Modbus performance tests"
)

add_custom_target(run_simple_example
    COMMAND ${CMAKE_BINARY_DIR}/examples/simple_optimization_example
    DEPENDS simple_optimization_example
    COMMENT "Running simple optimization example"
)

add_custom_target(run_optimization_example
    COMMAND ${CMAKE_BINARY_DIR}/examples/optimization_example
    DEPENDS optimization_example
    COMMENT "Running comprehensive optimization example"
)

add_custom_target(run_benchmark
    COMMAND ${CMAKE_BINARY_DIR}/examples/benchmark_example
    DEPENDS benchmark_example
    COMMENT "Running benchmark example"
)

# Documentation target (if Doxygen is available)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME "Modbus Performance Optimization")
    set(DOXYGEN_PROJECT_BRIEF "High-performance Modbus communication library")
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/doc)
    set(DOXYGEN_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../inc/modbus)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    
    doxygen_add_docs(doc
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/modbus
        COMMENT "Generating documentation"
    )
endif()

# Print configuration summary
message(STATUS "Modbus Performance Optimization Build Configuration:")
message(STATUS "  Qt5 Core: ${Qt5Core_VERSION}")
message(STATUS "  Qt5 Test: ${Qt5Test_VERSION}")
message(STATUS "  Qt5 SerialPort: ${Qt5SerialPort_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: 17")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}")
