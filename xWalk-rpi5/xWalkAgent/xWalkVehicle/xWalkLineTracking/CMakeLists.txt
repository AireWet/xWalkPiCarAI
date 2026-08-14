cmake_minimum_required(VERSION 3.16)

project(xWalkLineTracking LANGUAGES CXX)

option(XWALK_LINE_TRACKING_BUILD_HOST_TESTS "Build xWalkLineTracking host tests" OFF)
option(XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS "Compile xWalkLineTracking with RPi dependencies" OFF)

if(XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkLineTracking RPi mode requires Linux support")
endif()

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL "Line-tracking dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS ${XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS}
        CACHE BOOL "Line-tracking RPi dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkPicarx ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

add_library(xWalkLineTracking STATIC
    src/xAgent_Rpi5CarLineTracking.cpp
    src/xAgent_Rpi5CarLineTrackingLifecycle.cpp
)
target_link_libraries(xWalkLineTracking PRIVATE xWalkTrace)
add_library(xWalk::LineTracking ALIAS xWalkLineTracking)

target_include_directories(xWalkLineTracking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLineTracking PUBLIC cxx_std_17)
target_link_libraries(xWalkLineTracking PUBLIC xWalkPicarx)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLineTracking PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_LINE_TRACKING_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkLineTrackingTest test/src/xAgent_Rpi5CarLineTrackingTest.cpp)
    target_link_libraries(xWalkLineTrackingTest PRIVATE xWalkLineTracking)
    add_test(NAME xWalkLineTrackingHostTest COMMAND xWalkLineTrackingTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/line-tracking-test.conf)
    set_tests_properties(xWalkLineTrackingHostTest PROPERTIES LABELS "host")
endif()
