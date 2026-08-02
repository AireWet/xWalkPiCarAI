cmake_minimum_required(VERSION 3.16)
project(xWalkAdc LANGUAGES CXX)

option(XWALK_ADC_BUILD_HOST_TESTS "Build xWalkAdc host tests" OFF)
option(XWALK_ADC_BUILD_HARDWARE_TESTS "Build xWalkAdc RPi hardware tests" OFF)

set(XWALK_I2C_BUILD_HOST_TESTS ${XWALK_ADC_BUILD_HOST_TESTS})
set(XWALK_I2C_BUILD_HARDWARE_TESTS ${XWALK_ADC_BUILD_HARDWARE_TESTS})

if(XWALK_ADC_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkAdc hardware tests require Linux i2c-dev")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(NOT TARGET xWalkI2c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkI2c ${CMAKE_CURRENT_BINARY_DIR}/xWalkI2c)
endif()

add_library(xWalkAdc STATIC
    src/xHal_Rpi5CarAdc.cpp
    src/xHal_Rpi5CarAdcLifecycle.cpp
)

target_include_directories(xWalkAdc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAdc PUBLIC cxx_std_17)
target_link_libraries(xWalkAdc PUBLIC xWalkCommon xWalkI2c)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAdc PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ADC_BUILD_HOST_TESTS OR XWALK_ADC_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_ADC_BUILD_HOST_TESTS)
    add_executable(xWalkAdcTest test/src/xHal_Rpi5CarAdcTest.cpp)
    target_link_libraries(xWalkAdcTest PRIVATE xWalkAdc)
    add_test(NAME xWalkAdcHostTest COMMAND xWalkAdcTest)
    set_tests_properties(xWalkAdcHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_ADC_BUILD_HARDWARE_TESTS)
    add_executable(xWalkAdcHardwareTest test/hardware/src/xHal_Rpi5CarAdcHardwareTest.cpp)
    target_link_libraries(xWalkAdcHardwareTest PRIVATE xWalkAdc xWalkI2cLinux)
    add_test(NAME xWalkAdcHardwareReadTest COMMAND xWalkAdcHardwareTest)
    set_tests_properties(xWalkAdcHardwareReadTest PROPERTIES LABELS "hardware")
endif()
