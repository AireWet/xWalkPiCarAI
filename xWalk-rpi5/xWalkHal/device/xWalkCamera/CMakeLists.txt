cmake_minimum_required(VERSION 3.16)
project(xWalkCamera LANGUAGES CXX)

option(XWALK_CAMERA_BUILD_HOST_TESTS "Build xWalkCamera host tests" OFF)
option(XWALK_CAMERA_BUILD_HARDWARE_TESTS "Build xWalkCamera hardware test" OFF)
option(XWALK_CAMERA_BUILD_LINUX_BACKEND "Build the xWalkCamera Linux backend" OFF)
option(XWALK_CAMERA_BUILD_OPENCV_BACKEND "Build OpenCV encoded camera streaming" OFF)

set(XWALK_CAMERA_HARDWARE_CONNECTION "usb" CACHE STRING
    "Physical camera connection used by the opt-in hardware test")
set(XWALK_CAMERA_HARDWARE_EXECUTABLE "ffmpeg" CACHE STRING
    "Capture executable used by the opt-in hardware test")
set(XWALK_CAMERA_HARDWARE_DEVICE "/dev/video0" CACHE STRING
    "USB V4L2 device used by the opt-in hardware test")
set(XWALK_CAMERA_HARDWARE_OUTPUT "/tmp/xwalk-camera-hardware-test.jpg" CACHE STRING
    "JPEG destination used by the opt-in hardware test")

if((XWALK_CAMERA_BUILD_HARDWARE_TESTS OR XWALK_CAMERA_BUILD_LINUX_BACKEND OR
    XWALK_CAMERA_BUILD_OPENCV_BACKEND)
    AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkCamera Linux capture requires Linux process support")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkCamera STATIC
    core/src/xHal_Rpi5CarCamera.cpp
    core/src/xHal_Rpi5CarCameraStream.cpp)
add_library(xWalk::Camera ALIAS xWalkCamera)
target_include_directories(xWalkCamera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
target_compile_features(xWalkCamera PUBLIC cxx_std_17)
target_link_libraries(xWalkCamera PUBLIC xWalkLibraryCommon PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkCamera PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CAMERA_BUILD_OPENCV_BACKEND OR XWALK_CAMERA_BUILD_HOST_TESTS)
    find_package(OpenCV REQUIRED COMPONENTS core imgcodecs videoio)
    add_library(xWalkCameraStreamOpenCv STATIC
        hardware/src/xHal_Rpi5CarCameraStreamOpenCv.cpp)
    add_library(xWalk::CameraStreamOpenCv ALIAS xWalkCameraStreamOpenCv)
    target_include_directories(xWalkCameraStreamOpenCv PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkCameraStreamOpenCv PUBLIC cxx_std_17)
    target_link_libraries(xWalkCameraStreamOpenCv PUBLIC xWalkCamera ${OpenCV_LIBS})
    target_link_libraries(xWalkCameraStreamOpenCv PRIVATE xWalkTrace)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkCameraStreamOpenCv PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_CAMERA_BUILD_HOST_TESTS OR XWALK_CAMERA_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_CAMERA_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_CAMERA_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_CAMERA_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarCameraTraceConfig.py)
    set(XWALK_CAMERA_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkCameraTrace.xml)
    add_custom_command(OUTPUT ${XWALK_CAMERA_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_CAMERA_TRACE_GENERATOR}
            --input ${XWALK_CAMERA_TRACE_INVENTORY}
            --output ${XWALK_CAMERA_TRACE_CONFIG}
        DEPENDS ${XWALK_CAMERA_TRACE_INVENTORY} ${XWALK_CAMERA_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkCamera trace catalogue" VERBATIM)
    add_custom_target(xWalkCameraTestTraceConfig ALL DEPENDS ${XWALK_CAMERA_TRACE_CONFIG})
    add_dependencies(xWalkCameraTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkCameraTest
        test/src/xHal_Rpi5CarCameraTest.cpp
        test/src/xHal_Rpi5CarCameraTestSupport.cpp
        simulation/src/xHal_Rpi5CarCameraSimulationArguments.cpp)
    target_include_directories(xWalkCameraTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_CAMERA_TEST_TRACE_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkCameraTest.log)
    target_compile_definitions(xWalkCameraTest PRIVATE
        XWALK_CAMERA_SIMULATION_TRACE_CONFIG_PATH="${XWALK_CAMERA_TRACE_CONFIG}"
        XWALK_CAMERA_SIMULATION_TRACE_LOG_PATH="${XWALK_CAMERA_TEST_TRACE_LOG}")
    target_link_libraries(xWalkCameraTest PRIVATE xWalkCameraStreamOpenCv xWalkTrace)
    add_dependencies(xWalkCameraTest xWalkCameraTestTraceConfig)
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
    target_link_libraries(xWalkCameraLinux PUBLIC xWalkCamera PRIVATE xWalkTrace)
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
