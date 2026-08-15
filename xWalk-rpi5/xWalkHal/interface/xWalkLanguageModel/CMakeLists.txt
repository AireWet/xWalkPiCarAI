cmake_minimum_required(VERSION 3.16)
project(xWalkLanguageModel LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/XWalkDependencies.cmake)

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
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

add_library(xWalkLanguageModel STATIC
    src/xHal_Rpi5CarLanguageModel.cpp
    src/xHal_Rpi5CarLanguageModelLifecycle.cpp
)

target_include_directories(xWalkLanguageModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLanguageModel PUBLIC cxx_std_17)
target_link_libraries(xWalkLanguageModel PUBLIC xWalkLibraryCommon PRIVATE xWalkTrace)

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
    target_link_libraries(xWalkLanguageModelOllama PRIVATE xWalkTrace)
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
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(LANGUAGE_MODEL_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(LANGUAGE_MODEL_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarLanguageModelTraceConfig.py)
    set(LANGUAGE_MODEL_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLanguageModelTrace.xml)
    add_custom_command(OUTPUT ${LANGUAGE_MODEL_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${LANGUAGE_MODEL_TRACE_GENERATOR}
            --input ${LANGUAGE_MODEL_TRACE_INVENTORY} --output ${LANGUAGE_MODEL_TRACE_CONFIG}
        DEPENDS ${LANGUAGE_MODEL_TRACE_INVENTORY} ${LANGUAGE_MODEL_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkLanguageModel host-test catalogue" VERBATIM)
    add_custom_target(xWalkLanguageModelTestTraceConfig
        DEPENDS ${LANGUAGE_MODEL_TRACE_CONFIG})
    add_dependencies(xWalkLanguageModelTestTraceConfig xWalkTraceMetadata)

    add_executable(xWalkLanguageModelTest
        test/src/xHal_Rpi5CarLanguageModelTest.cpp
        test/src/xHal_Rpi5CarLanguageModelTestSupport.cpp
        simulation/src/xHal_Rpi5CarLanguageModelSimulationArguments.cpp)
    target_include_directories(xWalkLanguageModelTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(LANGUAGE_MODEL_TEST_TRACE_LOG
        "${CMAKE_CURRENT_BINARY_DIR}/log/xWalkLanguageModelTest.log")
    target_compile_definitions(xWalkLanguageModelTest PRIVATE
        XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH="${LANGUAGE_MODEL_TRACE_CONFIG}"
        XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH="${LANGUAGE_MODEL_TEST_TRACE_LOG}")
    target_link_libraries(xWalkLanguageModelTest PRIVATE xWalkLanguageModel xWalkTrace)
    add_dependencies(xWalkLanguageModelTest xWalkLanguageModelTestTraceConfig)
    add_test(NAME xWalkLanguageModelHostTest COMMAND xWalkLanguageModelTest)
    set_tests_properties(xWalkLanguageModelHostTest PROPERTIES LABELS "host")

    add_executable(xWalkLanguageModelOllamaTest
        hardware/test/src/xHal_Rpi5CarLanguageModelOllamaTest.cpp)
    target_include_directories(xWalkLanguageModelOllamaTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/test/include
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/include)
    set(LANGUAGE_MODEL_OLLAMA_TEST_TRACE_LOG
        "${CMAKE_CURRENT_BINARY_DIR}/log/xWalkLanguageModelOllamaTest.log")
    target_compile_definitions(xWalkLanguageModelOllamaTest PRIVATE
        XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH="${LANGUAGE_MODEL_TRACE_CONFIG}"
        XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH="${LANGUAGE_MODEL_OLLAMA_TEST_TRACE_LOG}")
    target_link_libraries(xWalkLanguageModelOllamaTest PRIVATE
        xWalkLanguageModelOllama xWalkTrace)
    add_dependencies(xWalkLanguageModelOllamaTest xWalkLanguageModelTestTraceConfig)
    add_test(NAME xWalkLanguageModelOllamaHostTest
        COMMAND xWalkLanguageModelOllamaTest ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(xWalkLanguageModelOllamaHostTest PROPERTIES LABELS "host")
endif()

if(XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS)
    add_executable(xWalkLanguageModelHardwareTest
        test/hardware/src/xHal_Rpi5CarLanguageModelHardwareTest.cpp)
target_link_libraries(xWalkLanguageModelHardwareTest PRIVATE xWalkTrace)
    target_link_libraries(xWalkLanguageModelHardwareTest PRIVATE xWalkLanguageModelOllama)
    add_test(NAME xWalkLanguageModelOllamaHardwareTest
        COMMAND xWalkLanguageModelHardwareTest)
    set_tests_properties(xWalkLanguageModelOllamaHardwareTest PROPERTIES LABELS "hardware")
endif()
