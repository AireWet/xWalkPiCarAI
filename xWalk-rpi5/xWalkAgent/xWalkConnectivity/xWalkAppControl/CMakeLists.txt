cmake_minimum_required(VERSION 3.16)

project(xWalkAppControl LANGUAGES CXX)

option(XWALK_APP_CONTROL_BUILD_WEBSOCKET_BACKEND
    "Build the SunFounder-compatible WebSocket provider" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "App-control dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "App-control dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkComputerVision)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS OFF CACHE BOOL
        "App-control dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_OPENCV_BACKEND OFF CACHE BOOL
        "App-control dependency setting" FORCE)
    set(XWALK_COMPUTER_VISION_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "App-control dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkComputerVision
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkComputerVision)
endif()

add_library(xWalkAppControl STATIC
    src/xAgent_Rpi5CarAppControl.cpp
    src/xAgent_Rpi5CarAppControlLifecycle.cpp)
target_link_libraries(xWalkAppControl PRIVATE xWalkTrace)
add_library(xWalk::AppControl ALIAS xWalkAppControl)
target_include_directories(xWalkAppControl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAppControl PUBLIC cxx_std_17)
target_link_libraries(xWalkAppControl PUBLIC xWalkPicarx xWalkComputerVision)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAppControl PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_APP_CONTROL_BUILD_WEBSOCKET_BACKEND)
    find_package(Boost 1.74 REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JSON_C REQUIRED IMPORTED_TARGET json-c)
    find_package(Threads REQUIRED)
    add_library(xWalkAppControlWebSocket STATIC
        hardware/src/xAgent_Rpi5CarAppControlWebSocket.cpp
        hardware/src/xAgent_Rpi5CarAppControlWebSocketState.cpp)
    add_library(xWalk::AppControlWebSocket ALIAS xWalkAppControlWebSocket)
    target_include_directories(xWalkAppControlWebSocket PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hardware/src)
    target_compile_features(xWalkAppControlWebSocket PUBLIC cxx_std_17)
    target_compile_definitions(xWalkAppControlWebSocket PRIVATE
        BOOST_ERROR_CODE_HEADER_ONLY=1)
    target_link_libraries(xWalkAppControlWebSocket PUBLIC
        xWalkAppControl Threads::Threads PRIVATE PkgConfig::JSON_C xWalkTrace)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkAppControlWebSocket PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()
