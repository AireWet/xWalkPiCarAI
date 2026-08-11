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
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkAdc)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../device/xWalkAdc ${CMAKE_CURRENT_BINARY_DIR}/xWalkAdc)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkLineTracker STATIC
    src/xHal_Rpi5CarGrayscaleModule.cpp
    src/xHal_Rpi5CarLineTrackerCalibration.cpp
    src/xHal_Rpi5CarLineTrackerLifecycle.cpp
    src/xHal_Rpi5CarLineTrackerReading.cpp
)

target_include_directories(xWalkLineTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLineTracker PUBLIC cxx_std_17)
target_link_libraries(xWalkLineTracker PUBLIC xWalkAdc xWalkLibraryCommon PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLineTracker PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_LINE_TRACKER_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_LINE_TRACKER_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_LINE_TRACKER_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarLineTrackerTraceConfig.py)
    set(XWALK_LINE_TRACKER_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLineTrackerTrace.xml)
    add_custom_command(OUTPUT ${XWALK_LINE_TRACKER_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_LINE_TRACKER_TRACE_GENERATOR}
            --input ${XWALK_LINE_TRACKER_TRACE_INVENTORY}
            --output ${XWALK_LINE_TRACKER_TRACE_CONFIG}
        DEPENDS ${XWALK_LINE_TRACKER_TRACE_INVENTORY} ${XWALK_LINE_TRACKER_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkLineTracker trace catalogue" VERBATIM)
    add_custom_target(xWalkLineTrackerTestTraceConfig ALL
        DEPENDS ${XWALK_LINE_TRACKER_TRACE_CONFIG})
    add_dependencies(xWalkLineTrackerTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkLineTrackerTest
        test/src/xHal_Rpi5CarLineTrackerTest.cpp
        test/src/xHal_Rpi5CarLineTrackerTestSupport.cpp
        simulation/src/xHal_Rpi5CarLineTrackerSimulationArguments.cpp)
    target_include_directories(xWalkLineTrackerTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_LINE_TRACKER_TEST_TRACE_LOG
        ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkLineTrackerTest.log)
    target_compile_definitions(xWalkLineTrackerTest PRIVATE
        XWALK_LINE_TRACKER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_LINE_TRACKER_TRACE_CONFIG}"
        XWALK_LINE_TRACKER_SIMULATION_TRACE_LOG_PATH="${XWALK_LINE_TRACKER_TEST_TRACE_LOG}")
    target_link_libraries(xWalkLineTrackerTest PRIVATE xWalkLineTracker xWalkTrace)
    add_dependencies(xWalkLineTrackerTest xWalkLineTrackerTestTraceConfig)
    add_test(NAME xWalkLineTrackerHostTest COMMAND xWalkLineTrackerTest)
    set_tests_properties(xWalkLineTrackerHostTest PROPERTIES LABELS "host")
endif()
