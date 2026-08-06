cmake_minimum_required(VERSION 3.16)

project(xWalkAgentVisionGroup LANGUAGES CXX)

option(XWALK_AGENT_VISION_BUILD_HOST_TESTS
    "Build the xWalk vision Agent group host test" OFF)
option(XWALK_AGENT_VISION_BUILD_HARDWARE_TESTS
    "Build the xWalk vision Agent group hardware-profile test" OFF)

if(XWALK_AGENT_VISION_BUILD_HOST_TESTS)
    set(XWALK_COMPUTER_VISION_BUILD_HOST_TESTS ON CACHE BOOL "Vision group test dependency" FORCE)
    set(XWALK_VIDEO_RECORDING_BUILD_HOST_TESTS ON CACHE BOOL "Vision group test dependency" FORCE)
    set(XWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS ON CACHE BOOL "Vision group test dependency" FORCE)
    set(XWALK_CAMERA_BUILD_HOST_TESTS ON CACHE BOOL "Vision group test dependency" FORCE)
endif()

if(XWALK_AGENT_VISION_BUILD_HOST_TESTS OR XWALK_AGENT_VISION_BUILD_HARDWARE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(xWalkComputerVision)
add_subdirectory(xWalkFaceTracking)
add_subdirectory(xWalkBullFight)
add_subdirectory(xWalkTreasureHunt)
add_subdirectory(xWalkVideoRecording)
add_subdirectory(xWalkVideoCar)
add_subdirectory(xWalkCameraCapture)

add_library(xWalkAgentVision INTERFACE)
add_library(xWalk::AgentVision ALIAS xWalkAgentVision)
target_compile_features(xWalkAgentVision INTERFACE cxx_std_17)
target_link_libraries(xWalkAgentVision INTERFACE
    xWalkComputerVision
    xWalkFaceTracking
    xWalkBullFight
    xWalkTreasureHunt
    xWalkVideoRecording
    xWalkVideoCar
    xWalkCameraCapture
)

if(XWALK_AGENT_VISION_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkAgentVisionGroupTest
        test/src/xAgent_Rpi5CarVisionGroupTest.cpp)
    target_compile_features(xWalkAgentVisionGroupTest PRIVATE cxx_std_17)
    target_include_directories(xWalkAgentVisionGroupTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../test/include)
    add_dependencies(xWalkAgentVisionGroupTest
        xWalkComputerVisionTest
        xWalkVideoRecordingTest
        xWalkCameraCaptureTest)
    target_link_libraries(xWalkAgentVisionGroupTest PRIVATE
        GTest::gtest_main xWalk::AgentVision)
    add_test(NAME xWalkAgentVisionGroupHostTest COMMAND xWalkAgentVisionGroupTest)
    set_tests_properties(xWalkAgentVisionGroupHostTest PROPERTIES LABELS "host;agent-group")
endif()

if(XWALK_AGENT_VISION_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkAgentVisionGroupHardwareTest
        test/hardware/src/xAgent_Rpi5CarVisionGroupHardwareTest.cpp)
    target_compile_features(xWalkAgentVisionGroupHardwareTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentVisionGroupHardwareTest PRIVATE
        GTest::gtest_main xWalk::AgentVision)
    add_test(NAME xWalkAgentVisionGroupHardwareTest
        COMMAND xWalkAgentVisionGroupHardwareTest)
    set_tests_properties(xWalkAgentVisionGroupHardwareTest PROPERTIES
        LABELS "hardware;agent-group")
endif()
