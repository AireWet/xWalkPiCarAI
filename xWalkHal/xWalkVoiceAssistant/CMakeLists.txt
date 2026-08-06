cmake_minimum_required(VERSION 3.16)
project(xWalkVoiceAssistant LANGUAGES CXX)

option(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS "Build xWalkVoiceAssistant host tests" OFF)
option(XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS "Build xWalkVoiceAssistant target tests" OFF)

set(XWALK_GPT_BUILD_HOST_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS})
set(XWALK_GPT_BUILD_HARDWARE_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS})
set(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS})
set(XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS})

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

if(NOT TARGET xWalkGPT)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGPT ${CMAKE_CURRENT_BINARY_DIR}/xWalkGPT)
endif()

if(NOT TARGET xWalkLanguageModel)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkLanguageModel
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLanguageModel)
endif()

add_library(xWalkVoiceAssistant STATIC
    src/xHal_Rpi5CarVoiceAssistant.cpp
    src/xHal_Rpi5CarVoiceAssistantLifecycle.cpp
)

target_include_directories(xWalkVoiceAssistant PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVoiceAssistant PUBLIC cxx_std_17)
target_link_libraries(xWalkVoiceAssistant PUBLIC xWalkGPT xWalkLanguageModel xWalkLibraryCommon)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVoiceAssistant PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS OR XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS)
    add_executable(xWalkVoiceAssistantTest test/src/xHal_Rpi5CarVoiceAssistantTest.cpp)
    target_link_libraries(xWalkVoiceAssistantTest PRIVATE xWalkVoiceAssistant)
    add_test(NAME xWalkVoiceAssistantHostTest COMMAND xWalkVoiceAssistantTest)
    set_tests_properties(xWalkVoiceAssistantHostTest PROPERTIES LABELS "host")

    add_executable(xWalkVoiceAssistantBackendsTest
        hardware/test/src/xHal_Rpi5CarVoiceAssistantBackendsTest.cpp)
    target_link_libraries(xWalkVoiceAssistantBackendsTest PRIVATE
        xWalkVoiceAssistant xWalkSpeechToTextAlsa xWalkTextToSpeechAlsa
        xWalkLanguageModelOllama)
    add_test(NAME xWalkVoiceAssistantBackendsHostTest
        COMMAND xWalkVoiceAssistantBackendsTest)
    set_tests_properties(xWalkVoiceAssistantBackendsHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS)
    add_executable(xWalkVoiceAssistantHardwareTest
        test/hardware/src/xHal_Rpi5CarVoiceAssistantHardwareTest.cpp)
    target_link_libraries(xWalkVoiceAssistantHardwareTest PRIVATE
        xWalkVoiceAssistant xWalkSpeechToTextAlsa xWalkTextToSpeechAlsa
        xWalkLanguageModelOllama)
    add_test(NAME xWalkVoiceAssistantBackendsHardwareTest
        COMMAND xWalkVoiceAssistantHardwareTest)
    set_tests_properties(xWalkVoiceAssistantBackendsHardwareTest PROPERTIES LABELS "hardware")
endif()
