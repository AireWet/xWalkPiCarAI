cmake_minimum_required(VERSION 3.16)

project(xWalkAgentConnectivityGroup LANGUAGES CXX)

option(XWALK_AGENT_CONNECTIVITY_BUILD_HOST_TESTS
    "Build the xWalk connectivity Agent group host test" OFF)
option(XWALK_AGENT_CONNECTIVITY_BUILD_HARDWARE_TESTS
    "Build the xWalk connectivity Agent group hardware-profile test" OFF)

if(XWALK_AGENT_CONNECTIVITY_BUILD_HOST_TESTS)
    set(XWALK_SPI_TRANSFER_BUILD_HOST_TESTS ON CACHE BOOL "Connectivity group test dependency" FORCE)
endif()

if(XWALK_AGENT_CONNECTIVITY_BUILD_HOST_TESTS OR
    XWALK_AGENT_CONNECTIVITY_BUILD_HARDWARE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(xWalkAppControl)
add_subdirectory(xWalkSpiTransfer)

add_library(xWalkAgentConnectivity INTERFACE)
add_library(xWalk::AgentConnectivity ALIAS xWalkAgentConnectivity)
target_compile_features(xWalkAgentConnectivity INTERFACE cxx_std_17)
target_link_libraries(xWalkAgentConnectivity INTERFACE
    xWalkAppControl
    xWalkSpiTransfer
)

if(XWALK_AGENT_CONNECTIVITY_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkAgentConnectivityGroupTest
        test/src/xAgent_Rpi5CarConnectivityGroupTest.cpp
        xWalkAppControl/test/src/xAgent_Rpi5CarAppControlTestSupport.cpp
        ../xWalkVehicle/xWalkPicarx/test/src/xAgent_Rpi5CarPicarxTestSupport.cpp)
    target_compile_features(xWalkAgentConnectivityGroupTest PRIVATE cxx_std_17)
    target_include_directories(xWalkAgentConnectivityGroupTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/xWalkAppControl/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../xWalkVehicle/xWalkPicarx/test/include)
    add_dependencies(xWalkAgentConnectivityGroupTest xWalkSpiTransferTest)
    target_link_libraries(xWalkAgentConnectivityGroupTest PRIVATE
        GTest::gtest_main xWalk::AgentConnectivity xWalkRobotHatSimulation)
    add_test(NAME xWalkAgentConnectivityGroupHostTest COMMAND xWalkAgentConnectivityGroupTest)
    set_tests_properties(xWalkAgentConnectivityGroupHostTest PROPERTIES LABELS "host;agent-group")
endif()

if(XWALK_AGENT_CONNECTIVITY_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkAgentConnectivityGroupHardwareTest
        test/hardware/src/xAgent_Rpi5CarConnectivityGroupHardwareTest.cpp)
    target_compile_features(xWalkAgentConnectivityGroupHardwareTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentConnectivityGroupHardwareTest PRIVATE
        GTest::gtest_main xWalk::AgentConnectivity)
    add_test(NAME xWalkAgentConnectivityGroupHardwareTest
        COMMAND xWalkAgentConnectivityGroupHardwareTest)
    set_tests_properties(xWalkAgentConnectivityGroupHardwareTest PROPERTIES
        LABELS "hardware;agent-group")
endif()
