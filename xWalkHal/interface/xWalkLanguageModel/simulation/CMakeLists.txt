cmake_minimum_required(VERSION 3.16)
project(xWalkLanguageModelSimulation LANGUAGES CXX)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkLanguageModel)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarLanguageModelTraceConfig.py)
set(CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkLanguageModelTrace.xml)
add_custom_command(OUTPUT ${CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${GENERATOR} --input ${INVENTORY} --output ${CONFIG}
    DEPENDS ${INVENTORY} ${GENERATOR}
    COMMENT "Preparing the persistent xWalkLanguageModel trace catalogue" VERBATIM)
add_custom_target(xWalkLanguageModelSimulationTraceConfig ALL DEPENDS ${CONFIG})
add_dependencies(xWalkLanguageModelSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkLanguageModelSimulation
    src/main.cpp
    src/xHal_Rpi5CarLanguageModelHostStub.cpp
    src/xHal_Rpi5CarLanguageModelSimulation.cpp
    src/xHal_Rpi5CarLanguageModelSimulationArguments.cpp)
target_include_directories(xWalkLanguageModelSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkLanguageModelSimulation PRIVATE cxx_std_17)
set(LANGUAGE_MODEL_SIMULATION_TRACE_LOG
    "${CMAKE_CURRENT_BINARY_DIR}/log/xWalkLanguageModelSimulation.log")
target_compile_definitions(xWalkLanguageModelSimulation PRIVATE
    XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_CONFIG_PATH="${CONFIG}"
    XWALK_LANGUAGE_MODEL_SIMULATION_TRACE_LOG_PATH="${LANGUAGE_MODEL_SIMULATION_TRACE_LOG}")
target_link_libraries(xWalkLanguageModelSimulation PRIVATE xWalkLanguageModel xWalkTrace)
add_dependencies(xWalkLanguageModelSimulation xWalkLanguageModelSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkLanguageModelSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
