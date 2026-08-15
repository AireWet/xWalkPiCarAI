cmake_minimum_required(VERSION 3.16)

project(xWalkLocalVoiceChatbot LANGUAGES CXX)

option(XWALK_LOCAL_VOICE_CHATBOT_BUILD_HOST_TESTS
    "Build xWalkLocalVoiceChatbot host tests" OFF)

if(NOT TARGET xWalkVoiceAssistant)
    set(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Voice-chatbot dependency setting" FORCE)
    set(XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Voice-chatbot dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/layer1/xWalkVoiceAssistant
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceAssistant)
endif()

add_library(xWalkLocalVoiceChatbot STATIC
    src/xAgent_Rpi5CarLocalVoiceChatbotLifecycle.cpp
    src/xAgent_Rpi5CarLocalVoiceChatbotResponse.cpp)
target_link_libraries(xWalkLocalVoiceChatbot PRIVATE xWalkTrace)
add_library(xWalk::LocalVoiceChatbot ALIAS xWalkLocalVoiceChatbot)
target_include_directories(xWalkLocalVoiceChatbot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLocalVoiceChatbot PUBLIC cxx_std_17)
target_link_libraries(xWalkLocalVoiceChatbot PUBLIC xWalkVoiceAssistant)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLocalVoiceChatbot PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_LOCAL_VOICE_CHATBOT_BUILD_HOST_TESTS)
    include(CTest)
    add_executable(xWalkLocalVoiceChatbotTest
        test/src/xAgent_Rpi5CarLocalVoiceChatbotTest.cpp)
    target_include_directories(xWalkLocalVoiceChatbotTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
    target_link_libraries(xWalkLocalVoiceChatbotTest PRIVATE xWalkLocalVoiceChatbot)
    add_test(NAME xWalkLocalVoiceChatbotHostTest COMMAND xWalkLocalVoiceChatbotTest)
    set_tests_properties(xWalkLocalVoiceChatbotHostTest PROPERTIES LABELS "host")
endif()
