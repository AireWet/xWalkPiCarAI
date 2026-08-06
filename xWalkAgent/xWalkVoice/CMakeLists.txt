cmake_minimum_required(VERSION 3.16)

project(xWalkAgentVoiceGroup LANGUAGES CXX)

option(XWALK_AGENT_VOICE_BUILD_HOST_TESTS
    "Build the xWalk voice Agent group host test" OFF)
option(XWALK_AGENT_VOICE_BUILD_HARDWARE_TESTS
    "Build the xWalk voice Agent group hardware-profile test" OFF)

if(XWALK_AGENT_VOICE_BUILD_HOST_TESTS)
    set(XWALK_LOCAL_VOICE_CHATBOT_BUILD_HOST_TESTS ON CACHE BOOL "Voice group test dependency" FORCE)
    set(XWALK_VOICE_ACTIVE_CAR_BUILD_HOST_TESTS ON CACHE BOOL "Voice group test dependency" FORCE)
    set(XWALK_VOICE_ACTIVE_CAR_GPT_BUILD_HOST_TESTS ON CACHE BOOL "Voice group test dependency" FORCE)
    set(XWALK_GPT_CAR_BUILD_HOST_TESTS ON CACHE BOOL "Voice group test dependency" FORCE)
endif()

if(XWALK_AGENT_VOICE_BUILD_HOST_TESTS OR XWALK_AGENT_VOICE_BUILD_HARDWARE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(xWalkLocalVoiceChatbot)
add_subdirectory(xWalkVoicePromptCar)
add_subdirectory(xWalkStorytellingRobot)
add_subdirectory(xWalkVoiceControlledCar)
add_subdirectory(xWalkTextVisionTalk)
add_subdirectory(xWalkOnlineLlmTest)
add_subdirectory(xWalkVoiceActiveCar)
add_subdirectory(xWalkVoiceActiveCarGpt)
add_subdirectory(xWalkGptCar)

add_library(xWalkAgentVoice INTERFACE)
add_library(xWalk::AgentVoice ALIAS xWalkAgentVoice)
target_compile_features(xWalkAgentVoice INTERFACE cxx_std_17)
target_link_libraries(xWalkAgentVoice INTERFACE
    xWalkLocalVoiceChatbot
    xWalkVoicePromptCar
    xWalkStorytellingRobot
    xWalkVoiceControlledCar
    xWalkTextVisionTalk
    xWalkOnlineLlmTest
    xWalkVoiceActiveCar
    xWalkVoiceActiveCarGpt
    xWalkGptCar
)

if(XWALK_AGENT_VOICE_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkAgentVoiceGroupTest
        test/src/xAgent_Rpi5CarVoiceGroupTest.cpp)
    target_compile_features(xWalkAgentVoiceGroupTest PRIVATE cxx_std_17)
    target_include_directories(xWalkAgentVoiceGroupTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../test/include)
    add_dependencies(xWalkAgentVoiceGroupTest
        xWalkLocalVoiceChatbotTest
        xWalkVoiceActiveCarTest
        xWalkVoiceActiveCarGptTest)
    add_dependencies(xWalkAgentVoiceGroupTest xWalkGptCarTest)
    target_link_libraries(xWalkAgentVoiceGroupTest PRIVATE
        GTest::gtest_main xWalk::AgentVoice)
    add_test(NAME xWalkAgentVoiceGroupHostTest COMMAND xWalkAgentVoiceGroupTest)
    set_tests_properties(xWalkAgentVoiceGroupHostTest PROPERTIES LABELS "host;agent-group")
endif()

if(XWALK_AGENT_VOICE_BUILD_HARDWARE_TESTS)
    include(CTest)
    add_executable(xWalkAgentVoiceGroupHardwareTest
        test/hardware/src/xAgent_Rpi5CarVoiceGroupHardwareTest.cpp)
    target_compile_features(xWalkAgentVoiceGroupHardwareTest PRIVATE cxx_std_17)
    target_link_libraries(xWalkAgentVoiceGroupHardwareTest PRIVATE
        GTest::gtest_main xWalk::AgentVoice)
    add_test(NAME xWalkAgentVoiceGroupHardwareTest
        COMMAND xWalkAgentVoiceGroupHardwareTest)
    set_tests_properties(xWalkAgentVoiceGroupHardwareTest PROPERTIES
        LABELS "hardware;agent-group")
endif()
