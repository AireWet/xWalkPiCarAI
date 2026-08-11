cmake_minimum_required(VERSION 3.16)

project(xWalkMoveExample LANGUAGES CXX)

option(XWALK_MOVE_EXAMPLE_BUILD_HOST_TESTS "Build xWalkMoveExample host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL "Move-example dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Move-example dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

add_library(xWalkMoveExample STATIC
    src/xAgent_Rpi5CarMoveExample.cpp
    src/xAgent_Rpi5CarMoveExampleLifecycle.cpp)
target_link_libraries(xWalkMoveExample PRIVATE xWalkTrace)
add_library(xWalk::MoveExample ALIAS xWalkMoveExample)

target_include_directories(xWalkMoveExample PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkMoveExample PUBLIC cxx_std_17)
target_link_libraries(xWalkMoveExample PUBLIC xWalkPicarx)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkMoveExample PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_MOVE_EXAMPLE_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkMoveExampleTest test/src/xAgent_Rpi5CarMoveExampleTest.cpp)
    target_link_libraries(xWalkMoveExampleTest PRIVATE xWalkMoveExample)
    add_test(NAME xWalkMoveExampleHostTest COMMAND xWalkMoveExampleTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/move-example-test.conf)
    set_tests_properties(xWalkMoveExampleHostTest PROPERTIES LABELS "host")
endif()
