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

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(NOT TARGET xWalkGpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGpio ${CMAKE_CURRENT_BINARY_DIR}/xWalkGpio)
endif()

add_library(xWalkUltrasonic STATIC
    src/xHal_Rpi5CarUltrasonic.cpp
    src/xHal_Rpi5CarUltrasonicLifecycle.cpp
)

target_include_directories(xWalkUltrasonic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUltrasonic PUBLIC cxx_std_17)
target_link_libraries(xWalkUltrasonic PUBLIC xWalkCommon xWalkGpio)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUltrasonic PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ULTRASONIC_BUILD_HOST_TESTS)
    add_executable(xWalkUltrasonicTest test/src/xHal_Rpi5CarUltrasonicTest.cpp)
    target_link_libraries(xWalkUltrasonicTest PRIVATE xWalkUltrasonic)
    add_test(NAME xWalkUltrasonicHostTest COMMAND xWalkUltrasonicTest)
    set_tests_properties(xWalkUltrasonicHostTest PROPERTIES LABELS "host")
endif()
