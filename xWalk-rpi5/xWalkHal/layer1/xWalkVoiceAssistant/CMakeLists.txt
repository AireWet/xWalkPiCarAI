cmake_minimum_required(VERSION 3.16)
project(xWalkVoiceAssistant LANGUAGES CXX)

option(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS "Build xWalkVoiceAssistant host tests" OFF)
option(XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS "Build xWalkVoiceAssistant target tests" OFF)

set(XWALK_GPT_BUILD_HOST_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS})
set(XWALK_GPT_BUILD_HARDWARE_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS})
set(XWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS})
set(XWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS ${XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS})

if(NOT TARGET xWalkLibraryCommon)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkLibrary/common ${CMAKE_CURRENT_BINARY_DIR}/common)
endif()
if(NOT TARGET xWalkTrace)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../xWalkTrace
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkTrace)
endif()

if(NOT TARGET xWalkGPT)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../xWalkGPT ${CMAKE_CURRENT_BINARY_DIR}/xWalkGPT)
endif()

if(NOT TARGET xWalkLanguageModel)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../interface/xWalkLanguageModel
        ${CMAKE_CURRENT_BINARY_DIR}/xWalkLanguageModel)
endif()

add_library(xWalkVoiceAssistant STATIC
    src/xHal_Rpi5CarVoiceAssistant.cpp
    src/xHal_Rpi5CarVoiceAssistantLifecycle.cpp
)

target_include_directories(xWalkVoiceAssistant PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVoiceAssistant PUBLIC cxx_std_17)
target_link_libraries(xWalkVoiceAssistant PUBLIC xWalkGPT xWalkLanguageModel xWalkLibraryCommon
    PRIVATE xWalkTrace)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkVoiceAssistant PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

if(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS OR XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS)
    include(CTest)
endif()

if(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(XWALK_VOICE_ASSISTANT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
    set(XWALK_VOICE_ASSISTANT_TRACE_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/simulation/config/xHal_Rpi5CarVoiceAssistantTraceConfig.py)
    set(XWALK_VOICE_ASSISTANT_TRACE_CONFIG
        ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkVoiceAssistantTrace.xml)
    add_custom_command(OUTPUT ${XWALK_VOICE_ASSISTANT_TRACE_CONFIG}
        COMMAND ${Python3_EXECUTABLE} ${XWALK_VOICE_ASSISTANT_TRACE_GENERATOR}
            --input ${XWALK_VOICE_ASSISTANT_TRACE_INVENTORY}
            --output ${XWALK_VOICE_ASSISTANT_TRACE_CONFIG}
        DEPENDS ${XWALK_VOICE_ASSISTANT_TRACE_INVENTORY}
            ${XWALK_VOICE_ASSISTANT_TRACE_GENERATOR}
        COMMENT "Preparing the persistent xWalkVoiceAssistant trace catalogue" VERBATIM)
    add_custom_target(xWalkVoiceAssistantTestTraceConfig ALL
        DEPENDS ${XWALK_VOICE_ASSISTANT_TRACE_CONFIG})
    add_dependencies(xWalkVoiceAssistantTestTraceConfig xWalkTraceMetadata)
    add_executable(xWalkVoiceAssistantTest
        test/src/xHal_Rpi5CarVoiceAssistantTest.cpp
        test/src/xHal_Rpi5CarVoiceAssistantTestSupport.cpp
        simulation/src/xHal_Rpi5CarVoiceAssistantSimulationArguments.cpp)
    target_include_directories(xWalkVoiceAssistantTest PRIVATE test/include simulation/include)
    target_compile_definitions(xWalkVoiceAssistantTest PRIVATE
        XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_VOICE_ASSISTANT_TRACE_CONFIG}"
        XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkVoiceAssistantTest.log")
    target_link_libraries(xWalkVoiceAssistantTest PRIVATE xWalkVoiceAssistant xWalkTrace)
    add_dependencies(xWalkVoiceAssistantTest xWalkVoiceAssistantTestTraceConfig)
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
target_link_libraries(xWalkVoiceAssistantHardwareTest PRIVATE xWalkTrace)
    target_link_libraries(xWalkVoiceAssistantHardwareTest PRIVATE
        xWalkVoiceAssistant xWalkSpeechToTextAlsa xWalkTextToSpeechAlsa
        xWalkLanguageModelOllama)
    add_test(NAME xWalkVoiceAssistantBackendsHardwareTest
        COMMAND xWalkVoiceAssistantHardwareTest)
    set_tests_properties(xWalkVoiceAssistantBackendsHardwareTest PROPERTIES LABELS "hardware")
endif()
