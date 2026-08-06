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
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkPwm)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPwm ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

add_library(xWalkMotor STATIC
    src/xHal_Rpi5CarMotor.cpp
    src/xHal_Rpi5CarMotorLifecycle.cpp
    src/xHal_Rpi5CarMotors.cpp
    src/xHal_Rpi5CarMotorsLifecycle.cpp
)

target_include_directories(xWalkMotor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkMotor PUBLIC cxx_std_17)
target_link_libraries(xWalkMotor PUBLIC xWalkLibraryCommon xWalkPwm xWalkGpio)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkMotor PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_MOTOR_BUILD_HOST_TESTS OR XWALK_MOTOR_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_MOTOR_BUILD_HOST_TESTS)
    add_executable(xWalkMotorTest test/src/xHal_Rpi5CarMotorTest.cpp)
    target_link_libraries(xWalkMotorTest PRIVATE xWalkMotor)
    add_test(NAME xWalkMotorHostTest COMMAND xWalkMotorTest)
    set_tests_properties(xWalkMotorHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_MOTOR_BUILD_HARDWARE_TESTS)
    add_executable(xWalkMotorHardwareTest test/hardware/src/xHal_Rpi5CarMotorHardwareTest.cpp)
    target_link_libraries(xWalkMotorHardwareTest PRIVATE xWalkMotor xWalkI2cLinux xWalkGpioLinux)
    add_test(NAME xWalkMotorHardwareStopTest COMMAND xWalkMotorHardwareTest)
    set_tests_properties(xWalkMotorHardwareStopTest PROPERTIES LABELS "hardware")
endif()
