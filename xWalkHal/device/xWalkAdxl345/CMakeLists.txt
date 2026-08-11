cmake_minimum_required(VERSION 3.16)
project(xWalkAdxl345 LANGUAGES CXX)

option(XWALK_ADXL345_BUILD_HOST_TESTS "Build xWalkAdxl345 host tests" OFF)
option(XWALK_ADXL345_BUILD_HARDWARE_TESTS "Build xWalkAdxl345 hardware tests" OFF)

set(XWALK_I2C_BUILD_HOST_TESTS ${XWALK_ADXL345_BUILD_HOST_TESTS})
set(XWALK_I2C_BUILD_HARDWARE_TESTS ${XWALK_ADXL345_BUILD_HARDWARE_TESTS})

if(XWALK_ADXL345_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkAdxl345 hardware tests require Linux i2c-dev")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkI2c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkI2c ${CMAKE_CURRENT_BINARY_DIR}/xWalkI2c)
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkAdxl345 STATIC
    src/xHal_Rpi5CarAdxl345.cpp
    src/xHal_Rpi5CarAdxl345Lifecycle.cpp
)

target_include_directories(xWalkAdxl345 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAdxl345 PUBLIC cxx_std_17)
target_link_libraries(xWalkAdxl345 PUBLIC xWalkLibraryCommon xWalkI2c PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAdxl345 PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ADXL345_BUILD_HOST_TESTS OR XWALK_ADXL345_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_ADXL345_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_ADXL345_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_ADXL345_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarAdxl345TraceConfig.py)
    set(XWALK_ADXL345_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAdxl345Trace.xml)
    add_custom_command(OUTPUT ${XWALK_ADXL345_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_ADXL345_TRACE_GENERATOR}
            --input ${XWALK_ADXL345_TRACE_INVENTORY} --output ${XWALK_ADXL345_TRACE_CONFIG}
        DEPENDS ${XWALK_ADXL345_TRACE_INVENTORY} ${XWALK_ADXL345_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkAdxl345 trace catalogue" VERBATIM)
    add_custom_target(xWalkAdxl345TestTraceConfig ALL DEPENDS ${XWALK_ADXL345_TRACE_CONFIG})
    add_dependencies(xWalkAdxl345TestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkAdxl345Test
        test/src/xHal_Rpi5CarAdxl345Test.cpp
        test/src/xHal_Rpi5CarAdxl345TestSupport.cpp
        simulation/src/xHal_Rpi5CarAdxl345SimulationArguments.cpp)
    target_include_directories(xWalkAdxl345Test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_ADXL345_TEST_TRACE_LOG
        ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkAdxl345Test.log)
    target_compile_definitions(xWalkAdxl345Test PRIVATE
        XWALK_ADXL345_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ADXL345_TRACE_CONFIG}"
        XWALK_ADXL345_SIMULATION_TRACE_LOG_PATH="${XWALK_ADXL345_TEST_TRACE_LOG}")
    target_link_libraries(xWalkAdxl345Test PRIVATE xWalkAdxl345 xWalkTrace)
    add_dependencies(xWalkAdxl345Test xWalkAdxl345TestTraceConfig)
    add_test(NAME xWalkAdxl345HostTest COMMAND xWalkAdxl345Test)
    set_tests_properties(xWalkAdxl345HostTest PROPERTIES LABELS "host")
endif()

if(XWALK_ADXL345_BUILD_HARDWARE_TESTS)
    add_executable(xWalkAdxl345HardwareTest
        test/hardware/src/xHal_Rpi5CarAdxl345HardwareTest.cpp)
    target_link_libraries(xWalkAdxl345HardwareTest PRIVATE xWalkAdxl345 xWalkI2cLinux)
    add_test(NAME xWalkAdxl345HardwareReadTest COMMAND xWalkAdxl345HardwareTest)
    set_tests_properties(xWalkAdxl345HardwareReadTest PROPERTIES LABELS "hardware")
endif()
