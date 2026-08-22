cmake_minimum_required(VERSION 3.16)

project(xWalkScheduler LANGUAGES CXX)

option(XWALK_SCHEDULER_BUILD_HOST_TESTS "Build Controller scheduler host tests" OFF)

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkScheduler STATIC
    src/xControllerScheduler.cpp
    src/xControllerSchedulerRuntime.cpp
    src/xControllerSchedulerSignal.cpp)
add_library(xWalk::Scheduler ALIAS xWalkScheduler)

target_include_directories(xWalkScheduler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common/include)
target_compile_features(xWalkScheduler PUBLIC cxx_std_17)
target_link_libraries(xWalkScheduler
    PUBLIC xWalkLibraryCommon
    PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkScheduler PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_SCHEDULER_BUILD_HOST_TESTS)
    include(CTest)
    find_package(GTest CONFIG REQUIRED)
    add_executable(xWalkSchedulerTest
        test/src/xControllerSchedulerTest.cpp
        test/src/xControllerSchedulerSignalTest.cpp
        test/src/xControllerSchedulerTestSupport.cpp)
    target_include_directories(xWalkSchedulerTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkHandler/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkAgent/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkHal/include)
    target_compile_features(xWalkSchedulerTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkSchedulerTest PRIVATE
        GTest::gtest_main
        xWalkScheduler)
    add_test(NAME xWalkSchedulerHostTest COMMAND xWalkSchedulerTest)
    set_tests_properties(xWalkSchedulerHostTest PROPERTIES
        LABELS "host;scheduler"
        TIMEOUT 20)
    add_test(NAME xWalkSchedulerNamingTest
        COMMAND ${CMAKE_COMMAND}
            -DXWALK_SOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/../..
            -P ${CMAKE_CURRENT_SOURCE_DIR}/test/xControllerSchedulerNamingCheck.cmake)
    set_tests_properties(xWalkSchedulerNamingTest PROPERTIES
        LABELS "host;scheduler;naming"
        TIMEOUT 10)
endif()
