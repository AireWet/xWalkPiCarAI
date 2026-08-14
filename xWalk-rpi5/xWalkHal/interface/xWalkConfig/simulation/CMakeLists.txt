cmake_minimum_required(VERSION 3.16)
project(xWalkConfigSimulation LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkConfig)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_CONFIG_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_CONFIG_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarConfigTraceConfig.py)
set(XWALK_CONFIG_TRACE_CONFIG
    ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkConfigTrace.xml)

add_custom_command(
    OUTPUT ${XWALK_CONFIG_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_CONFIG_TRACE_GENERATOR}
        --input ${XWALK_CONFIG_TRACE_INVENTORY}
        --output ${XWALK_CONFIG_TRACE_CONFIG}
    DEPENDS ${XWALK_CONFIG_TRACE_INVENTORY} ${XWALK_CONFIG_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkConfig trace catalogue"
    VERBATIM)
add_custom_target(xWalkConfigSimulationTraceConfig ALL
    DEPENDS ${XWALK_CONFIG_TRACE_CONFIG})
add_dependencies(xWalkConfigSimulationTraceConfig xWalkTraceMetadata)

add_executable(xWalkConfigSimulation
    src/main.cpp
    src/xHal_Rpi5CarConfigSimulation.cpp
    src/xHal_Rpi5CarConfigSimulationArguments.cpp)
target_include_directories(xWalkConfigSimulation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkConfigSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkConfigSimulation PRIVATE
    XWALK_CONFIG_SIMULATION_TRACE_CONFIG_PATH="${XWALK_CONFIG_TRACE_CONFIG}"
    XWALK_CONFIG_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkConfigSimulation.log"
    XWALK_CONFIG_SIMULATION_DATA_PATH="${CMAKE_CURRENT_BINARY_DIR}/simulation-data")
target_link_libraries(xWalkConfigSimulation PRIVATE xWalkConfig xWalkTrace)
add_dependencies(xWalkConfigSimulation xWalkConfigSimulationTraceConfig)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkConfigSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
