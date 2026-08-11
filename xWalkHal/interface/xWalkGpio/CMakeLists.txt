cmake_minimum_required(VERSION 3.16)
project(xWalkGpio LANGUAGES CXX)

option(XWALK_GPIO_BUILD_HOST_TESTS "Build xWalkGpio host tests" OFF)
option(XWALK_GPIO_BUILD_HARDWARE_TESTS "Build xWalkGpio Linux hardware tests" OFF)
option(XWALK_GPIO_BUILD_LINUX_BACKEND "Build the xWalkGpio Linux backend" OFF)
set(XWALK_GPIO_HARDWARE_DEVICE "/dev/gpiochip0" CACHE FILEPATH
    "Linux GPIO chip used by the opt-in hardware test")
set(XWALK_GPIO_HARDWARE_CHIP_NAME "" CACHE STRING
    "Optional exact GPIO chip name used by the opt-in hardware test")
set(XWALK_GPIO_HARDWARE_CHIP_LABEL "" CACHE STRING
    "Optional exact GPIO chip label used by the opt-in hardware test")

if((XWALK_GPIO_BUILD_HOST_TESTS OR XWALK_GPIO_BUILD_HARDWARE_TESTS
    OR XWALK_GPIO_BUILD_LINUX_BACKEND)
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkGpio hardware tests require the Linux GPIO character-device API")
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

add_library(xWalkGpio STATIC
    core/src/xHal_Rpi5CarGpio.cpp
    core/src/xHal_Rpi5CarGpioLifecycle.cpp
)

target_include_directories(xWalkGpio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
target_compile_features(xWalkGpio PUBLIC cxx_std_17)
target_link_libraries(xWalkGpio
    PUBLIC xWalkLibraryCommon
    PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkGpio PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_GPIO_BUILD_HOST_TESTS OR XWALK_GPIO_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_GPIO_BUILD_HOST_TESTS OR XWALK_GPIO_BUILD_HARDWARE_TESTS
    OR XWALK_GPIO_BUILD_LINUX_BACKEND)
    find_package(Threads REQUIRED)
    add_library(xWalkGpioLinux STATIC
        hardware/src/xHal_Rpi5CarGpioLinux.cpp
        hardware/src/xHal_Rpi5CarGpioLinuxLifecycle.cpp
        hardware/src/xHal_Rpi5CarGpioDeviceLinux.cpp
    )
    target_include_directories(xWalkGpioLinux PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkGpioLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkGpioLinux
        PUBLIC xWalkGpio Threads::Threads
        PRIVATE xWalkTrace)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkGpioLinux PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_GPIO_BUILD_HARDWARE_TESTS)
        add_executable(xWalkGpioHardwareTest hardware/test/src/xHal_Rpi5CarGpioHardwareTest.cpp)
        target_link_libraries(xWalkGpioHardwareTest PRIVATE xWalkGpioLinux)
        add_test(NAME xWalkGpioHardwareOutputTest COMMAND xWalkGpioHardwareTest
            ${XWALK_GPIO_HARDWARE_DEVICE}
            "${XWALK_GPIO_HARDWARE_CHIP_NAME}"
            "${XWALK_GPIO_HARDWARE_CHIP_LABEL}")
        set_tests_properties(xWalkGpioHardwareOutputTest PROPERTIES LABELS "hardware")
    endif()
endif()

if(XWALK_GPIO_BUILD_HOST_TESTS)
    add_subdirectory(test)
endif()
