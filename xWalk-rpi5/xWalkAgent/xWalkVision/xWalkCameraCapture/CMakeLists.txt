cmake_minimum_required(VERSION 3.16)
project(xWalkCameraCapture LANGUAGES CXX)

option(XWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS
    "Build xWalkCameraCapture host tests" OFF)

if(NOT TARGET xWalkCamera)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/device/xWalkCamera
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCamera)
endif()

add_library(xWalkCameraCapture STATIC
    src/xAgent_Rpi5CarCameraCapture.cpp)
target_link_libraries(xWalkCameraCapture PRIVATE xWalkTrace)
add_library(xWalk::CameraCapture ALIAS xWalkCameraCapture)
target_include_directories(xWalkCameraCapture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkCameraCapture PUBLIC cxx_std_17)
target_link_libraries(xWalkCameraCapture PUBLIC xWalkCamera)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkCameraCapture PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkCameraCaptureTest
        test/src/xAgent_Rpi5CarCameraCaptureTest.cpp)
    target_link_libraries(xWalkCameraCaptureTest PRIVATE xWalkCameraCapture)
    add_test(NAME xWalkCameraCaptureHostTest COMMAND xWalkCameraCaptureTest)
    set_tests_properties(xWalkCameraCaptureHostTest PROPERTIES LABELS "host")
endif()
