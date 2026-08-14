cmake_minimum_required(VERSION 3.16)
project(xWalkGptSimulation LANGUAGES CXX)
set(XWALK_GPT_BUILD_HOST_TESTS OFF CACHE BOOL "Disable module tests" FORCE)
set(XWALK_GPT_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkGPT)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_GPT_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_GPT_TRACE_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarGptTraceConfig.py)
set(XWALK_GPT_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkGptTrace.xml)
add_custom_command(OUTPUT ${XWALK_GPT_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_GPT_TRACE_GENERATOR}
        --input ${XWALK_GPT_TRACE_INVENTORY} --output ${XWALK_GPT_TRACE_CONFIG}
    DEPENDS ${XWALK_GPT_TRACE_INVENTORY} ${XWALK_GPT_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkGPT trace catalogue" VERBATIM)
add_custom_target(xWalkGptSimulationTraceConfig ALL DEPENDS ${XWALK_GPT_TRACE_CONFIG})
add_dependencies(xWalkGptSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkGptSimulation
    src/main.cpp
    src/xHal_Rpi5CarGptHostStub.cpp
    src/xHal_Rpi5CarGptSimulation.cpp
    src/xHal_Rpi5CarGptSimulationArguments.cpp)
target_include_directories(xWalkGptSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkGptSimulation PRIVATE cxx_std_17)
target_compile_definitions(xWalkGptSimulation PRIVATE
    XWALK_GPT_SIMULATION_TRACE_CONFIG_PATH="${XWALK_GPT_TRACE_CONFIG}"
    XWALK_GPT_SIMULATION_TRACE_LOG_PATH="${CMAKE_CURRENT_BINARY_DIR}/log/xWalkGptSimulation.log")
target_link_libraries(xWalkGptSimulation PRIVATE xWalkGPT xWalkTrace)
add_dependencies(xWalkGptSimulation xWalkGptSimulationTraceConfig)
