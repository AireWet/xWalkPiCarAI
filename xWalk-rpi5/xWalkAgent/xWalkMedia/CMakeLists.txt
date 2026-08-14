cmake_minimum_required(VERSION 3.16)

project(xWalkAgentMediaGroup LANGUAGES CXX)

option(XWALK_AGENT_MEDIA_BUILD_HOST_TESTS
    "Build the xWalk media Agent group host test" OFF)
option(XWALK_AGENT_MEDIA_BUILD_HARDWARE_TESTS
    "Build the xWalk media Agent group hardware-profile test" OFF)

if(XWALK_AGENT_MEDIA_BUILD_HOST_TESTS OR XWALK_AGENT_MEDIA_BUILD_HARDWARE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(xWalkSoundBackgroundMusic)

add_library(xWalkAgentMedia INTERFACE)
add_library(xWalk::AgentMedia ALIAS xWalkAgentMedia)
target_compile_features(xWalkAgentMedia INTERFACE cxx_std_17)
target_link_libraries(xWalkAgentMedia INTERFACE xWalkSoundBackgroundMusic)

if(XWALK_AGENT_MEDIA_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkAgentMediaGroupTest
        test/src/xAgent_Rpi5CarMediaGroupTest.cpp)
    target_compile_features(xWalkAgentMediaGroupTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentMediaGroupTest PRIVATE
        GTest::gtest_main xWalk::AgentMedia)
    add_test(NAME xWalkAgentMediaGroupHostTest COMMAND xWalkAgentMediaGroupTest)
    set_tests_properties(xWalkAgentMediaGroupHostTest PROPERTIES LABELS "host;agent-group")
endif()

if(XWALK_AGENT_MEDIA_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkAgentMediaGroupHardwareTest
        test/hardware/src/xAgent_Rpi5CarMediaGroupHardwareTest.cpp)
    target_compile_features(xWalkAgentMediaGroupHardwareTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentMediaGroupHardwareTest PRIVATE
        GTest::gtest_main xWalk::AgentMedia)
    add_test(NAME xWalkAgentMediaGroupHardwareTest
        COMMAND xWalkAgentMediaGroupHardwareTest)
    set_tests_properties(xWalkAgentMediaGroupHardwareTest PROPERTIES
        LABELS "hardware;agent-group")
endif()
