cmake_minimum_required(VERSION 3.16)

project(xWalkTreasureHunt LANGUAGES CXX)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Treasure-hunt dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Treasure-hunt dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkComputerVision)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS OFF CACHE BOOL
        "Treasure-hunt dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND OFF CACHE BOOL
        "Treasure-hunt dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Treasure-hunt dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkComputerVision
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkComputerVision)
endif()

if(NOT TARGET xWalkGPT)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/xWalkGPT
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkGPT)
endif()

add_library(xWalkTreasureHunt STATIC
    src/xAgent_Rpi5CarTreasureHunt.cpp
    src/xAgent_Rpi5CarTreasureHuntLifecycle.cpp)
add_library(xWalk::TreasureHunt ALIAS xWalkTreasureHunt)
target_include_directories(xWalkTreasureHunt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkTreasureHunt PUBLIC cxx_std_17)
target_link_libraries(xWalkTreasureHunt PUBLIC
    xWalkPicarx xWalkComputerVision xWalkGPT)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkTreasureHunt PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
