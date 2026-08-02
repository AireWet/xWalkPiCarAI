cmake_minimum_required(VERSION 3.16)
project(xWalkCamera LANGUAGES CXX)

option(XWALK_CAMERA_BUILD_HOST_TESTS "Build xWalkCamera host tests" OFF)
option(XWALK_CAMERA_BUILD_HARDWARE_TESTS "Build xWalkCamera hardware test" OFF)
option(XWALK_CAMERA_BUILD_LINUX_BACKEND "Build the xWalkCamera Linux backend" OFF)

set(XWALK_CAMERA_HARDWARE_CONNECTION "usb" CACHE STRING
    "Physical camera connection used by the opt-in hardware test")
set(XWALK_CAMERA_HARDWARE_EXECUTABLE "ffmpeg" CACHE STRING
    "Capture executable used by the opt-in hardware test")
set(XWALK_CAMERA_HARDWARE_DEVICE "/dev/video0" CACHE STRING
    "USB V4L2 device used by the opt-in hardware test")
set(XWALK_CAMERA_HARDWARE_OUTPUT "/tmp/xwalk-camera-hardware-test.jpg" CACHE STRING
    "JPEG destination used by the opt-in hardware test")

if((XWALK_CAMERA_BUILD_HARDWARE_TESTS OR XWALK_CAMERA_BUILD_LINUX_BACKEND)
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkCamera Linux capture requires Linux process support")
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

add_library(xWalkCamera STATIC core/src/xHal_Rpi5CarCamera.cpp)
add_library(xWalk::Camera ALIAS xWalkCamera)
target_include_directories(xWalkCamera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
target_compile_features(xWalkCamera PUBLIC cxx_std_17)
target_link_libraries(xWalkCamera PUBLIC xWalkCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkCamera PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CAMERA_BUILD_HOST_TESTS OR XWALK_CAMERA_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_CAMERA_BUILD_HOST_TESTS)
    add_executable(xWalkCameraTest test/src/xHal_Rpi5CarCameraTest.cpp)
    target_link_libraries(xWalkCameraTest PRIVATE xWalkCamera)
    add_test(NAME xWalkCameraHostTest COMMAND xWalkCameraTest)
    set_tests_properties(xWalkCameraHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_CAMERA_BUILD_HARDWARE_TESTS OR XWALK_CAMERA_BUILD_LINUX_BACKEND)
    add_library(xWalkCameraLinux STATIC
        hardware/src/xHal_Rpi5CarCameraLinux.cpp)
    add_library(xWalk::CameraLinux ALIAS xWalkCameraLinux)
    target_include_directories(xWalkCameraLinux PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkCameraLinux PUBLIC cxx_std_17)
    target_link_libraries(xWalkCameraLinux PUBLIC xWalkCamera)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkCameraLinux PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()

    if(XWALK_CAMERA_BUILD_HARDWARE_TESTS)
        add_executable(xWalkCameraHardwareTest
            hardware/test/src/xHal_Rpi5CarCameraHardwareTest.cpp)
        target_link_libraries(xWalkCameraHardwareTest PRIVATE xWalkCameraLinux)
        add_test(NAME xWalkCameraStillHardwareTest COMMAND xWalkCameraHardwareTest
            ${XWALK_CAMERA_HARDWARE_CONNECTION}
            ${XWALK_CAMERA_HARDWARE_EXECUTABLE}
            ${XWALK_CAMERA_HARDWARE_DEVICE}
            ${XWALK_CAMERA_HARDWARE_OUTPUT})
        set_tests_properties(xWalkCameraStillHardwareTest PROPERTIES LABELS "hardware")
    endif()
endif()
