cmake_minimum_required(VERSION 3.16)
project(xWalkLanguageModel LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/XWalkDependencies.cmake)

option(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS "Build xWalkLanguageModel host tests" OFF)
option(XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS "Build xWalkLanguageModel target tests" OFF)
option(XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER "Build the Ollama provider backend" OFF)

set(XWALK_LANGUAGE_MODEL_NEEDS_OLLAMA OFF)
if(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS OR
    XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS OR
    XWALK_LANGUAGE_MODEL_BUILD_OLLAMA_PROVIDER)
    set(XWALK_LANGUAGE_MODEL_NEEDS_OLLAMA ON)
endif()

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()

add_library(xWalkLanguageModel STATIC
    src/xHal_Rpi5CarLanguageModel.cpp
    src/xHal_Rpi5CarLanguageModelLifecycle.cpp
)

target_include_directories(xWalkLanguageModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLanguageModel PUBLIC cxx_std_17)
target_link_libraries(xWalkLanguageModel PUBLIC xWalkLibraryCommon)

if(XWALK_LANGUAGE_MODEL_NEEDS_OLLAMA)
    find_package(CURL REQUIRED)
    add_library(xWalkLanguageModelOllama STATIC
        hardware/src/xHal_Rpi5CarLanguageModelOllamaCallbacks.cpp
        hardware/src/xHal_Rpi5CarLanguageModelOllamaJson.cpp
        hardware/src/xHal_Rpi5CarLanguageModelOllamaLifecycle.cpp
        hardware/src/xHal_Rpi5CarLanguageModelOllamaSystem.cpp)
    target_include_directories(xWalkLanguageModelOllama PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/include)
    target_compile_features(xWalkLanguageModelOllama PUBLIC cxx_std_17)
    target_link_libraries(xWalkLanguageModelOllama PUBLIC
        xWalkLanguageModel CURL::libcurl)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(xWalkLanguageModelOllama PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLanguageModel PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS OR XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS)
    add_executable(xWalkLanguageModelTest test/src/xHal_Rpi5CarLanguageModelTest.cpp)
    target_link_libraries(xWalkLanguageModelTest PRIVATE xWalkLanguageModel)
    add_test(NAME xWalkLanguageModelHostTest COMMAND xWalkLanguageModelTest)
    set_tests_properties(xWalkLanguageModelHostTest PROPERTIES LABELS "host")

    add_executable(xWalkLanguageModelOllamaTest
        hardware/test/src/xHal_Rpi5CarLanguageModelOllamaTest.cpp)
    target_link_libraries(xWalkLanguageModelOllamaTest PRIVATE xWalkLanguageModelOllama)
    add_test(NAME xWalkLanguageModelOllamaHostTest
        COMMAND xWalkLanguageModelOllamaTest ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(xWalkLanguageModelOllamaHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS)
    add_executable(xWalkLanguageModelHardwareTest
        test/hardware/src/xHal_Rpi5CarLanguageModelHardwareTest.cpp)
    target_link_libraries(xWalkLanguageModelHardwareTest PRIVATE xWalkLanguageModelOllama)
    add_test(NAME xWalkLanguageModelOllamaHardwareTest
        COMMAND xWalkLanguageModelHardwareTest)
    set_tests_properties(xWalkLanguageModelOllamaHardwareTest PROPERTIES LABELS "hardware")
endif()
