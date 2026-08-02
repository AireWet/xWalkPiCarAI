cmake_minimum_required(VERSION 3.16)

project(xWalkPicarx LANGUAGES CXX)

set(XWALK_PICARX_DEFAULT_CONFIG_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCLI/xWalkController/config/picar-x.conf")
if(NOT DEFINED XWALK_PICARX_CONFIG_FILE)
    set(XWALK_PICARX_CONFIG_FILE "${XWALK_PICARX_DEFAULT_CONFIG_FILE}"
        CACHE FILEPATH "Writable PiCar-X calibration configuration file" FORCE)
endif()

option(XWALK_PICARX_BUILD_HOST_TESTS "Build xWalkPicarx host tests" OFF)
option(XWALK_PICARX_BUILD_HARDWARE_TESTS "Build xWalkPicarx hardware tests" OFF)

if(XWALK_PICARX_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkPicarx hardware tests require Linux I2C and GPIO support")
endif()

set(XWALK_HAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkHal CACHE PATH
    "Path to the xWalk HAL source tree")
set(XWALK_ADC_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_CONFIG_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_GPIO_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_I2C_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_LINE_TRACKER_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_MOTOR_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_PWM_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_SERVO_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)
set(XWALK_ULTRASONIC_BUILD_HOST_TESTS OFF CACHE BOOL "Agent dependency setting" FORCE)

if(XWALK_PICARX_BUILD_HARDWARE_TESTS)
    set(XWALK_ADC_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_GPIO_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_I2C_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_MOTOR_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_PWM_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_SERVO_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_ULTRASONIC_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
    set(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS ON CACHE BOOL "Agent hardware dependency" FORCE)
endif()

if(NOT TARGET xWalkConfig)
    add_subdirectory(${XWALK_HAL_ROOT}/xWalkConfig ${CMAKE_CURRENT_BINARY_DIR}/xWalkConfig)
endif()
if(NOT TARGET xWalkLineTracker)
    add_subdirectory(${XWALK_HAL_ROOT}/xWalkLineTracker ${CMAKE_CURRENT_BINARY_DIR}/xWalkLineTracker)
endif()
if(NOT TARGET xWalkMotor)
    add_subdirectory(${XWALK_HAL_ROOT}/xWalkMotor ${CMAKE_CURRENT_BINARY_DIR}/xWalkMotor)
endif()
if(NOT TARGET xWalkServo)
    add_subdirectory(${XWALK_HAL_ROOT}/xWalkServo ${CMAKE_CURRENT_BINARY_DIR}/xWalkServo)
endif()
if(NOT TARGET xWalkUltrasonic)
    add_subdirectory(${XWALK_HAL_ROOT}/xWalkUltrasonic ${CMAKE_CURRENT_BINARY_DIR}/xWalkUltrasonic)
endif()
if(XWALK_PICARX_BUILD_HARDWARE_TESTS AND NOT TARGET xWalkBoardControl)
    add_subdirectory(${XWALK_HAL_ROOT}/xWalkBoardControl ${CMAKE_CURRENT_BINARY_DIR}/xWalkBoardControl)
endif()

add_library(xWalkPicarx STATIC
    src/xAgent_Rpi5CarPicarxCalibration.cpp
    src/xAgent_Rpi5CarPicarxDrive.cpp
    src/xAgent_Rpi5CarPicarxLifecycle.cpp
    src/xAgent_Rpi5CarPicarxSafetyGuard.cpp
    src/xAgent_Rpi5CarPicarxSensing.cpp
    src/xAgent_Rpi5CarPicarxValidation.cpp
)
add_library(xWalk::Picarx ALIAS xWalkPicarx)

target_include_directories(xWalkPicarx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkPicarx PUBLIC cxx_std_17)
target_link_libraries(xWalkPicarx PUBLIC
    xWalkConfig
    xWalkLineTracker
    xWalkMotor
    xWalkServo
    xWalkUltrasonic
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkPicarx PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_PICARX_BUILD_HOST_TESTS OR XWALK_PICARX_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_PICARX_BUILD_HOST_TESTS)
    add_executable(xWalkPicarxTest test/src/xAgent_Rpi5CarPicarxTest.cpp)
    target_link_libraries(xWalkPicarxTest PRIVATE xWalkPicarx)
    add_test(NAME xWalkPicarxHostTest COMMAND xWalkPicarxTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/picarx-test.conf)
    set_tests_properties(xWalkPicarxHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_PICARX_BUILD_HARDWARE_TESTS)
    add_executable(xWalkPicarxHardwareTest test/hardware/src/xAgent_Rpi5CarPicarxHardwareTest.cpp)
    target_compile_definitions(xWalkPicarxHardwareTest PRIVATE
        XWALK_PICARX_CONFIG_FILE="${XWALK_PICARX_CONFIG_FILE}")
    target_link_libraries(xWalkPicarxHardwareTest PRIVATE
        xWalkPicarx
        xWalkBoardControl
        xWalkI2cLinux
        xWalkGpioLinux
    )
    add_test(NAME xWalkPicarxHardwareResetTest COMMAND xWalkPicarxHardwareTest)
    set_tests_properties(xWalkPicarxHardwareResetTest PROPERTIES LABELS "hardware")
endif()
