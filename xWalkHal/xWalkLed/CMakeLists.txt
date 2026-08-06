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
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

if(NOT TARGET xWalkPwm)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPwm ${CMAKE_CURRENT_BINARY_DIR}/xWalkPwm)
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
target_link_libraries(xWalkLed PUBLIC xWalkLibraryCommon xWalkGpio xWalkPwm)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLed PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_LED_BUILD_HOST_TESTS OR XWALK_LED_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_LED_BUILD_HOST_TESTS)
    add_executable(xWalkLedTest test/src/xHal_Rpi5CarLedTest.cpp)
    target_link_libraries(xWalkLedTest PRIVATE xWalkLed)
    add_test(NAME xWalkLedHostTest COMMAND xWalkLedTest)
    add_executable(xWalkRgbLedTest test/src/xHal_Rpi5CarRgbLedTest.cpp)
    target_link_libraries(xWalkRgbLedTest PRIVATE xWalkLed)
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
