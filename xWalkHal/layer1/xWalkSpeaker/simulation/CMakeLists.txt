cmake_minimum_required(VERSION 3.16)
project(xWalkSpeakerSimulation LANGUAGES CXX)
set(XWALK_SPEAKER_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_SPEAKER_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_SPEAKER_BUILD_ALSA_BACKEND OFF CACHE BOOL "Disable ALSA" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkSpeaker)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_SPEAKER_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_SPEAKER_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarSpeakerTraceConfig.py)
set(XWALK_SPEAKER_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkSpeakerTrace.xml)
add_custom_command(OUTPUT ${XWALK_SPEAKER_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_SPEAKER_TRACE_GENERATOR}
        --input ${XWALK_SPEAKER_TRACE_INVENTORY} --output ${XWALK_SPEAKER_TRACE_CONFIG}
    DEPENDS ${XWALK_SPEAKER_TRACE_INVENTORY} ${XWALK_SPEAKER_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkSpeaker trace catalogue" VERBATIM)
add_custom_target(xWalkSpeakerSimulationTraceConfig ALL
    DEPENDS ${XWALK_SPEAKER_TRACE_CONFIG})
add_dependencies(xWalkSpeakerSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkSpeakerSimulation
    src/main.cpp
    src/xHal_Rpi5CarSpeakerHostStub.cpp
    src/xHal_Rpi5CarSpeakerSimulation.cpp
    src/xHal_Rpi5CarSpeakerSimulationArguments.cpp)
target_include_directories(xWalkSpeakerSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkSpeakerSimulation PRIVATE cxx_std_17)
set(XWALK_SPEAKER_SIMULATION_LOG
    ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkSpeakerSimulation.log)
set(XWALK_SPEAKER_SIMULATION_AUDIO
    ${CMAKE_CURRENT_BINARY_DIR}/speaker-simulation.wav)
target_compile_definitions(xWalkSpeakerSimulation PRIVATE
    XWALK_SPEAKER_SIMULATION_TRACE_CONFIG_PATH="${XWALK_SPEAKER_TRACE_CONFIG}"
    XWALK_SPEAKER_SIMULATION_TRACE_LOG_PATH="${XWALK_SPEAKER_SIMULATION_LOG}"
    XWALK_SPEAKER_SIMULATION_AUDIO_PATH="${XWALK_SPEAKER_SIMULATION_AUDIO}")
target_link_libraries(xWalkSpeakerSimulation PRIVATE xWalkSpeaker xWalkTrace)
add_dependencies(xWalkSpeakerSimulation xWalkSpeakerSimulationTraceConfig)
