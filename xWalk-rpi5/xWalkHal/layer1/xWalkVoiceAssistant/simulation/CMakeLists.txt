cmake_minimum_required(VERSION 3.16)
project(xWalkVoiceAssistantSimulation LANGUAGES CXX)
set(XWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkVoiceAssistant)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_VOICE_ASSISTANT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_VOICE_ASSISTANT_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarVoiceAssistantTraceConfig.py)
set(XWALK_VOICE_ASSISTANT_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkVoiceAssistantTrace.xml)
add_custom_command(OUTPUT ${XWALK_VOICE_ASSISTANT_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_VOICE_ASSISTANT_TRACE_GENERATOR}
        --input ${XWALK_VOICE_ASSISTANT_TRACE_INVENTORY}
        --output ${XWALK_VOICE_ASSISTANT_TRACE_CONFIG}
    DEPENDS ${XWALK_VOICE_ASSISTANT_TRACE_INVENTORY} ${XWALK_VOICE_ASSISTANT_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkVoiceAssistant trace catalogue" VERBATIM)
add_custom_target(xWalkVoiceAssistantSimulationTraceConfig ALL
    DEPENDS ${XWALK_VOICE_ASSISTANT_TRACE_CONFIG})
add_dependencies(xWalkVoiceAssistantSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkVoiceAssistantSimulation
    src/main.cpp
    src/xHal_Rpi5CarVoiceAssistantHostStub.cpp
    src/xHal_Rpi5CarVoiceAssistantSimulation.cpp
    src/xHal_Rpi5CarVoiceAssistantSimulationArguments.cpp)
target_include_directories(xWalkVoiceAssistantSimulation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkVoiceAssistantSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkVoiceAssistantSimulation PRIVATE
    XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_VOICE_ASSISTANT_TRACE_CONFIG}"
    XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkVoiceAssistantSimulation.log")
target_link_libraries(xWalkVoiceAssistantSimulation PRIVATE xWalkVoiceAssistant xWalkTrace)
add_dependencies(xWalkVoiceAssistantSimulation xWalkVoiceAssistantSimulationTraceConfig)
