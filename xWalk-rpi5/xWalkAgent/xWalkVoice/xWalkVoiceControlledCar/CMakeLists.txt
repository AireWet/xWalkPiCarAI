cmake_minimum_required(VERSION 3.16)

project(xWalkVoiceControlledCar LANGUAGES CXX)

if(NOT TARGET xWalkPicarx)
    set(XWALK_PICARX_BUILD_HOST_TESTS OFF CACHE BOOL
        "Voice-controlled-car dependency setting" FORCE)
    set(XWALK_PICARX_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Voice-controlled-car dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVehicle/xWalkPicarx
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkPicarx)
endif()

if(NOT TARGET xWalkGPT)
    set(XWALK_GPT_BUILD_HOST_TESTS OFF CACHE BOOL
        "Voice-controlled-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_STT_ALSA_BACKEND OFF CACHE BOOL
        "Voice-controlled-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_TTS_ALSA_BACKEND OFF CACHE BOOL
        "Voice-controlled-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_VOSK_PROVIDER OFF CACHE BOOL
        "Voice-controlled-car dependency setting" FORCE)
    set(XWALK_GPT_BUILD_ESPEAK_PROVIDER OFF CACHE BOOL
        "Voice-controlled-car dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/layer1/xWalkGPT
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkGPT)
endif()

add_library(xWalkVoiceControlledCar STATIC
    src/xAgent_Rpi5CarVoiceControlledCar.cpp
    src/xAgent_Rpi5CarVoiceControlledCarLifecycle.cpp)
target_link_libraries(xWalkVoiceControlledCar PRIVATE xWalkTrace)
add_library(xWalk::VoiceControlledCar ALIAS xWalkVoiceControlledCar)
target_include_directories(xWalkVoiceControlledCar PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVoiceControlledCar PUBLIC cxx_std_17)
target_link_libraries(xWalkVoiceControlledCar PUBLIC xWalkPicarx xWalkGPT)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVoiceControlledCar PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
