cmake_minimum_required(VERSION 3.16)
project(xWalkTrace LANGUAGES CXX)

option(XWALK_TRACE_BUILD_HOST_TESTS "Build xWalkTrace host tests" OFF)
option(XWALK_TRACE_BUILD_HARDWARE_TESTS "Build xWalkTrace target compile tests" OFF)

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

add_library(xWalkTrace STATIC
    src/xHal_Rpi5CarTrace.cpp
    src/xHal_Rpi5CarTraceLifecycle.cpp
)

target_include_directories(xWalkTrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkTrace PUBLIC cxx_std_17)
target_link_libraries(xWalkTrace PUBLIC xWalkCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkTrace PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_TRACE_BUILD_HOST_TESTS OR XWALK_TRACE_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_TRACE_BUILD_HOST_TESTS)
    add_executable(xWalkTraceTest test/src/xHal_Rpi5CarTraceTest.cpp)
    target_link_libraries(xWalkTraceTest PRIVATE xWalkTrace)
    add_test(NAME xWalkTraceHostTest COMMAND xWalkTraceTest)
    set_tests_properties(xWalkTraceHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_TRACE_BUILD_HARDWARE_TESTS)
    add_executable(xWalkTraceHardwareTest
        test/hardware/src/xHal_Rpi5CarTraceHardwareTest.cpp)
    target_link_libraries(xWalkTraceHardwareTest PRIVATE xWalkTrace)
    add_test(NAME xWalkTraceTargetCompileTest COMMAND xWalkTraceHardwareTest)
    set_tests_properties(xWalkTraceTargetCompileTest PROPERTIES LABELS "hardware")
endif()
