cmake_minimum_required(VERSION 3.16)

project(xWalkFaceTracking LANGUAGES CXX)

option(XWALK_FACE_TRACKING_BUILD_HOST_TESTS
    "Build xWalkFaceTracking host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Face-tracking dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Face-tracking dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkComputerVision)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS OFF CACHE BOOL
        "Face-tracking dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND OFF CACHE BOOL
        "Face-tracking dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Face-tracking dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkComputerVision
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkComputerVision)
endif()

add_library(xWalkFaceTracking STATIC
    src/xAgent_Rpi5CarFaceTracking.cpp
    src/xAgent_Rpi5CarFaceTrackingLifecycle.cpp)
add_library(xWalk::FaceTracking ALIAS xWalkFaceTracking)

target_include_directories(xWalkFaceTracking PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkFaceTracking PUBLIC cxx_std_17)
target_link_libraries(xWalkFaceTracking PUBLIC xWalkPicarx xWalkComputerVision)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkFaceTracking PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
