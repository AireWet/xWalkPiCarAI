cmake_minimum_required(VERSION 3.16)

project(xWalkVideoStreaming LANGUAGES CXX)

option(XWALK_VIDEO_STREAMING_BUILD_HOST_TESTS
    "Build deterministic in-process MJPEG stream tests" OFF)

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()
if(NOT TARGET xWalkCamera)
    set(XWALK_CAMERA_BUILD_HOST_TESTS OFF CACHE BOOL
        "Video-streaming camera dependency setting" FORCE)
    set(XWALK_CAMERA_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Video-streaming camera dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/device/xWalkCamera
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCamera)
endif()

add_library(xWalkVideoStreaming STATIC
    src/xAgent_Rpi5CarMjpegStream.cpp
    src/xAgent_Rpi5CarMjpegHttpServer.cpp
    src/xAgent_Rpi5CarVideoStreaming.cpp)
add_library(xWalk::VideoStreaming ALIAS xWalkVideoStreaming)
target_include_directories(xWalkVideoStreaming PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVideoStreaming PUBLIC cxx_std_17)
target_link_libraries(xWalkVideoStreaming PUBLIC xWalkLibraryCommon xWalkCamera)
target_link_libraries(xWalkVideoStreaming PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVideoStreaming PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_VIDEO_STREAMING_BUILD_HOST_TESTS)
    include(CTest)
    find_package(GTest CONFIG REQUIRED)
    add_executable(xWalkVideoStreamingTest
        test/src/xAgent_Rpi5CarMjpegStreamTest.cpp
        test/src/xAgent_Rpi5CarMjpegHttpServerTest.cpp
        test/src/xAgent_Rpi5CarMjpegStreamTestSupport.cpp)
    target_include_directories(xWalkVideoStreamingTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkVideoStreamingTest PRIVATE
        xWalkVideoStreaming GTest::gtest_main)
    add_test(NAME xWalkVideoStreamingHostTest COMMAND xWalkVideoStreamingTest)
    set_tests_properties(xWalkVideoStreamingHostTest PROPERTIES
        LABELS "host;vision;streaming;backpressure;fault-injection")
endif()
