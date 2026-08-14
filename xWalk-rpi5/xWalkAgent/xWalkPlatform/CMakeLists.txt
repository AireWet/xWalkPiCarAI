cmake_minimum_required(VERSION 3.16)

project(xWalkAgentPlatformGroup LANGUAGES CXX)

option(XWALK_AGENT_PLATFORM_BUILD_HOST_TESTS
    "Build the xWalk platform Agent group host test" OFF)
option(XWALK_AGENT_PLATFORM_BUILD_HARDWARE_TESTS
    "Build the xWalk platform Agent group hardware-profile test" OFF)

if(XWALK_AGENT_PLATFORM_BUILD_HOST_TESTS)
    set(XWALK_BOOT_BUILD_HOST_TESTS ON CACHE BOOL "Platform group test dependency" FORCE)
endif()

if(XWALK_AGENT_PLATFORM_BUILD_HOST_TESTS OR XWALK_AGENT_PLATFORM_BUILD_HARDWARE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(xWalkBoot)

add_library(xWalkAgentPlatform INTERFACE)
add_library(xWalk::AgentPlatform ALIAS xWalkAgentPlatform)
target_compile_features(xWalkAgentPlatform INTERFACE cxx_std_17)
target_link_libraries(xWalkAgentPlatform INTERFACE xWalkBoot)

if(XWALK_AGENT_PLATFORM_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkAgentPlatformGroupTest
        test/src/xAgent_Rpi5CarPlatformGroupTest.cpp)
    target_compile_features(xWalkAgentPlatformGroupTest PRIVATE cxx_std_17)
    target_include_directories(xWalkAgentPlatformGroupTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../test/include)
    add_dependencies(xWalkAgentPlatformGroupTest xWalkBootTest)
    target_link_libraries(xWalkAgentPlatformGroupTest PRIVATE
        GTest::gtest_main xWalk::AgentPlatform)
    add_test(NAME xWalkAgentPlatformGroupHostTest COMMAND xWalkAgentPlatformGroupTest)
    set_tests_properties(xWalkAgentPlatformGroupHostTest PROPERTIES LABELS "host;agent-group")
endif()

if(XWALK_AGENT_PLATFORM_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkAgentPlatformGroupHardwareTest
        test/hardware/src/xAgent_Rpi5CarPlatformGroupHardwareTest.cpp)
    target_compile_features(xWalkAgentPlatformGroupHardwareTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentPlatformGroupHardwareTest PRIVATE
        GTest::gtest_main xWalk::AgentPlatform)
    add_test(NAME xWalkAgentPlatformGroupHardwareTest
        COMMAND xWalkAgentPlatformGroupHardwareTest)
    set_tests_properties(xWalkAgentPlatformGroupHardwareTest PROPERTIES
        LABELS "hardware;agent-group")
endif()
