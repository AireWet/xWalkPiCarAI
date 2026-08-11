cmake_minimum_required(VERSION 3.16)
project(xWalkAdc LANGUAGES CXX)

option(XWALK_ADC_BUILD_HOST_TESTS "Build xWalkAdc host tests" OFF)
option(XWALK_ADC_BUILD_HARDWARE_TESTS "Build xWalkAdc RPi hardware tests" OFF)

set(XWALK_I2C_BUILD_HOST_TESTS ${XWALK_ADC_BUILD_HOST_TESTS})
set(XWALK_I2C_BUILD_HARDWARE_TESTS ${XWALK_ADC_BUILD_HARDWARE_TESTS})

if(XWALK_ADC_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkAdc hardware tests require Linux i2c-dev")
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

add_library(xWalkAdc STATIC
    src/xHal_Rpi5CarAdc.cpp
    src/xHal_Rpi5CarAdcLifecycle.cpp
)

target_include_directories(xWalkAdc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAdc PUBLIC cxx_std_17)
target_link_libraries(xWalkAdc PUBLIC xWalkLibraryCommon xWalkI2c PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAdc PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ADC_BUILD_HOST_TESTS OR XWALK_ADC_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_ADC_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_ADC_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_ADC_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarAdcTraceConfig.py)
    set(XWALK_ADC_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAdcTrace.xml)
    add_custom_command(OUTPUT ${XWALK_ADC_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_ADC_TRACE_GENERATOR}
            --input ${XWALK_ADC_TRACE_INVENTORY} --output ${XWALK_ADC_TRACE_CONFIG}
        DEPENDS ${XWALK_ADC_TRACE_INVENTORY} ${XWALK_ADC_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkAdc trace catalogue" VERBATIM)
    add_custom_target(xWalkAdcTestTraceConfig ALL DEPENDS ${XWALK_ADC_TRACE_CONFIG})
    add_dependencies(xWalkAdcTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkAdcTest
        test/src/xHal_Rpi5CarAdcTest.cpp
        test/src/xHal_Rpi5CarAdcTestSupport.cpp
        simulation/src/xHal_Rpi5CarAdcSimulationArguments.cpp)
    target_include_directories(xWalkAdcTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_ADC_TEST_TRACE_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkAdcTest.log)
    target_compile_definitions(xWalkAdcTest PRIVATE
        XWALK_ADC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ADC_TRACE_CONFIG}"
        XWALK_ADC_SIMULATION_TRACE_LOG_PATH="${XWALK_ADC_TEST_TRACE_LOG}")
    target_link_libraries(xWalkAdcTest PRIVATE xWalkAdc xWalkTrace)
    add_dependencies(xWalkAdcTest xWalkAdcTestTraceConfig)
    add_test(NAME xWalkAdcHostTest COMMAND xWalkAdcTest)
    set_tests_properties(xWalkAdcHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_ADC_BUILD_HARDWARE_TESTS)
    add_executable(xWalkAdcHardwareTest test/hardware/src/xHal_Rpi5CarAdcHardwareTest.cpp)
    target_link_libraries(xWalkAdcHardwareTest PRIVATE xWalkAdc xWalkI2cLinux)
    add_test(NAME xWalkAdcHardwareReadTest COMMAND xWalkAdcHardwareTest)
    set_tests_properties(xWalkAdcHardwareReadTest PROPERTIES LABELS "hardware")
endif()
