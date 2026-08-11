cmake_minimum_required(VERSION 3.16)
project(xWalkUltrasonic LANGUAGES CXX)

option(XWALK_ULTRASONIC_BUILD_HOST_TESTS "Build xWalkUltrasonic host tests" OFF)
option(XWALK_ULTRASONIC_BUILD_HARDWARE_TESTS "Compile xWalkUltrasonic hardware dependencies" OFF)

set(XWALK_GPIO_BUILD_HOST_TESTS ${XWALK_ULTRASONIC_BUILD_HOST_TESTS})
set(XWALK_GPIO_BUILD_HARDWARE_TESTS ${XWALK_ULTRASONIC_BUILD_HARDWARE_TESTS})

if(XWALK_ULTRASONIC_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkUltrasonic hardware dependencies require Linux GPIO support")
endif()

if(XWALK_ULTRASONIC_BUILD_HOST_TESTS OR XWALK_ULTRASONIC_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkUltrasonic STATIC
    src/xHal_Rpi5CarUltrasonic.cpp
    src/xHal_Rpi5CarUltrasonicLifecycle.cpp
)

target_include_directories(xWalkUltrasonic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUltrasonic PUBLIC cxx_std_17)
target_link_libraries(xWalkUltrasonic PUBLIC xWalkLibraryCommon xWalkGpio PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUltrasonic PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ULTRASONIC_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_ULTRASONIC_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_ULTRASONIC_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarUltrasonicTraceConfig.py)
    set(XWALK_ULTRASONIC_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUltrasonicTrace.xml)
    add_custom_command(OUTPUT ${XWALK_ULTRASONIC_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_ULTRASONIC_TRACE_GENERATOR}
            --input ${XWALK_ULTRASONIC_TRACE_INVENTORY}
            --output ${XWALK_ULTRASONIC_TRACE_CONFIG}
        DEPENDS ${XWALK_ULTRASONIC_TRACE_INVENTORY} ${XWALK_ULTRASONIC_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkUltrasonic trace catalogue" VERBATIM)
    add_custom_target(xWalkUltrasonicTestTraceConfig ALL
        DEPENDS ${XWALK_ULTRASONIC_TRACE_CONFIG})
    add_dependencies(xWalkUltrasonicTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkUltrasonicTest
        test/src/xHal_Rpi5CarUltrasonicTest.cpp
        test/src/xHal_Rpi5CarUltrasonicTestSupport.cpp
        simulation/src/xHal_Rpi5CarUltrasonicSimulationArguments.cpp)
    target_include_directories(xWalkUltrasonicTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(XWALK_ULTRASONIC_TEST_TRACE_LOG
        ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkUltrasonicTest.log)
    target_compile_definitions(xWalkUltrasonicTest PRIVATE
        XWALK_ULTRASONIC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ULTRASONIC_TRACE_CONFIG}"
        XWALK_ULTRASONIC_SIMULATION_TRACE_LOG_PATH="${XWALK_ULTRASONIC_TEST_TRACE_LOG}")
    target_link_libraries(xWalkUltrasonicTest PRIVATE xWalkUltrasonic xWalkTrace)
    add_dependencies(xWalkUltrasonicTest xWalkUltrasonicTestTraceConfig)
    add_test(NAME xWalkUltrasonicHostTest COMMAND xWalkUltrasonicTest)
    set_tests_properties(xWalkUltrasonicHostTest PROPERTIES LABELS "host")
endif()
