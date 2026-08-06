cmake_minimum_required(VERSION 3.16)

project(xWalkVoicePromptCar LANGUAGES CXX)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Voice-prompt-car dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Voice-prompt-car dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkGPT)
    set(XWALK_GPT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Voice-prompt-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_STT_ALSA_BACKEND OFF CACHE BOOL
        "Voice-prompt-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_TTS_ALSA_BACKEND OFF CACHE BOOL
        "Voice-prompt-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_VOSK_PROVIDER OFF CACHE BOOL
        "Voice-prompt-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_ESPEAK_PROVIDER OFF CACHE BOOL
        "Voice-prompt-car dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/xWalkGPT
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkGPT)
endif()

add_library(xWalkVoicePromptCar STATIC
    src/xAgent_Rpi5CarVoicePromptCar.cpp
    src/xAgent_Rpi5CarVoicePromptCarLifecycle.cpp)
add_library(xWalk::VoicePromptCar ALIAS xWalkVoicePromptCar)
target_include_directories(xWalkVoicePromptCar PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVoicePromptCar PUBLIC cxx_std_17)
target_link_libraries(xWalkVoicePromptCar PUBLIC xWalkPicarx xWalkGPT)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVoicePromptCar PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
