cmake_minimum_required(VERSION 3.16)

project(xWalkTextVisionTalk LANGUAGES CXX)

if(NOT TARGET xWalkCameraCapture)
    set(XWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS OFF CACHE BOOL
        "Text-vision-talk dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkVision/xWalkCameraCapture
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkCameraCapture)
endif()

if(NOT TARGET xWalkLanguageModel)
    set(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS OFF CACHE BOOL
        "Text-vision-talk dependency setting" FORCE)
    set(XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Text-vision-talk dependency setting" FORCE)
    set(XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER OFF CACHE BOOL
        "Text-vision-talk dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/interface/xWalkLanguageModel
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLanguageModel)
endif()

add_library(xWalkTextVisionTalk STATIC
    src/xAgent_Rpi5CarTextVisionTalk.cpp
    src/xAgent_Rpi5CarTextVisionTalkLifecycle.cpp)
target_link_libraries(xWalkTextVisionTalk PRIVATE xWalkTrace)
add_library(xWalk::TextVisionTalk ALIAS xWalkTextVisionTalk)
target_include_directories(xWalkTextVisionTalk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkTextVisionTalk PUBLIC cxx_std_17)
target_link_libraries(xWalkTextVisionTalk PUBLIC
    xWalkCameraCapture xWalkLanguageModel)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkTextVisionTalk PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
