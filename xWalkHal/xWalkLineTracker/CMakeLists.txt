cmake_minimum_required(VERSION 3.16)
project(xWalkLineTracker LANGUAGES CXX)

option(XWALK_LINE_TRACKER_BUILD_HOST_TESTS "Build xWalkLineTracker host tests" OFF)
option(XWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS "Compile xWalkLineTracker hardware dependencies" OFF)

set(XWALK_ADC_BUILD_HOST_TESTS ${XWALK_LINE_TRACKER_BUILD_HOST_TESTS})
set(XWALK_ADC_BUILD_HARDWARE_TESTS ${XWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS})

if(XWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkLineTracker hardware dependencies require Linux i2c-dev")
endif()

if(XWALK_LINE_TRACKER_BUILD_HOST_TESTS OR XWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkAdc)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkAdc ${CMAKE_CURRENT_BINARY_DIR}/xWalkAdc)
endif()

add_library(xWalkLineTracker STATIC
    src/xHal_Rpi5CarGrayscaleModule.cpp
    src/xHal_Rpi5CarLineTrackerCalibration.cpp
    src/xHal_Rpi5CarLineTrackerLifecycle.cpp
    src/xHal_Rpi5CarLineTrackerReading.cpp
)

target_include_directories(xWalkLineTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLineTracker PUBLIC cxx_std_17)
target_link_libraries(xWalkLineTracker PUBLIC xWalkAdc xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLineTracker PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_LINE_TRACKER_BUILD_HOST_TESTS)
    add_executable(xWalkLineTrackerTest test/src/xHal_Rpi5CarLineTrackerTest.cpp)
    target_link_libraries(xWalkLineTrackerTest PRIVATE xWalkLineTracker)
    add_test(NAME xWalkLineTrackerHostTest COMMAND xWalkLineTrackerTest)
    set_tests_properties(xWalkLineTrackerHostTest PROPERTIES LABELS "host")
endif()
