cmake_minimum_required(VERSION 3.16)
project(xWalkPwm LANGUAGES CXX)

option(XWALK_PWM_BUILD_HOST_TESTS "Build xWalkPwm host tests" OFF)
option(XWALK_PWM_BUILD_HARDWARE_TESTS "Build xWalkPwm RPi hardware tests" OFF)

# Propagate the parent module's test mode to the I2C dependency. When xWalkI2c
# is configured directly, its own options remain independent and default OFF.
set(XWALK_I2C_BUILD_HOST_TESTS ${XWALK_PWM_BUILD_HOST_TESTS})
set(XWALK_I2C_BUILD_HARDWARE_TESTS ${XWALK_PWM_BUILD_HARDWARE_TESTS})

if(XWALK_PWM_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkPwm hardware tests require Linux i2c-dev")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common
    )
endif()

if(NOT TARGET xWalkI2c)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkI2c
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkI2c
    )
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkPwm STATIC
    src/xHal_Rpi5CarPwm.cpp
    src/xHal_Rpi5CarPwmLifecycle.cpp
    src/xHal_Rpi5CarPwmTimer.cpp
    src/xHal_Rpi5CarPwmTimerState.cpp
    src/xHal_Rpi5CarPwmTimerStateLifecycle.cpp
    src/xHal_Rpi5CarPwmOutput.cpp
)

target_include_directories(xWalkPwm
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(xWalkPwm PUBLIC cxx_std_17)
target_link_libraries(xWalkPwm PUBLIC xWalkLibraryCommon xWalkI2c PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkPwm PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_PWM_BUILD_HOST_TESTS OR XWALK_PWM_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_PWM_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_PWM_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_PWM_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarPwmTraceConfig.py)
    set(XWALK_PWM_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkPwmTrace.xml)
    add_custom_command(OUTPUT ${XWALK_PWM_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_PWM_TRACE_GENERATOR}
            --input ${XWALK_PWM_TRACE_INVENTORY} --output ${XWALK_PWM_TRACE_CONFIG}
        DEPENDS ${XWALK_PWM_TRACE_INVENTORY} ${XWALK_PWM_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkPwm trace catalogue" VERBATIM)
    add_custom_target(xWalkPwmTestTraceConfig ALL DEPENDS ${XWALK_PWM_TRACE_CONFIG})
    add_dependencies(xWalkPwmTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkPwmTest
        test/src/xHal_Rpi5CarPwmTestMain.cpp
        test/src/xHal_Rpi5CarPwmTestI2c.cpp
        test/src/xHal_Rpi5CarPwmTestI2cLifecycle.cpp
        test/src/xHal_Rpi5CarPwmTestAddress.cpp
        test/src/xHal_Rpi5CarPwmTestOutput.cpp
        test/src/xHal_Rpi5CarPwmTestTimer.cpp
        test/src/xHal_Rpi5CarPwmTestValidation.cpp
        test/src/xHal_Rpi5CarPwmTestTrace.cpp
        simulation/src/xHal_Rpi5CarPwmSimulationArguments.cpp
    )
    target_include_directories(xWalkPwmTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include
    )
    set(XWALK_PWM_TEST_TRACE_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkPwmTest.log)
    target_compile_definitions(xWalkPwmTest PRIVATE
        XWALK_PWM_SIMULATION_TRACE_CONFIG_PATH="${XWALK_PWM_TRACE_CONFIG}"
        XWALK_PWM_SIMULATION_TRACE_LOG_PATH="${XWALK_PWM_TEST_TRACE_LOG}")
    target_link_libraries(xWalkPwmTest PRIVATE xWalkPwm xWalkTrace)
    add_dependencies(xWalkPwmTest xWalkPwmTestTraceConfig)
    add_test(NAME xWalkPwmAddressTest
        COMMAND xWalkPwmTest address)
    add_test(NAME xWalkPwmTimerMappingTest
        COMMAND xWalkPwmTest mapping)
    add_test(NAME xWalkPwmRegisterDataTest
        COMMAND xWalkPwmTest register)
    add_test(NAME xWalkPwmPercentageTest
        COMMAND xWalkPwmTest percentage)
    add_test(NAME xWalkPwmFrequencyTest
        COMMAND xWalkPwmTest frequency)
    add_test(NAME xWalkPwmValidationTest
        COMMAND xWalkPwmTest validation)
    add_test(NAME xWalkPwmTraceSelectionTest
        COMMAND xWalkPwmTest trace)
    set_tests_properties(
        xWalkPwmAddressTest
        xWalkPwmTimerMappingTest
        xWalkPwmRegisterDataTest
        xWalkPwmPercentageTest
        xWalkPwmFrequencyTest
        xWalkPwmValidationTest
        xWalkPwmTraceSelectionTest
        PROPERTIES LABELS "host")
endif()

if(XWALK_PWM_BUILD_HARDWARE_TESTS)
    add_executable(xWalkPwmHardwareTest
        test/hardware/src/xHal_Rpi5CarPwmHardwareTest.cpp
    )
    target_link_libraries(xWalkPwmHardwareTest PRIVATE
        xWalkPwm
        xWalkI2cLinux
    )
    add_test(NAME xWalkPwmHardwareZeroOutputTest
        COMMAND xWalkPwmHardwareTest)
    set_tests_properties(xWalkPwmHardwareZeroOutputTest PROPERTIES
        LABELS "hardware")
endif()
