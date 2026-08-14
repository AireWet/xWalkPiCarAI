cmake_minimum_required(VERSION 3.16)
project(xWalkMusicSimulation LANGUAGES CXX)
set(XWALK_MUSIC_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_MUSIC_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
set(XWALK_MUSIC_BUILD_ALSA_BACKEND OFF CACHE BOOL "Disable ALSA" FORCE)
set(XWALK_MUSIC_BUILD_SNDFILE_DECODER OFF CACHE BOOL "Disable file decoding" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkMusic)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_MUSIC_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_MUSIC_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarMusicTraceConfig.py)
set(XWALK_MUSIC_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkMusicTrace.xml)
add_custom_command(OUTPUT ${XWALK_MUSIC_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_MUSIC_TRACE_GENERATOR}
        --input ${XWALK_MUSIC_TRACE_INVENTORY} --output ${XWALK_MUSIC_TRACE_CONFIG}
    DEPENDS ${XWALK_MUSIC_TRACE_INVENTORY} ${XWALK_MUSIC_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkMusic trace catalogue" VERBATIM)
add_custom_target(xWalkMusicSimulationTraceConfig ALL DEPENDS ${XWALK_MUSIC_TRACE_CONFIG})
add_dependencies(xWalkMusicSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkMusicSimulation
    src/main.cpp
    src/xHal_Rpi5CarMusicHostStub.cpp
    src/xHal_Rpi5CarMusicSimulation.cpp
    src/xHal_Rpi5CarMusicSimulationArguments.cpp)
target_include_directories(xWalkMusicSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkMusicSimulation PRIVATE cxx_std_17)
set(XWALK_MUSIC_SIMULATION_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkMusicSimulation.log)
target_compile_definitions(xWalkMusicSimulation PRIVATE
    XWALK_MUSIC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_MUSIC_TRACE_CONFIG}"
    XWALK_MUSIC_SIMULATION_TRACE_LOG_PATH="${XWALK_MUSIC_SIMULATION_LOG}")
target_link_libraries(xWalkMusicSimulation PRIVATE xWalkMusic xWalkTrace)
add_dependencies(xWalkMusicSimulation xWalkMusicSimulationTraceConfig)
