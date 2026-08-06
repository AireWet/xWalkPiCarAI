cmake_minimum_required(VERSION 3.16)

project(xWalkBullFight LANGUAGES CXX)

option(XWALK_BULL_FIGHT_BUILD_HOST_TESTS "Build xWalkBullFight host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Bull-fight dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Bull-fight dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkComputerVision)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS OFF CACHE BOOL
        "Bull-fight dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND OFF CACHE BOOL
        "Bull-fight dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Bull-fight dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkComputerVision
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkComputerVision)
endif()

add_library(xWalkBullFight STATIC
    src/xAgent_Rpi5CarBullFight.cpp
    src/xAgent_Rpi5CarBullFightLifecycle.cpp)
add_library(xWalk::BullFight ALIAS xWalkBullFight)
target_include_directories(xWalkBullFight PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkBullFight PUBLIC cxx_std_17)
target_link_libraries(xWalkBullFight PUBLIC xWalkPicarx xWalkComputerVision)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkBullFight PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
