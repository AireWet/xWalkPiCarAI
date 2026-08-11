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

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

if(NOT TARGET xWalkServo)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../device/xWalkServo ${CMAKE_CURRENT_BINARY_DIR}/xWalkServo)
endif()

if(NOT TARGET xWalkConfig)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkConfig
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkConfig)
endif()

add_library(xWalkRobot STATIC
    src/xHal_Rpi5CarRobotLifecycle.cpp
    src/xHal_Rpi5CarRobotMovement.cpp
    src/xHal_Rpi5CarRobotPosition.cpp
)

target_include_directories(xWalkRobot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkRobot PUBLIC cxx_std_17)
target_link_libraries(xWalkRobot PUBLIC xWalkLibraryCommon xWalkServo xWalkConfig PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkRobot PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_ROBOT_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_ROBOT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_ROBOT_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarRobotTraceConfig.py)
    set(XWALK_ROBOT_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkRobotTrace.xml)
    add_custom_command(OUTPUT ${XWALK_ROBOT_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_ROBOT_TRACE_GENERATOR}
            --input ${XWALK_ROBOT_TRACE_INVENTORY} --output ${XWALK_ROBOT_TRACE_CONFIG}
        DEPENDS ${XWALK_ROBOT_TRACE_INVENTORY} ${XWALK_ROBOT_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkRobot trace catalogue" VERBATIM)
    add_custom_target(xWalkRobotTestTraceConfig ALL DEPENDS ${XWALK_ROBOT_TRACE_CONFIG})
    add_dependencies(xWalkRobotTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkRobotTest
        test/src/xHal_Rpi5CarRobotTest.cpp
        test/src/xHal_Rpi5CarRobotTestSupport.cpp
        simulation/src/xHal_Rpi5CarRobotSimulationArguments.cpp)
    target_include_directories(xWalkRobotTest PRIVATE test/include simulation/include)
    target_compile_definitions(xWalkRobotTest PRIVATE
        XWALK_ROBOT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ROBOT_TRACE_CONFIG}"
        XWALK_ROBOT_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkRobotTest.log")
    target_link_libraries(xWalkRobotTest PRIVATE xWalkRobot xWalkTrace)
    add_dependencies(xWalkRobotTest xWalkRobotTestTraceConfig)
    add_test(NAME xWalkRobotHostTest COMMAND xWalkRobotTest
        ${CMAKE_CURRENT_BINARY_DIR}/test-data/robot.config)
    set_tests_properties(xWalkRobotHostTest PROPERTIES LABELS "host")
endif()
