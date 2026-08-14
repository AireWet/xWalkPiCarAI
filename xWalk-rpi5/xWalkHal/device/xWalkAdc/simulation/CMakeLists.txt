cmake_minimum_required(VERSION 3.16)
project(xWalkAdcSimulation LANGUAGES CXX)
set(XWALK_I2C_BUILD_HOST_TESTS OFF CACHE BOOL "Disable dependency tests" FORCE)
set(XWALK_I2C_BUILD_HARDWARE_TESTS OFF CACHE BOOL "Disable hardware dependency tests" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/xWalkAdc)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(XWALK_ADC_TRACE_INVENTORY ${CMAKE_BINARY_DIR}/generated/xwalk-traces.xml)
set(XWALK_ADC_TRACE_GENERATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/config/xHal_Rpi5CarAdcTraceConfig.py)
set(XWALK_ADC_TRACE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/generated/xWalkAdcTrace.xml)
add_custom_command(OUTPUT ${XWALK_ADC_TRACE_CONFIG}
    COMMAND ${Python3_EXECUTABLE} ${XWALK_ADC_TRACE_GENERATOR}
        --input ${XWALK_ADC_TRACE_INVENTORY} --output ${XWALK_ADC_TRACE_CONFIG}
    DEPENDS ${XWALK_ADC_TRACE_INVENTORY} ${XWALK_ADC_TRACE_GENERATOR}
    COMMENT "Preparing the persistent xWalkAdc trace catalogue" VERBATIM)
add_custom_target(xWalkAdcSimulationTraceConfig ALL DEPENDS ${XWALK_ADC_TRACE_CONFIG})
add_dependencies(xWalkAdcSimulationTraceConfig xWalkTraceMetadata)
add_executable(xWalkAdcSimulation
    src/main.cpp
    src/xHal_Rpi5CarAdcHostStub.cpp
    src/xHal_Rpi5CarAdcSimulation.cpp
    src/xHal_Rpi5CarAdcSimulationArguments.cpp)
target_include_directories(xWalkAdcSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(xWalkAdcSimulation PRIVATE cxx_std_17)
set(XWALK_ADC_SIMULATION_LOG ${CMAKE_CURRENT_BINARY_DIR}/log/xWalkAdcSimulation.log)
target_compile_definitions(xWalkAdcSimulation PRIVATE
    XWALK_ADC_SIMULATION_TRACE_CONFIG_PATH="${XWALK_ADC_TRACE_CONFIG}"
    XWALK_ADC_SIMULATION_TRACE_LOG_PATH="${XWALK_ADC_SIMULATION_LOG}")
target_link_libraries(xWalkAdcSimulation PRIVATE xWalkAdc xWalkTrace)
add_dependencies(xWalkAdcSimulation xWalkAdcSimulationTraceConfig)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(xWalkAdcSimulation PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()
