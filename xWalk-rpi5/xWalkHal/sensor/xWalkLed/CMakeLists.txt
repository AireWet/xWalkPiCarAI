cmake_minimum_required(VERSION 3.16)
project(xWalkLed LANGUAGES CXX)

option(XWALK_LED_BUILD_HOST_TESTS "Build xWalkLed host tests" OFF)
option(XWALK_LED_BUILD_HARDWARE_TESTS "Build xWalkLed hardware tests" OFF)

set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_LED_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_LED_BUILD_HARDWARE_TESTS})
set(XWALK_PWM_BUILD_HOST_TESTS ${XWALK_LED_BUILD_HOST_TESTS})
set(XWALK_PWM_BUILD_HARDWARE_TESTS ${XWALK_LED_BUILD_HARDWARE_TESTS})

if(XWALK_LED_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkLed hardware tests require the Linux GPIO character-device API")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

if(NOT TARGET xWalkPwm)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../device/xWalkPwm ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkLed STATIC
    src/xHal_Rpi5CarLed.cpp
    src/xHal_Rpi5CarLedLifecycle.cpp
    src/xHal_Rpi5CarRgbLed.cpp
    src/xHal_Rpi5CarRgbLedLifecycle.cpp
)
add_library(xWalkRgbLed ALIAS xWalkLed)

target_include_directories(xWalkLed PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLed PUBLIC cxx_std_17)
target_link_libraries(xWalkLed PUBLIC xWalkLibraryCommon xWalkGpio xWalkPwm PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLed PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_LED_BUILD_HOST_TESTS OR XWALK_LED_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_LED_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_LED_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_LED_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarLedTraceConfig.py)
    set(XWALK_LED_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLedTrace.xml)
    add_custom_command(OUTPUT ${XWALK_LED_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_LED_TRACE_GENERATOR}
            --input ${XWALK_LED_TRACE_INVENTORY} --output ${XWALK_LED_TRACE_CONFIG}
        DEPENDS ${XWALK_LED_TRACE_INVENTORY} ${XWALK_LED_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkLed trace catalogue" VERBATIM)
    add_custom_target(xWalkLedTestTraceConfig ALL DEPENDS ${XWALK_LED_TRACE_CONFIG})
    add_dependencies(xWalkLedTestTraceConfig xWalkTraceMetadata)
    add_library(xWalkLedTestSupport STATIC
        test/src/xHal_Rpi5CarLedTestSupport.cpp
        simulation/src/xHal_Rpi5CarLedSimulationArguments.cpp)
    target_include_directories(xWalkLedTestSupport PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_LED_TEST_TRACE_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkLedTest.log)
    target_compile_definitions(xWalkLedTestSupport PUBLIC
        XWALK_LED_SIMULATION_TRACE_CONFIG_PATH="${XWALK_LED_TRACE_CONFIG}"
        XWALK_LED_SIMULATION_TRACE_LOG_PATH="${XWALK_LED_TEST_TRACE_LOG}")
    target_link_libraries(xWalkLedTestSupport PUBLIC xWalkLed xWalkTrace)
    add_dependencies(xWalkLedTestSupport xWalkLedTestTraceConfig)
    add_executable(xWalkLedTest test/src/xHal_Rpi5CarLedTest.cpp)
    target_link_libraries(xWalkLedTest PRIVATE xWalkLedTestSupport)
    add_test(NAME xWalkLedHostTest COMMAND xWalkLedTest)
    add_executable(xWalkRgbLedTest test/src/xHal_Rpi5CarRgbLedTest.cpp)
    target_link_libraries(xWalkRgbLedTest PRIVATE xWalkLedTestSupport)
    add_test(NAME xWalkRgbLedHostTest COMMAND xWalkRgbLedTest)
    set_tests_properties(xWalkLedHostTest xWalkRgbLedHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_LED_BUILD_HARDWARE_TESTS)
    add_executable(xWalkLedHardwareTest
        test/hardware/src/xHal_Rpi5CarLedHardwareTest.cpp)
    target_link_libraries(xWalkLedHardwareTest PRIVATE xWalkLed xWalkGpioLinux)
    add_test(NAME xWalkLedHardwareInactiveTest COMMAND xWalkLedHardwareTest)
    add_executable(xWalkRgbLedHardwareTest
        test/hardware/src/xHal_Rpi5CarRgbLedHardwareTest.cpp)
    target_link_libraries(xWalkRgbLedHardwareTest PRIVATE xWalkLed xWalkI2cLinux)
    add_test(NAME xWalkRgbLedHardwareOutputTest COMMAND xWalkRgbLedHardwareTest)
    set_tests_properties(
        xWalkLedHardwareInactiveTest
        xWalkRgbLedHardwareOutputTest
        PROPERTIES LABELS "hardware")
endif()
