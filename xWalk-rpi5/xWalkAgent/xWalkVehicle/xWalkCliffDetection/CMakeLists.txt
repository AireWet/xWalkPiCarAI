cmake_minimum_required(VERSION 3.16)

project(xWalkCliffDetection LANGUAGES CXX)

option(XWALK_CLIFF_DETECTION_BUILD_HOST_TESTS
    "Build xWalkCliffDetection host tests" OFF)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Cliff-detection dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Cliff-detection dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

add_library(xWalkCliffDetection STATIC
    src/xAgent_Rpi5CarCliffDetection.cpp
    src/xAgent_Rpi5CarCliffDetectionLifecycle.cpp)
target_link_libraries(xWalkCliffDetection PRIVATE xWalkTrace)
add_library(xWalk::CliffDetection ALIAS xWalkCliffDetection)

target_include_directories(xWalkCliffDetection PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkCliffDetection PUBLIC cxx_std_17)
target_link_libraries(xWalkCliffDetection PUBLIC xWalkPicarx)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkCliffDetection PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_CLIFF_DETECTION_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkCliffDetectionTest
        test/src/xAgent_Rpi5CarCliffDetectionTest.cpp)
    target_include_directories(xWalkCliffDetectionTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkCliffDetectionTest PRIVATE xWalkCliffDetection)
    add_test(NAME xWalkCliffDetectionHostTest COMMAND xWalkCliffDetectionTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/cliff-detection-test.conf)
    set_tests_properties(xWalkCliffDetectionHostTest PROPERTIES LABELS "host")
endif()
