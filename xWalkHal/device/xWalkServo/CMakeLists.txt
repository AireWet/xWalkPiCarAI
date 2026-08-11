cmake_minimum_required(VERSION 3.16)
project(xWalkServo LANGUAGES CXX)

option(XWALK_SERVO_BUILD_HOST_TESTS "Build xWalkServo host tests" OFF)
option(XWALK_SERVO_BUILD_HARDWARE_TESTS "Build xWalkServo RPi hardware tests" OFF)

# Propagate the selected verification mode through the PWM and I2C dependency
# chain when this module is configured as the top-level project.
set(XWALK_PWM_BUILD_HOST_TESTS ${XWALK_SERVO_BUILD_HOST_TESTS})
set(XWALK_PWM_BUILD_HARDWARE_TESTS ${XWALK_SERVO_BUILD_HARDWARE_TESTS})

if(XWALK_SERVO_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkServo hardware tests require Linux i2c-dev")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common
    )
endif()

if(NOT TARGET xWalkPwm)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPwm
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm
    )
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkServo STATIC
    src/xHal_Rpi5CarServo.cpp
    src/xHal_Rpi5CarServoLifecycle.cpp
)

target_include_directories(xWalkServo
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(xWalkServo PUBLIC cxx_std_17)
target_link_libraries(xWalkServo PUBLIC xWalkLibraryCommon xWalkPwm PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkServo PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SERVO_BUILD_HOST_TESTS OR XWALK_SERVO_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_SERVO_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_SERVO_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_SERVO_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarServoTraceConfig.py)
    set(XWALK_SERVO_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkServoTrace.xml)
    add_custom_command(OUTPUT ${XWALK_SERVO_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_SERVO_TRACE_GENERATOR}
            --input ${XWALK_SERVO_TRACE_INVENTORY} --output ${XWALK_SERVO_TRACE_CONFIG}
        DEPENDS ${XWALK_SERVO_TRACE_INVENTORY} ${XWALK_SERVO_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkServo trace catalogue" VERBATIM)
    add_custom_target(xWalkServoTestTraceConfig ALL DEPENDS ${XWALK_SERVO_TRACE_CONFIG})
    add_dependencies(xWalkServoTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkServoTest
        test/src/xHal_Rpi5CarServoTestMain.cpp
        test/src/xHal_Rpi5CarServoTestI2c.cpp
        test/src/xHal_Rpi5CarServoTestI2cLifecycle.cpp
        test/src/xHal_Rpi5CarServoTestInitialization.cpp
        test/src/xHal_Rpi5CarServoTestAngle.cpp
        test/src/xHal_Rpi5CarServoTestPulse.cpp
        test/src/xHal_Rpi5CarServoTestValidation.cpp
        test/src/xHal_Rpi5CarServoTestTrace.cpp
        simulation/src/xHal_Rpi5CarServoSimulationArguments.cpp
    )
    target_include_directories(xWalkServoTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include
    )
    set(XWALK_SERVO_TEST_TRACE_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkServoTest.log)
    target_compile_definitions(xWalkServoTest PRIVATE
        XWALK_SERVO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_SERVO_TRACE_CONFIG}"
        XWALK_SERVO_SIMULATION_TRACE_LOG_PATH="${XWALK_SERVO_TEST_TRACE_LOG}")
    target_link_libraries(xWalkServoTest PRIVATE xWalkServo xWalkTrace)
    add_dependencies(xWalkServoTest xWalkServoTestTraceConfig)
    add_test(NAME xWalkServoInitializationTest
        COMMAND xWalkServoTest initialization)
    add_test(NAME xWalkServoAngleTest
        COMMAND xWalkServoTest angle)
    add_test(NAME xWalkServoPulseWidthTest
        COMMAND xWalkServoTest pulse)
    add_test(NAME xWalkServoValidationTest
        COMMAND xWalkServoTest validation)
    add_test(NAME xWalkServoTraceSelectionTest
        COMMAND xWalkServoTest trace)
    set_tests_properties(
        xWalkServoInitializationTest
        xWalkServoAngleTest
        xWalkServoPulseWidthTest
        xWalkServoValidationTest
        xWalkServoTraceSelectionTest
        PROPERTIES LABELS "host")
endif()

if(XWALK_SERVO_BUILD_HARDWARE_TESTS)
    add_executable(xWalkServoHardwareTest
        test/hardware/src/xHal_Rpi5CarServoHardwareTest.cpp
    )
    target_link_libraries(xWalkServoHardwareTest PRIVATE
        xWalkServo
        xWalkI2cLinux
    )
    add_test(NAME xWalkServoHardwareInitializationTest
        COMMAND xWalkServoHardwareTest)
    set_tests_properties(xWalkServoHardwareInitializationTest PROPERTIES
        LABELS "hardware")
endif()
