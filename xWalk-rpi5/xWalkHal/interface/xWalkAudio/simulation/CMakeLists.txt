cmake_minimum_required(VERSION 3.16)
project(xWalkAudioSimulation LANGUAGES CXX)

set(XWALK_AUDIO_SIMULATION_BACKEND "stub" CACHE STRING
    "Simulation backend: stub or hardware")
set_property(CACHE XWALK_AUDIO_SIMULATION_BACKEND PROPERTY STRINGS stub hardware)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "xWalkAudio simulation requires Linux")
endif()
if(NOT XWALK_AUDIO_SIMULATION_BACKEND STREQUAL "stub"
    AND NOT XWALK_AUDIO_SIMULATION_BACKEND STREQUAL "hardware")
    message(FATAL_ERROR "XWALK_AUDIO_SIMULATION_BACKEND must be stub or hardware")
endif()

set(XWALK_AUDIO_BUILD_LINUX_BACKEND ON CACHE BOOL
    "Required by the standalone simulation" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkAudio)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_AUDIO_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_AUDIO_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarAudioTraceConfig.py)
set(XWALK_AUDIO_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAudioTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_AUDIO_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_AUDIO_TRACE_GENERATOR}
        --input ${XWALK_AUDIO_TRACE_INVENTORY}
        --output ${XWALK_AUDIO_TRACE_CONFIG}
    DEPENDS ${XWALK_AUDIO_TRACE_INVENTORY} ${XWALK_AUDIO_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkAudio trace catalogue"
    VERBATIM)
add_custom_target(xWalkAudioSimulationTraceConfig ALL DEPENDS ${XWALK_AUDIO_TRACE_CONFIG})
add_dependencies(xWalkAudioSimulationTraceConfig xWalkTraceMetadata)

set(XWALK_AUDIO_SIMULATION_SOURCES
    src/main.cpp
    src/xHal_Rpi5CarAudioHandler.cpp
    src/xHal_Rpi5CarAudioSimulationArguments.cpp)
if(XWALK_AUDIO_SIMULATION_BACKEND STREQUAL "stub")
    list(APPEND XWALK_AUDIO_SIMULATION_SOURCES
        src/xHal_Rpi5CarAudioHostStub.cpp
        src/xHal_Rpi5CarAudioSimulationStub.cpp)
else()
    list(APPEND XWALK_AUDIO_SIMULATION_SOURCES
        src/xHal_Rpi5CarAudioSimulationHardware.cpp)
endif()

add_executable(xWalkAudioSimulation ${XWALK_AUDIO_SIMULATION_SOURCES})
target_include_directories(xWalkAudioSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAudioSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkAudioSimulation PRIVATE
    XWALK_AUDIO_SIMULATION_TRACE_CONFIG_PATH="${XWALK_AUDIO_TRACE_CONFIG}"
    XWALK_AUDIO_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkAudioSimulation.log")
target_link_libraries(xWalkAudioSimulation PRIVATE xWalkAudioAlsa xWalkTrace)
add_dependencies(xWalkAudioSimulation xWalkAudioSimulationTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAudioSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
