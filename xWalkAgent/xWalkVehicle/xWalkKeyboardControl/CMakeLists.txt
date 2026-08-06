cmake_minimum_required(VERSION 3.16)

project(xWalkKeyboardControl LANGUAGES CXX)

option(XWALK_KEYBOARD_CONTROL_BUILD_HOST_TESTS
    "Build xWalkKeyboardControl host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Keyboard-control dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Keyboard-control dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

add_library(xWalkKeyboardControl STATIC
    src/xAgent_Rpi5CarKeyboardControl.cpp
    src/xAgent_Rpi5CarKeyboardControlLifecycle.cpp)
add_library(xWalk::KeyboardControl ALIAS xWalkKeyboardControl)

target_include_directories(xWalkKeyboardControl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkKeyboardControl PUBLIC cxx_std_17)
target_link_libraries(xWalkKeyboardControl PUBLIC xWalkPicarx)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkKeyboardControl PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_KEYBOARD_CONTROL_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkKeyboardControlTest
        test/src/xAgent_Rpi5CarKeyboardControlTest.cpp)
    target_link_libraries(xWalkKeyboardControlTest PRIVATE xWalkKeyboardControl)
    add_test(NAME xWalkKeyboardControlHostTest COMMAND xWalkKeyboardControlTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/keyboard-control-test.conf)
    set_tests_properties(xWalkKeyboardControlHostTest PROPERTIES LABELS "host")
endif()
