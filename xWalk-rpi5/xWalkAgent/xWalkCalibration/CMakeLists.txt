cmake_minimum_required(VERSION 3.16)

project(xWalkAgentCalibrationGroup LANGUAGES CXX)

option(XWALK_AGENT_CALIBRATION_BUILD_HOST_TESTS
    "Build the xWalk calibration Agent group host test" OFF)
option(XWALK_AGENT_CALIBRATION_BUILD_HARDWARE_TESTS
    "Build the xWalk calibration Agent group hardware-profile test" OFF)

if(XWALK_AGENT_CALIBRATION_BUILD_HOST_TESTS)
    set(XWALK_GRAYSCALE_CALIBRATION_BUILD_HOST_TESTS ON CACHE BOOL
        "Calibration group test dependency" FORCE)
    set(XWALK_SERVO_MOTOR_CALIBRATION_BUILD_HOST_TESTS ON CACHE BOOL
        "Calibration group test dependency" FORCE)
    set(XWALK_SERVO_ZEROING_BUILD_HOST_TESTS ON CACHE BOOL
        "Calibration group test dependency" FORCE)
endif()

if(XWALK_AGENT_CALIBRATION_BUILD_HOST_TESTS OR
    XWALK_AGENT_CALIBRATION_BUILD_HARDWARE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(xWalkGrayscaleCalibration)
add_subdirectory(xWalkServoMotorCalibration)
add_subdirectory(xWalkServoZeroing)

add_library(xWalkAgentCalibration INTERFACE)
add_library(xWalk::AgentCalibration ALIAS xWalkAgentCalibration)
target_compile_features(xWalkAgentCalibration INTERFACE cxx_std_17)
target_link_libraries(xWalkAgentCalibration INTERFACE
    xWalkGrayscaleCalibration
    xWalkServoMotorCalibration
    xWalkServoZeroing
)

if(XWALK_AGENT_CALIBRATION_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkAgentCalibrationGroupTest
        test/src/xAgent_Rpi5CarCalibrationGroupTest.cpp)
    target_compile_features(xWalkAgentCalibrationGroupTest PRIVATE cxx_std_17)
    target_include_directories(xWalkAgentCalibrationGroupTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../test/include)
    add_dependencies(xWalkAgentCalibrationGroupTest
        xWalkGrayscaleCalibrationTest
        xWalkServoMotorCalibrationTest
        xWalkServoZeroingTest)
    target_link_libraries(xWalkAgentCalibrationGroupTest PRIVATE
        GTest::gtest_main xWalk::AgentCalibration)
    add_test(NAME xWalkAgentCalibrationGroupHostTest COMMAND xWalkAgentCalibrationGroupTest)
    set_tests_properties(xWalkAgentCalibrationGroupHostTest PROPERTIES LABELS "host;agent-group")
endif()

if(XWALK_AGENT_CALIBRATION_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkAgentCalibrationGroupHardwareTest
        test/hardware/src/xAgent_Rpi5CarCalibrationGroupHardwareTest.cpp)
    target_compile_features(xWalkAgentCalibrationGroupHardwareTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentCalibrationGroupHardwareTest PRIVATE
        GTest::gtest_main xWalk::AgentCalibration)
    add_test(NAME xWalkAgentCalibrationGroupHardwareTest
        COMMAND xWalkAgentCalibrationGroupHardwareTest)
    set_tests_properties(xWalkAgentCalibrationGroupHardwareTest PROPERTIES
        LABELS "hardware;agent-group")
endif()
