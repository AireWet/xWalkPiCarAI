cmake_minimum_required(VERSION 3.16)
project(xWalkUserButton LANGUAGES CXX)

option(XWALK_USER_BUTTON_BUILD_HOST_TESTS "Build xWalkUserButton host tests" OFF)
option(XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS "Build xWalkUserButton hardware tests" OFF)

set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_USER_BUTTON_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS})

if(XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkUserButton hardware tests require the Linux GPIO API")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

add_library(xWalkUserButton STATIC
    src/xHal_Rpi5CarUserButton.cpp
    src/xHal_Rpi5CarUserButtonLifecycle.cpp
)

target_include_directories(xWalkUserButton PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUserButton PUBLIC cxx_std_17)
target_link_libraries(xWalkUserButton PUBLIC xWalkCommon xWalkGpio)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUserButton PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_USER_BUTTON_BUILD_HOST_TESTS OR XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_USER_BUTTON_BUILD_HOST_TESTS)
    add_executable(xWalkUserButtonTest test/src/xHal_Rpi5CarUserButtonTest.cpp)
    target_link_libraries(xWalkUserButtonTest PRIVATE xWalkUserButton)
    add_test(NAME xWalkUserButtonHostTest COMMAND xWalkUserButtonTest)
    set_tests_properties(xWalkUserButtonHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_USER_BUTTON_BUILD_HARDWARE_TESTS)
    add_executable(xWalkUserButtonHardwareTest
        test/hardware/src/xHal_Rpi5CarUserButtonHardwareTest.cpp)
    target_link_libraries(xWalkUserButtonHardwareTest PRIVATE xWalkUserButton xWalkGpioLinux)
    add_test(NAME xWalkUserButtonHardwareMonitorTest COMMAND xWalkUserButtonHardwareTest)
    set_tests_properties(xWalkUserButtonHardwareMonitorTest PROPERTIES LABELS "hardware")
endif()
