cmake_minimum_required(VERSION 3.16)
project(xWalkAdxl345 LANGUAGES CXX)

option(XWALK_ADXL345_BUILD_HOST_TESTS "Build xWalkAdxl345 host tests" OFF)
option(XWALK_ADXL345_BUILD_HARDWARE_TESTS "Build xWalkAdxl345 hardware tests" OFF)

set(XWALK_I2C_BUILD_HOST_TESTS ${XWALK_ADXL345_BUILD_HOST_TESTS})
set(XWALK_I2C_BUILD_HARDWARE_TESTS ${XWALK_ADXL345_BUILD_HARDWARE_TESTS})

if(XWALK_ADXL345_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkAdxl345 hardware tests require Linux i2c-dev")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(NOT TARGET xWalkI2c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkI2c ${CMAKE_CURRENT_BINARY_DIR}/xWalkI2c)
endif()

add_library(xWalkAdxl345 STATIC
    src/xHal_Rpi5CarAdxl345.cpp
    src/xHal_Rpi5CarAdxl345Lifecycle.cpp
)

target_include_directories(xWalkAdxl345 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAdxl345 PUBLIC cxx_std_17)
target_link_libraries(xWalkAdxl345 PUBLIC xWalkCommon xWalkI2c)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAdxl345 PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ADXL345_BUILD_HOST_TESTS OR XWALK_ADXL345_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_ADXL345_BUILD_HOST_TESTS)
    add_executable(xWalkAdxl345Test test/src/xHal_Rpi5CarAdxl345Test.cpp)
    target_link_libraries(xWalkAdxl345Test PRIVATE xWalkAdxl345)
    add_test(NAME xWalkAdxl345HostTest COMMAND xWalkAdxl345Test)
    set_tests_properties(xWalkAdxl345HostTest PROPERTIES LABELS "host")
endif()

if(XWALK_ADXL345_BUILD_HARDWARE_TESTS)
    add_executable(xWalkAdxl345HardwareTest
        test/hardware/src/xHal_Rpi5CarAdxl345HardwareTest.cpp)
    target_link_libraries(xWalkAdxl345HardwareTest PRIVATE xWalkAdxl345 xWalkI2cLinux)
    add_test(NAME xWalkAdxl345HardwareReadTest COMMAND xWalkAdxl345HardwareTest)
    set_tests_properties(xWalkAdxl345HardwareReadTest PROPERTIES LABELS "hardware")
endif()
