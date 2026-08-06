cmake_minimum_required(VERSION 3.16)

project(xWalkServoZeroing LANGUAGES CXX)

option(XWALK_SERVO_ZEROING_BUILD_HOST_TESTS "Build servo-zeroing host tests" OFF)

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common
        ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

add_library(xWalkServoZeroing STATIC
    src/xAgent_Rpi5CarServoZeroing.cpp
    src/xAgent_Rpi5CarServoZeroingLifecycle.cpp)
add_library(xWalk::ServoZeroing ALIAS xWalkServoZeroing)
target_include_directories(xWalkServoZeroing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkServoZeroing PUBLIC cxx_std_17)
target_link_libraries(xWalkServoZeroing PUBLIC xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkServoZeroing PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SERVO_ZEROING_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkServoZeroingTest
        test/src/xAgent_Rpi5CarServoZeroingTest.cpp)
    target_link_libraries(xWalkServoZeroingTest PRIVATE xWalkServoZeroing)
    add_test(NAME xWalkServoZeroingHostTest COMMAND xWalkServoZeroingTest)
    set_tests_properties(xWalkServoZeroingHostTest PROPERTIES LABELS "host")
endif()
