cmake_minimum_required(VERSION 3.16)
project(xWalkUtilsSimulation LANGUAGES CXX)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkUtils)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_UTILS_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_UTILS_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarUtilsTraceConfig.py)
set(XWALK_UTILS_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkUtilsTrace.xml)
add_custom_command(
    OUTPUT ${XWALK_UTILS_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_UTILS_TRACE_GENERATOR}
        --input ${XWALK_UTILS_TRACE_INVENTORY} --output ${XWALK_UTILS_TRACE_CONFIG}
    DEPENDS ${XWALK_UTILS_TRACE_INVENTORY} ${XWALK_UTILS_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkUtils trace catalogue" VERBATIM)
add_custom_target(xWalkUtilsSimulationTraceConfig ALL DEPENDS ${XWALK_UTILS_TRACE_CONFIG})
add_dependencies(xWalkUtilsSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkUtilsSimulation
    src/main.cpp
    src/xHal_Rpi5CarUtilsHostStub.cpp
    src/xHal_Rpi5CarUtilsSimulation.cpp
    src/xHal_Rpi5CarUtilsSimulationArguments.cpp)
target_include_directories(xWalkUtilsSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkUtilsSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkUtilsSimulation PRIVATE
    XWALK_UTILS_SIMULATION_TRACE_CONFIG_PATH="${XWALK_UTILS_TRACE_CONFIG}"
    XWALK_UTILS_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkUtilsSimulation.log")
target_link_libraries(xWalkUtilsSimulation PRIVATE xWalkUtils xWalkTrace)
add_dependencies(xWalkUtilsSimulation xWalkUtilsSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkUtilsSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
