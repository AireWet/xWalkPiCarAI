cmake_minimum_required(VERSION 3.16)
project(xWalkI2c LANGUAGES CXX)

option(XWALK_I2C_BUILD_HOST_TESTS "Build xWalkI2c host tests" OFF)
option(XWALK_I2C_BUILD_HARDWARE_TESTS "Build xWalkI2c RPi hardware tests" OFF)
option(XWALK_I2C_BUILD_LINUX_BACKEND "Build the xWalkI2c Linux backend" OFF)
set(XWALK_I2C_HARDWARE_DEVICE "/dev/i2c-1" CACHE FILEPATH
    "Linux I2C device used by the opt-in hardware test")

if((XWALK_I2C_BUILD_HARDWARE_TESTS OR XWALK_I2C_BUILD_LINUX_BACKEND)
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkI2c hardware tests require Linux i2c-dev")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common
    )
endif()

add_library(xWalkI2c STATIC
    core/src/xHal_Rpi5CarI2c.cpp
    core/src/xHal_Rpi5CarI2cLifecycle.cpp
)

target_include_directories(xWalkI2c
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

target_compile_features(xWalkI2c PUBLIC cxx_std_17)
target_link_libraries(xWalkI2c PUBLIC xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkI2c PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_I2C_BUILD_HOST_TESTS OR XWALK_I2C_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_I2C_BUILD_HOST_TESTS)
    add_executable(xWalkI2cTest
        test/xHal_Rpi5CarI2cTest.cpp
    )
    target_link_libraries(xWalkI2cTest PRIVATE xWalkI2c)
    add_test(NAME xWalkI2cHostTest COMMAND xWalkI2cTest)
    set_tests_properties(xWalkI2cHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_I2C_BUILD_HARDWARE_TESTS OR XWALK_I2C_BUILD_LINUX_BACKEND)
    add_library(xWalkI2cLinux STATIC
        hardware/src/xHal_Rpi5CarI2cLinux.cpp
        hardware/src/xHal_Rpi5CarI2cLinuxLifecycle.cpp
    )
    target_include_directories(xWalkI2cLinux PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include
    )
    target_compile_features(xWalkI2cLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkI2cLinux PUBLIC xWalkI2c)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkI2cLinux PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_I2C_BUILD_HARDWARE_TESTS)
        add_executable(xWalkI2cLinuxHardwareTest
            hardware/test/src/xHal_Rpi5CarI2cLinuxHardwareTest.cpp
        )
        target_link_libraries(xWalkI2cLinuxHardwareTest PRIVATE xWalkI2cLinux)
        add_test(NAME xWalkI2cLinuxHardwareProbeTest
            COMMAND xWalkI2cLinuxHardwareTest ${XWALK_I2C_HARDWARE_DEVICE})
        set_tests_properties(xWalkI2cLinuxHardwareProbeTest PROPERTIES
            LABELS "hardware")
    endif()
endif()
