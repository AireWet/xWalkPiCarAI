cmake_minimum_required(VERSION 3.16)
project(xWalkSpi LANGUAGES CXX)

option(XWALK_SPI_BUILD_HOST_TESTS "Build xWalkSpi host tests" OFF)
option(XWALK_SPI_BUILD_HARDWARE_TESTS "Build xWalkSpi RPi hardware test" OFF)
option(XWALK_SPI_BUILD_LINUX_BACKEND "Build the xWalkSpi Linux backend" OFF)
set(XWALK_SPI_HARDWARE_DEVICE "/dev/spidev0.0" CACHE FILEPATH
    "Linux SPI device used by the opt-in hardware test")

if((XWALK_SPI_BUILD_HARDWARE_TESTS OR XWALK_SPI_BUILD_LINUX_BACKEND)
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkSpi Linux support requires Linux spidev")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

add_library(xWalkSpi STATIC
    core/src/xHal_Rpi5CarSpi.cpp
    core/src/xHal_Rpi5CarSpiLifecycle.cpp)
add_library(xWalk::Spi ALIAS xWalkSpi)
target_include_directories(xWalkSpi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
target_compile_features(xWalkSpi PUBLIC cxx_std_17)
target_link_libraries(xWalkSpi PUBLIC xWalkCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkSpi PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SPI_BUILD_HOST_TESTS OR XWALK_SPI_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_SPI_BUILD_HOST_TESTS)
    add_executable(xWalkSpiTest test/src/xHal_Rpi5CarSpiTest.cpp)
    target_link_libraries(xWalkSpiTest PRIVATE xWalkSpi)
    add_test(NAME xWalkSpiHostTest COMMAND xWalkSpiTest)
    set_tests_properties(xWalkSpiHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_SPI_BUILD_HARDWARE_TESTS OR XWALK_SPI_BUILD_LINUX_BACKEND)
    add_library(xWalkSpiLinux STATIC
        hardware/src/xHal_Rpi5CarSpiLinux.cpp
        hardware/src/xHal_Rpi5CarSpiLinuxLifecycle.cpp)
    target_include_directories(xWalkSpiLinux PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkSpiLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkSpiLinux PUBLIC xWalkSpi)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkSpiLinux PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_SPI_BUILD_HARDWARE_TESTS)
        add_executable(xWalkSpiLinuxHardwareTest
            hardware/test/src/xHal_Rpi5CarSpiLinuxHardwareTest.cpp)
        target_link_libraries(xWalkSpiLinuxHardwareTest PRIVATE xWalkSpiLinux)
        add_test(NAME xWalkSpiLinuxHardwareTransferTest
            COMMAND xWalkSpiLinuxHardwareTest ${XWALK_SPI_HARDWARE_DEVICE})
        set_tests_properties(xWalkSpiLinuxHardwareTransferTest PROPERTIES LABELS "hardware")
    endif()
endif()
