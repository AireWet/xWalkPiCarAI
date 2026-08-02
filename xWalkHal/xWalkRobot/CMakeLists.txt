cmake_minimum_required(VERSION 3.16)
project(xWalkRobot LANGUAGES CXX)

option(XWALK_ROBOT_BUILD_HOST_TESTS "Build xWalkRobot host tests" OFF)
option(XWALK_ROBOT_BUILD_HARDWARE_TESTS "Compile xWalkRobot hardware dependencies" OFF)

set(XWALK_SERVO_BUILD_HOST_TESTS ${XWALK_ROBOT_BUILD_HOST_TESTS})
set(XWALK_SERVO_BUILD_HARDWARE_TESTS ${XWALK_ROBOT_BUILD_HARDWARE_TESTS})
set(XWALK_CONFIG_BUILD_HOST_TESTS ${XWALK_ROBOT_BUILD_HOST_TESTS})

if(XWALK_ROBOT_BUILD_HARDWARE_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkRobot hardware dependencies require Linux i2c-dev")
endif()

if(XWALK_ROBOT_BUILD_HOST_TESTS OR XWALK_ROBOT_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(NOT TARGET xWalkCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkCommon ${CMAKE_CURRENT_BINARY_DIR}/xWalkCommon)
endif()

if(NOT TARGET xWalkServo)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkServo ${CMAKE_CURRENT_BINARY_DIR}/xWalkServo)
endif()

if(NOT TARGET xWalkConfig)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkConfig
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkConfig)
endif()

add_library(xWalkRobot STATIC
    src/xHal_Rpi5CarRobotLifecycle.cpp
    src/xHal_Rpi5CarRobotMovement.cpp
    src/xHal_Rpi5CarRobotPosition.cpp
)

target_include_directories(xWalkRobot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkRobot PUBLIC cxx_std_17)
target_link_libraries(xWalkRobot PUBLIC xWalkCommon xWalkServo xWalkConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkRobot PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ROBOT_BUILD_HOST_TESTS)
    add_executable(xWalkRobotTest test/src/xHal_Rpi5CarRobotTest.cpp)
    target_link_libraries(xWalkRobotTest PRIVATE xWalkRobot)
    add_test(NAME xWalkRobotHostTest COMMAND xWalkRobotTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/robot.config)
    set_tests_properties(xWalkRobotHostTest PROPERTIES LABELS "host")
endif()
