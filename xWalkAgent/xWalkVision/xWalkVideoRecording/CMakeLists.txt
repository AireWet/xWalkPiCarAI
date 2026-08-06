cmake_minimum_required(VERSION 3.16)

project(xWalkVideoRecording LANGUAGES CXX)

option(XWALK_VIDEO_RECORDING_BUILD_HOST_TESTS
    "Build xWalkVideoRecording host tests" OFF)
option(XWALK_VIDEO_RECORDING_BUILD_OPENCV_BACKEND
    "Build the OpenCV video-recording provider" OFF)
option(XWALK_VIDEO_RECORDING_BUILD_HARDWARE_TESTS
    "Build the opt-in physical-camera recording test" OFF)

if(XWALK_VIDEO_RECORDING_BUILD_HARDWARE_TESTS AND
    NOT XWALK_VIDEO_RECORDING_BUILD_OPENCV_BACKEND)
    message(FATAL_ERROR "Video hardware tests require the OpenCV backend")
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

add_library(xWalkVideoRecording STATIC
    src/xAgent_Rpi5CarVideoRecording.cpp
    src/xAgent_Rpi5CarVideoRecordingLifecycle.cpp)
add_library(xWalk::VideoRecording ALIAS xWalkVideoRecording)
target_include_directories(xWalkVideoRecording PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVideoRecording PUBLIC cxx_std_17)
target_link_libraries(xWalkVideoRecording PUBLIC xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVideoRecording PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_VIDEO_RECORDING_BUILD_OPENCV_BACKEND)
    find_package(OpenCV 4 REQUIRED COMPONENTS core videoio)
    find_package(Threads REQUIRED)
    add_library(xWalkVideoRecordingOpenCv STATIC
        hardware/src/xAgent_Rpi5CarVideoRecordingOpenCv.cpp)
    add_library(xWalk::VideoRecordingOpenCv ALIAS xWalkVideoRecordingOpenCv)
    target_include_directories(xWalkVideoRecordingOpenCv PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_include_directories(xWalkVideoRecordingOpenCv SYSTEM PUBLIC
        ${OpenCV_INCLUDE_DIRS})
    target_compile_features(xWalkVideoRecordingOpenCv PUBLIC cxx_std_17)
    target_link_libraries(xWalkVideoRecordingOpenCv
        PUBLIC xWalkVideoRecording
        PRIVATE ${OpenCV_LIBS} Threads::Threads)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkVideoRecordingOpenCv PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(XWALK_VIDEO_RECORDING_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkVideoRecordingHardwareTest
        hardware/test/src/xAgent_Rpi5CarVideoRecordingHardwareTest.cpp)
    target_link_libraries(xWalkVideoRecordingHardwareTest PRIVATE
        xWalkVideoRecordingOpenCv)
    add_test(NAME xWalkVideoRecordingHardwareTest
        COMMAND xWalkVideoRecordingHardwareTest
            /dev/video0 /tmp/xwalk-video-recording-hardware-test)
    set_tests_properties(xWalkVideoRecordingHardwareTest PROPERTIES
        LABELS "hardware;camera;recording")
endif()

if(XWALK_VIDEO_RECORDING_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkVideoRecordingTest
        test/src/xAgent_Rpi5CarVideoRecordingTest.cpp)
    target_link_libraries(xWalkVideoRecordingTest PRIVATE xWalkVideoRecording)
    add_test(NAME xWalkVideoRecordingHostTest COMMAND xWalkVideoRecordingTest)
    set_tests_properties(xWalkVideoRecordingHostTest PROPERTIES LABELS "host")
endif()
