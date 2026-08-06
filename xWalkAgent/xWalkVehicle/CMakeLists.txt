cmake_minimum_required(VERSION 3.16)

project(xWalkAgentVehicleGroup LANGUAGES CXX)

option(XWALK_AGENT_VEHICLE_BUILD_HOST_TESTS
    "Build the xWalk vehicle Agent group host test" OFF)
option(XWALK_AGENT_VEHICLE_BUILD_HARDWARE_TESTS
    "Build the xWalk vehicle Agent group hardware-profile test" OFF)

if(XWALK_AGENT_VEHICLE_BUILD_HOST_TESTS)
    set(XWALK_PICARX_BUILD_HOST_TESTS ON CACHE BOOL "Vehicle group test dependency" FORCE)
    set(XWALK_LINE_TRACKING_BUILD_HOST_TESTS ON CACHE BOOL "Vehicle group test dependency" FORCE)
    set(XWALK_MOVE_EXAMPLE_BUILD_HOST_TESTS ON CACHE BOOL "Vehicle group test dependency" FORCE)
    set(XWALK_KEYBOARD_CONTROL_BUILD_HOST_TESTS ON CACHE BOOL "Vehicle group test dependency" FORCE)
    set(XWALK_OBSTACLE_AVOIDANCE_BUILD_HOST_TESTS ON CACHE BOOL "Vehicle group test dependency" FORCE)
    set(XWALK_CLIFF_DETECTION_BUILD_HOST_TESTS ON CACHE BOOL "Vehicle group test dependency" FORCE)
    set(XWALK_SELF_DRIVE_BUILD_HOST_TESTS ON CACHE BOOL "Vehicle group test dependency" FORCE)
endif()

if(XWALK_AGENT_VEHICLE_BUILD_HOST_TESTS OR XWALK_AGENT_VEHICLE_BUILD_HARDWARE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(xWalkPicarx)
add_subdirectory(xWalkLineTracking)
add_subdirectory(xWalkMoveExample)
add_subdirectory(xWalkKeyboardControl)
add_subdirectory(xWalkObstacleAvoidance)
add_subdirectory(xWalkCliffDetection)
add_subdirectory(xWalkSelfDrive)

add_library(xWalkAgentVehicle INTERFACE)
add_library(xWalk::AgentVehicle ALIAS xWalkAgentVehicle)
target_compile_features(xWalkAgentVehicle INTERFACE cxx_std_17)
target_link_libraries(xWalkAgentVehicle INTERFACE
    xWalkPicarx
    xWalkLineTracking
    xWalkMoveExample
    xWalkKeyboardControl
    xWalkObstacleAvoidance
    xWalkCliffDetection
    xWalkSelfDrive
)

if(XWALK_AGENT_VEHICLE_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkAgentVehicleGroupTest
        test/src/xAgent_Rpi5CarVehicleGroupTest.cpp)
    target_compile_features(xWalkAgentVehicleGroupTest PRIVATE cxx_std_17)
    target_include_directories(xWalkAgentVehicleGroupTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../test/include)
    add_dependencies(xWalkAgentVehicleGroupTest
        xWalkPicarxTest
        xWalkLineTrackingTest
        xWalkMoveExampleTest
        xWalkKeyboardControlTest
        xWalkObstacleAvoidanceTest
        xWalkCliffDetectionTest
        xWalkSelfDriveTest)
    target_link_libraries(xWalkAgentVehicleGroupTest PRIVATE
        GTest::gtest_main xWalk::AgentVehicle)
    add_test(NAME xWalkAgentVehicleGroupHostTest COMMAND xWalkAgentVehicleGroupTest)
    set_tests_properties(xWalkAgentVehicleGroupHostTest PROPERTIES LABELS "host;agent-group")
endif()

if(XWALK_AGENT_VEHICLE_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkAgentVehicleGroupHardwareTest
        test/hardware/src/xAgent_Rpi5CarVehicleGroupHardwareTest.cpp)
    target_compile_features(xWalkAgentVehicleGroupHardwareTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentVehicleGroupHardwareTest PRIVATE
        GTest::gtest_main xWalk::AgentVehicle)
    add_test(NAME xWalkAgentVehicleGroupHardwareTest
        COMMAND xWalkAgentVehicleGroupHardwareTest)
    set_tests_properties(xWalkAgentVehicleGroupHardwareTest PROPERTIES
        LABELS "hardware;agent-group")
endif()
