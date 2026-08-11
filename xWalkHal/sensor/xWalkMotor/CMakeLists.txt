cmake_minimum_required(VERSION 3.16)
project(xWalkMotor LANGUAGES CXX)

option(XWALK_MOTOR_BUILD_HOST_TESTS "Build xWalkMotor host tests" OFF)
option(XWALK_MOTOR_BUILD_HARDWARE_TESTS "Build xWalkMotor hardware tests" OFF)

set(XWALK_PWM_BUILD_HOST_TESTS ${XWALK_MOTOR_BUILD_HOST_TESTS})
set(XWALK_PWM_BUILD_HARDWARE_TESTS ${XWALK_MOTOR_BUILD_HARDWARE_TESTS})
set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_MOTOR_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_MOTOR_BUILD_HARDWARE_TESTS})

if(XWALK_MOTOR_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkMotor hardware tests require Linux I2C and GPIO support")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkPwm)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../device/xWalkPwm ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkMotor STATIC
    src/xHal_Rpi5CarMotor.cpp
    src/xHal_Rpi5CarMotorLifecycle.cpp
    src/xHal_Rpi5CarMotors.cpp
    src/xHal_Rpi5CarMotorsLifecycle.cpp
)

target_include_directories(xWalkMotor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkMotor PUBLIC cxx_std_17)
target_link_libraries(xWalkMotor PUBLIC xWalkLibraryCommon xWalkPwm xWalkGpio PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkMotor PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_MOTOR_BUILD_HOST_TESTS OR XWALK_MOTOR_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_MOTOR_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_MOTOR_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_MOTOR_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarMotorTraceConfig.py)
    set(XWALK_MOTOR_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkMotorTrace.xml)
    add_custom_command(OUTPUT ${XWALK_MOTOR_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_MOTOR_TRACE_GENERATOR}
            --input ${XWALK_MOTOR_TRACE_INVENTORY} --output ${XWALK_MOTOR_TRACE_CONFIG}
        DEPENDS ${XWALK_MOTOR_TRACE_INVENTORY} ${XWALK_MOTOR_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkMotor trace catalogue" VERBATIM)
    add_custom_target(xWalkMotorTestTraceConfig ALL DEPENDS ${XWALK_MOTOR_TRACE_CONFIG})
    add_dependencies(xWalkMotorTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkMotorTest
        test/src/xHal_Rpi5CarMotorTest.cpp
        test/src/xHal_Rpi5CarMotorTestSupport.cpp
        simulation/src/xHal_Rpi5CarMotorSimulationArguments.cpp)
    target_include_directories(xWalkMotorTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_MOTOR_TEST_TRACE_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkMotorTest.log)
    target_compile_definitions(xWalkMotorTest PRIVATE
        XWALK_MOTOR_SIMULATION_TRACE_CONFIG_PATH="${XWALK_MOTOR_TRACE_CONFIG}"
        XWALK_MOTOR_SIMULATION_TRACE_LOG_PATH="${XWALK_MOTOR_TEST_TRACE_LOG}")
    target_link_libraries(xWalkMotorTest PRIVATE xWalkMotor xWalkTrace)
    add_dependencies(xWalkMotorTest xWalkMotorTestTraceConfig)
    add_test(NAME xWalkMotorHostTest COMMAND xWalkMotorTest)
    set_tests_properties(xWalkMotorHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_MOTOR_BUILD_HARDWARE_TESTS)
    add_executable(xWalkMotorHardwareTest test/hardware/src/xHal_Rpi5CarMotorHardwareTest.cpp)
    target_link_libraries(xWalkMotorHardwareTest PRIVATE xWalkMotor xWalkI2cLinux xWalkGpioLinux)
    add_test(NAME xWalkMotorHardwareStopTest COMMAND xWalkMotorHardwareTest)
    set_tests_properties(xWalkMotorHardwareStopTest PROPERTIES LABELS "hardware")
endif()
