cmake_minimum_required(VERSION 3.16)
project(xWalkUtils LANGUAGES CXX)

option(XWALK_UTILS_BUILD_HOST_TESTS "Build xWalkUtils host tests" OFF)
option(XWALK_UTILS_BUILD_HARDWARE_TESTS "Build xWalkUtils RPi hardware tests" OFF)
option(XWALK_UTILS_BUILD_LINUX_BACKEND "Build the xWalkUtils Linux backend" OFF)

if((XWALK_UTILS_BUILD_HARDWARE_TESTS OR XWALK_UTILS_BUILD_LINUX_BACKEND)
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkUtils Linux backend requires Linux platform APIs")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

add_library(xWalkUtils STATIC
    src/xHal_Rpi5CarStderrGuard.cpp
    src/xHal_Rpi5CarUtils.cpp
    src/xHal_Rpi5CarUtilsLifecycle.cpp
)

target_include_directories(xWalkUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUtils PUBLIC cxx_std_17)
target_link_libraries(xWalkUtils PUBLIC xWalkCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUtils PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_UTILS_BUILD_HOST_TESTS OR XWALK_UTILS_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_UTILS_BUILD_HOST_TESTS)
    add_executable(xWalkUtilsTest test/src/xHal_Rpi5CarUtilsTest.cpp)
    target_link_libraries(xWalkUtilsTest PRIVATE xWalkUtils)
    add_test(NAME xWalkUtilsHostTest COMMAND xWalkUtilsTest)
    set_tests_properties(xWalkUtilsHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_UTILS_BUILD_HARDWARE_TESTS OR XWALK_UTILS_BUILD_LINUX_BACKEND OR
    (XWALK_UTILS_BUILD_HOST_TESTS AND CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    add_library(xWalkUtilsLinux STATIC
        hardware/src/xHal_Rpi5CarUtilsLinux.cpp
        hardware/src/xHal_Rpi5CarUtilsLinuxCallbacks.cpp
    )
    target_include_directories(xWalkUtilsLinux PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include
    )
    target_compile_features(xWalkUtilsLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkUtilsLinux PUBLIC xWalkUtils)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkUtilsLinux PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_UTILS_BUILD_HOST_TESTS)
        add_executable(xWalkUtilsLinuxTest
            hardware/test/src/xHal_Rpi5CarUtilsLinuxTest.cpp
        )
        target_link_libraries(xWalkUtilsLinuxTest PRIVATE xWalkUtilsLinux)
        add_test(NAME xWalkUtilsLinuxSoftwareTest COMMAND xWalkUtilsLinuxTest)
        set_tests_properties(xWalkUtilsLinuxSoftwareTest PROPERTIES LABELS "host")
    endif()

    if(XWALK_UTILS_BUILD_HARDWARE_TESTS)
        add_executable(xWalkUtilsLinuxHardwareTest
            test/hardware/src/xHal_Rpi5CarUtilsHardwareTest.cpp
        )
        target_link_libraries(xWalkUtilsLinuxHardwareTest PRIVATE xWalkUtilsLinux)
        add_test(NAME xWalkUtilsLinuxHardwarePlatformTest
            COMMAND xWalkUtilsLinuxHardwareTest)
        set_tests_properties(xWalkUtilsLinuxHardwarePlatformTest PROPERTIES
            LABELS "hardware")
    endif()
endif()
