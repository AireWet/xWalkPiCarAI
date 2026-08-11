cmake_minimum_required(VERSION 3.16)

project(xWalkOnlineLlmTest LANGUAGES CXX)

if(NOT TARGET xWalkLanguageModel)
    set(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS OFF CACHE BOOL
        "Online-LLM-test dependency setting" FORCE)
    set(XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS OFF CACHE BOOL
        "Online-LLM-test dependency setting" FORCE)
    set(XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER OFF CACHE BOOL
        "Online-LLM-test dependency setting" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkHal/interface/xWalkLanguageModel
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLanguageModel)
endif()

add_library(xWalkOnlineLlmTest STATIC
    src/xAgent_Rpi5CarOnlineLlmTest.cpp
    src/xAgent_Rpi5CarOnlineLlmTestLifecycle.cpp)
target_link_libraries(xWalkOnlineLlmTest PRIVATE xWalkTrace)
add_library(xWalk::OnlineLlmTest ALIAS xWalkOnlineLlmTest)
target_include_directories(xWalkOnlineLlmTest PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkOnlineLlmTest PUBLIC cxx_std_17)
target_link_libraries(xWalkOnlineLlmTest PUBLIC xWalkLanguageModel)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkOnlineLlmTest PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
