cmake_minimum_required(VERSION 3.16)
project(xWalkBoardControl LANGUAGES CXX)

option(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS "Build xWalkBoardControl host tests" OFF)
option(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS "Build xWalkBoardControl hardware tests" OFF)

set(XWALK_ADC_BUILD_HOST_TESTS ${XWALK_BOARD_CONTROL_BUILD_HOST_TESTS})
set(XWALK_ADC_BUILD_HARDWARE_TESTS ${XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS})
set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_BOARD_CONTROL_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS})
set(XWALK_I2C_BUILD_HOST_TESTS ${XWALK_BOARD_CONTROL_BUILD_HOST_TESTS})
set(XWALK_I2C_BUILD_HARDWARE_TESTS ${XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS})

if(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkBoardControl hardware tests require Linux GPIO and I2C APIs")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(NOT TARGET xWalkAdc)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAdc ${CMAKE_CURRENT_BINARY_DIR}/xWalkAdc)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

if(NOT TARGET xWalkI2c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkI2c ${CMAKE_CURRENT_BINARY_DIR}/xWalkI2c)
endif()

add_library(xWalkBoardControl STATIC
    src/xHal_Rpi5CarBoardControl.cpp
    src/xHal_Rpi5CarBoardControlLifecycle.cpp
    src/xHal_Rpi5CarDevice.cpp
    src/xHal_Rpi5CarDeviceLifecycle.cpp
    src/xHal_Rpi5CarFirmwareInfo.cpp
    src/xHal_Rpi5CarFirmwareInfoLifecycle.cpp
)

target_include_directories(xWalkBoardControl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkBoardControl PUBLIC cxx_std_17)
target_link_libraries(xWalkBoardControl PUBLIC xWalkAdc xWalkCommon xWalkGpio xWalkI2c)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkBoardControl PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS OR XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_BOARD_CONTROL_BUILD_HOST_TESTS)
    add_executable(xWalkBoardControlTest test/src/xHal_Rpi5CarBoardControlTest.cpp)
    target_link_libraries(xWalkBoardControlTest PRIVATE xWalkBoardControl)
    add_test(NAME xWalkBoardControlHostTest COMMAND xWalkBoardControlTest)
    set_tests_properties(xWalkBoardControlHostTest PROPERTIES LABELS "host")

    add_executable(xWalkDeviceTest test/src/xHal_Rpi5CarDeviceTest.cpp)
    target_link_libraries(xWalkDeviceTest PRIVATE xWalkBoardControl)
    add_test(NAME xWalkDeviceHostTest COMMAND xWalkDeviceTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-device-tree)
    set_tests_properties(xWalkDeviceHostTest PROPERTIES LABELS "host")

    add_executable(xWalkFirmwareInfoTest test/src/xHal_Rpi5CarFirmwareInfoTest.cpp)
    target_link_libraries(xWalkFirmwareInfoTest PRIVATE xWalkBoardControl)
    add_test(NAME xWalkFirmwareInfoHostTest COMMAND xWalkFirmwareInfoTest)
    set_tests_properties(xWalkFirmwareInfoHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS)
    add_executable(xWalkBoardControlHardwareTest
        test/hardware/src/xHal_Rpi5CarBoardControlHardwareTest.cpp)
    target_link_libraries(xWalkBoardControlHardwareTest PRIVATE
        xWalkBoardControl xWalkGpioLinux xWalkI2cLinux)
    add_test(NAME xWalkBoardControlHardwareDisableTest COMMAND xWalkBoardControlHardwareTest)
    set_tests_properties(xWalkBoardControlHardwareDisableTest PROPERTIES LABELS "hardware")

    add_executable(xWalkDeviceHardwareTest
        test/hardware/src/xHal_Rpi5CarDeviceHardwareTest.cpp)
    target_link_libraries(xWalkDeviceHardwareTest PRIVATE xWalkBoardControl)
    add_test(NAME xWalkDeviceHardwareDiscoveryTest COMMAND xWalkDeviceHardwareTest)
    set_tests_properties(xWalkDeviceHardwareDiscoveryTest PROPERTIES LABELS "hardware")

    add_executable(xWalkFirmwareInfoHardwareTest
        test/hardware/src/xHal_Rpi5CarFirmwareInfoHardwareTest.cpp)
    target_link_libraries(xWalkFirmwareInfoHardwareTest PRIVATE
        xWalkBoardControl xWalkI2cLinux)
    add_test(NAME xWalkFirmwareInfoHardwareReadTest COMMAND xWalkFirmwareInfoHardwareTest)
    set_tests_properties(xWalkFirmwareInfoHardwareReadTest PROPERTIES LABELS "hardware")
endif()
