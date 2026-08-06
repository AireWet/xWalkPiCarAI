cmake_minimum_required(VERSION 3.16)

project(xWalkVideoCar LANGUAGES CXX)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Video-car dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Video-car dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkComputerVision)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS OFF CACHE BOOL
        "Video-car dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND OFF CACHE BOOL
        "Video-car dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Video-car dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkComputerVision
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkComputerVision)
endif()

add_library(xWalkVideoCar STATIC
    src/xAgent_Rpi5CarVideoCar.cpp
    src/xAgent_Rpi5CarVideoCarLifecycle.cpp)
add_library(xWalk::VideoCar ALIAS xWalkVideoCar)
target_include_directories(xWalkVideoCar PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVideoCar PUBLIC cxx_std_17)
target_link_libraries(xWalkVideoCar PUBLIC xWalkPicarx xWalkComputerVision)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVideoCar PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
